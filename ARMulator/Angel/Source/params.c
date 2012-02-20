/* -*-C-*-
 *
 * $Revision: 1.5.6.4 $
 *   $Author: rivimey $
 *     $Date: 1998/01/28 15:37:57 $
 *
 * Copyright (c) 1996 Advanced RISC Machines Limited.
 * All Rights Reserved.
 *
 *   Project: ANGEL
 *
 *     Title: Parameter negotiation utility functions
 */

#include "params.h"

#include "endian.h"
#include "logging.h"


/*
 * Function: Angel_FindParam
 *  Purpose: find the value of a given parameter from a config.
 *
 * see params.h for details
 */
bool 
Angel_FindParam(ADP_Parameter type,
                const ParameterConfig * config,
                unsigned int *value)
{
    unsigned int i;

    for (i = 0; i < config->num_parameters; ++i)
        if (config->param[i].type == type)
        {
            *value = config->param[i].value;
            return TRUE;
        }

    return FALSE;
}


#if !defined(TARGET) || !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0

ParameterList *
Angel_FindParamList(const ParameterOptions * options,
                    ADP_Parameter type)
{
    unsigned int i;

    for (i = 0; i < options->num_param_lists; ++i)
        if (options->param_list[i].type == type)
            return &options->param_list[i];

    return NULL;
}


#if defined(TARGET) || defined(TEST_PARAMS)
/*
 * Function: Angel_MatchParams
 *  Purpose: find a configuration from the requested options which is
 *           the best match from the supported options.
 *
 * see params.h for details
 */
const ParameterConfig *
Angel_MatchParams(const ParameterOptions * requested,
                  const ParameterOptions * supported)
{
    static Parameter chosen_params[AP_NUM_PARAMS];
    static ParameterConfig chosen_config =
    {
        AP_NUM_PARAMS,
        chosen_params
    };
    unsigned int i;

    ASSERT(requested != NULL, ("Angel_MatchParams: requested is NULL"));
    ASSERT(supported->num_param_lists <= AP_NUM_PARAMS, ("supp_num too big"));

    if (requested->num_param_lists > supported->num_param_lists)
    {
        LogWarning(LOG_PARAM, ( "Angel_MatchParams: req_num %d exceeds supp_num %d\n",
                                requested->num_param_lists, supported->num_param_lists));
        return NULL;
    }

    for (i = 0; i < requested->num_param_lists; ++i)
    {
        bool match;
        unsigned int j;

        const ParameterList *req_list = &requested->param_list[i];
        ADP_Parameter req_type = req_list->type;
        const ParameterList *sup_list = Angel_FindParamList(
                                                         supported, req_type);

        if (sup_list == NULL)
        {
            LogWarning(LOG_PARAM, ( "option %x not supported\n", req_type));
            return NULL;
        }

        for (j = 0, match = FALSE;
             (j < req_list->num_options) && !match;
             ++j
            )
        {
            unsigned int k;

            for (k = 0;
                 (k < sup_list->num_options) && !match;
                 ++k
                )
            {
                if (req_list->option[j] == sup_list->option[k])
                {
                    LogInfo(LOG_PARAM, ("chose value %d for option %x\n",
                              req_list->option[j], req_type));
                    match = TRUE;
                    chosen_config.param[i].type = req_type;
                    chosen_config.param[i].value = req_list->option[j];
                }
            }
        }

        if (!match)
        {
            LogWarning(LOG_PARAM, ( "Angel_MatchParams: no match found for option %x\n", req_type));
            return NULL;
        }
    }

    chosen_config.num_parameters = i;
    LogInfo(LOG_PARAM, ( "match succeeded\n"));
    return &chosen_config;
}
#endif /* defined(TARGET) || defined(TEST_PARAMS) */


#if !defined(TARGET) || defined(TEST_PARAMS)
/*
 * Function: Angel_StoreParam
 *  Purpose: store the value of a given parameter to a config.
 *
 * see params.h for details
 */
bool 
Angel_StoreParam(ParameterConfig * config,
                 ADP_Parameter type,
                 unsigned int value)
{
    unsigned int i;

    for (i = 0; i < config->num_parameters; ++i)
        if (config->param[i].type == type)
        {
            config->param[i].value = value;
            return TRUE;
        }

    return FALSE;
}
#endif /* !defined(TARGET) || defined(TEST_PARAMS) */


#if defined(TARGET) || defined(LINK_RECOVERY) || defined(TEST_PARAMS)
/*
 * Function: Angel_BuildParamConfigMessage
 *  Purpose: write a parameter config to a buffer in ADP format.
 *
 * see params.h for details
 */
unsigned int 
Angel_BuildParamConfigMessage(unsigned char *buffer,
                              const ParameterConfig * config)
{
    unsigned char *start = buffer;
    unsigned int i;

    PUT32LE(buffer, config->num_parameters);
    buffer += 4;
    
    for (i = 0; i < config->num_parameters; ++i)
    {
        PUT32LE(buffer, config->param[i].type);
        buffer += 4;
        PUT32LE(buffer, config->param[i].value);
        buffer += 4;
    }

    return (buffer - start);
}
#endif /* defined(TARGET) || defined(LINK_RECOVERY) || defined(TEST_PARAMS) */


