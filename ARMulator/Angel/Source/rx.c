/*-*-C-*-
 *
 * $Revision: 1.9.6.7 $
 *   $Author: rivimey $
 *     $Date: 1998/03/03 13:46:47 $
 *
 * Copyright Advanced RISC Machines Limited, 1995.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title:  Character reception engine
 */

#include <stdarg.h>             /* ANSI varargs support */
#include "angel.h"              /* Angel system definitions */
#include "endian.h"             /* Endian independant memory access macros */
#include "crc.h"                /* crc generation definitions and headers */
#include "rxtx.h"
#include "channels.h"
#include "buffers.h"
#ifdef TARGET
#include "devdriv.h"
#endif
#include "logging.h"

static re_status unexp_stx(struct re_state *rxstate);
static re_status unexp_etx(struct re_state *rxstate);

/* bitfield for the rx_engine state */
typedef enum rx_state_flag
{
    RST_STX,
    RST_TYP,
    RST_LEN,
    RST_DAT,
    RST_CRC,
    RST_ETX,
    RST_ESC = (0x1 << 0x3)
}
rx_state_flag;

void 
Angel_RxEngineInit(const struct re_config *rxconfig,
                   struct re_state *rxstate)
{
    rxstate->rx_state = RST_STX;
    rxstate->field_c = 0;
    rxstate->index = 0;
    rxstate->crc = 0;
    rxstate->error = RE_OKAY;
    rxstate->config = rxconfig;
}

re_status 
Angel_RxEngine(unsigned char new_ch, unsigned char ovr, struct data_packet *packet,
               struct re_state *rxstate)
{
    /*
     * TODO: add the flow control bits in
     * Note: We test for the data field in a seperate case so we can
     * completely avoid entering the switch for most chars
     */

    /*
     * If overrun signalled, forget this packet and move on to the next.
     */
    if (ovr)
    {
        LogWarning(LOG_RX, ( "RxEngine: overflow; reset to STX\n"));
        rxstate->field_c = 0;
        rxstate->rx_state = RST_STX;
        rxstate->error = RE_OVR;
        return RS_BAD_PKT;
    }

    /* see if we're expecting a escaped char */
    if ((rxstate->rx_state & RST_ESC) == RST_ESC)
    {
        /* unescape the char and unset the flag */
        new_ch &= ~serial_ESCAPE;
        LogTrace1("rxe-echar-%2x ", new_ch);
        rxstate->rx_state &= ~RST_ESC;
    }
    else if (new_ch < 32)
    {
        if ((1 << new_ch) & rxstate->config->fc_set)
        {
            LogInfo(LOG_RX, ("RxEngine: xon/xoff\n"));
#if 0 && defined(TARGET)  /* disabled for the present -- ric */
            rxstate->config->fc_callback(new_ch, rxstate->config->fc_data);
#endif
        }
        else if ((1 << new_ch) & rxstate->config->esc_set)
        {
            /* see if the incoming char is a special one */
            if (new_ch == rxstate->config->esc)
            {
                LogTrace("rxe-esc ");
                rxstate->rx_state |= RST_ESC;
                return RS_IN_PKT;
            }
            else
            {
                /*
                 * must be a normal packet so do some unexpected etx/stx checking
                 * we haven't been told to escape or received an escape so unless
                 * we are expecting an stx or etx then we can take the unexpected 
                 * stx/etx trap
                 */
                if ((new_ch == (rxstate->config->stx)) && (rxstate->rx_state != RST_STX))
                    return unexp_stx(rxstate);
                if ((new_ch == (rxstate->config->etx)) && (rxstate->rx_state != RST_ETX))
                    return unexp_etx(rxstate);
            }
        }
    }
    

    if (rxstate->rx_state == RST_DAT)
    {
        /*
         * do this to speed up the common case, no real penalty for
         * other cases 
         */
        LogTrace1("rxe-dat-%2x", new_ch);

        rxstate->crc = crc32(&new_ch, 1, rxstate->crc);
        (packet->data)[rxstate->index++] = (unsigned int)new_ch & 0xff;

        if (rxstate->index == packet->len)
            rxstate->rx_state = RST_CRC;

        return RS_IN_PKT;
    }

