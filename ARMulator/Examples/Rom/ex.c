
#ifdef __thumb
/* Define Angel Semihosting SWI to be Thumb one */
#define SemiSWI 0xAB
#else
/* Define Angel Semihosting SWI to be ARM one */
#define SemiSWI 0x123456
#endif

/* We use the following Debug Monitor SWIs to write things out
 * in this example
 */

/* Write a character */ 
__swi(SemiSWI) void _WriteC(unsigned op, const char *c);
#define WriteC(c) _WriteC (0x3,c)

/* Exit */
__swi(SemiSWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit (0x18,0x20026)

/* The following symbols are defined by the linker and define
 * various memory regions which may need to be copied or initialised
 */
extern char Image$$RO$$Limit[];
extern char Image$$RW$$Base[];

/* We define some more meaningfull names here */
#define rom_data_base Image$$RO$$Limit
#define ram_data_base Image$$RW$$Base

/* This is an example of a pre-initialised variable. */
static unsigned factory_id = 0xAA55AA55;  /* Factory set ID */

/* This is an example of an uninitialised (or 0 initialised) variable */
static char display[8][40];               /* Screen buffer */

static const char hex[17] = "0123456789ABCDEF";

static void pr_hex(unsigned n)
{
    int i;
    for (i = 0; i < 8; i++) {
        WriteC(&hex[n >> 28]);
        n <<= 4;
    }
}

void Write0 (const char *string)
{ int pos = 0;
  while (string[pos] != 0)
    WriteC(&string[pos++]);
}

void C_Entry(void)
{
  if (rom_data_base == ram_data_base) {
    Write0("Warning: Image has been linked as an application. To link as a ROM image\r\n");
    Write0("         link with the options -RO <rom-base> -RW <ram-base>\r\n");
  }

  Write0("'factory_id' is at address ");
  pr_hex((unsigned)&factory_id);
  Write0(", contents = ");
  pr_hex((unsigned)factory_id);
  Write0("\r\n");

  Write0("'display' is at address ");
  pr_hex((unsigned)display);
  Write0("\r\n");
  Exit();
}