#if !defined(TARGET) || defined(TEST_PARAMS)
/*
 * Function: Angel_BuildParamOptionsMessage
 *  Purpose: write a parameter Options to a buffer in ADP format.
 *
 * see params.h for details
 */
unsigned int 
Angel_BuildParamOptionsMessage(unsigned char *buffer,
                               const ParameterOptions * options)
{
    unsigned char *start = buffer;
    unsigned int i, j, nopt;
    

    PUT32LE(buffer, options->num_param_lists);
    buffer += 4;

    for (i = 0; i < options->num_param_lists; ++i)
    {
        /*
         * HACK: originally, AP_MAX_OPTIONS was 5 (currently, it is 9); sending
         * more params than this causes old ROM's to fail.
         *
         * This code just causes some options to be ignored (e.g. in the case
         * of baud rates, the list: 38400, 19200, 9600, 4800, 2400, 1200 is 6
         * options so the last (1200) would not be sent.
         *
         * -- RIC 1/98
         */
#define MAX_OPTIONS 5

        if (options->param_list[i].num_options > MAX_OPTIONS)
            nopt = MAX_OPTIONS;
        else
            nopt = options->param_list[i].num_options;
        
        LogInfo(LOG_PARAM, ("BPOM: Type %d Num: %d (%d)\n",
                            options->param_list[i].type,
                            options->param_list[i].num_options,
                            nopt));
        
        PUT32LE(buffer, options->param_list[i].type);
        buffer += 4;
        PUT32LE(buffer, nopt);
        buffer += 4;

        for (j = 0; j < nopt; ++j)
        {
            LogInfo(LOG_PARAM, ("BPOM: Option %d (%x)\n",
                                options->param_list[i].option[j], options->param_list[i].option[j]));
            
            PUT32LE(buffer, options->param_list[i].option[j]);
            buffer += 4;
        }
    }

    return (buffer - start);
}
#endif /* !defined(TARGET) || defined(TEST_PARAMS) */


#if !defined(TARGET) || defined(LINK_RECOVERY) || defined(TEST_PARAMS)
/*
 * Function: Angel_ReadParamConfigMessage
 *  Purpose: read a parameter config from a buffer where it is in ADP format.
 *
 * see params.h for details
 */
bool 
Angel_ReadParamConfigMessage(const unsigned char *buffer,
                             ParameterConfig * config)
{
    unsigned int word;
    unsigned int i;

    word = GET32LE(buffer);
    buffer += 4;
    if (word > config->num_parameters)
    {
        LogWarning(LOG_PARAM, ( "not enough space (%d > %d)", word, config->num_parameters));
        return FALSE;
    }
    config->num_parameters = word;

    for (i = 0; i < config->num_parameters; ++i)
    {
        config->param[i].type = (ADP_Parameter) GET32LE(buffer);
        buffer += 4;
        config->param[i].value = GET32LE(buffer);
        buffer += 4;
        LogInfo(LOG_PARAM, ("ReadParam %d: type %d value %d (0x%x)\n", 
                            i, config->param[i].type,
                            config->param[i].value,
                            config->param[i].value ));
    }

    return TRUE;
}
#endif /* !defined(TARGET) || defined(LINK_RECOVERY) || defined(TEST_PARAMS) */


#if defined(TARGET) || defined(TEST_PARAMS)
/*
 * Function: Angel_ReadParamOptionsMessage
 *  Purpose: read a parameter options block from a buffer
 *             where it is in ADP format.
 *
 * see params.h for details
 */
bool 
Angel_ReadParamOptionsMessage(const unsigned char *buffer,
                              ParameterOptions * options)
{
    unsigned int word;
    unsigned int i, j, parleft;

    word = GET32LE(buffer);
    buffer += 4;
    if (word > options->num_param_lists)
    {
        LogWarning(LOG_PARAM, ( "not enough space (%d > %d)", word, options->num_param_lists));
        parleft = word - options->num_param_lists;
    }
    else
    {
        options->num_param_lists = word;
        parleft = 0;
    }
    
    for (i = 0; i < options->num_param_lists; ++i)
    {
        ParameterList *list = &options->param_list[i];
        int optleft;

        list->type = (ADP_Parameter) GET32LE(buffer);
        buffer += 4;
        word = GET32LE(buffer);
        buffer += 4;
        
        if (word > list->num_options)
        {
            LogWarning(LOG_PARAM, ( "not enough list space (%d > %d)\n", word, list->num_options));
            optleft = word - list->num_options;
        }
        else
        {
            list->num_options = word;
            optleft = 0;
        }

        for (j = 0; j < list->num_options; ++j)
        {
            list->option[j] = GET32LE(buffer);
            buffer += 4;
        }
        if (optleft > 0)
        {
            LogWarning(LOG_PARAM, ( "Skipping %d options...\n", optleft));
            buffer += (optleft * 4);
        }
    }
    if (parleft)
    {
        LogWarning(LOG_PARAM, ( "Skipping %d parameters...\n", parleft));
        /* don't actually have to do it! */
    }

    return TRUE;
}
#endif /* defined(TARGET) || defined(TEST_PARAMS) */

#endif /* !define(TARGET) || !defined(MINIMAL_ANGEL) || MINIMAL_ANGEL == 0 */

/* EOF params.c */