    /*
     * Now that the common case is out of the way we can test for everything
     * else without worrying quite so much about the speed, changing the
     * order to len,crc,stx,etx,typ might gain a tiny bit of speed but lets
     * leave that for the moment
     */
    switch (rxstate->rx_state)
    {
        case RST_STX:
            if (new_ch == rxstate->config->stx)
            {
                rxstate->rx_state = RST_TYP;
                rxstate->error = RE_OKAY;
                rxstate->crc = startCRC32;
                rxstate->index = 0;
                return RS_IN_PKT;
            }
            else
            {
                rxstate->error = RE_OKAY;
                return RS_WAIT_PKT;
            }

        case RST_TYP:
            packet->type = (DevChanID) new_ch;
            rxstate->rx_state = RST_LEN;
            rxstate->error = RE_OKAY;
            rxstate->field_c = 0;  /* set up here for the length that follows */
            LogTrace1("rxe-type-%2x ", packet->type);
            rxstate->crc = crc32(&new_ch, 1, rxstate->crc);

            return RS_IN_PKT;

        case RST_LEN:
            rxstate->crc = crc32(&new_ch, 1, rxstate->crc);

            if (rxstate->field_c++ == 0)
            {
                /* first length byte */
                packet->len = ((unsigned int)new_ch) << 8;
                return RS_IN_PKT;
            }
            else
            {
                /* got the whole legth */
                packet->len |= new_ch;
                LogTrace1("rxe-len-%4x\n", packet->len);

                /* check that the length is ok */
                if (packet->len == 0)
                {
                    /* empty pkt */
                    rxstate->field_c = 0;
                    rxstate->rx_state = RST_CRC;
                    return RS_IN_PKT;
                }
                else
                {
                    if (packet->data == NULL)
                    {
                        /* need to alloc the data buffer */
                        if (!rxstate->config->ba_callback(
                                            packet, rxstate->config->ba_data))
                        {
                            LogWarning(LOG_RX, ("RxEngine: Couldn't alloc %d byte buffer; "
                                                  "dropping packet\n", packet->len));
                            rxstate->rx_state = RST_STX;
                            rxstate->error = RE_INTERNAL;
                            return RS_NO_BUF;
                        }
                    }

                    if (packet->len > packet->buf_len)
                    {
                        /* pkt bigger than buffer */
                        rxstate->field_c = 0;
                        rxstate->rx_state = RST_STX;
                        rxstate->error = RE_LEN;
                        return RS_BAD_PKT;
                    }
                    else
                    {
                        /* packet ok */
                        rxstate->field_c = 0;
                        rxstate->rx_state = RST_DAT;
                        return RS_IN_PKT;
                    }
                }
            }

        case RST_DAT:
            /* dummy case (dealt with earlier) */
            LogWarning(LOG_RX, ( "ERROR: hit RST_dat in switch\n"));
            rxstate->rx_state = RST_STX;
            rxstate->error = RE_INTERNAL;
            return RS_BAD_PKT;

        case RST_CRC:
            if (rxstate->field_c == 0)
                packet->crc = 0;

            packet->crc |= (new_ch & 0xFF) << ((3 - rxstate->field_c) * 8);
            rxstate->field_c++;

            if (rxstate->field_c == 4)
            {
                /* last crc field */
                rxstate->field_c = 0;
                rxstate->rx_state = RST_ETX;
                LogTrace1("rxe-rcrc-%8x ", packet->crc);
            }

            return RS_IN_PKT;

        case RST_ETX:
            if (new_ch == rxstate->config->etx)
            {
                /* check crc */
                if (rxstate->crc == packet->crc)
                {
                    /* crc ok */
                    rxstate->rx_state = RST_STX;
                    rxstate->field_c = 0;
                    return RS_GOOD_PKT;
                }
                else
                {
                    LogWarning(LOG_RX, ( "Bad crc (0x%x), rx calculates it should be 0x%x\n",
                                         packet->crc, rxstate->crc));
                    rxstate->rx_state = RST_STX;
                    rxstate->error = RE_CRC;
                    return RS_BAD_PKT;
                }
            }
            else if (new_ch == rxstate->config->stx)
                return unexp_stx(rxstate);
            else
            {
                rxstate->rx_state = RST_STX;
                rxstate->error = RE_NETX;
                return RS_BAD_PKT;
            }

        default:
            LogWarning(LOG_RX, ( "ERROR fell through rxengine\n"));
            rxstate->rx_state = RST_STX;
            rxstate->error = RE_INTERNAL;
            return RS_BAD_PKT;
    }
}

static re_status 
unexp_stx(struct re_state *rxstate)
{
    LogWarning(LOG_RX, ( "Unexpected stx\n"));
    rxstate->crc = startCRC32;
    rxstate->index = 0;
    rxstate->rx_state = RST_TYP;
    rxstate->error = RE_U_STX;
    rxstate->field_c = 0;
    return RS_BAD_PKT;
}

static re_status 
unexp_etx(struct re_state *rxstate)
{
    LogWarning(LOG_RX, ( "Unexpected etx, rxstate: index= 0x%x, field_c=0x%x, state=0x%x\n",
                         rxstate->index, rxstate->field_c, rxstate->rx_state));
    rxstate->crc = 0;
    rxstate->index = 0;
    rxstate->rx_state = RST_STX;
    rxstate->error = RE_U_ETX;
    rxstate->field_c = 0;
    return RS_BAD_PKT;
}

/*
 * This can be used as the buffer allocation callback for the rx engine,
 * and makes use of angel_DD_GetBuffer() [in devdrv.h]. 
 *
 * Saves duplicating this callback function in every device driver that
 * uses the rx engine.
 *
 * Note that this REQUIRES that the device id is installed as cb_data
 * in the rx engine config structure for the driver.
 */
bool 
angel_DD_RxEng_BufferAlloc(struct data_packet * packet, void *cb_data)
{
#ifdef TARGET
    DeviceID devid = (DeviceID) cb_data;

#else
    IGNORE(cb_data);
#endif

    if (packet->type < DC_NUM_CHANNELS)
    {
        /* request a buffer down from the channels layer */
#ifdef TARGET
        packet->data = angel_DD_GetBuffer(devid, packet->type,
                                          packet->len);
#else
        packet->data = malloc(packet->len);
#endif
        if (packet->data == NULL)
            return FALSE;
        else
        {
            packet->buf_len = packet->len;
            return TRUE;
        }
    }
    else
    {
        /* bad type field */
        return FALSE;
    }
}

/* EOF rx.c */
