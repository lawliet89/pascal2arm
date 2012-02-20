MKTMP2=MkTmp2
MKTMP1=MkTmp1

CC=cc
LD=link
AS=objasm
LIBFILE=libfile
RM=remove
RENAME=rename
COPY=copy
ECHO=^.util.echo

CFLAGS=-ffah -c -J../stdh -I..,.
DCFLAGS=-ffah -c -J ../stdh -I..,. -M
AFLAGS=-s -I..,.
DAFLAGS=-s -I..,. -D $(MKTMP2)

#.SILENT:

all:    lib.AnsiLib

clean:; -$(RM) lib.AnsiLib o*.*

lib.AnsiLib:    \
        o.ctype o.isalnum o.isalpha o.iscntrl o.isdigit o.isgraph o.islower\
        o.isprint o.ispunct o.isspace o.isupper o.isxdigit o.tolower o.toupper\
        o._set8859 o.abs o.cosh o.div o.fmod o.frexp o.labs o.ldexp o.ldiv\
        o.sinh o.tanh o.xmath o.acos o.asin o.atan o.atan2 o.ceil o.cos o.exp\
        o.fabs o.floor o.log o.log10 o.modf o.pow o.sin o.sqrt o.tan o._edom_mh\
        o._ernge_0 o._ernge_h o._ernge_mh o._ldfp o._set_errno o._stfp\
        o._fmapstore o._mapstore o._map_store o._write_pro o.image o.fprintf\
        o.printf o.sprintf o.vfprintf o.vprintf o.vsprintf o._fprintf\
        o._fp_disp o._nofp_disp o._printf o._sprintf o._vfprintf o._vsprintf\
        o.__vfprintf o._chval o._strtoul o.strtod o.strtol o.strtoul o._scanf\
        o.scanf o.fscanf o.sscanf o1.atof o1.atoi o1.atol o1.backspace\
        o1.clearerr o1.feof o1.ferror o1.fflush o1.fgetc o1.fgetpos o1.fgets\
        o1.fisatty o1.filbuf o1.flsbuf o1.fputc o1.fputs o1.fread o1.fseek\
        o1.fsetpos o1.ftell o1.fwrite o1.getc o1.getchar o1.gets o1.osf\
        o1.perror o1.putc o1.putchar o1.puts o1.rewind o1.setbuf o1.setvbuf\
        o1.stdio o1.tmpnam o1.ungetc o1._sys_msg o1.__dup o1.alloc o1.calloc\
        o1.realloc o1.rand o1.ANSI_rand o1.atexit o1.exit o1.abort o1.qsort\
        o1.bsearch o1._version o1.signal o1.assert o1.locale o1.localeconv\
        o1._lc o1.mblen o1.mbtowc o1.wctomb o1.mbstowcs o1.wcstombs o1.memchr\
        o1.memcmp o1.strcat o1.strchr o1.strcmp o1.strcoll o1.strcpy o1.strcspn\
        o1.strerror o1.strlen o1.strncat o1.strncmp o1.strncpy o1.strpbrk\
        o1.strrchr o1.strspn o1.strstr o1.strtok o1.strxfrm o2._strerror\
        o2.memcpy o2.memset o2.asctime o2.ctime o2.difftime o2.gmtime\
        o2.localtime o2.mktime o2.strftime o2._monlen o2.rt_count o2.rt_count1\
        o2.rt_divtest o2.rt_sdiv10 o2.rt_udiv10 o2.rt_sdiv o2.rt_udiv\
        o2.rt_div0 o2.rt_rwcheck o2.setjmp o2.stkchunks o2.chunk_copy o2.unwind\
        o2.reg_alloc o2.lib_init o2._sysdie o2._sys_alloc o2._main o2.handlers\
        o2.hosterror o2.hostsignal o2.remove o2.rename o2._sys_open o2._ttyread\
        o2._ttywrite o2.sys_istty o2.sys_seek o2.sys_flen o2.sys_iserr\
        o2.sys_write o2.sys_read o2.sys_ensure o2.sys_close o2.clock o2.time\
        o2.system o2.getenv o2.overmgr o2._tmpnam o2.kernel o2.k_escseen\
        o2.k_system o2.traphandle o2.heapsuppt o2.osargs o2.osbget o2.osbput\
        o2.osbyte o2.oscli o2.osfile o2.osfind o2.osgbpb o2.osrdch o2.osword\
        o2.oswrch o2.swi o2.k_getenv o2.k_setenv o2.k_hostos o2.lasterr\
        o2.infpe
                $(RM) lib.AnsiLib
                $(LIBFILE) -c lib.AnsiLib -v lfile
                $(LIBFILE) -oc lib.AnsiLib

o.ctype:;       ${CC} ${CFLAGS} -Dctype_c -o o.ctype ^.c.ctype
o.isalnum:;     ${CC} ${CFLAGS} -Disalnum_c -o o.isalnum ^.c.ctype
o.isalpha:;     ${CC} ${CFLAGS} -Disalpha_c -o o.isalpha ^.c.ctype
o.iscntrl:;     ${CC} ${CFLAGS} -Discntrl_c -o o.iscntrl ^.c.ctype
o.isdigit:;     ${CC} ${CFLAGS} -Disdigit_c -o o.isdigit ^.c.ctype
o.isgraph:;     ${CC} ${CFLAGS} -Disgraph_c -o o.isgraph ^.c.ctype
o.islower:;     ${CC} ${CFLAGS} -Dislower_c -o o.islower ^.c.ctype
o.isprint:;     ${CC} ${CFLAGS} -Disprint_c -o o.isprint ^.c.ctype
o.ispunct:;     ${CC} ${CFLAGS} -Dispunct_c -o o.ispunct ^.c.ctype
o.isspace:;     ${CC} ${CFLAGS} -Disspace_c -o o.isspace ^.c.ctype
o.isupper:;     ${CC} ${CFLAGS} -Disupper_c -o o.isupper ^.c.ctype
o.isxdigit:;    ${CC} ${CFLAGS} -Disxdigit_c -o o.isxdigit ^.c.ctype
o.tolower:;     ${CC} ${CFLAGS} -Dtolower_c -o o.tolower ^.c.ctype
o.toupper:;     ${CC} ${CFLAGS} -Dtoupper_c -o o.toupper ^.c.ctype
o._set8859:;    ${CC} ${CFLAGS} -D_set8859_c -o o._set8859 ^.c.ctype
o.abs:;         ${CC} ${CFLAGS} -Dabs_c -o o.abs ^.c.math
o.cosh:;        ${CC} ${CFLAGS} -Dcosh_c -o o.cosh ^.c.math
o.div:;         ${CC} ${CFLAGS} -Ddiv_c -o o.div ^.c.math
o.fmod:;        ${CC} ${CFLAGS} -Dfmod_c -o o.fmod ^.c.math
o.frexp:;       ${CC} ${CFLAGS} -Dfrexp_c -o o.frexp ^.c.math
o.labs:;        ${CC} ${CFLAGS} -Dlabs_c -o o.labs ^.c.math
o.ldexp:;       ${CC} ${CFLAGS} -Dldexp_c -o o.ldexp ^.c.math
o.ldiv:;        ${CC} ${CFLAGS} -Dldiv_c -o o.ldiv ^.c.math
o.sinh:;        ${CC} ${CFLAGS} -Dsinh_c -o o.sinh ^.c.math
o.tanh:;        ${CC} ${CFLAGS} -Dtanh_c -o o.tanh ^.c.math
o.xmath:;       ${CC} ${CFLAGS}  -o o.xmath ^.c.xmath
o.acos:;        ${AS} ${AFLAGS} -PD "make SETS \"acos\"" -o o.acos ^.s.mathf
o.asin:;        ${AS} ${AFLAGS} -PD "make SETS \"asin\"" -o o.asin ^.s.mathf
o.atan:;        ${AS} ${AFLAGS} -PD "make SETS \"atan\"" -o o.atan ^.s.mathf
o.atan2:;       ${AS} ${AFLAGS} -PD "make SETS \"atan2\"" -o o.atan2 ^.s.mathf
o.ceil:;        ${AS} ${AFLAGS} -PD "make SETS \"ceil\"" -o o.ceil ^.s.mathf
o.cos:;         ${AS} ${AFLAGS} -PD "make SETS \"cos\"" -o o.cos ^.s.mathf
o.exp:;         ${AS} ${AFLAGS} -PD "make SETS \"exp\"" -o o.exp ^.s.mathf
o.fabs:;        ${AS} ${AFLAGS} -PD "make SETS \"fabs\"" -o o.fabs ^.s.mathf
o.floor:;       ${AS} ${AFLAGS} -PD "make SETS \"floor\"" -o o.floor ^.s.mathf
o.log:;         ${AS} ${AFLAGS} -PD "make SETS \"log\"" -o o.log ^.s.mathf
o.log10:;       ${AS} ${AFLAGS} -PD "make SETS \"log10\"" -o o.log10 ^.s.mathf
o.modf:;        ${AS} ${AFLAGS} -PD "make SETS \"modf\"" -o o.modf ^.s.mathf
o.pow:;         ${AS} ${AFLAGS} -PD "make SETS \"pow\"" -o o.pow ^.s.mathf
o.sin:;         ${AS} ${AFLAGS} -PD "make SETS \"sin\"" -o o.sin ^.s.mathf
o.sqrt:;        ${AS} ${AFLAGS} -PD "make SETS \"sqrt\"" -o o.sqrt ^.s.mathf
o.tan:;         ${AS} ${AFLAGS} -PD "make SETS \"tan\"" -o o.tan ^.s.mathf
o._edom_mh:;    ${AS} ${AFLAGS} -PD "make SETS \"_edom_mh\"" -o o._edom_mh ^.s.mathf
o._ernge_0:;    ${AS} ${AFLAGS} -PD "make SETS \"_ernge_0\"" -o o._ernge_0 ^.s.mathf
o._ernge_h:;    ${AS} ${AFLAGS} -PD "make SETS \"_ernge_h\"" -o o._ernge_h ^.s.mathf
o._ernge_mh:;   ${AS} ${AFLAGS} -PD "make SETS \"_ernge_mh\"" -o o._ernge_mh ^.s.mathf
o._ldfp:;       ${AS} ${AFLAGS} -PD "make SETS \"_ldfp\"" -o o._ldfp ^.s.mathf
o._set_errno:;  ${AS} ${AFLAGS} -PD "make SETS \"_set_errno\"" -o o._set_errno ^.s.mathf
o._stfp:;       ${AS} ${AFLAGS} -PD "make SETS \"_stfp\"" -o o._stfp ^.s.mathf
o._fmapstore:;  ${CC} ${CFLAGS} -D_fmapstore_c -o o._fmapstore ^.c.profile
o._mapstore:;   ${CC} ${CFLAGS} -D_mapstore_c -o o._mapstore ^.c.profile
o._map_store:;  ${CC} ${CFLAGS} -D_map_store_c -o o._map_store ^.c.profile
o._write_pro:;  ${CC} ${CFLAGS} -D_write_pro_c -o o._write_pro ^.c.profile
o.image:;       ${AS} ${AFLAGS}  -o o.image ^.s.image
o.fprintf:;     ${CC} ${CFLAGS} -Dfprintf_c -o o.fprintf ^.c.printf
o.printf:;      ${CC} ${CFLAGS} -Dprintf_c -o o.printf ^.c.printf
o.sprintf:;     ${CC} ${CFLAGS} -Dsprintf_c -o o.sprintf ^.c.printf
o.vfprintf:;    ${CC} ${CFLAGS} -Dvfprintf_c -o o.vfprintf ^.c.printf
o.vprintf:;     ${CC} ${CFLAGS} -Dvprintf_c -o o.vprintf ^.c.printf
o.vsprintf:;    ${CC} ${CFLAGS} -Dvsprintf_c -o o.vsprintf ^.c.printf
o._fprintf:;    ${CC} ${CFLAGS} -D_fprintf_c -o o._fprintf ^.c.printf
o._fp_disp:;    ${CC} ${CFLAGS} -D_fp_disp_c -o o._fp_disp ^.c.printf
o._nofp_disp:;  ${CC} ${CFLAGS} -D_nofp_disp_c -o o._nofp_disp ^.c.printf
o._printf:;     ${CC} ${CFLAGS} -D_printf_c -o o._printf ^.c.printf
o._sprintf:;    ${CC} ${CFLAGS} -D_sprintf_c -o o._sprintf ^.c.printf
o._vfprintf:;   ${CC} ${CFLAGS} -D_vfprintf_c -o o._vfprintf ^.c.printf
o._vsprintf:;   ${CC} ${CFLAGS} -D_vsprintf_c -o o._vsprintf ^.c.printf
o.__vfprintf:;  ${CC} ${CFLAGS} -D__vfprintf_c -o o.__vfprintf ^.c.printf
o._chval:;      ${CC} ${CFLAGS} -D_chval_c -o o._chval ^.c.scanf
o._strtoul:;    ${CC} ${CFLAGS} -D_strtoul_c -o o._strtoul ^.c.scanf
o.strtod:;      ${CC} ${CFLAGS} -Dstrtod_c -o o.strtod ^.c.scanf
o.strtol:;      ${CC} ${CFLAGS} -Dstrtol_c -o o.strtol ^.c.scanf
o.strtoul:;     ${CC} ${CFLAGS} -Dstrtoul_c -o o.strtoul ^.c.scanf
o._scanf:;      ${CC} ${CFLAGS} -D_scanf_c -o o._scanf ^.c.scanf
o.scanf:;       ${CC} ${CFLAGS} -Dscanf_c -o o.scanf ^.c.scanf
o.fscanf:;      ${CC} ${CFLAGS} -Dfscanf_c -o o.fscanf ^.c.scanf
o.sscanf:;      ${CC} ${CFLAGS} -Dsscanf_c -o o.sscanf ^.c.scanf
o1.atof:;       ${CC} ${CFLAGS} -Datof_c -o o1.atof ^.c.scanf
o1.atoi:;       ${CC} ${CFLAGS} -Datoi_c -o o1.atoi ^.c.scanf
o1.atol:;       ${CC} ${CFLAGS} -Datol_c -o o1.atol ^.c.scanf
o1.backspace:;  ${CC} ${CFLAGS} -Dbackspace_c -o o1.backspace ^.c.stdio
o1.clearerr:;   ${CC} ${CFLAGS} -Dclearerr_c -o o1.clearerr ^.c.stdio
o1.feof:;       ${CC} ${CFLAGS} -Dfeof_c -o o1.feof ^.c.stdio
o1.ferror:;     ${CC} ${CFLAGS} -Dferror_c -o o1.ferror ^.c.stdio
o1.fflush:;     ${CC} ${CFLAGS} -Dfflush_c -o o1.fflush ^.c.stdio
o1.fgetc:;      ${CC} ${CFLAGS} -Dfgetc_c -o o1.fgetc ^.c.stdio
o1.fgetpos:;    ${CC} ${CFLAGS} -Dfgetpos_c -o o1.fgetpos ^.c.stdio
o1.fgets:;      ${CC} ${CFLAGS} -Dfgets_c -o o1.fgets ^.c.stdio
o1.fisatty:;    ${CC} ${CFLAGS} -Dfisatty_c -o o1.fisatty ^.c.stdio
o1.filbuf:;     ${CC} ${CFLAGS} -Dfilbuf_c -o o1.filbuf ^.c.stdio
o1.flsbuf:;     ${CC} ${CFLAGS} -Dflsbuf_c -o o1.flsbuf ^.c.stdio
o1.fputc:;      ${CC} ${CFLAGS} -Dfputc_c -o o1.fputc ^.c.stdio
o1.fputs:;      ${CC} ${CFLAGS} -Dfputs_c -o o1.fputs ^.c.stdio
o1.fread:;      ${CC} ${CFLAGS} -Dfread_c -o o1.fread ^.c.stdio
o1.fseek:;      ${CC} ${CFLAGS} -Dfseek_c -o o1.fseek ^.c.stdio
o1.fsetpos:;    ${CC} ${CFLAGS} -Dfsetpos_c -o o1.fsetpos ^.c.stdio
o1.ftell:;      ${CC} ${CFLAGS} -Dftell_c -o o1.ftell ^.c.stdio
o1.fwrite:;     ${CC} ${CFLAGS} -Dfwrite_c -o o1.fwrite ^.c.stdio
o1.getc:;       ${CC} ${CFLAGS} -Dgetc_c -o o1.getc ^.c.stdio
o1.getchar:;    ${CC} ${CFLAGS} -Dgetchar_c -o o1.getchar ^.c.stdio
o1.gets:;       ${CC} ${CFLAGS} -Dgets_c -o o1.gets ^.c.stdio
o1.osf:;        ${CC} ${CFLAGS} -Dosf_c -o o1.osf ^.c.stdio
o1.perror:;     ${CC} ${CFLAGS} -Dperror_c -o o1.perror ^.c.stdio
o1.putc:;       ${CC} ${CFLAGS} -Dputc_c -o o1.putc ^.c.stdio
o1.putchar:;    ${CC} ${CFLAGS} -Dputchar_c -o o1.putchar ^.c.stdio
o1.puts:;       ${CC} ${CFLAGS} -Dputs_c -o o1.puts ^.c.stdio
o1.rewind:;     ${CC} ${CFLAGS} -Drewind_c -o o1.rewind ^.c.stdio
o1.setbuf:;     ${CC} ${CFLAGS} -Dsetbuf_c -o o1.setbuf ^.c.stdio
o1.setvbuf:;    ${CC} ${CFLAGS} -Dsetvbuf_c -o o1.setvbuf ^.c.stdio
o1.stdio:;      ${CC} ${CFLAGS} -Dstdio_c -o o1.stdio ^.c.stdio
o1.tmpnam:;     ${CC} ${CFLAGS} -Dtmpnam_c -o o1.tmpnam ^.c.stdio
o1.ungetc:;     ${CC} ${CFLAGS} -Dungetc_c -o o1.ungetc ^.c.stdio
o1._sys_msg:;   ${CC} ${CFLAGS} -D_sys_msg_c -o o1._sys_msg ^.c.stdio
o1.__dup:;      ${CC} ${CFLAGS} -D__dup_c -o o1.__dup ^.c.stdio
o1.alloc:;      ${CC} ${CFLAGS} -Dalloc_c -o o1.alloc ^.c.alloc
o1.calloc:;     ${CC} ${CFLAGS} -Dcalloc_c -o o1.calloc ^.c.alloc
o1.realloc:;    ${CC} ${CFLAGS} -Drealloc_c -o o1.realloc ^.c.alloc
o1.rand:;       ${CC} ${CFLAGS} -Drand_c -o o1.rand ^.c.stdlib
o1.ANSI_rand:;  ${CC} ${CFLAGS} -DANSI_rand_c -o o1.ANSI_rand ^.c.stdlib
o1.atexit:;     ${CC} ${CFLAGS} -Datexit_c -o o1.atexit ^.c.stdlib
o1.exit:;       ${CC} ${CFLAGS} -Dexit_c -o o1.exit ^.c.stdlib
o1.abort:;      ${CC} ${CFLAGS} -Dabort_c -o o1.abort ^.c.stdlib
o1.qsort:;      ${CC} ${CFLAGS} -Dqsort_c -o o1.qsort ^.c.stdlib
o1.bsearch:;    ${CC} ${CFLAGS} -Dbsearch_c -o o1.bsearch ^.c.stdlib
o1._version:;   ${CC} ${CFLAGS} -D_version_c -o o1._version ^.c.stdlib
o1.signal:;     ${CC} ${CFLAGS}  -o o1.signal ^.c.signal
o1.assert:;     ${CC} ${CFLAGS}  -o o1.assert ^.c.assert
o1.locale:;     ${CC} ${CFLAGS} -Dlocale_c -o o1.locale ^.c.locale
o1.localeconv:; ${CC} ${CFLAGS} -Dlocaleconv_c -o o1.localeconv ^.c.locale
o1._lc:;        ${CC} ${CFLAGS} -D_lc_c -o o1._lc ^.c.locale
o1.mblen:;      ${CC} ${CFLAGS} -Dmblen_c -o o1.mblen ^.c.locale
o1.mbtowc:;     ${CC} ${CFLAGS} -Dmbtowc_c -o o1.mbtowc ^.c.locale
o1.wctomb:;     ${CC} ${CFLAGS} -Dwctomb_c -o o1.wctomb ^.c.locale
o1.mbstowcs:;   ${CC} ${CFLAGS} -Dmbstowcs_c -o o1.mbstowcs ^.c.locale
o1.wcstombs:;   ${CC} ${CFLAGS} -Dwcstombs_c -o o1.wcstombs ^.c.locale
o1.memchr:;     ${CC} ${CFLAGS} -Dmemchr_c -o o1.memchr ^.c.string
o1.memcmp:;     ${CC} ${CFLAGS} -Dmemcmp_c -o o1.memcmp ^.c.string
o1.strcat:;     ${CC} ${CFLAGS} -Dstrcat_c -o o1.strcat ^.c.string
o1.strchr:;     ${CC} ${CFLAGS} -Dstrchr_c -o o1.strchr ^.c.string
o1.strcmp:;     ${CC} ${CFLAGS} -Dstrcmp_c -o o1.strcmp ^.c.string
o1.strcoll:;    ${CC} ${CFLAGS} -Dstrcoll_c -o o1.strcoll ^.c.string
o1.strcpy:;     ${CC} ${CFLAGS} -Dstrcpy_c -o o1.strcpy ^.c.string
o1.strcspn:;    ${CC} ${CFLAGS} -Dstrcspn_c -o o1.strcspn ^.c.string
o1.strerror:;   ${CC} ${CFLAGS} -Dstrerror_c -o o1.strerror ^.c.string
o1.strlen:;     ${CC} ${CFLAGS} -Dstrlen_c -o o1.strlen ^.c.string
o1.strncat:;    ${CC} ${CFLAGS} -Dstrncat_c -o o1.strncat ^.c.string
o1.strncmp:;    ${CC} ${CFLAGS} -Dstrncmp_c -o o1.strncmp ^.c.string
o1.strncpy:;    ${CC} ${CFLAGS} -Dstrncpy_c -o o1.strncpy ^.c.string
o1.strpbrk:;    ${CC} ${CFLAGS} -Dstrpbrk_c -o o1.strpbrk ^.c.string
o1.strrchr:;    ${CC} ${CFLAGS} -Dstrrchr_c -o o1.strrchr ^.c.string
o1.strspn:;     ${CC} ${CFLAGS} -Dstrspn_c -o o1.strspn ^.c.string
o1.strstr:;     ${CC} ${CFLAGS} -Dstrstr_c -o o1.strstr ^.c.string
o1.strtok:;     ${CC} ${CFLAGS} -Dstrtok_c -o o1.strtok ^.c.string
o1.strxfrm:;    ${CC} ${CFLAGS} -Dstrxfrm_c -o o1.strxfrm ^.c.string
o2._strerror:;  ${CC} ${CFLAGS} -D_strerror_c -o o2._strerror ^.c.string
o2.memcpy:;     ${AS} ${AFLAGS} -PD "make SETS \"memcpy\"" -o o2.memcpy ^.s.memcpyset
o2.memset:;     ${AS} ${AFLAGS} -PD "make SETS \"memset\"" -o o2.memset ^.s.memcpyset
o2.asctime:;    ${CC} ${CFLAGS} -Dasctime_c -o o2.asctime ^.c.time
o2.ctime:;      ${CC} ${CFLAGS} -Dctime_c -o o2.ctime ^.c.time
o2.difftime:;   ${CC} ${CFLAGS} -Ddifftime_c -o o2.difftime ^.c.time
o2.gmtime:;     ${CC} ${CFLAGS} -Dgmtime_c -o o2.gmtime ^.c.time
o2.localtime:;  ${CC} ${CFLAGS} -Dlocaltime_c -o o2.localtime ^.c.time
o2.mktime:;     ${CC} ${CFLAGS} -Dmktime_c -o o2.mktime ^.c.time
o2.strftime:;   ${CC} ${CFLAGS} -Dstrftime_c -o o2.strftime ^.c.time
o2._monlen:;    ${CC} ${CFLAGS} -D_monlen_c -o o2._monlen ^.c.time
o2.rt_count:;   ${AS} ${AFLAGS} -PD "make SETS \"count\"" -o o2.rt_count ^.s.unhosted
o2.rt_count1:;  ${AS} ${AFLAGS} -PD "make SETS \"count1\"" -o o2.rt_count1 ^.s.unhosted
o2.rt_divtest:; ${AS} ${AFLAGS} -PD "make SETS \"divtest\"" -o o2.rt_divtest ^.s.unhosted
o2.rt_sdiv10:;  ${AS} ${AFLAGS} -PD "make SETS \"sdiv10\"" -o o2.rt_sdiv10 ^.s.unhosted
o2.rt_udiv10:;  ${AS} ${AFLAGS} -PD "make SETS \"udiv10\"" -o o2.rt_udiv10 ^.s.unhosted
o2.rt_sdiv:;    ${AS} ${AFLAGS} -PD "make SETS \"sdiv_unrolled8\"" -o o2.rt_sdiv ^.s.unhosted
o2.rt_udiv:;    ${AS} ${AFLAGS} -PD "make SETS \"udiv_unrolled8\"" -o o2.rt_udiv ^.s.unhosted
o2.rt_div0:;    ${AS} ${AFLAGS} -PD "make SETS \"div0\"" -o o2.rt_div0 ^.s.unhosted
o2.rt_rwcheck:; ${AS} ${AFLAGS} -PD "make SETS \"rwcheck\"" -o o2.rt_rwcheck ^.s.unhosted
o2.setjmp:;     ${AS} ${AFLAGS}  -o o2.setjmp ^.s.setjmp
o2.stkchunks:;  ${AS} ${AFLAGS} -PD "make SETS \"stkchunks\"" -o o2.stkchunks ^.s.stkchunk
o2.chunk_copy:; ${AS} ${AFLAGS} -PD "make SETS \"chunk_copy\"" -o o2.chunk_copy ^.s.stkchunk
o2.unwind:;     ${AS} ${AFLAGS}  -o o2.unwind ^.s.unwind
o2.reg_alloc:;  ${AS} ${AFLAGS}  -o o2.reg_alloc ^.s.reg_alloc
o2.lib_init:;   ${CC} ${CFLAGS} -Dlib_init_c -o o2.lib_init ^.c.armsys
o2._sysdie:;    ${CC} ${CFLAGS} -D_sysdie_c -o o2._sysdie ^.c.armsys
o2._sys_alloc:; ${CC} ${CFLAGS} -D_sys_alloc_c -o o2._sys_alloc ^.c.armsys
o2._main:;      ${CC} ${CFLAGS} -D_main_c -DSTDFILE_REDIRECTION -o o2._main ^.c.armsys
o2.handlers:;   ${AS} ${AFLAGS} -PD "backtrace_enabled SETL {FALSE}" -o o2.handlers ^.s.handlers
o2.hosterror:;  ${CC} ${CFLAGS}  -o o2.hosterror c.hosterror
o2.hostsignal:; ${CC} ${CFLAGS}  -o o2.hostsignal c.hostsignal
o2.remove:;     ${CC} ${CFLAGS} -Dremove_c -o o2.remove c.iosuppt
o2.rename:;     ${CC} ${CFLAGS} -Drename_c -o o2.rename c.iosuppt
o2._sys_open:;  ${CC} ${CFLAGS} -Dsys_open_c -o o2._sys_open c.iosuppt
o2._ttyread:;   ${CC} ${CFLAGS} -Dttyread_c -o o2._ttyread c.iosuppt
o2._ttywrite:;  ${CC} ${CFLAGS} -Dttywrite_c -o o2._ttywrite c.iosuppt
o2.sys_istty:;  ${CC} ${CFLAGS} -Dsys_istty_c -o o2.sys_istty c.iosuppt
o2.sys_seek:;   ${CC} ${CFLAGS} -Dsys_seek_c -o o2.sys_seek c.iosuppt
o2.sys_flen:;   ${CC} ${CFLAGS} -Dsys_flen_c -o o2.sys_flen c.iosuppt
o2.sys_iserr:;  ${CC} ${CFLAGS} -Dsys_iserror_c -o o2.sys_iserr c.iosuppt
o2.sys_write:;  ${CC} ${CFLAGS} -Dsys_write_c -o o2.sys_write c.iosuppt
o2.sys_read:;   ${CC} ${CFLAGS} -Dsys_read_c -o o2.sys_read c.iosuppt
o2.sys_ensure:; ${CC} ${CFLAGS} -Dsys_ensure_c -o o2.sys_ensure c.iosuppt
o2.sys_close:;  ${CC} ${CFLAGS} -Dsys_close_c -o o2.sys_close c.iosuppt
o2.clock:;      ${CC} ${CFLAGS} -Dclock_c -o o2.clock c.iosuppt
o2.time:;       ${CC} ${CFLAGS} -Dtime_c -o o2.time c.iosuppt
o2.system:;     ${CC} ${CFLAGS} -Dsystem_c -o o2.system c.iosuppt
o2.getenv:;     ${CC} ${CFLAGS} -Dgetenv_c -o o2.getenv c.iosuppt
o2.overmgr:;    ${AS} ${AFLAGS}  -o o2.overmgr s.overmgr
o2._tmpnam:;    ${AS} ${AFLAGS}  -o o2._tmpnam s._tmpnam
o2.kernel:;     ${AS} ${AFLAGS}  -o o2.kernel s.kernel
o2.k_escseen:;  ${AS} ${AFLAGS}  -o o2.k_escseen s.k_escseen
o2.k_system:;   ${AS} ${AFLAGS}  -o o2.k_system s.k_system
o2.traphandle:; ${AS} ${AFLAGS}  -o o2.traphandle s.traphandle
o2.heapsuppt:;  ${AS} ${AFLAGS}  -o o2.heapsuppt s.heapsuppt
o2.osargs:;     ${AS} ${AFLAGS} -PD "make SETS \"osargs\"" -o o2.osargs s.osveneer
o2.osbget:;     ${AS} ${AFLAGS} -PD "make SETS \"osbget\"" -o o2.osbget s.osveneer
o2.osbput:;     ${AS} ${AFLAGS} -PD "make SETS \"osbput\"" -o o2.osbput s.osveneer
o2.osbyte:;     ${AS} ${AFLAGS} -PD "make SETS \"osbyte\"" -o o2.osbyte s.osveneer
o2.oscli:;      ${AS} ${AFLAGS} -PD "make SETS \"oscli\"" -o o2.oscli s.osveneer
o2.osfile:;     ${AS} ${AFLAGS} -PD "make SETS \"osfile\"" -o o2.osfile s.osveneer
o2.osfind:;     ${AS} ${AFLAGS} -PD "make SETS \"osfind\"" -o o2.osfind s.osveneer
o2.osgbpb:;     ${AS} ${AFLAGS} -PD "make SETS \"osgbpb\"" -o o2.osgbpb s.osveneer
o2.osrdch:;     ${AS} ${AFLAGS} -PD "make SETS \"osrdch\"" -o o2.osrdch s.osveneer
o2.osword:;     ${AS} ${AFLAGS} -PD "make SETS \"osword\"" -o o2.osword s.osveneer
o2.oswrch:;     ${AS} ${AFLAGS} -PD "make SETS \"oswrch\"" -o o2.oswrch s.osveneer
o2.swi:;        ${AS} ${AFLAGS} -PD "make SETS \"swi\"" -o o2.swi s.osveneer
o2.k_getenv:;   ${AS} ${AFLAGS} -PD "make SETS \"k_getenv\"" -o o2.k_getenv s.osveneer
o2.k_setenv:;   ${AS} ${AFLAGS} -PD "make SETS \"k_setenv\"" -o o2.k_setenv s.osveneer
o2.k_hostos:;   ${AS} ${AFLAGS} -PD "make SETS \"k_hostos\"" -o o2.k_hostos s.osveneer
o2.lasterr:;    ${AS} ${AFLAGS} -PD "make SETS \"lasterr\"" -o o2.lasterr s.osveneer
o2.infpe:;      ${AS} ${AFLAGS}  -o o2.infpe s.infpe

depend: d_000 d_001 d_002 d_003 d_004 d_005 d_006 d_007 d_008 d_009 d_00a \
        d_00b d_00c d_00d d_00e d_00f d_010 d_011 d_012 d_013 d_014 d_015 \
        d_016 d_017 d_018 d_019 d_01a d_01b d_01c d_01d d_01e d_01f d_020 \
        d_021 d_022 d_023 d_024 d_025 d_026 d_027 d_028 d_029 d_02a d_02b \
        d_02c d_02d d_02e d_02f d_030 d_031 d_032 d_033 d_034 d_035 d_036 \
        d_037 d_038 d_039 d_03a d_03b d_03c d_03d d_03e d_03f d_040 d_041 \
        d_042 d_043 d_044 d_045 d_046 d_047 d_048 d_049 d_04a d_04b d_04c \
        d_04d d_04e d_04f d_050 d_051 d_052 d_053 d_054 d_055 d_056 d_057 \
        d_058 d_059 d_05a d_05b d_05c d_05d d_05e d_05f d_060 d_061 d_062 \
        d_063 d_064 d_065 d_066 d_067 d_068 d_069 d_06a d_06b d_06c d_06d \
        d_06e d_06f d_070 d_071 d_072 d_073 d_074 d_075 d_076 d_077 d_078 \
        d_079 d_07a d_07b d_07c d_07d d_07e d_07f d_080 d_081 d_082 d_083 \
        d_084 d_085 d_086 d_087 d_088 d_089 d_08a d_08b d_08c d_08d d_08e \
        d_08f d_090 d_091 d_092 d_093 d_094 d_095 d_096 d_097 d_098 d_099 \
        d_09a d_09b d_09c d_09d d_09e d_09f d_0a0 d_0a1 d_0a2 d_0a3 d_0a4 \
        d_0a5 d_0a6 d_0a7 d_0a8 d_0a9 d_0aa d_0ab d_0ac d_0ad d_0ae d_0af \
        d_0b0 d_0b1 d_0b2 d_0b3 d_0b4 d_0b5 d_0b6 d_0b7 d_0b8 d_0b9 d_0ba \
        d_0bb d_0bc d_0bd d_0be d_0bf d_0c0 d_0c1 d_0c2 d_0c3 d_0c4 d_0c5 \
        d_0c6 d_0c7 d_0c8 d_0c9 d_0ca d_0cb d_0cc d_0cd d_0ce d_0cf d_0d0 \
        d_0d1 d_0d2 d_0d3 d_0d4 d_0d5 d_0d6 d_0d7 d_0d8 d_0d9 d_0da d_0db \
        d_0dc d_0dd d_0de d_0df d_0e0 d_0e1 d_0e2
                ${RM} $(MKTMP2) Makefile
                ${RENAME} $(MKTMP1) Makefile

$(MKTMP1):;             ${COPY} MakeProto $(MKTMP1)

d_000:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Dctype_c -o o.ctype ^.c.ctype >> $(MKTMP1)
d_001:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disalnum_c -o o.isalnum ^.c.ctype >> $(MKTMP1)
d_002:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disalpha_c -o o.isalpha ^.c.ctype >> $(MKTMP1)
d_003:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Discntrl_c -o o.iscntrl ^.c.ctype >> $(MKTMP1)
d_004:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disdigit_c -o o.isdigit ^.c.ctype >> $(MKTMP1)
d_005:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disgraph_c -o o.isgraph ^.c.ctype >> $(MKTMP1)
d_006:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Dislower_c -o o.islower ^.c.ctype >> $(MKTMP1)
d_007:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disprint_c -o o.isprint ^.c.ctype >> $(MKTMP1)
d_008:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Dispunct_c -o o.ispunct ^.c.ctype >> $(MKTMP1)
d_009:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disspace_c -o o.isspace ^.c.ctype >> $(MKTMP1)
d_00a:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disupper_c -o o.isupper ^.c.ctype >> $(MKTMP1)
d_00b:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Disxdigit_c -o o.isxdigit ^.c.ctype >> $(MKTMP1)
d_00c:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Dtolower_c -o o.tolower ^.c.ctype >> $(MKTMP1)
d_00d:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -Dtoupper_c -o o.toupper ^.c.ctype >> $(MKTMP1)
d_00e:  ^.c.ctype $(MKTMP1)
        ${CC} ${DCFLAGS} -D_set8859_c -o o._set8859 ^.c.ctype >> $(MKTMP1)
d_00f:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dabs_c -o o.abs ^.c.math >> $(MKTMP1)
d_010:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dcosh_c -o o.cosh ^.c.math >> $(MKTMP1)
d_011:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Ddiv_c -o o.div ^.c.math >> $(MKTMP1)
d_012:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfmod_c -o o.fmod ^.c.math >> $(MKTMP1)
d_013:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfrexp_c -o o.frexp ^.c.math >> $(MKTMP1)
d_014:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dlabs_c -o o.labs ^.c.math >> $(MKTMP1)
d_015:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dldexp_c -o o.ldexp ^.c.math >> $(MKTMP1)
d_016:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dldiv_c -o o.ldiv ^.c.math >> $(MKTMP1)
d_017:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsinh_c -o o.sinh ^.c.math >> $(MKTMP1)
d_018:  ^.c.math $(MKTMP1)
        ${CC} ${DCFLAGS} -Dtanh_c -o o.tanh ^.c.math >> $(MKTMP1)
d_019:  ^.c.xmath $(MKTMP1)
        ${CC} ${DCFLAGS}  -o o.xmath ^.c.xmath >> $(MKTMP1)
d_01a:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"acos\"" -o o.acos ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_01b:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"asin\"" -o o.asin ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_01c:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"atan\"" -o o.atan ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_01d:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"atan2\"" -o o.atan2 ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_01e:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"ceil\"" -o o.ceil ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_01f:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"cos\"" -o o.cos ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_020:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"exp\"" -o o.exp ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_021:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"fabs\"" -o o.fabs ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_022:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"floor\"" -o o.floor ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_023:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"log\"" -o o.log ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_024:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"log10\"" -o o.log10 ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_025:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"modf\"" -o o.modf ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_026:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"pow\"" -o o.pow ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_027:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"sin\"" -o o.sin ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_028:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"sqrt\"" -o o.sqrt ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_029:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"tan\"" -o o.tan ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02a:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_edom_mh\"" -o o._edom_mh ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02b:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_ernge_0\"" -o o._ernge_0 ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02c:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_ernge_h\"" -o o._ernge_h ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02d:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_ernge_mh\"" -o o._ernge_mh ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02e:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_ldfp\"" -o o._ldfp ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_02f:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_set_errno\"" -o o._set_errno ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_030:  ^.s.mathf $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"_stfp\"" -o o._stfp ^.s.mathf
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_031:  ^.c.profile $(MKTMP1)
        ${CC} ${DCFLAGS} -D_fmapstore_c -o o._fmapstore ^.c.profile >> $(MKTMP1)
d_032:  ^.c.profile $(MKTMP1)
        ${CC} ${DCFLAGS} -D_mapstore_c -o o._mapstore ^.c.profile >> $(MKTMP1)
d_033:  ^.c.profile $(MKTMP1)
        ${CC} ${DCFLAGS} -D_map_store_c -o o._map_store ^.c.profile >> $(MKTMP1)
d_034:  ^.c.profile $(MKTMP1)
        ${CC} ${DCFLAGS} -D_write_pro_c -o o._write_pro ^.c.profile >> $(MKTMP1)
d_035:  ^.s.image $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o.image ^.s.image
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_036:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfprintf_c -o o.fprintf ^.c.printf >> $(MKTMP1)
d_037:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dprintf_c -o o.printf ^.c.printf >> $(MKTMP1)
d_038:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsprintf_c -o o.sprintf ^.c.printf >> $(MKTMP1)
d_039:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dvfprintf_c -o o.vfprintf ^.c.printf >> $(MKTMP1)
d_03a:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dvprintf_c -o o.vprintf ^.c.printf >> $(MKTMP1)
d_03b:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dvsprintf_c -o o.vsprintf ^.c.printf >> $(MKTMP1)
d_03c:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_fprintf_c -o o._fprintf ^.c.printf >> $(MKTMP1)
d_03d:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_fp_disp_c -o o._fp_disp ^.c.printf >> $(MKTMP1)
d_03e:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_nofp_disp_c -o o._nofp_disp ^.c.printf >> $(MKTMP1)
d_03f:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_printf_c -o o._printf ^.c.printf >> $(MKTMP1)
d_040:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_sprintf_c -o o._sprintf ^.c.printf >> $(MKTMP1)
d_041:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_vfprintf_c -o o._vfprintf ^.c.printf >> $(MKTMP1)
d_042:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_vsprintf_c -o o._vsprintf ^.c.printf >> $(MKTMP1)
d_043:  ^.c.printf $(MKTMP1)
        ${CC} ${DCFLAGS} -D__vfprintf_c -o o.__vfprintf ^.c.printf >> $(MKTMP1)
d_044:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_chval_c -o o._chval ^.c.scanf >> $(MKTMP1)
d_045:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_strtoul_c -o o._strtoul ^.c.scanf >> $(MKTMP1)
d_046:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrtod_c -o o.strtod ^.c.scanf >> $(MKTMP1)
d_047:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrtol_c -o o.strtol ^.c.scanf >> $(MKTMP1)
d_048:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrtoul_c -o o.strtoul ^.c.scanf >> $(MKTMP1)
d_049:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -D_scanf_c -o o._scanf ^.c.scanf >> $(MKTMP1)
d_04a:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dscanf_c -o o.scanf ^.c.scanf >> $(MKTMP1)
d_04b:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfscanf_c -o o.fscanf ^.c.scanf >> $(MKTMP1)
d_04c:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsscanf_c -o o.sscanf ^.c.scanf >> $(MKTMP1)
d_04d:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Datof_c -o o1.atof ^.c.scanf >> $(MKTMP1)
d_04e:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Datoi_c -o o1.atoi ^.c.scanf >> $(MKTMP1)
d_04f:  ^.c.scanf $(MKTMP1)
        ${CC} ${DCFLAGS} -Datol_c -o o1.atol ^.c.scanf >> $(MKTMP1)
d_050:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dbackspace_c -o o1.backspace ^.c.stdio >> $(MKTMP1)
d_051:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dclearerr_c -o o1.clearerr ^.c.stdio >> $(MKTMP1)
d_052:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfeof_c -o o1.feof ^.c.stdio >> $(MKTMP1)
d_053:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dferror_c -o o1.ferror ^.c.stdio >> $(MKTMP1)
d_054:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfflush_c -o o1.fflush ^.c.stdio >> $(MKTMP1)
d_055:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfgetc_c -o o1.fgetc ^.c.stdio >> $(MKTMP1)
d_056:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfgetpos_c -o o1.fgetpos ^.c.stdio >> $(MKTMP1)
d_057:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfgets_c -o o1.fgets ^.c.stdio >> $(MKTMP1)
d_058:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfisatty_c -o o1.fisatty ^.c.stdio >> $(MKTMP1)
d_059:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfilbuf_c -o o1.filbuf ^.c.stdio >> $(MKTMP1)
d_05a:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dflsbuf_c -o o1.flsbuf ^.c.stdio >> $(MKTMP1)
d_05b:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfputc_c -o o1.fputc ^.c.stdio >> $(MKTMP1)
d_05c:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfputs_c -o o1.fputs ^.c.stdio >> $(MKTMP1)
d_05d:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfread_c -o o1.fread ^.c.stdio >> $(MKTMP1)
d_05e:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfseek_c -o o1.fseek ^.c.stdio >> $(MKTMP1)
d_05f:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfsetpos_c -o o1.fsetpos ^.c.stdio >> $(MKTMP1)
d_060:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dftell_c -o o1.ftell ^.c.stdio >> $(MKTMP1)
d_061:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dfwrite_c -o o1.fwrite ^.c.stdio >> $(MKTMP1)
d_062:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dgetc_c -o o1.getc ^.c.stdio >> $(MKTMP1)
d_063:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dgetchar_c -o o1.getchar ^.c.stdio >> $(MKTMP1)
d_064:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dgets_c -o o1.gets ^.c.stdio >> $(MKTMP1)
d_065:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dosf_c -o o1.osf ^.c.stdio >> $(MKTMP1)
d_066:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dperror_c -o o1.perror ^.c.stdio >> $(MKTMP1)
d_067:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dputc_c -o o1.putc ^.c.stdio >> $(MKTMP1)
d_068:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dputchar_c -o o1.putchar ^.c.stdio >> $(MKTMP1)
d_069:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dputs_c -o o1.puts ^.c.stdio >> $(MKTMP1)
d_06a:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Drewind_c -o o1.rewind ^.c.stdio >> $(MKTMP1)
d_06b:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsetbuf_c -o o1.setbuf ^.c.stdio >> $(MKTMP1)
d_06c:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsetvbuf_c -o o1.setvbuf ^.c.stdio >> $(MKTMP1)
d_06d:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstdio_c -o o1.stdio ^.c.stdio >> $(MKTMP1)
d_06e:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dtmpnam_c -o o1.tmpnam ^.c.stdio >> $(MKTMP1)
d_06f:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -Dungetc_c -o o1.ungetc ^.c.stdio >> $(MKTMP1)
d_070:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -D_sys_msg_c -o o1._sys_msg ^.c.stdio >> $(MKTMP1)
d_071:  ^.c.stdio $(MKTMP1)
        ${CC} ${DCFLAGS} -D__dup_c -o o1.__dup ^.c.stdio >> $(MKTMP1)
d_072:  ^.c.alloc $(MKTMP1)
        ${CC} ${DCFLAGS} -Dalloc_c -o o1.alloc ^.c.alloc >> $(MKTMP1)
d_073:  ^.c.alloc $(MKTMP1)
        ${CC} ${DCFLAGS} -Dcalloc_c -o o1.calloc ^.c.alloc >> $(MKTMP1)
d_074:  ^.c.alloc $(MKTMP1)
        ${CC} ${DCFLAGS} -Drealloc_c -o o1.realloc ^.c.alloc >> $(MKTMP1)
d_075:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Drand_c -o o1.rand ^.c.stdlib >> $(MKTMP1)
d_076:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -DANSI_rand_c -o o1.ANSI_rand ^.c.stdlib >> $(MKTMP1)
d_077:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Datexit_c -o o1.atexit ^.c.stdlib >> $(MKTMP1)
d_078:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Dexit_c -o o1.exit ^.c.stdlib >> $(MKTMP1)
d_079:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Dabort_c -o o1.abort ^.c.stdlib >> $(MKTMP1)
d_07a:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Dqsort_c -o o1.qsort ^.c.stdlib >> $(MKTMP1)
d_07b:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -Dbsearch_c -o o1.bsearch ^.c.stdlib >> $(MKTMP1)
d_07c:  ^.c.stdlib $(MKTMP1)
        ${CC} ${DCFLAGS} -D_version_c -o o1._version ^.c.stdlib >> $(MKTMP1)
d_07d:  ^.c.signal $(MKTMP1)
        ${CC} ${DCFLAGS}  -o o1.signal ^.c.signal >> $(MKTMP1)
d_07e:  ^.c.assert $(MKTMP1)
        ${CC} ${DCFLAGS}  -o o1.assert ^.c.assert >> $(MKTMP1)
d_07f:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dlocale_c -o o1.locale ^.c.locale >> $(MKTMP1)
d_080:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dlocaleconv_c -o o1.localeconv ^.c.locale >> $(MKTMP1)
d_081:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -D_lc_c -o o1._lc ^.c.locale >> $(MKTMP1)
d_082:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmblen_c -o o1.mblen ^.c.locale >> $(MKTMP1)
d_083:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmbtowc_c -o o1.mbtowc ^.c.locale >> $(MKTMP1)
d_084:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dwctomb_c -o o1.wctomb ^.c.locale >> $(MKTMP1)
d_085:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmbstowcs_c -o o1.mbstowcs ^.c.locale >> $(MKTMP1)
d_086:  ^.c.locale $(MKTMP1)
        ${CC} ${DCFLAGS} -Dwcstombs_c -o o1.wcstombs ^.c.locale >> $(MKTMP1)
d_087:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmemchr_c -o o1.memchr ^.c.string >> $(MKTMP1)
d_088:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmemcmp_c -o o1.memcmp ^.c.string >> $(MKTMP1)
d_089:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrcat_c -o o1.strcat ^.c.string >> $(MKTMP1)
d_08a:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrchr_c -o o1.strchr ^.c.string >> $(MKTMP1)
d_08b:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrcmp_c -o o1.strcmp ^.c.string >> $(MKTMP1)
d_08c:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrcoll_c -o o1.strcoll ^.c.string >> $(MKTMP1)
d_08d:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrcpy_c -o o1.strcpy ^.c.string >> $(MKTMP1)
d_08e:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrcspn_c -o o1.strcspn ^.c.string >> $(MKTMP1)
d_08f:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrerror_c -o o1.strerror ^.c.string >> $(MKTMP1)
d_090:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrlen_c -o o1.strlen ^.c.string >> $(MKTMP1)
d_091:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrncat_c -o o1.strncat ^.c.string >> $(MKTMP1)
d_092:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrncmp_c -o o1.strncmp ^.c.string >> $(MKTMP1)
d_093:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrncpy_c -o o1.strncpy ^.c.string >> $(MKTMP1)
d_094:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrpbrk_c -o o1.strpbrk ^.c.string >> $(MKTMP1)
d_095:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrrchr_c -o o1.strrchr ^.c.string >> $(MKTMP1)
d_096:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrspn_c -o o1.strspn ^.c.string >> $(MKTMP1)
d_097:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrstr_c -o o1.strstr ^.c.string >> $(MKTMP1)
d_098:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrtok_c -o o1.strtok ^.c.string >> $(MKTMP1)
d_099:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrxfrm_c -o o1.strxfrm ^.c.string >> $(MKTMP1)
d_09a:  ^.c.string $(MKTMP1)
        ${CC} ${DCFLAGS} -D_strerror_c -o o2._strerror ^.c.string >> $(MKTMP1)
d_09b:  ^.s.memcpyset $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"memcpy\"" -o o2.memcpy ^.s.memcpyset
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_09c:  ^.s.memcpyset $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"memset\"" -o o2.memset ^.s.memcpyset
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_09d:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dasctime_c -o o2.asctime ^.c.time >> $(MKTMP1)
d_09e:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dctime_c -o o2.ctime ^.c.time >> $(MKTMP1)
d_09f:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Ddifftime_c -o o2.difftime ^.c.time >> $(MKTMP1)
d_0a0:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dgmtime_c -o o2.gmtime ^.c.time >> $(MKTMP1)
d_0a1:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dlocaltime_c -o o2.localtime ^.c.time >> $(MKTMP1)
d_0a2:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dmktime_c -o o2.mktime ^.c.time >> $(MKTMP1)
d_0a3:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -Dstrftime_c -o o2.strftime ^.c.time >> $(MKTMP1)
d_0a4:  ^.c.time $(MKTMP1)
        ${CC} ${DCFLAGS} -D_monlen_c -o o2._monlen ^.c.time >> $(MKTMP1)
d_0a5:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"count\"" -o o2.rt_count ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0a6:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"count1\"" -o o2.rt_count1 ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0a7:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"divtest\"" -o o2.rt_divtest ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0a8:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"sdiv10\"" -o o2.rt_sdiv10 ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0a9:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"udiv10\"" -o o2.rt_udiv10 ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0aa:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"sdiv_unrolled8\"" -o o2.rt_sdiv ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0ab:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"udiv_unrolled8\"" -o o2.rt_udiv ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0ac:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"div0\"" -o o2.rt_div0 ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0ad:  ^.s.unhosted $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"rwcheck\"" -o o2.rt_rwcheck ^.s.unhosted
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0ae:  ^.s.setjmp $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.setjmp ^.s.setjmp
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0af:  ^.s.stkchunk $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"stkchunks\"" -o o2.stkchunks ^.s.stkchunk
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0b0:  ^.s.stkchunk $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"chunk_copy\"" -o o2.chunk_copy ^.s.stkchunk
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0b1:  ^.s.unwind $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.unwind ^.s.unwind
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0b2:  ^.s.reg_alloc $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.reg_alloc ^.s.reg_alloc
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0b3:  ^.c.armsys $(MKTMP1)
        ${CC} ${DCFLAGS} -Dlib_init_c -o o2.lib_init ^.c.armsys >> $(MKTMP1)
d_0b4:  ^.c.armsys $(MKTMP1)
        ${CC} ${DCFLAGS} -D_sysdie_c -o o2._sysdie ^.c.armsys >> $(MKTMP1)
d_0b5:  ^.c.armsys $(MKTMP1)
        ${CC} ${DCFLAGS} -D_sys_alloc_c -o o2._sys_alloc ^.c.armsys >> $(MKTMP1)
d_0b6:  ^.c.armsys $(MKTMP1)
        ${CC} ${DCFLAGS} -D_main_c -DSTDFILE_REDIRECTION -o o2._main ^.c.armsys >> $(MKTMP1)
d_0b7:  ^.s.handlers $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "backtrace_enabled SETL {FALSE}" -o o2.handlers ^.s.handlers
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0b8:  c.hosterror $(MKTMP1)
        ${CC} ${DCFLAGS}  -o o2.hosterror c.hosterror >> $(MKTMP1)
d_0b9:  c.hostsignal $(MKTMP1)
        ${CC} ${DCFLAGS}  -o o2.hostsignal c.hostsignal >> $(MKTMP1)
d_0ba:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dremove_c -o o2.remove c.iosuppt >> $(MKTMP1)
d_0bb:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Drename_c -o o2.rename c.iosuppt >> $(MKTMP1)
d_0bc:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_open_c -o o2._sys_open c.iosuppt >> $(MKTMP1)
d_0bd:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dttyread_c -o o2._ttyread c.iosuppt >> $(MKTMP1)
d_0be:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dttywrite_c -o o2._ttywrite c.iosuppt >> $(MKTMP1)
d_0bf:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_istty_c -o o2.sys_istty c.iosuppt >> $(MKTMP1)
d_0c0:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_seek_c -o o2.sys_seek c.iosuppt >> $(MKTMP1)
d_0c1:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_flen_c -o o2.sys_flen c.iosuppt >> $(MKTMP1)
d_0c2:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_iserror_c -o o2.sys_iserr c.iosuppt >> $(MKTMP1)
d_0c3:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_write_c -o o2.sys_write c.iosuppt >> $(MKTMP1)
d_0c4:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_read_c -o o2.sys_read c.iosuppt >> $(MKTMP1)
d_0c5:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_ensure_c -o o2.sys_ensure c.iosuppt >> $(MKTMP1)
d_0c6:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsys_close_c -o o2.sys_close c.iosuppt >> $(MKTMP1)
d_0c7:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dclock_c -o o2.clock c.iosuppt >> $(MKTMP1)
d_0c8:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dtime_c -o o2.time c.iosuppt >> $(MKTMP1)
d_0c9:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dsystem_c -o o2.system c.iosuppt >> $(MKTMP1)
d_0ca:  c.iosuppt $(MKTMP1)
        ${CC} ${DCFLAGS} -Dgetenv_c -o o2.getenv c.iosuppt >> $(MKTMP1)
d_0cb:  s.overmgr $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.overmgr s.overmgr
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0cc:  s._tmpnam $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2._tmpnam s._tmpnam
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0cd:  s.kernel $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.kernel s.kernel
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0ce:  s.k_escseen $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.k_escseen s.k_escseen
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0cf:  s.k_system $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.k_system s.k_system
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d0:  s.traphandle $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.traphandle s.traphandle
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d1:  s.heapsuppt $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.heapsuppt s.heapsuppt
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d2:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osargs\"" -o o2.osargs s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d3:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osbget\"" -o o2.osbget s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d4:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osbput\"" -o o2.osbput s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d5:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osbyte\"" -o o2.osbyte s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d6:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"oscli\"" -o o2.oscli s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d7:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osfile\"" -o o2.osfile s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d8:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osfind\"" -o o2.osfind s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0d9:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osgbpb\"" -o o2.osgbpb s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0da:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osrdch\"" -o o2.osrdch s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0db:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"osword\"" -o o2.osword s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0dc:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"oswrch\"" -o o2.oswrch s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0dd:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"swi\"" -o o2.swi s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0de:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"k_getenv\"" -o o2.k_getenv s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0df:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"k_setenv\"" -o o2.k_setenv s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0e0:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"k_hostos\"" -o o2.k_hostos s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0e1:  s.osveneer $(MKTMP1)
        ${AS} ${DAFLAGS} -PD "make SETS \"lasterr\"" -o o2.lasterr s.osveneer
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)
d_0e2:  s.infpe $(MKTMP1)
        ${AS} ${DAFLAGS}  -o o2.infpe s.infpe
        ${ECHO} <$(MKTMP2) >>$(MKTMP1)

# THE ABOVE BLANK LINE IS VITAL - DO NOT DELETE IT
# TO MAKE MakeProto and Makefile DELETE EVERYTHING FOLLOWING THIS COMMENT,
# SAVE THE REMAINDER AS MakeProto, THEN RUN amu depend.
o2.infpe: s.infpe s.h_brazil
o2.lasterr: s.osveneer s.h_brazil ^.s.h_la_obj
o2.k_hostos: s.osveneer s.h_brazil ^.s.h_la_obj
o2.k_setenv: s.osveneer s.h_brazil ^.s.h_la_obj
o2.k_getenv: s.osveneer s.h_brazil ^.s.h_la_obj
o2.swi: s.osveneer s.h_brazil ^.s.h_la_obj
o2.oswrch: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osword: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osrdch: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osgbpb: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osfind: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osfile: s.osveneer s.h_brazil ^.s.h_la_obj
o2.oscli: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osbyte: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osbput: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osbget: s.osveneer s.h_brazil ^.s.h_la_obj
o2.osargs: s.osveneer s.h_brazil ^.s.h_la_obj
o2.heapsuppt: s.heapsuppt s.h_brazil ^.s.h_objmacro ^.s.h_la_obj
o2.traphandle: s.traphandle ^.s.h_objmacro ^.s.h_la_obj s.h_brazil ^.s.h_errors ^.s.h_hw
o2.k_system: s.k_system s.h_brazil ^.s.h_objmacro ^.s.h_la_obj
o2.k_escseen: s.k_escseen s.h_brazil ^.s.h_la_obj ^.s.h_hw
o2.kernel: s.kernel s.h_brazil ^.s.h_objmacro ^.s.h_la_obj ^.s.h_errors ^.s.h_uwb ^.s.h_stack ^.s.h_hw
o2._tmpnam: s._tmpnam s.h_brazil
o2.overmgr: s.overmgr ^.s.h_hw
o2.getenv:      c.iosuppt
o2.getenv:      h.hostsys
o2.getenv:      ^.stdh.h.stdio
o2.getenv:      ^.stdh.h.time
o2.getenv:      h.riscos
o2.getenv:      ^.stdh.h.stdlib
o2.getenv:      ^.h.internals
o2.getenv:      ^.stdh.h.stddef
o2.getenv:      ^.stdh.h.stdio
o2.getenv:      ^.stdh.h.stdarg
o2.getenv:      ^.stdh.h.locale
o2.getenv:      h.hostsys
o2.getenv:      ^.h.rt
o2.system:      c.iosuppt
o2.system:      h.hostsys
o2.system:      ^.stdh.h.stdio
o2.system:      ^.stdh.h.time
o2.system:      h.riscos
o2.system:      ^.stdh.h.stdio
o2.system:      ^.stdh.h.stdlib
o2.system:      ^.h.internals
o2.system:      ^.stdh.h.stddef
o2.system:      ^.stdh.h.stdio
o2.system:      ^.stdh.h.stdarg
o2.system:      ^.stdh.h.locale
o2.system:      h.hostsys
o2.system:      ^.h.rt
o2.time:        c.iosuppt
o2.time:        h.hostsys
o2.time:        ^.stdh.h.stdio
o2.time:        ^.stdh.h.time
o2.time:        h.riscos
o2.clock:       c.iosuppt
o2.clock:       h.hostsys
o2.clock:       ^.stdh.h.stdio
o2.clock:       ^.stdh.h.time
o2.clock:       h.riscos
o2.sys_close:   c.iosuppt
o2.sys_close:   h.hostsys
o2.sys_close:   ^.stdh.h.stdio
o2.sys_close:   ^.stdh.h.time
o2.sys_close:   h.riscos
o2.sys_ensure:  c.iosuppt
o2.sys_ensure:  h.hostsys
o2.sys_ensure:  ^.stdh.h.stdio
o2.sys_ensure:  ^.stdh.h.time
o2.sys_ensure:  h.riscos
o2.sys_read:    c.iosuppt
o2.sys_read:    h.hostsys
o2.sys_read:    ^.stdh.h.stdio
o2.sys_read:    ^.stdh.h.time
o2.sys_read:    h.riscos
o2.sys_write:   c.iosuppt
o2.sys_write:   h.hostsys
o2.sys_write:   ^.stdh.h.stdio
o2.sys_write:   ^.stdh.h.time
o2.sys_write:   h.riscos
o2.sys_iserr:   c.iosuppt
o2.sys_iserr:   h.hostsys
o2.sys_iserr:   ^.stdh.h.stdio
o2.sys_iserr:   ^.stdh.h.time
o2.sys_iserr:   h.riscos
o2.sys_flen:    c.iosuppt
o2.sys_flen:    h.hostsys
o2.sys_flen:    ^.stdh.h.stdio
o2.sys_flen:    ^.stdh.h.time
o2.sys_flen:    h.riscos
o2.sys_seek:    c.iosuppt
o2.sys_seek:    h.hostsys
o2.sys_seek:    ^.stdh.h.stdio
o2.sys_seek:    ^.stdh.h.time
o2.sys_seek:    h.riscos
o2.sys_istty:   c.iosuppt
o2.sys_istty:   h.hostsys
o2.sys_istty:   ^.stdh.h.stdio
o2.sys_istty:   ^.stdh.h.time
o2.sys_istty:   h.riscos
o2.sys_istty:   ^.h.ioguts
o2._ttywrite:   c.iosuppt
o2._ttywrite:   h.hostsys
o2._ttywrite:   ^.stdh.h.stdio
o2._ttywrite:   ^.stdh.h.time
o2._ttywrite:   h.riscos
o2._ttywrite:   ^.stdh.h.ctype
o2._ttywrite:   ^.h.ioguts
o2._ttyread:    c.iosuppt
o2._ttyread:    h.hostsys
o2._ttyread:    ^.stdh.h.stdio
o2._ttyread:    ^.stdh.h.time
o2._ttyread:    h.riscos
o2._ttyread:    ^.stdh.h.stdio
o2._ttyread:    ^.h.ioguts
o2._ttyread:    ^.h.internals
o2._ttyread:    ^.stdh.h.stddef
o2._ttyread:    ^.stdh.h.stdio
o2._ttyread:    ^.stdh.h.stdarg
o2._ttyread:    ^.stdh.h.locale
o2._ttyread:    h.hostsys
o2._ttyread:    ^.h.rt
o2._sys_open:   c.iosuppt
o2._sys_open:   h.hostsys
o2._sys_open:   ^.stdh.h.stdio
o2._sys_open:   ^.stdh.h.time
o2._sys_open:   h.riscos
o2._sys_open:   ^.stdh.h.errno
o2.rename:      c.iosuppt
o2.rename:      h.hostsys
o2.rename:      ^.stdh.h.stdio
o2.rename:      ^.stdh.h.time
o2.rename:      h.riscos
o2.remove:      c.iosuppt
o2.remove:      h.hostsys
o2.remove:      ^.stdh.h.stdio
o2.remove:      ^.stdh.h.time
o2.remove:      h.riscos
o2.hostsignal:  c.hostsignal
o2.hostsignal:  ^.stdh.h.stddef
o2.hostsignal:  ^.stdh.h.signal
o2.hostsignal:  h.riscos
o2.hostsignal:  h.hostsys
o2.hostsignal:  ^.stdh.h.stdio
o2.hostsignal:  ^.stdh.h.time
o2.hosterror:   c.hosterror
o2.hosterror:   ^.stdh.h.stddef
o2.hosterror:   h.riscos
o2.hosterror:   h.hostsys
o2.hosterror:   ^.stdh.h.stdio
o2.hosterror:   ^.stdh.h.time
o2.handlers: ^.s.handlers ^.s.h_la_obj ^.s.h_errors
o2._main:       ^.c.armsys
o2._main:       ^.stdh.h.stddef
o2._main:       h.hostsys
o2._main:       ^.stdh.h.stdio
o2._main:       ^.stdh.h.time
o2._main:       ^.h.internals
o2._main:       ^.stdh.h.stddef
o2._main:       ^.stdh.h.stdio
o2._main:       ^.stdh.h.stdarg
o2._main:       ^.stdh.h.locale
o2._main:       h.hostsys
o2._main:       ^.h.rt
o2._main:       ^.stdh.h.stdio
o2._main:       ^.stdh.h.stdlib
o2._main:       ^.stdh.h.ctype
o2._main:       ^.h.ioguts
o2._main:       ^.h.externals
o2._main:       ^.stdh.h.stdio
o2._main:       ^.stdh.h.stdarg
o2._sys_alloc:  ^.c.armsys
o2._sys_alloc:  ^.stdh.h.stddef
o2._sys_alloc:  h.hostsys
o2._sys_alloc:  ^.stdh.h.stdio
o2._sys_alloc:  ^.stdh.h.time
o2._sys_alloc:  ^.h.internals
o2._sys_alloc:  ^.stdh.h.stddef
o2._sys_alloc:  ^.stdh.h.stdio
o2._sys_alloc:  ^.stdh.h.stdarg
o2._sys_alloc:  ^.stdh.h.locale
o2._sys_alloc:  h.hostsys
o2._sys_alloc:  ^.h.rt
o2._sysdie:     ^.c.armsys
o2._sysdie:     ^.stdh.h.stddef
o2._sysdie:     h.hostsys
o2._sysdie:     ^.stdh.h.stdio
o2._sysdie:     ^.stdh.h.time
o2._sysdie:     ^.h.internals
o2._sysdie:     ^.stdh.h.stddef
o2._sysdie:     ^.stdh.h.stdio
o2._sysdie:     ^.stdh.h.stdarg
o2._sysdie:     ^.stdh.h.locale
o2._sysdie:     h.hostsys
o2._sysdie:     ^.h.rt
o2.lib_init:    ^.c.armsys
o2.lib_init:    ^.stdh.h.stddef
o2.lib_init:    h.hostsys
o2.lib_init:    ^.stdh.h.stdio
o2.lib_init:    ^.stdh.h.time
o2.lib_init:    ^.h.internals
o2.lib_init:    ^.stdh.h.stddef
o2.lib_init:    ^.stdh.h.stdio
o2.lib_init:    ^.stdh.h.stdarg
o2.lib_init:    ^.stdh.h.locale
o2.lib_init:    h.hostsys
o2.lib_init:    ^.h.rt
o2.reg_alloc: ^.s.reg_alloc ^.s.h_la_obj
o2.unwind: ^.s.unwind ^.s.h_uwb ^.s.h_stack ^.s.h_hw ^.s.h_la_obj
o2.chunk_copy: ^.s.stkchunk ^.s.h_objmacro ^.s.h_la_obj ^.s.h_stack ^.s.h_hw ^.s.h_errors
o2.stkchunks: ^.s.stkchunk ^.s.h_objmacro ^.s.h_la_obj ^.s.h_stack ^.s.h_hw ^.s.h_errors
o2.setjmp: ^.s.setjmp ^.s.h_la_obj ^.s.h_stack ^.s.h_signal
o2.rt_rwcheck: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_div0: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_udiv: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_sdiv: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_udiv10: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_sdiv10: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_divtest: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_count1: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2.rt_count: ^.s.unhosted ^.s.h_la_obj ^.s.h_errors
o2._monlen:     ^.c.time
o2._monlen:     ^.stdh.h.time
o2._monlen:     ^.stdh.h.stdio
o2._monlen:     ^.stdh.h.string
o2.strftime:    ^.c.time
o2.strftime:    ^.stdh.h.time
o2.strftime:    ^.stdh.h.stdio
o2.strftime:    ^.stdh.h.string
o2.mktime:      ^.c.time
o2.mktime:      ^.stdh.h.time
o2.mktime:      ^.stdh.h.stdio
o2.mktime:      ^.stdh.h.string
o2.localtime:   ^.c.time
o2.localtime:   ^.stdh.h.time
o2.localtime:   ^.stdh.h.stdio
o2.localtime:   ^.stdh.h.string
o2.gmtime:      ^.c.time
o2.gmtime:      ^.stdh.h.time
o2.gmtime:      ^.stdh.h.stdio
o2.gmtime:      ^.stdh.h.string
o2.difftime:    ^.c.time
o2.difftime:    ^.stdh.h.time
o2.difftime:    ^.stdh.h.stdio
o2.difftime:    ^.stdh.h.string
o2.ctime:       ^.c.time
o2.ctime:       ^.stdh.h.time
o2.ctime:       ^.stdh.h.stdio
o2.ctime:       ^.stdh.h.string
o2.asctime:     ^.c.time
o2.asctime:     ^.stdh.h.time
o2.asctime:     ^.stdh.h.stdio
o2.asctime:     ^.stdh.h.string
o2.memset: ^.s.memcpyset
o2.memcpy: ^.s.memcpyset
o2._strerror:   ^.c.string
o2._strerror:   h.config
o2._strerror:   ^.stdh.h.stddef
o2._strerror:   ^.stdh.h.string
o2._strerror:   ^.h.internals
o2._strerror:   ^.stdh.h.stddef
o2._strerror:   ^.stdh.h.stdio
o2._strerror:   ^.stdh.h.stdarg
o2._strerror:   ^.stdh.h.locale
o2._strerror:   h.hostsys
o2._strerror:   ^.stdh.h.stdio
o2._strerror:   ^.stdh.h.time
o2._strerror:   ^.h.rt
o2._strerror:   ^.stdh.h.errno
o2._strerror:   ^.stdh.h.stdio
o1.strxfrm:     ^.c.string
o1.strxfrm:     h.config
o1.strxfrm:     ^.stdh.h.stddef
o1.strxfrm:     ^.stdh.h.string
o1.strtok:      ^.c.string
o1.strtok:      h.config
o1.strtok:      ^.stdh.h.stddef
o1.strtok:      ^.stdh.h.string
o1.strstr:      ^.c.string
o1.strstr:      h.config
o1.strstr:      ^.stdh.h.stddef
o1.strstr:      ^.stdh.h.string
o1.strspn:      ^.c.string
o1.strspn:      h.config
o1.strspn:      ^.stdh.h.stddef
o1.strspn:      ^.stdh.h.string
o1.strrchr:     ^.c.string
o1.strrchr:     h.config
o1.strrchr:     ^.stdh.h.stddef
o1.strrchr:     ^.stdh.h.string
o1.strpbrk:     ^.c.string
o1.strpbrk:     h.config
o1.strpbrk:     ^.stdh.h.stddef
o1.strpbrk:     ^.stdh.h.string
o1.strncpy:     ^.c.string
o1.strncpy:     h.config
o1.strncpy:     ^.stdh.h.stddef
o1.strncpy:     ^.stdh.h.string
o1.strncmp:     ^.c.string
o1.strncmp:     h.config
o1.strncmp:     ^.stdh.h.stddef
o1.strncmp:     ^.stdh.h.string
o1.strncat:     ^.c.string
o1.strncat:     h.config
o1.strncat:     ^.stdh.h.stddef
o1.strncat:     ^.stdh.h.string
o1.strlen:      ^.c.string
o1.strlen:      h.config
o1.strlen:      ^.stdh.h.stddef
o1.strlen:      ^.stdh.h.string
o1.strerror:    ^.c.string
o1.strerror:    h.config
o1.strerror:    ^.stdh.h.stddef
o1.strerror:    ^.stdh.h.string
o1.strerror:    ^.h.internals
o1.strerror:    ^.stdh.h.stddef
o1.strerror:    ^.stdh.h.stdio
o1.strerror:    ^.stdh.h.stdarg
o1.strerror:    ^.stdh.h.locale
o1.strerror:    h.hostsys
o1.strerror:    ^.stdh.h.stdio
o1.strerror:    ^.stdh.h.time
o1.strerror:    ^.h.rt
o1.strcspn:     ^.c.string
o1.strcspn:     h.config
o1.strcspn:     ^.stdh.h.stddef
o1.strcspn:     ^.stdh.h.string
o1.strcpy:      ^.c.string
o1.strcpy:      h.config
o1.strcpy:      ^.stdh.h.stddef
o1.strcpy:      ^.stdh.h.string
o1.strcoll:     ^.c.string
o1.strcoll:     h.config
o1.strcoll:     ^.stdh.h.stddef
o1.strcoll:     ^.stdh.h.string
o1.strcoll:     ^.h.internals
o1.strcoll:     ^.stdh.h.stddef
o1.strcoll:     ^.stdh.h.stdio
o1.strcoll:     ^.stdh.h.stdarg
o1.strcoll:     ^.stdh.h.locale
o1.strcoll:     h.hostsys
o1.strcoll:     ^.stdh.h.stdio
o1.strcoll:     ^.stdh.h.time
o1.strcoll:     ^.h.rt
o1.strcmp:      ^.c.string
o1.strcmp:      h.config
o1.strcmp:      ^.stdh.h.stddef
o1.strcmp:      ^.stdh.h.string
o1.strchr:      ^.c.string
o1.strchr:      h.config
o1.strchr:      ^.stdh.h.stddef
o1.strchr:      ^.stdh.h.string
o1.strcat:      ^.c.string
o1.strcat:      h.config
o1.strcat:      ^.stdh.h.stddef
o1.strcat:      ^.stdh.h.string
o1.memcmp:      ^.c.string
o1.memcmp:      h.config
o1.memcmp:      ^.stdh.h.stddef
o1.memcmp:      ^.stdh.h.string
o1.memchr:      ^.c.string
o1.memchr:      h.config
o1.memchr:      ^.stdh.h.stddef
o1.memchr:      ^.stdh.h.string
o1.wcstombs:    ^.c.locale
o1.wcstombs:    ^.stdh.h.locale
o1.wcstombs:    ^.stdh.h.stddef
o1.wcstombs:    ^.stdh.h.stdio
o1.wcstombs:    ^.stdh.h.time
o1.wcstombs:    ^.stdh.h.string
o1.wcstombs:    ^.stdh.h.stdlib
o1.wcstombs:    ^.stdh.h.limits
o1.wcstombs:    ^.h.internals
o1.wcstombs:    ^.stdh.h.stddef
o1.wcstombs:    ^.stdh.h.stdio
o1.wcstombs:    ^.stdh.h.stdarg
o1.wcstombs:    ^.stdh.h.locale
o1.wcstombs:    h.hostsys
o1.wcstombs:    ^.stdh.h.stdio
o1.wcstombs:    ^.stdh.h.time
o1.wcstombs:    ^.h.rt
o1.mbstowcs:    ^.c.locale
o1.mbstowcs:    ^.stdh.h.locale
o1.mbstowcs:    ^.stdh.h.stddef
o1.mbstowcs:    ^.stdh.h.stdio
o1.mbstowcs:    ^.stdh.h.time
o1.mbstowcs:    ^.stdh.h.string
o1.mbstowcs:    ^.stdh.h.stdlib
o1.mbstowcs:    ^.stdh.h.limits
o1.mbstowcs:    ^.h.internals
o1.mbstowcs:    ^.stdh.h.stddef
o1.mbstowcs:    ^.stdh.h.stdio
o1.mbstowcs:    ^.stdh.h.stdarg
o1.mbstowcs:    ^.stdh.h.locale
o1.mbstowcs:    h.hostsys
o1.mbstowcs:    ^.stdh.h.stdio
o1.mbstowcs:    ^.stdh.h.time
o1.mbstowcs:    ^.h.rt
o1.wctomb:      ^.c.locale
o1.wctomb:      ^.stdh.h.locale
o1.wctomb:      ^.stdh.h.stddef
o1.wctomb:      ^.stdh.h.stdio
o1.wctomb:      ^.stdh.h.time
o1.wctomb:      ^.stdh.h.string
o1.wctomb:      ^.stdh.h.stdlib
o1.wctomb:      ^.stdh.h.limits
o1.wctomb:      ^.h.internals
o1.wctomb:      ^.stdh.h.stddef
o1.wctomb:      ^.stdh.h.stdio
o1.wctomb:      ^.stdh.h.stdarg
o1.wctomb:      ^.stdh.h.locale
o1.wctomb:      h.hostsys
o1.wctomb:      ^.stdh.h.stdio
o1.wctomb:      ^.stdh.h.time
o1.wctomb:      ^.h.rt
o1.mbtowc:      ^.c.locale
o1.mbtowc:      ^.stdh.h.locale
o1.mbtowc:      ^.stdh.h.stddef
o1.mbtowc:      ^.stdh.h.stdio
o1.mbtowc:      ^.stdh.h.time
o1.mbtowc:      ^.stdh.h.string
o1.mbtowc:      ^.stdh.h.stdlib
o1.mbtowc:      ^.stdh.h.limits
o1.mbtowc:      ^.h.internals
o1.mbtowc:      ^.stdh.h.stddef
o1.mbtowc:      ^.stdh.h.stdio
o1.mbtowc:      ^.stdh.h.stdarg
o1.mbtowc:      ^.stdh.h.locale
o1.mbtowc:      h.hostsys
o1.mbtowc:      ^.stdh.h.stdio
o1.mbtowc:      ^.stdh.h.time
o1.mbtowc:      ^.h.rt
o1.mblen:       ^.c.locale
o1.mblen:       ^.stdh.h.locale
o1.mblen:       ^.stdh.h.stddef
o1.mblen:       ^.stdh.h.stdio
o1.mblen:       ^.stdh.h.time
o1.mblen:       ^.stdh.h.string
o1.mblen:       ^.stdh.h.stdlib
o1.mblen:       ^.stdh.h.limits
o1.mblen:       ^.h.internals
o1.mblen:       ^.stdh.h.stddef
o1.mblen:       ^.stdh.h.stdio
o1.mblen:       ^.stdh.h.stdarg
o1.mblen:       ^.stdh.h.locale
o1.mblen:       h.hostsys
o1.mblen:       ^.stdh.h.stdio
o1.mblen:       ^.stdh.h.time
o1.mblen:       ^.h.rt
o1._lc: ^.c.locale
o1._lc: ^.stdh.h.locale
o1._lc: ^.stdh.h.stddef
o1._lc: ^.stdh.h.stdio
o1._lc: ^.stdh.h.time
o1._lc: ^.stdh.h.string
o1._lc: ^.stdh.h.stdlib
o1._lc: ^.stdh.h.limits
o1._lc: ^.h.internals
o1._lc: ^.stdh.h.stddef
o1._lc: ^.stdh.h.stdio
o1._lc: ^.stdh.h.stdarg
o1._lc: ^.stdh.h.locale
o1._lc: h.hostsys
o1._lc: ^.stdh.h.stdio
o1._lc: ^.stdh.h.time
o1._lc: ^.h.rt
o1.localeconv:  ^.c.locale
o1.localeconv:  ^.stdh.h.locale
o1.localeconv:  ^.stdh.h.stddef
o1.localeconv:  ^.stdh.h.stdio
o1.localeconv:  ^.stdh.h.time
o1.localeconv:  ^.stdh.h.string
o1.localeconv:  ^.stdh.h.stdlib
o1.localeconv:  ^.stdh.h.limits
o1.localeconv:  ^.h.internals
o1.localeconv:  ^.stdh.h.stddef
o1.localeconv:  ^.stdh.h.stdio
o1.localeconv:  ^.stdh.h.stdarg
o1.localeconv:  ^.stdh.h.locale
o1.localeconv:  h.hostsys
o1.localeconv:  ^.stdh.h.stdio
o1.localeconv:  ^.stdh.h.time
o1.localeconv:  ^.h.rt
o1.locale:      ^.c.locale
o1.locale:      ^.stdh.h.locale
o1.locale:      ^.stdh.h.stddef
o1.locale:      ^.stdh.h.stdio
o1.locale:      ^.stdh.h.time
o1.locale:      ^.stdh.h.string
o1.locale:      ^.stdh.h.stdlib
o1.locale:      ^.stdh.h.limits
o1.locale:      ^.h.internals
o1.locale:      ^.stdh.h.stddef
o1.locale:      ^.stdh.h.stdio
o1.locale:      ^.stdh.h.stdarg
o1.locale:      ^.stdh.h.locale
o1.locale:      h.hostsys
o1.locale:      ^.stdh.h.stdio
o1.locale:      ^.stdh.h.time
o1.locale:      ^.h.rt
o1.assert:      ^.c.assert
o1.assert:      ^.stdh.h.stdio
o1.assert:      ^.stdh.h.assert
o1.assert:      ^.stdh.h.stdlib
o1.signal:      ^.c.signal
o1.signal:      h.hostsys
o1.signal:      ^.stdh.h.stdio
o1.signal:      ^.stdh.h.time
o1.signal:      ^.stdh.h.signal
o1.signal:      ^.stdh.h.stdio
o1.signal:      ^.stdh.h.stddef
o1.signal:      ^.stdh.h.errno
o1.signal:      ^.stdh.h.string
o1.signal:      ^.h.internals
o1.signal:      ^.stdh.h.stddef
o1.signal:      ^.stdh.h.stdio
o1.signal:      ^.stdh.h.stdarg
o1.signal:      ^.stdh.h.locale
o1.signal:      h.hostsys
o1.signal:      ^.h.rt
o1._version:    ^.c.stdlib
o1._version:    ^.stdh.h.stdlib
o1._version:    ^.stdh.h.stddef
o1._version:    ^.h.externals
o1._version:    ^.stdh.h.stdio
o1._version:    ^.stdh.h.stdarg
o1._version:    ^.h.version
o1.bsearch:     ^.c.stdlib
o1.bsearch:     ^.stdh.h.stdlib
o1.bsearch:     ^.stdh.h.stddef
o1.qsort:       ^.c.stdlib
o1.qsort:       ^.stdh.h.stdlib
o1.qsort:       ^.stdh.h.stddef
o1.abort:       ^.c.stdlib
o1.abort:       ^.stdh.h.stdlib
o1.abort:       ^.stdh.h.stddef
o1.abort:       ^.stdh.h.signal
o1.abort:       ^.h.internals
o1.abort:       ^.stdh.h.stddef
o1.abort:       ^.stdh.h.stdio
o1.abort:       ^.stdh.h.stdarg
o1.abort:       ^.stdh.h.locale
o1.abort:       h.hostsys
o1.abort:       ^.stdh.h.stdio
o1.abort:       ^.stdh.h.time
o1.abort:       ^.h.rt
o1.exit:        ^.c.stdlib
o1.exit:        ^.stdh.h.stdlib
o1.exit:        ^.stdh.h.stddef
o1.exit:        ^.h.internals
o1.exit:        ^.stdh.h.stddef
o1.exit:        ^.stdh.h.stdio
o1.exit:        ^.stdh.h.stdarg
o1.exit:        ^.stdh.h.locale
o1.exit:        h.hostsys
o1.exit:        ^.stdh.h.stdio
o1.exit:        ^.stdh.h.time
o1.exit:        ^.h.rt
o1.atexit:      ^.c.stdlib
o1.atexit:      ^.stdh.h.stdlib
o1.atexit:      ^.stdh.h.stddef
o1.atexit:      ^.h.internals
o1.atexit:      ^.stdh.h.stddef
o1.atexit:      ^.stdh.h.stdio
o1.atexit:      ^.stdh.h.stdarg
o1.atexit:      ^.stdh.h.locale
o1.atexit:      h.hostsys
o1.atexit:      ^.stdh.h.stdio
o1.atexit:      ^.stdh.h.time
o1.atexit:      ^.h.rt
o1.ANSI_rand:   ^.c.stdlib
o1.ANSI_rand:   ^.stdh.h.stdlib
o1.ANSI_rand:   ^.stdh.h.stddef
o1.rand:        ^.c.stdlib
o1.rand:        ^.stdh.h.stdlib
o1.rand:        ^.stdh.h.stddef
o1.realloc:     ^.c.alloc
o1.realloc:     ^.stdh.h.stdlib
o1.realloc:     ^.stdh.h.stddef
o1.realloc:     ^.stdh.h.string
o1.realloc:     ^.h.alloc
o1.realloc:     ^.stdh.h.stddef
o1.realloc:     ^.h.rt
o1.realloc:     ^.h.externals
o1.realloc:     ^.stdh.h.stdio
o1.realloc:     ^.stdh.h.stdarg
o1.realloc:     ^.h.internals
o1.realloc:     ^.stdh.h.stddef
o1.realloc:     ^.stdh.h.stdio
o1.realloc:     ^.stdh.h.stdarg
o1.realloc:     ^.stdh.h.locale
o1.realloc:     h.hostsys
o1.realloc:     ^.stdh.h.stdio
o1.realloc:     ^.stdh.h.time
o1.realloc:     ^.h.rt
o1.calloc:      ^.c.alloc
o1.calloc:      ^.stdh.h.stdlib
o1.calloc:      ^.stdh.h.stddef
o1.calloc:      ^.stdh.h.string
o1.calloc:      ^.h.alloc
o1.calloc:      ^.stdh.h.stddef
o1.calloc:      ^.h.rt
o1.calloc:      ^.h.externals
o1.calloc:      ^.stdh.h.stdio
o1.calloc:      ^.stdh.h.stdarg
o1.calloc:      ^.h.internals
o1.calloc:      ^.stdh.h.stddef
o1.calloc:      ^.stdh.h.stdio
o1.calloc:      ^.stdh.h.stdarg
o1.calloc:      ^.stdh.h.locale
o1.calloc:      h.hostsys
o1.calloc:      ^.stdh.h.stdio
o1.calloc:      ^.stdh.h.time
o1.calloc:      ^.h.rt
o1.alloc:       ^.c.alloc
o1.alloc:       ^.stdh.h.stdlib
o1.alloc:       ^.stdh.h.stddef
o1.alloc:       ^.stdh.h.string
o1.alloc:       ^.h.alloc
o1.alloc:       ^.stdh.h.stddef
o1.alloc:       ^.h.rt
o1.alloc:       ^.h.externals
o1.alloc:       ^.stdh.h.stdio
o1.alloc:       ^.stdh.h.stdarg
o1.alloc:       ^.h.internals
o1.alloc:       ^.stdh.h.stddef
o1.alloc:       ^.stdh.h.stdio
o1.alloc:       ^.stdh.h.stdarg
o1.alloc:       ^.stdh.h.locale
o1.alloc:       h.hostsys
o1.alloc:       ^.stdh.h.stdio
o1.alloc:       ^.stdh.h.time
o1.alloc:       ^.h.rt
o1.__dup:       ^.c.stdio
o1.__dup:       h.hostsys
o1.__dup:       ^.stdh.h.stdio
o1.__dup:       ^.stdh.h.time
o1.__dup:       ^.stdh.h.stdio
o1.__dup:       ^.stdh.h.string
o1.__dup:       ^.stdh.h.stdlib
o1.__dup:       ^.stdh.h.errno
o1.__dup:       ^.h.ioguts
o1.__dup:       ^.h.internals
o1.__dup:       ^.stdh.h.stddef
o1.__dup:       ^.stdh.h.stdio
o1.__dup:       ^.stdh.h.stdarg
o1.__dup:       ^.stdh.h.locale
o1.__dup:       h.hostsys
o1.__dup:       ^.h.rt
o1.__dup:       ^.h.externals
o1.__dup:       ^.stdh.h.stdio
o1.__dup:       ^.stdh.h.stdarg
o1._sys_msg:    ^.c.stdio
o1._sys_msg:    h.hostsys
o1._sys_msg:    ^.stdh.h.stdio
o1._sys_msg:    ^.stdh.h.time
o1._sys_msg:    ^.stdh.h.stdio
o1._sys_msg:    ^.stdh.h.string
o1._sys_msg:    ^.stdh.h.stdlib
o1._sys_msg:    ^.stdh.h.errno
o1._sys_msg:    ^.h.ioguts
o1._sys_msg:    ^.h.internals
o1._sys_msg:    ^.stdh.h.stddef
o1._sys_msg:    ^.stdh.h.stdio
o1._sys_msg:    ^.stdh.h.stdarg
o1._sys_msg:    ^.stdh.h.locale
o1._sys_msg:    h.hostsys
o1._sys_msg:    ^.h.rt
o1._sys_msg:    ^.h.externals
o1._sys_msg:    ^.stdh.h.stdio
o1._sys_msg:    ^.stdh.h.stdarg
o1.ungetc:      ^.c.stdio
o1.ungetc:      h.hostsys
o1.ungetc:      ^.stdh.h.stdio
o1.ungetc:      ^.stdh.h.time
o1.ungetc:      ^.stdh.h.stdio
o1.ungetc:      ^.stdh.h.string
o1.ungetc:      ^.stdh.h.stdlib
o1.ungetc:      ^.stdh.h.errno
o1.ungetc:      ^.h.ioguts
o1.ungetc:      ^.h.internals
o1.ungetc:      ^.stdh.h.stddef
o1.ungetc:      ^.stdh.h.stdio
o1.ungetc:      ^.stdh.h.stdarg
o1.ungetc:      ^.stdh.h.locale
o1.ungetc:      h.hostsys
o1.ungetc:      ^.h.rt
o1.ungetc:      ^.h.externals
o1.ungetc:      ^.stdh.h.stdio
o1.ungetc:      ^.stdh.h.stdarg
o1.tmpnam:      ^.c.stdio
o1.tmpnam:      h.hostsys
o1.tmpnam:      ^.stdh.h.stdio
o1.tmpnam:      ^.stdh.h.time
o1.tmpnam:      ^.stdh.h.stdio
o1.tmpnam:      ^.stdh.h.string
o1.tmpnam:      ^.stdh.h.stdlib
o1.tmpnam:      ^.stdh.h.errno
o1.tmpnam:      ^.h.ioguts
o1.tmpnam:      ^.h.internals
o1.tmpnam:      ^.stdh.h.stddef
o1.tmpnam:      ^.stdh.h.stdio
o1.tmpnam:      ^.stdh.h.stdarg
o1.tmpnam:      ^.stdh.h.locale
o1.tmpnam:      h.hostsys
o1.tmpnam:      ^.h.rt
o1.tmpnam:      ^.h.externals
o1.tmpnam:      ^.stdh.h.stdio
o1.tmpnam:      ^.stdh.h.stdarg
o1.tmpnam:      ^.stdh.h.time
o1.stdio:       ^.c.stdio
o1.stdio:       h.hostsys
o1.stdio:       ^.stdh.h.stdio
o1.stdio:       ^.stdh.h.time
o1.stdio:       ^.stdh.h.stdio
o1.stdio:       ^.stdh.h.string
o1.stdio:       ^.stdh.h.stdlib
o1.stdio:       ^.stdh.h.errno
o1.stdio:       ^.h.ioguts
o1.stdio:       ^.h.internals
o1.stdio:       ^.stdh.h.stddef
o1.stdio:       ^.stdh.h.stdio
o1.stdio:       ^.stdh.h.stdarg
o1.stdio:       ^.stdh.h.locale
o1.stdio:       h.hostsys
o1.stdio:       ^.h.rt
o1.stdio:       ^.h.externals
o1.stdio:       ^.stdh.h.stdio
o1.stdio:       ^.stdh.h.stdarg
o1.setvbuf:     ^.c.stdio
o1.setvbuf:     h.hostsys
o1.setvbuf:     ^.stdh.h.stdio
o1.setvbuf:     ^.stdh.h.time
o1.setvbuf:     ^.stdh.h.stdio
o1.setvbuf:     ^.stdh.h.string
o1.setvbuf:     ^.stdh.h.stdlib
o1.setvbuf:     ^.stdh.h.errno
o1.setvbuf:     ^.h.ioguts
o1.setvbuf:     ^.h.internals
o1.setvbuf:     ^.stdh.h.stddef
o1.setvbuf:     ^.stdh.h.stdio
o1.setvbuf:     ^.stdh.h.stdarg
o1.setvbuf:     ^.stdh.h.locale
o1.setvbuf:     h.hostsys
o1.setvbuf:     ^.h.rt
o1.setvbuf:     ^.h.externals
o1.setvbuf:     ^.stdh.h.stdio
o1.setvbuf:     ^.stdh.h.stdarg
o1.setbuf:      ^.c.stdio
o1.setbuf:      h.hostsys
o1.setbuf:      ^.stdh.h.stdio
o1.setbuf:      ^.stdh.h.time
o1.setbuf:      ^.stdh.h.stdio
o1.setbuf:      ^.stdh.h.string
o1.setbuf:      ^.stdh.h.stdlib
o1.setbuf:      ^.stdh.h.errno
o1.setbuf:      ^.h.ioguts
o1.setbuf:      ^.h.internals
o1.setbuf:      ^.stdh.h.stddef
o1.setbuf:      ^.stdh.h.stdio
o1.setbuf:      ^.stdh.h.stdarg
o1.setbuf:      ^.stdh.h.locale
o1.setbuf:      h.hostsys
o1.setbuf:      ^.h.rt
o1.setbuf:      ^.h.externals
o1.setbuf:      ^.stdh.h.stdio
o1.setbuf:      ^.stdh.h.stdarg
o1.rewind:      ^.c.stdio
o1.rewind:      h.hostsys
o1.rewind:      ^.stdh.h.stdio
o1.rewind:      ^.stdh.h.time
o1.rewind:      ^.stdh.h.stdio
o1.rewind:      ^.stdh.h.string
o1.rewind:      ^.stdh.h.stdlib
o1.rewind:      ^.stdh.h.errno
o1.rewind:      ^.h.ioguts
o1.rewind:      ^.h.internals
o1.rewind:      ^.stdh.h.stddef
o1.rewind:      ^.stdh.h.stdio
o1.rewind:      ^.stdh.h.stdarg
o1.rewind:      ^.stdh.h.locale
o1.rewind:      h.hostsys
o1.rewind:      ^.h.rt
o1.rewind:      ^.h.externals
o1.rewind:      ^.stdh.h.stdio
o1.rewind:      ^.stdh.h.stdarg
o1.puts:        ^.c.stdio
o1.puts:        h.hostsys
o1.puts:        ^.stdh.h.stdio
o1.puts:        ^.stdh.h.time
o1.puts:        ^.stdh.h.stdio
o1.puts:        ^.stdh.h.string
o1.puts:        ^.stdh.h.stdlib
o1.puts:        ^.stdh.h.errno
o1.puts:        ^.h.ioguts
o1.puts:        ^.h.internals
o1.puts:        ^.stdh.h.stddef
o1.puts:        ^.stdh.h.stdio
o1.puts:        ^.stdh.h.stdarg
o1.puts:        ^.stdh.h.locale
o1.puts:        h.hostsys
o1.puts:        ^.h.rt
o1.puts:        ^.h.externals
o1.puts:        ^.stdh.h.stdio
o1.puts:        ^.stdh.h.stdarg
o1.putchar:     ^.c.stdio
o1.putchar:     h.hostsys
o1.putchar:     ^.stdh.h.stdio
o1.putchar:     ^.stdh.h.time
o1.putchar:     ^.stdh.h.stdio
o1.putchar:     ^.stdh.h.string
o1.putchar:     ^.stdh.h.stdlib
o1.putchar:     ^.stdh.h.errno
o1.putchar:     ^.h.ioguts
o1.putchar:     ^.h.internals
o1.putchar:     ^.stdh.h.stddef
o1.putchar:     ^.stdh.h.stdio
o1.putchar:     ^.stdh.h.stdarg
o1.putchar:     ^.stdh.h.locale
o1.putchar:     h.hostsys
o1.putchar:     ^.h.rt
o1.putchar:     ^.h.externals
o1.putchar:     ^.stdh.h.stdio
o1.putchar:     ^.stdh.h.stdarg
o1.putc:        ^.c.stdio
o1.putc:        h.hostsys
o1.putc:        ^.stdh.h.stdio
o1.putc:        ^.stdh.h.time
o1.putc:        ^.stdh.h.stdio
o1.putc:        ^.stdh.h.string
o1.putc:        ^.stdh.h.stdlib
o1.putc:        ^.stdh.h.errno
o1.putc:        ^.h.ioguts
o1.putc:        ^.h.internals
o1.putc:        ^.stdh.h.stddef
o1.putc:        ^.stdh.h.stdio
o1.putc:        ^.stdh.h.stdarg
o1.putc:        ^.stdh.h.locale
o1.putc:        h.hostsys
o1.putc:        ^.h.rt
o1.putc:        ^.h.externals
o1.putc:        ^.stdh.h.stdio
o1.putc:        ^.stdh.h.stdarg
o1.perror:      ^.c.stdio
o1.perror:      h.hostsys
o1.perror:      ^.stdh.h.stdio
o1.perror:      ^.stdh.h.time
o1.perror:      ^.stdh.h.stdio
o1.perror:      ^.stdh.h.string
o1.perror:      ^.stdh.h.stdlib
o1.perror:      ^.stdh.h.errno
o1.perror:      ^.h.ioguts
o1.perror:      ^.h.internals
o1.perror:      ^.stdh.h.stddef
o1.perror:      ^.stdh.h.stdio
o1.perror:      ^.stdh.h.stdarg
o1.perror:      ^.stdh.h.locale
o1.perror:      h.hostsys
o1.perror:      ^.h.rt
o1.perror:      ^.h.externals
o1.perror:      ^.stdh.h.stdio
o1.perror:      ^.stdh.h.stdarg
o1.osf: ^.c.stdio
o1.osf: h.hostsys
o1.osf: ^.stdh.h.stdio
o1.osf: ^.stdh.h.time
o1.osf: ^.stdh.h.stdio
o1.osf: ^.stdh.h.string
o1.osf: ^.stdh.h.stdlib
o1.osf: ^.stdh.h.errno
o1.osf: ^.h.ioguts
o1.osf: ^.h.internals
o1.osf: ^.stdh.h.stddef
o1.osf: ^.stdh.h.stdio
o1.osf: ^.stdh.h.stdarg
o1.osf: ^.stdh.h.locale
o1.osf: h.hostsys
o1.osf: ^.h.rt
o1.osf: ^.h.externals
o1.osf: ^.stdh.h.stdio
o1.osf: ^.stdh.h.stdarg
o1.osf: ^.h.externals
o1.osf: ^.stdh.h.stdio
o1.osf: ^.stdh.h.stdarg
o1.gets:        ^.c.stdio
o1.gets:        h.hostsys
o1.gets:        ^.stdh.h.stdio
o1.gets:        ^.stdh.h.time
o1.gets:        ^.stdh.h.stdio
o1.gets:        ^.stdh.h.string
o1.gets:        ^.stdh.h.stdlib
o1.gets:        ^.stdh.h.errno
o1.gets:        ^.h.ioguts
o1.gets:        ^.h.internals
o1.gets:        ^.stdh.h.stddef
o1.gets:        ^.stdh.h.stdio
o1.gets:        ^.stdh.h.stdarg
o1.gets:        ^.stdh.h.locale
o1.gets:        h.hostsys
o1.gets:        ^.h.rt
o1.gets:        ^.h.externals
o1.gets:        ^.stdh.h.stdio
o1.gets:        ^.stdh.h.stdarg
o1.getchar:     ^.c.stdio
o1.getchar:     h.hostsys
o1.getchar:     ^.stdh.h.stdio
o1.getchar:     ^.stdh.h.time
o1.getchar:     ^.stdh.h.stdio
o1.getchar:     ^.stdh.h.string
o1.getchar:     ^.stdh.h.stdlib
o1.getchar:     ^.stdh.h.errno
o1.getchar:     ^.h.ioguts
o1.getchar:     ^.h.internals
o1.getchar:     ^.stdh.h.stddef
o1.getchar:     ^.stdh.h.stdio
o1.getchar:     ^.stdh.h.stdarg
o1.getchar:     ^.stdh.h.locale
o1.getchar:     h.hostsys
o1.getchar:     ^.h.rt
o1.getchar:     ^.h.externals
o1.getchar:     ^.stdh.h.stdio
o1.getchar:     ^.stdh.h.stdarg
o1.getc:        ^.c.stdio
o1.getc:        h.hostsys
o1.getc:        ^.stdh.h.stdio
o1.getc:        ^.stdh.h.time
o1.getc:        ^.stdh.h.stdio
o1.getc:        ^.stdh.h.string
o1.getc:        ^.stdh.h.stdlib
o1.getc:        ^.stdh.h.errno
o1.getc:        ^.h.ioguts
o1.getc:        ^.h.internals
o1.getc:        ^.stdh.h.stddef
o1.getc:        ^.stdh.h.stdio
o1.getc:        ^.stdh.h.stdarg
o1.getc:        ^.stdh.h.locale
o1.getc:        h.hostsys
o1.getc:        ^.h.rt
o1.getc:        ^.h.externals
o1.getc:        ^.stdh.h.stdio
o1.getc:        ^.stdh.h.stdarg
o1.fwrite:      ^.c.stdio
o1.fwrite:      h.hostsys
o1.fwrite:      ^.stdh.h.stdio
o1.fwrite:      ^.stdh.h.time
o1.fwrite:      ^.stdh.h.stdio
o1.fwrite:      ^.stdh.h.string
o1.fwrite:      ^.stdh.h.stdlib
o1.fwrite:      ^.stdh.h.errno
o1.fwrite:      ^.h.ioguts
o1.fwrite:      ^.h.internals
o1.fwrite:      ^.stdh.h.stddef
o1.fwrite:      ^.stdh.h.stdio
o1.fwrite:      ^.stdh.h.stdarg
o1.fwrite:      ^.stdh.h.locale
o1.fwrite:      h.hostsys
o1.fwrite:      ^.h.rt
o1.fwrite:      ^.h.externals
o1.fwrite:      ^.stdh.h.stdio
o1.fwrite:      ^.stdh.h.stdarg
o1.ftell:       ^.c.stdio
o1.ftell:       h.hostsys
o1.ftell:       ^.stdh.h.stdio
o1.ftell:       ^.stdh.h.time
o1.ftell:       ^.stdh.h.stdio
o1.ftell:       ^.stdh.h.string
o1.ftell:       ^.stdh.h.stdlib
o1.ftell:       ^.stdh.h.errno
o1.ftell:       ^.h.ioguts
o1.ftell:       ^.h.internals
o1.ftell:       ^.stdh.h.stddef
o1.ftell:       ^.stdh.h.stdio
o1.ftell:       ^.stdh.h.stdarg
o1.ftell:       ^.stdh.h.locale
o1.ftell:       h.hostsys
o1.ftell:       ^.h.rt
o1.ftell:       ^.h.externals
o1.ftell:       ^.stdh.h.stdio
o1.ftell:       ^.stdh.h.stdarg
o1.fsetpos:     ^.c.stdio
o1.fsetpos:     h.hostsys
o1.fsetpos:     ^.stdh.h.stdio
o1.fsetpos:     ^.stdh.h.time
o1.fsetpos:     ^.stdh.h.stdio
o1.fsetpos:     ^.stdh.h.string
o1.fsetpos:     ^.stdh.h.stdlib
o1.fsetpos:     ^.stdh.h.errno
o1.fsetpos:     ^.h.ioguts
o1.fsetpos:     ^.h.internals
o1.fsetpos:     ^.stdh.h.stddef
o1.fsetpos:     ^.stdh.h.stdio
o1.fsetpos:     ^.stdh.h.stdarg
o1.fsetpos:     ^.stdh.h.locale
o1.fsetpos:     h.hostsys
o1.fsetpos:     ^.h.rt
o1.fsetpos:     ^.h.externals
o1.fsetpos:     ^.stdh.h.stdio
o1.fsetpos:     ^.stdh.h.stdarg
o1.fseek:       ^.c.stdio
o1.fseek:       h.hostsys
o1.fseek:       ^.stdh.h.stdio
o1.fseek:       ^.stdh.h.time
o1.fseek:       ^.stdh.h.stdio
o1.fseek:       ^.stdh.h.string
o1.fseek:       ^.stdh.h.stdlib
o1.fseek:       ^.stdh.h.errno
o1.fseek:       ^.h.ioguts
o1.fseek:       ^.h.internals
o1.fseek:       ^.stdh.h.stddef
o1.fseek:       ^.stdh.h.stdio
o1.fseek:       ^.stdh.h.stdarg
o1.fseek:       ^.stdh.h.locale
o1.fseek:       h.hostsys
o1.fseek:       ^.h.rt
o1.fseek:       ^.h.externals
o1.fseek:       ^.stdh.h.stdio
o1.fseek:       ^.stdh.h.stdarg
o1.fread:       ^.c.stdio
o1.fread:       h.hostsys
o1.fread:       ^.stdh.h.stdio
o1.fread:       ^.stdh.h.time
o1.fread:       ^.stdh.h.stdio
o1.fread:       ^.stdh.h.string
o1.fread:       ^.stdh.h.stdlib
o1.fread:       ^.stdh.h.errno
o1.fread:       ^.h.ioguts
o1.fread:       ^.h.internals
o1.fread:       ^.stdh.h.stddef
o1.fread:       ^.stdh.h.stdio
o1.fread:       ^.stdh.h.stdarg
o1.fread:       ^.stdh.h.locale
o1.fread:       h.hostsys
o1.fread:       ^.h.rt
o1.fread:       ^.h.externals
o1.fread:       ^.stdh.h.stdio
o1.fread:       ^.stdh.h.stdarg
o1.fputs:       ^.c.stdio
o1.fputs:       h.hostsys
o1.fputs:       ^.stdh.h.stdio
o1.fputs:       ^.stdh.h.time
o1.fputs:       ^.stdh.h.stdio
o1.fputs:       ^.stdh.h.string
o1.fputs:       ^.stdh.h.stdlib
o1.fputs:       ^.stdh.h.errno
o1.fputs:       ^.h.ioguts
o1.fputs:       ^.h.internals
o1.fputs:       ^.stdh.h.stddef
o1.fputs:       ^.stdh.h.stdio
o1.fputs:       ^.stdh.h.stdarg
o1.fputs:       ^.stdh.h.locale
o1.fputs:       h.hostsys
o1.fputs:       ^.h.rt
o1.fputs:       ^.h.externals
o1.fputs:       ^.stdh.h.stdio
o1.fputs:       ^.stdh.h.stdarg
o1.fputc:       ^.c.stdio
o1.fputc:       h.hostsys
o1.fputc:       ^.stdh.h.stdio
o1.fputc:       ^.stdh.h.time
o1.fputc:       ^.stdh.h.stdio
o1.fputc:       ^.stdh.h.string
o1.fputc:       ^.stdh.h.stdlib
o1.fputc:       ^.stdh.h.errno
o1.fputc:       ^.h.ioguts
o1.fputc:       ^.h.internals
o1.fputc:       ^.stdh.h.stddef
o1.fputc:       ^.stdh.h.stdio
o1.fputc:       ^.stdh.h.stdarg
o1.fputc:       ^.stdh.h.locale
o1.fputc:       h.hostsys
o1.fputc:       ^.h.rt
o1.fputc:       ^.h.externals
o1.fputc:       ^.stdh.h.stdio
o1.fputc:       ^.stdh.h.stdarg
o1.flsbuf:      ^.c.stdio
o1.flsbuf:      h.hostsys
o1.flsbuf:      ^.stdh.h.stdio
o1.flsbuf:      ^.stdh.h.time
o1.flsbuf:      ^.stdh.h.stdio
o1.flsbuf:      ^.stdh.h.string
o1.flsbuf:      ^.stdh.h.stdlib
o1.flsbuf:      ^.stdh.h.errno
o1.flsbuf:      ^.h.ioguts
o1.flsbuf:      ^.h.internals
o1.flsbuf:      ^.stdh.h.stddef
o1.flsbuf:      ^.stdh.h.stdio
o1.flsbuf:      ^.stdh.h.stdarg
o1.flsbuf:      ^.stdh.h.locale
o1.flsbuf:      h.hostsys
o1.flsbuf:      ^.h.rt
o1.flsbuf:      ^.h.externals
o1.flsbuf:      ^.stdh.h.stdio
o1.flsbuf:      ^.stdh.h.stdarg
o1.filbuf:      ^.c.stdio
o1.filbuf:      h.hostsys
o1.filbuf:      ^.stdh.h.stdio
o1.filbuf:      ^.stdh.h.time
o1.filbuf:      ^.stdh.h.stdio
o1.filbuf:      ^.stdh.h.string
o1.filbuf:      ^.stdh.h.stdlib
o1.filbuf:      ^.stdh.h.errno
o1.filbuf:      ^.h.ioguts
o1.filbuf:      ^.h.internals
o1.filbuf:      ^.stdh.h.stddef
o1.filbuf:      ^.stdh.h.stdio
o1.filbuf:      ^.stdh.h.stdarg
o1.filbuf:      ^.stdh.h.locale
o1.filbuf:      h.hostsys
o1.filbuf:      ^.h.rt
o1.filbuf:      ^.h.externals
o1.filbuf:      ^.stdh.h.stdio
o1.filbuf:      ^.stdh.h.stdarg
o1.fisatty:     ^.c.stdio
o1.fisatty:     h.hostsys
o1.fisatty:     ^.stdh.h.stdio
o1.fisatty:     ^.stdh.h.time
o1.fisatty:     ^.stdh.h.stdio
o1.fisatty:     ^.stdh.h.string
o1.fisatty:     ^.stdh.h.stdlib
o1.fisatty:     ^.stdh.h.errno
o1.fisatty:     ^.h.ioguts
o1.fisatty:     ^.h.internals
o1.fisatty:     ^.stdh.h.stddef
o1.fisatty:     ^.stdh.h.stdio
o1.fisatty:     ^.stdh.h.stdarg
o1.fisatty:     ^.stdh.h.locale
o1.fisatty:     h.hostsys
o1.fisatty:     ^.h.rt
o1.fisatty:     ^.h.externals
o1.fisatty:     ^.stdh.h.stdio
o1.fisatty:     ^.stdh.h.stdarg
o1.fisatty:     ^.h.externals
o1.fisatty:     ^.stdh.h.stdio
o1.fisatty:     ^.stdh.h.stdarg
o1.fgets:       ^.c.stdio
o1.fgets:       h.hostsys
o1.fgets:       ^.stdh.h.stdio
o1.fgets:       ^.stdh.h.time
o1.fgets:       ^.stdh.h.stdio
o1.fgets:       ^.stdh.h.string
o1.fgets:       ^.stdh.h.stdlib
o1.fgets:       ^.stdh.h.errno
o1.fgets:       ^.h.ioguts
o1.fgets:       ^.h.internals
o1.fgets:       ^.stdh.h.stddef
o1.fgets:       ^.stdh.h.stdio
o1.fgets:       ^.stdh.h.stdarg
o1.fgets:       ^.stdh.h.locale
o1.fgets:       h.hostsys
o1.fgets:       ^.h.rt
o1.fgets:       ^.h.externals
o1.fgets:       ^.stdh.h.stdio
o1.fgets:       ^.stdh.h.stdarg
o1.fgetpos:     ^.c.stdio
o1.fgetpos:     h.hostsys
o1.fgetpos:     ^.stdh.h.stdio
o1.fgetpos:     ^.stdh.h.time
o1.fgetpos:     ^.stdh.h.stdio
o1.fgetpos:     ^.stdh.h.string
o1.fgetpos:     ^.stdh.h.stdlib
o1.fgetpos:     ^.stdh.h.errno
o1.fgetpos:     ^.h.ioguts
o1.fgetpos:     ^.h.internals
o1.fgetpos:     ^.stdh.h.stddef
o1.fgetpos:     ^.stdh.h.stdio
o1.fgetpos:     ^.stdh.h.stdarg
o1.fgetpos:     ^.stdh.h.locale
o1.fgetpos:     h.hostsys
o1.fgetpos:     ^.h.rt
o1.fgetpos:     ^.h.externals
o1.fgetpos:     ^.stdh.h.stdio
o1.fgetpos:     ^.stdh.h.stdarg
o1.fgetc:       ^.c.stdio
o1.fgetc:       h.hostsys
o1.fgetc:       ^.stdh.h.stdio
o1.fgetc:       ^.stdh.h.time
o1.fgetc:       ^.stdh.h.stdio
o1.fgetc:       ^.stdh.h.string
o1.fgetc:       ^.stdh.h.stdlib
o1.fgetc:       ^.stdh.h.errno
o1.fgetc:       ^.h.ioguts
o1.fgetc:       ^.h.internals
o1.fgetc:       ^.stdh.h.stddef
o1.fgetc:       ^.stdh.h.stdio
o1.fgetc:       ^.stdh.h.stdarg
o1.fgetc:       ^.stdh.h.locale
o1.fgetc:       h.hostsys
o1.fgetc:       ^.h.rt
o1.fgetc:       ^.h.externals
o1.fgetc:       ^.stdh.h.stdio
o1.fgetc:       ^.stdh.h.stdarg
o1.fflush:      ^.c.stdio
o1.fflush:      h.hostsys
o1.fflush:      ^.stdh.h.stdio
o1.fflush:      ^.stdh.h.time
o1.fflush:      ^.stdh.h.stdio
o1.fflush:      ^.stdh.h.string
o1.fflush:      ^.stdh.h.stdlib
o1.fflush:      ^.stdh.h.errno
o1.fflush:      ^.h.ioguts
o1.fflush:      ^.h.internals
o1.fflush:      ^.stdh.h.stddef
o1.fflush:      ^.stdh.h.stdio
o1.fflush:      ^.stdh.h.stdarg
o1.fflush:      ^.stdh.h.locale
o1.fflush:      h.hostsys
o1.fflush:      ^.h.rt
o1.fflush:      ^.h.externals
o1.fflush:      ^.stdh.h.stdio
o1.fflush:      ^.stdh.h.stdarg
o1.ferror:      ^.c.stdio
o1.ferror:      h.hostsys
o1.ferror:      ^.stdh.h.stdio
o1.ferror:      ^.stdh.h.time
o1.ferror:      ^.stdh.h.stdio
o1.ferror:      ^.stdh.h.string
o1.ferror:      ^.stdh.h.stdlib
o1.ferror:      ^.stdh.h.errno
o1.ferror:      ^.h.ioguts
o1.ferror:      ^.h.internals
o1.ferror:      ^.stdh.h.stddef
o1.ferror:      ^.stdh.h.stdio
o1.ferror:      ^.stdh.h.stdarg
o1.ferror:      ^.stdh.h.locale
o1.ferror:      h.hostsys
o1.ferror:      ^.h.rt
o1.ferror:      ^.h.externals
o1.ferror:      ^.stdh.h.stdio
o1.ferror:      ^.stdh.h.stdarg
o1.feof:        ^.c.stdio
o1.feof:        h.hostsys
o1.feof:        ^.stdh.h.stdio
o1.feof:        ^.stdh.h.time
o1.feof:        ^.stdh.h.stdio
o1.feof:        ^.stdh.h.string
o1.feof:        ^.stdh.h.stdlib
o1.feof:        ^.stdh.h.errno
o1.feof:        ^.h.ioguts
o1.feof:        ^.h.internals
o1.feof:        ^.stdh.h.stddef
o1.feof:        ^.stdh.h.stdio
o1.feof:        ^.stdh.h.stdarg
o1.feof:        ^.stdh.h.locale
o1.feof:        h.hostsys
o1.feof:        ^.h.rt
o1.feof:        ^.h.externals
o1.feof:        ^.stdh.h.stdio
o1.feof:        ^.stdh.h.stdarg
o1.clearerr:    ^.c.stdio
o1.clearerr:    h.hostsys
o1.clearerr:    ^.stdh.h.stdio
o1.clearerr:    ^.stdh.h.time
o1.clearerr:    ^.stdh.h.stdio
o1.clearerr:    ^.stdh.h.string
o1.clearerr:    ^.stdh.h.stdlib
o1.clearerr:    ^.stdh.h.errno
o1.clearerr:    ^.h.ioguts
o1.clearerr:    ^.h.internals
o1.clearerr:    ^.stdh.h.stddef
o1.clearerr:    ^.stdh.h.stdio
o1.clearerr:    ^.stdh.h.stdarg
o1.clearerr:    ^.stdh.h.locale
o1.clearerr:    h.hostsys
o1.clearerr:    ^.h.rt
o1.clearerr:    ^.h.externals
o1.clearerr:    ^.stdh.h.stdio
o1.clearerr:    ^.stdh.h.stdarg
o1.backspace:   ^.c.stdio
o1.backspace:   h.hostsys
o1.backspace:   ^.stdh.h.stdio
o1.backspace:   ^.stdh.h.time
o1.backspace:   ^.stdh.h.stdio
o1.backspace:   ^.stdh.h.string
o1.backspace:   ^.stdh.h.stdlib
o1.backspace:   ^.stdh.h.errno
o1.backspace:   ^.h.ioguts
o1.backspace:   ^.h.internals
o1.backspace:   ^.stdh.h.stddef
o1.backspace:   ^.stdh.h.stdio
o1.backspace:   ^.stdh.h.stdarg
o1.backspace:   ^.stdh.h.locale
o1.backspace:   h.hostsys
o1.backspace:   ^.h.rt
o1.backspace:   ^.h.externals
o1.backspace:   ^.stdh.h.stdio
o1.backspace:   ^.stdh.h.stdarg
o1.atol:        ^.c.scanf
o1.atol:        h.hostsys
o1.atol:        ^.stdh.h.stdio
o1.atol:        ^.stdh.h.time
o1.atol:        ^.stdh.h.stdio
o1.atol:        ^.stdh.h.stdlib
o1.atol:        ^.stdh.h.math
o1.atol:        ^.stdh.h.errno
o1.atol:        ^.stdh.h.string
o1.atol:        ^.stdh.h.ctype
o1.atol:        ^.stdh.h.stddef
o1.atol:        ^.stdh.h.stdarg
o1.atol:        ^.stdh.h.limits
o1.atol:        h.config
o1.atol:        ^.h.internals
o1.atol:        ^.stdh.h.stddef
o1.atol:        ^.stdh.h.stdio
o1.atol:        ^.stdh.h.stdarg
o1.atol:        ^.stdh.h.locale
o1.atol:        h.hostsys
o1.atol:        ^.h.rt
o1.atoi:        ^.c.scanf
o1.atoi:        h.hostsys
o1.atoi:        ^.stdh.h.stdio
o1.atoi:        ^.stdh.h.time
o1.atoi:        ^.stdh.h.stdio
o1.atoi:        ^.stdh.h.stdlib
o1.atoi:        ^.stdh.h.math
o1.atoi:        ^.stdh.h.errno
o1.atoi:        ^.stdh.h.string
o1.atoi:        ^.stdh.h.ctype
o1.atoi:        ^.stdh.h.stddef
o1.atoi:        ^.stdh.h.stdarg
o1.atoi:        ^.stdh.h.limits
o1.atoi:        h.config
o1.atoi:        ^.h.internals
o1.atoi:        ^.stdh.h.stddef
o1.atoi:        ^.stdh.h.stdio
o1.atoi:        ^.stdh.h.stdarg
o1.atoi:        ^.stdh.h.locale
o1.atoi:        h.hostsys
o1.atoi:        ^.h.rt
o1.atof:        ^.c.scanf
o1.atof:        h.hostsys
o1.atof:        ^.stdh.h.stdio
o1.atof:        ^.stdh.h.time
o1.atof:        ^.stdh.h.stdio
o1.atof:        ^.stdh.h.stdlib
o1.atof:        ^.stdh.h.math
o1.atof:        ^.stdh.h.errno
o1.atof:        ^.stdh.h.string
o1.atof:        ^.stdh.h.ctype
o1.atof:        ^.stdh.h.stddef
o1.atof:        ^.stdh.h.stdarg
o1.atof:        ^.stdh.h.limits
o1.atof:        h.config
o1.atof:        ^.h.internals
o1.atof:        ^.stdh.h.stddef
o1.atof:        ^.stdh.h.stdio
o1.atof:        ^.stdh.h.stdarg
o1.atof:        ^.stdh.h.locale
o1.atof:        h.hostsys
o1.atof:        ^.h.rt
o.sscanf:       ^.c.scanf
o.sscanf:       h.hostsys
o.sscanf:       ^.stdh.h.stdio
o.sscanf:       ^.stdh.h.time
o.sscanf:       ^.stdh.h.stdio
o.sscanf:       ^.stdh.h.stdlib
o.sscanf:       ^.stdh.h.math
o.sscanf:       ^.stdh.h.errno
o.sscanf:       ^.stdh.h.string
o.sscanf:       ^.stdh.h.ctype
o.sscanf:       ^.stdh.h.stddef
o.sscanf:       ^.stdh.h.stdarg
o.sscanf:       ^.stdh.h.limits
o.sscanf:       h.config
o.sscanf:       ^.h.internals
o.sscanf:       ^.stdh.h.stddef
o.sscanf:       ^.stdh.h.stdio
o.sscanf:       ^.stdh.h.stdarg
o.sscanf:       ^.stdh.h.locale
o.sscanf:       h.hostsys
o.sscanf:       ^.h.rt
o.sscanf:       ^.h.ioguts
o.fscanf:       ^.c.scanf
o.fscanf:       h.hostsys
o.fscanf:       ^.stdh.h.stdio
o.fscanf:       ^.stdh.h.time
o.fscanf:       ^.stdh.h.stdio
o.fscanf:       ^.stdh.h.stdlib
o.fscanf:       ^.stdh.h.math
o.fscanf:       ^.stdh.h.errno
o.fscanf:       ^.stdh.h.string
o.fscanf:       ^.stdh.h.ctype
o.fscanf:       ^.stdh.h.stddef
o.fscanf:       ^.stdh.h.stdarg
o.fscanf:       ^.stdh.h.limits
o.fscanf:       h.config
o.fscanf:       ^.h.internals
o.fscanf:       ^.stdh.h.stddef
o.fscanf:       ^.stdh.h.stdio
o.fscanf:       ^.stdh.h.stdarg
o.fscanf:       ^.stdh.h.locale
o.fscanf:       h.hostsys
o.fscanf:       ^.h.rt
o.scanf:        ^.c.scanf
o.scanf:        h.hostsys
o.scanf:        ^.stdh.h.stdio
o.scanf:        ^.stdh.h.time
o.scanf:        ^.stdh.h.stdio
o.scanf:        ^.stdh.h.stdlib
o.scanf:        ^.stdh.h.math
o.scanf:        ^.stdh.h.errno
o.scanf:        ^.stdh.h.string
o.scanf:        ^.stdh.h.ctype
o.scanf:        ^.stdh.h.stddef
o.scanf:        ^.stdh.h.stdarg
o.scanf:        ^.stdh.h.limits
o.scanf:        h.config
o.scanf:        ^.h.internals
o.scanf:        ^.stdh.h.stddef
o.scanf:        ^.stdh.h.stdio
o.scanf:        ^.stdh.h.stdarg
o.scanf:        ^.stdh.h.locale
o.scanf:        h.hostsys
o.scanf:        ^.h.rt
o._scanf:       ^.c.scanf
o._scanf:       h.hostsys
o._scanf:       ^.stdh.h.stdio
o._scanf:       ^.stdh.h.time
o._scanf:       ^.stdh.h.stdio
o._scanf:       ^.stdh.h.stdlib
o._scanf:       ^.stdh.h.math
o._scanf:       ^.stdh.h.errno
o._scanf:       ^.stdh.h.string
o._scanf:       ^.stdh.h.ctype
o._scanf:       ^.stdh.h.stddef
o._scanf:       ^.stdh.h.stdarg
o._scanf:       ^.stdh.h.limits
o._scanf:       h.config
o._scanf:       ^.h.internals
o._scanf:       ^.stdh.h.stddef
o._scanf:       ^.stdh.h.stdio
o._scanf:       ^.stdh.h.stdarg
o._scanf:       ^.stdh.h.locale
o._scanf:       h.hostsys
o._scanf:       ^.h.rt
o.strtoul:      ^.c.scanf
o.strtoul:      h.hostsys
o.strtoul:      ^.stdh.h.stdio
o.strtoul:      ^.stdh.h.time
o.strtoul:      ^.stdh.h.stdio
o.strtoul:      ^.stdh.h.stdlib
o.strtoul:      ^.stdh.h.math
o.strtoul:      ^.stdh.h.errno
o.strtoul:      ^.stdh.h.string
o.strtoul:      ^.stdh.h.ctype
o.strtoul:      ^.stdh.h.stddef
o.strtoul:      ^.stdh.h.stdarg
o.strtoul:      ^.stdh.h.limits
o.strtoul:      h.config
o.strtoul:      ^.h.internals
o.strtoul:      ^.stdh.h.stddef
o.strtoul:      ^.stdh.h.stdio
o.strtoul:      ^.stdh.h.stdarg
o.strtoul:      ^.stdh.h.locale
o.strtoul:      h.hostsys
o.strtoul:      ^.h.rt
o.strtol:       ^.c.scanf
o.strtol:       h.hostsys
o.strtol:       ^.stdh.h.stdio
o.strtol:       ^.stdh.h.time
o.strtol:       ^.stdh.h.stdio
o.strtol:       ^.stdh.h.stdlib
o.strtol:       ^.stdh.h.math
o.strtol:       ^.stdh.h.errno
o.strtol:       ^.stdh.h.string
o.strtol:       ^.stdh.h.ctype
o.strtol:       ^.stdh.h.stddef
o.strtol:       ^.stdh.h.stdarg
o.strtol:       ^.stdh.h.limits
o.strtol:       h.config
o.strtol:       ^.h.internals
o.strtol:       ^.stdh.h.stddef
o.strtol:       ^.stdh.h.stdio
o.strtol:       ^.stdh.h.stdarg
o.strtol:       ^.stdh.h.locale
o.strtol:       h.hostsys
o.strtol:       ^.h.rt
o.strtod:       ^.c.scanf
o.strtod:       h.hostsys
o.strtod:       ^.stdh.h.stdio
o.strtod:       ^.stdh.h.time
o.strtod:       ^.stdh.h.stdio
o.strtod:       ^.stdh.h.stdlib
o.strtod:       ^.stdh.h.math
o.strtod:       ^.stdh.h.errno
o.strtod:       ^.stdh.h.string
o.strtod:       ^.stdh.h.ctype
o.strtod:       ^.stdh.h.stddef
o.strtod:       ^.stdh.h.stdarg
o.strtod:       ^.stdh.h.limits
o.strtod:       h.config
o.strtod:       ^.h.internals
o.strtod:       ^.stdh.h.stddef
o.strtod:       ^.stdh.h.stdio
o.strtod:       ^.stdh.h.stdarg
o.strtod:       ^.stdh.h.locale
o.strtod:       h.hostsys
o.strtod:       ^.h.rt
o._strtoul:     ^.c.scanf
o._strtoul:     h.hostsys
o._strtoul:     ^.stdh.h.stdio
o._strtoul:     ^.stdh.h.time
o._strtoul:     ^.stdh.h.stdio
o._strtoul:     ^.stdh.h.stdlib
o._strtoul:     ^.stdh.h.math
o._strtoul:     ^.stdh.h.errno
o._strtoul:     ^.stdh.h.string
o._strtoul:     ^.stdh.h.ctype
o._strtoul:     ^.stdh.h.stddef
o._strtoul:     ^.stdh.h.stdarg
o._strtoul:     ^.stdh.h.limits
o._strtoul:     h.config
o._strtoul:     ^.h.internals
o._strtoul:     ^.stdh.h.stddef
o._strtoul:     ^.stdh.h.stdio
o._strtoul:     ^.stdh.h.stdarg
o._strtoul:     ^.stdh.h.locale
o._strtoul:     h.hostsys
o._strtoul:     ^.h.rt
o._chval:       ^.c.scanf
o._chval:       h.hostsys
o._chval:       ^.stdh.h.stdio
o._chval:       ^.stdh.h.time
o._chval:       ^.stdh.h.stdio
o._chval:       ^.stdh.h.stdlib
o._chval:       ^.stdh.h.math
o._chval:       ^.stdh.h.errno
o._chval:       ^.stdh.h.string
o._chval:       ^.stdh.h.ctype
o._chval:       ^.stdh.h.stddef
o._chval:       ^.stdh.h.stdarg
o._chval:       ^.stdh.h.limits
o._chval:       h.config
o._chval:       ^.h.internals
o._chval:       ^.stdh.h.stddef
o._chval:       ^.stdh.h.stdio
o._chval:       ^.stdh.h.stdarg
o._chval:       ^.stdh.h.locale
o._chval:       h.hostsys
o._chval:       ^.h.rt
o.__vfprintf:   ^.c.printf
o.__vfprintf:   h.hostsys
o.__vfprintf:   ^.stdh.h.stdio
o.__vfprintf:   ^.stdh.h.time
o.__vfprintf:   ^.stdh.h.stdio
o.__vfprintf:   ^.stdh.h.stdarg
o.__vfprintf:   ^.stdh.h.ctype
o.__vfprintf:   ^.stdh.h.string
o.__vfprintf:   h.config
o.__vfprintf:   ^.h.externals
o.__vfprintf:   ^.stdh.h.stdio
o.__vfprintf:   ^.stdh.h.stdarg
o.__vfprintf:   ^.h.internals
o.__vfprintf:   ^.stdh.h.stddef
o.__vfprintf:   ^.stdh.h.stdio
o.__vfprintf:   ^.stdh.h.stdarg
o.__vfprintf:   ^.stdh.h.locale
o.__vfprintf:   h.hostsys
o.__vfprintf:   ^.h.rt
o._vsprintf:    ^.c.printf
o._vsprintf:    h.hostsys
o._vsprintf:    ^.stdh.h.stdio
o._vsprintf:    ^.stdh.h.time
o._vsprintf:    ^.stdh.h.stdio
o._vsprintf:    ^.stdh.h.stdarg
o._vsprintf:    ^.stdh.h.ctype
o._vsprintf:    ^.stdh.h.string
o._vsprintf:    h.config
o._vsprintf:    ^.h.externals
o._vsprintf:    ^.stdh.h.stdio
o._vsprintf:    ^.stdh.h.stdarg
o._vsprintf:    ^.h.internals
o._vsprintf:    ^.stdh.h.stddef
o._vsprintf:    ^.stdh.h.stdio
o._vsprintf:    ^.stdh.h.stdarg
o._vsprintf:    ^.stdh.h.locale
o._vsprintf:    h.hostsys
o._vsprintf:    ^.h.rt
o._vsprintf:    ^.h.ioguts
o._vfprintf:    ^.c.printf
o._vfprintf:    h.hostsys
o._vfprintf:    ^.stdh.h.stdio
o._vfprintf:    ^.stdh.h.time
o._vfprintf:    ^.stdh.h.stdio
o._vfprintf:    ^.stdh.h.stdarg
o._vfprintf:    ^.stdh.h.ctype
o._vfprintf:    ^.stdh.h.string
o._vfprintf:    h.config
o._vfprintf:    ^.h.externals
o._vfprintf:    ^.stdh.h.stdio
o._vfprintf:    ^.stdh.h.stdarg
o._vfprintf:    ^.h.internals
o._vfprintf:    ^.stdh.h.stddef
o._vfprintf:    ^.stdh.h.stdio
o._vfprintf:    ^.stdh.h.stdarg
o._vfprintf:    ^.stdh.h.locale
o._vfprintf:    h.hostsys
o._vfprintf:    ^.h.rt
o._sprintf:     ^.c.printf
o._sprintf:     h.hostsys
o._sprintf:     ^.stdh.h.stdio
o._sprintf:     ^.stdh.h.time
o._sprintf:     ^.stdh.h.stdio
o._sprintf:     ^.stdh.h.stdarg
o._sprintf:     ^.stdh.h.ctype
o._sprintf:     ^.stdh.h.string
o._sprintf:     h.config
o._sprintf:     ^.h.externals
o._sprintf:     ^.stdh.h.stdio
o._sprintf:     ^.stdh.h.stdarg
o._sprintf:     ^.h.internals
o._sprintf:     ^.stdh.h.stddef
o._sprintf:     ^.stdh.h.stdio
o._sprintf:     ^.stdh.h.stdarg
o._sprintf:     ^.stdh.h.locale
o._sprintf:     h.hostsys
o._sprintf:     ^.h.rt
o._sprintf:     ^.h.ioguts
o._printf:      ^.c.printf
o._printf:      h.hostsys
o._printf:      ^.stdh.h.stdio
o._printf:      ^.stdh.h.time
o._printf:      ^.stdh.h.stdio
o._printf:      ^.stdh.h.stdarg
o._printf:      ^.stdh.h.ctype
o._printf:      ^.stdh.h.string
o._printf:      h.config
o._printf:      ^.h.externals
o._printf:      ^.stdh.h.stdio
o._printf:      ^.stdh.h.stdarg
o._printf:      ^.h.internals
o._printf:      ^.stdh.h.stddef
o._printf:      ^.stdh.h.stdio
o._printf:      ^.stdh.h.stdarg
o._printf:      ^.stdh.h.locale
o._printf:      h.hostsys
o._printf:      ^.h.rt
o._nofp_disp:   ^.c.printf
o._nofp_disp:   h.hostsys
o._nofp_disp:   ^.stdh.h.stdio
o._nofp_disp:   ^.stdh.h.time
o._nofp_disp:   ^.stdh.h.stdio
o._nofp_disp:   ^.stdh.h.stdarg
o._nofp_disp:   ^.stdh.h.ctype
o._nofp_disp:   ^.stdh.h.string
o._nofp_disp:   h.config
o._nofp_disp:   ^.h.externals
o._nofp_disp:   ^.stdh.h.stdio
o._nofp_disp:   ^.stdh.h.stdarg
o._nofp_disp:   ^.h.internals
o._nofp_disp:   ^.stdh.h.stddef
o._nofp_disp:   ^.stdh.h.stdio
o._nofp_disp:   ^.stdh.h.stdarg
o._nofp_disp:   ^.stdh.h.locale
o._nofp_disp:   h.hostsys
o._nofp_disp:   ^.h.rt
o._fp_disp:     ^.c.printf
o._fp_disp:     h.hostsys
o._fp_disp:     ^.stdh.h.stdio
o._fp_disp:     ^.stdh.h.time
o._fp_disp:     ^.stdh.h.stdio
o._fp_disp:     ^.stdh.h.stdarg
o._fp_disp:     ^.stdh.h.ctype
o._fp_disp:     ^.stdh.h.string
o._fp_disp:     h.config
o._fp_disp:     ^.h.externals
o._fp_disp:     ^.stdh.h.stdio
o._fp_disp:     ^.stdh.h.stdarg
o._fp_disp:     ^.h.internals
o._fp_disp:     ^.stdh.h.stddef
o._fp_disp:     ^.stdh.h.stdio
o._fp_disp:     ^.stdh.h.stdarg
o._fp_disp:     ^.stdh.h.locale
o._fp_disp:     h.hostsys
o._fp_disp:     ^.h.rt
o._fp_disp:     ^.stdh.h.math
o._fprintf:     ^.c.printf
o._fprintf:     h.hostsys
o._fprintf:     ^.stdh.h.stdio
o._fprintf:     ^.stdh.h.time
o._fprintf:     ^.stdh.h.stdio
o._fprintf:     ^.stdh.h.stdarg
o._fprintf:     ^.stdh.h.ctype
o._fprintf:     ^.stdh.h.string
o._fprintf:     h.config
o._fprintf:     ^.h.externals
o._fprintf:     ^.stdh.h.stdio
o._fprintf:     ^.stdh.h.stdarg
o._fprintf:     ^.h.internals
o._fprintf:     ^.stdh.h.stddef
o._fprintf:     ^.stdh.h.stdio
o._fprintf:     ^.stdh.h.stdarg
o._fprintf:     ^.stdh.h.locale
o._fprintf:     h.hostsys
o._fprintf:     ^.h.rt
o.vsprintf:     ^.c.printf
o.vsprintf:     h.hostsys
o.vsprintf:     ^.stdh.h.stdio
o.vsprintf:     ^.stdh.h.time
o.vsprintf:     ^.stdh.h.stdio
o.vsprintf:     ^.stdh.h.stdarg
o.vsprintf:     ^.stdh.h.ctype
o.vsprintf:     ^.stdh.h.string
o.vsprintf:     h.config
o.vsprintf:     ^.h.externals
o.vsprintf:     ^.stdh.h.stdio
o.vsprintf:     ^.stdh.h.stdarg
o.vsprintf:     ^.h.internals
o.vsprintf:     ^.stdh.h.stddef
o.vsprintf:     ^.stdh.h.stdio
o.vsprintf:     ^.stdh.h.stdarg
o.vsprintf:     ^.stdh.h.locale
o.vsprintf:     h.hostsys
o.vsprintf:     ^.h.rt
o.vsprintf:     ^.h.ioguts
o.vprintf:      ^.c.printf
o.vprintf:      h.hostsys
o.vprintf:      ^.stdh.h.stdio
o.vprintf:      ^.stdh.h.time
o.vprintf:      ^.stdh.h.stdio
o.vprintf:      ^.stdh.h.stdarg
o.vprintf:      ^.stdh.h.ctype
o.vprintf:      ^.stdh.h.string
o.vprintf:      h.config
o.vprintf:      ^.h.externals
o.vprintf:      ^.stdh.h.stdio
o.vprintf:      ^.stdh.h.stdarg
o.vprintf:      ^.h.internals
o.vprintf:      ^.stdh.h.stddef
o.vprintf:      ^.stdh.h.stdio
o.vprintf:      ^.stdh.h.stdarg
o.vprintf:      ^.stdh.h.locale
o.vprintf:      h.hostsys
o.vprintf:      ^.h.rt
o.vfprintf:     ^.c.printf
o.vfprintf:     h.hostsys
o.vfprintf:     ^.stdh.h.stdio
o.vfprintf:     ^.stdh.h.time
o.vfprintf:     ^.stdh.h.stdio
o.vfprintf:     ^.stdh.h.stdarg
o.vfprintf:     ^.stdh.h.ctype
o.vfprintf:     ^.stdh.h.string
o.vfprintf:     h.config
o.vfprintf:     ^.h.externals
o.vfprintf:     ^.stdh.h.stdio
o.vfprintf:     ^.stdh.h.stdarg
o.vfprintf:     ^.h.internals
o.vfprintf:     ^.stdh.h.stddef
o.vfprintf:     ^.stdh.h.stdio
o.vfprintf:     ^.stdh.h.stdarg
o.vfprintf:     ^.stdh.h.locale
o.vfprintf:     h.hostsys
o.vfprintf:     ^.h.rt
o.sprintf:      ^.c.printf
o.sprintf:      h.hostsys
o.sprintf:      ^.stdh.h.stdio
o.sprintf:      ^.stdh.h.time
o.sprintf:      ^.stdh.h.stdio
o.sprintf:      ^.stdh.h.stdarg
o.sprintf:      ^.stdh.h.ctype
o.sprintf:      ^.stdh.h.string
o.sprintf:      h.config
o.sprintf:      ^.h.externals
o.sprintf:      ^.stdh.h.stdio
o.sprintf:      ^.stdh.h.stdarg
o.sprintf:      ^.h.internals
o.sprintf:      ^.stdh.h.stddef
o.sprintf:      ^.stdh.h.stdio
o.sprintf:      ^.stdh.h.stdarg
o.sprintf:      ^.stdh.h.locale
o.sprintf:      h.hostsys
o.sprintf:      ^.h.rt
o.sprintf:      ^.h.ioguts
o.printf:       ^.c.printf
o.printf:       h.hostsys
o.printf:       ^.stdh.h.stdio
o.printf:       ^.stdh.h.time
o.printf:       ^.stdh.h.stdio
o.printf:       ^.stdh.h.stdarg
o.printf:       ^.stdh.h.ctype
o.printf:       ^.stdh.h.string
o.printf:       h.config
o.printf:       ^.h.externals
o.printf:       ^.stdh.h.stdio
o.printf:       ^.stdh.h.stdarg
o.printf:       ^.h.internals
o.printf:       ^.stdh.h.stddef
o.printf:       ^.stdh.h.stdio
o.printf:       ^.stdh.h.stdarg
o.printf:       ^.stdh.h.locale
o.printf:       h.hostsys
o.printf:       ^.h.rt
o.fprintf:      ^.c.printf
o.fprintf:      h.hostsys
o.fprintf:      ^.stdh.h.stdio
o.fprintf:      ^.stdh.h.time
o.fprintf:      ^.stdh.h.stdio
o.fprintf:      ^.stdh.h.stdarg
o.fprintf:      ^.stdh.h.ctype
o.fprintf:      ^.stdh.h.string
o.fprintf:      h.config
o.fprintf:      ^.h.externals
o.fprintf:      ^.stdh.h.stdio
o.fprintf:      ^.stdh.h.stdarg
o.fprintf:      ^.h.internals
o.fprintf:      ^.stdh.h.stddef
o.fprintf:      ^.stdh.h.stdio
o.fprintf:      ^.stdh.h.stdarg
o.fprintf:      ^.stdh.h.locale
o.fprintf:      h.hostsys
o.fprintf:      ^.h.rt
o.image: ^.s.image
o._write_pro:   ^.c.profile
o._write_pro:   ^.stdh.h.stdio
o._write_pro:   ^.h.externals
o._write_pro:   ^.stdh.h.stdio
o._write_pro:   ^.stdh.h.stdarg
o._map_store:   ^.c.profile
o._map_store:   ^.stdh.h.stdio
o._map_store:   ^.h.externals
o._map_store:   ^.stdh.h.stdio
o._map_store:   ^.stdh.h.stdarg
o._mapstore:    ^.c.profile
o._mapstore:    ^.stdh.h.stdio
o._mapstore:    ^.h.externals
o._mapstore:    ^.stdh.h.stdio
o._mapstore:    ^.stdh.h.stdarg
o._fmapstore:   ^.c.profile
o._fmapstore:   ^.stdh.h.stdio
o._fmapstore:   ^.h.externals
o._fmapstore:   ^.stdh.h.stdio
o._fmapstore:   ^.stdh.h.stdarg
o._stfp: ^.s.mathf ^.s.h_callstd
o._set_errno: ^.s.mathf ^.s.h_callstd
o._ldfp: ^.s.mathf ^.s.h_callstd
o._ernge_mh: ^.s.mathf ^.s.h_callstd
o._ernge_h: ^.s.mathf ^.s.h_callstd
o._ernge_0: ^.s.mathf ^.s.h_callstd
o._edom_mh: ^.s.mathf ^.s.h_callstd
o.tan: ^.s.mathf ^.s.h_callstd
o.sqrt: ^.s.mathf ^.s.h_callstd
o.sin: ^.s.mathf ^.s.h_callstd
o.pow: ^.s.mathf ^.s.h_callstd
o.modf: ^.s.mathf ^.s.h_callstd
o.log10: ^.s.mathf ^.s.h_callstd
o.log: ^.s.mathf ^.s.h_callstd
o.floor: ^.s.mathf ^.s.h_callstd
o.fabs: ^.s.mathf ^.s.h_callstd
o.exp: ^.s.mathf ^.s.h_callstd
o.cos: ^.s.mathf ^.s.h_callstd
o.ceil: ^.s.mathf ^.s.h_callstd
o.atan2: ^.s.mathf ^.s.h_callstd
o.atan: ^.s.mathf ^.s.h_callstd
o.asin: ^.s.mathf ^.s.h_callstd
o.acos: ^.s.mathf ^.s.h_callstd
o.xmath:        ^.c.xmath
o.xmath:        ^.stdh.h.math
o.xmath:        ^.h.externals
o.xmath:        ^.stdh.h.stdio
o.xmath:        ^.stdh.h.stdarg
o.tanh: ^.c.math
o.tanh: h.config
o.tanh: ^.stdh.h.math
o.tanh: ^.stdh.h.stdlib
o.tanh: ^.stdh.h.limits
o.tanh: ^.stdh.h.errno
o.sinh: ^.c.math
o.sinh: h.config
o.sinh: ^.stdh.h.math
o.sinh: ^.stdh.h.stdlib
o.sinh: ^.stdh.h.limits
o.sinh: ^.stdh.h.errno
o.ldiv: ^.c.math
o.ldiv: h.config
o.ldiv: ^.stdh.h.math
o.ldiv: ^.stdh.h.stdlib
o.ldiv: ^.stdh.h.limits
o.ldiv: ^.stdh.h.errno
o.ldexp:        ^.c.math
o.ldexp:        h.config
o.ldexp:        ^.stdh.h.math
o.ldexp:        ^.stdh.h.stdlib
o.ldexp:        ^.stdh.h.limits
o.ldexp:        ^.stdh.h.errno
o.labs: ^.c.math
o.labs: h.config
o.labs: ^.stdh.h.math
o.labs: ^.stdh.h.stdlib
o.labs: ^.stdh.h.limits
o.labs: ^.stdh.h.errno
o.frexp:        ^.c.math
o.frexp:        h.config
o.frexp:        ^.stdh.h.math
o.frexp:        ^.stdh.h.stdlib
o.frexp:        ^.stdh.h.limits
o.frexp:        ^.stdh.h.errno
o.fmod: ^.c.math
o.fmod: h.config
o.fmod: ^.stdh.h.math
o.fmod: ^.stdh.h.stdlib
o.fmod: ^.stdh.h.limits
o.fmod: ^.stdh.h.errno
o.div:  ^.c.math
o.div:  h.config
o.div:  ^.stdh.h.math
o.div:  ^.stdh.h.stdlib
o.div:  ^.stdh.h.limits
o.div:  ^.stdh.h.errno
o.cosh: ^.c.math
o.cosh: h.config
o.cosh: ^.stdh.h.math
o.cosh: ^.stdh.h.stdlib
o.cosh: ^.stdh.h.limits
o.cosh: ^.stdh.h.errno
o.abs:  ^.c.math
o.abs:  h.config
o.abs:  ^.stdh.h.math
o.abs:  ^.stdh.h.stdlib
o.abs:  ^.stdh.h.limits
o.abs:  ^.stdh.h.errno
o._set8859:     ^.c.ctype
o._set8859:     ^.stdh.h.ctype
o._set8859:     ^.h.internals
o._set8859:     ^.stdh.h.stddef
o._set8859:     ^.stdh.h.stdio
o._set8859:     ^.stdh.h.stdarg
o._set8859:     ^.stdh.h.locale
o._set8859:     h.hostsys
o._set8859:     ^.stdh.h.stdio
o._set8859:     ^.stdh.h.time
o._set8859:     ^.h.rt
o.toupper:      ^.c.ctype
o.toupper:      ^.stdh.h.ctype
o.toupper:      ^.h.internals
o.toupper:      ^.stdh.h.stddef
o.toupper:      ^.stdh.h.stdio
o.toupper:      ^.stdh.h.stdarg
o.toupper:      ^.stdh.h.locale
o.toupper:      h.hostsys
o.toupper:      ^.stdh.h.stdio
o.toupper:      ^.stdh.h.time
o.toupper:      ^.h.rt
o.tolower:      ^.c.ctype
o.tolower:      ^.stdh.h.ctype
o.tolower:      ^.h.internals
o.tolower:      ^.stdh.h.stddef
o.tolower:      ^.stdh.h.stdio
o.tolower:      ^.stdh.h.stdarg
o.tolower:      ^.stdh.h.locale
o.tolower:      h.hostsys
o.tolower:      ^.stdh.h.stdio
o.tolower:      ^.stdh.h.time
o.tolower:      ^.h.rt
o.isxdigit:     ^.c.ctype
o.isxdigit:     ^.stdh.h.ctype
o.isxdigit:     ^.h.internals
o.isxdigit:     ^.stdh.h.stddef
o.isxdigit:     ^.stdh.h.stdio
o.isxdigit:     ^.stdh.h.stdarg
o.isxdigit:     ^.stdh.h.locale
o.isxdigit:     h.hostsys
o.isxdigit:     ^.stdh.h.stdio
o.isxdigit:     ^.stdh.h.time
o.isxdigit:     ^.h.rt
o.isupper:      ^.c.ctype
o.isupper:      ^.stdh.h.ctype
o.isupper:      ^.h.internals
o.isupper:      ^.stdh.h.stddef
o.isupper:      ^.stdh.h.stdio
o.isupper:      ^.stdh.h.stdarg
o.isupper:      ^.stdh.h.locale
o.isupper:      h.hostsys
o.isupper:      ^.stdh.h.stdio
o.isupper:      ^.stdh.h.time
o.isupper:      ^.h.rt
o.isspace:      ^.c.ctype
o.isspace:      ^.stdh.h.ctype
o.isspace:      ^.h.internals
o.isspace:      ^.stdh.h.stddef
o.isspace:      ^.stdh.h.stdio
o.isspace:      ^.stdh.h.stdarg
o.isspace:      ^.stdh.h.locale
o.isspace:      h.hostsys
o.isspace:      ^.stdh.h.stdio
o.isspace:      ^.stdh.h.time
o.isspace:      ^.h.rt
o.ispunct:      ^.c.ctype
o.ispunct:      ^.stdh.h.ctype
o.ispunct:      ^.h.internals
o.ispunct:      ^.stdh.h.stddef
o.ispunct:      ^.stdh.h.stdio
o.ispunct:      ^.stdh.h.stdarg
o.ispunct:      ^.stdh.h.locale
o.ispunct:      h.hostsys
o.ispunct:      ^.stdh.h.stdio
o.ispunct:      ^.stdh.h.time
o.ispunct:      ^.h.rt
o.isprint:      ^.c.ctype
o.isprint:      ^.stdh.h.ctype
o.isprint:      ^.h.internals
o.isprint:      ^.stdh.h.stddef
o.isprint:      ^.stdh.h.stdio
o.isprint:      ^.stdh.h.stdarg
o.isprint:      ^.stdh.h.locale
o.isprint:      h.hostsys
o.isprint:      ^.stdh.h.stdio
o.isprint:      ^.stdh.h.time
o.isprint:      ^.h.rt
o.islower:      ^.c.ctype
o.islower:      ^.stdh.h.ctype
o.islower:      ^.h.internals
o.islower:      ^.stdh.h.stddef
o.islower:      ^.stdh.h.stdio
o.islower:      ^.stdh.h.stdarg
o.islower:      ^.stdh.h.locale
o.islower:      h.hostsys
o.islower:      ^.stdh.h.stdio
o.islower:      ^.stdh.h.time
o.islower:      ^.h.rt
o.isgraph:      ^.c.ctype
o.isgraph:      ^.stdh.h.ctype
o.isgraph:      ^.h.internals
o.isgraph:      ^.stdh.h.stddef
o.isgraph:      ^.stdh.h.stdio
o.isgraph:      ^.stdh.h.stdarg
o.isgraph:      ^.stdh.h.locale
o.isgraph:      h.hostsys
o.isgraph:      ^.stdh.h.stdio
o.isgraph:      ^.stdh.h.time
o.isgraph:      ^.h.rt
o.isdigit:      ^.c.ctype
o.isdigit:      ^.stdh.h.ctype
o.isdigit:      ^.h.internals
o.isdigit:      ^.stdh.h.stddef
o.isdigit:      ^.stdh.h.stdio
o.isdigit:      ^.stdh.h.stdarg
o.isdigit:      ^.stdh.h.locale
o.isdigit:      h.hostsys
o.isdigit:      ^.stdh.h.stdio
o.isdigit:      ^.stdh.h.time
o.isdigit:      ^.h.rt
o.iscntrl:      ^.c.ctype
o.iscntrl:      ^.stdh.h.ctype
o.iscntrl:      ^.h.internals
o.iscntrl:      ^.stdh.h.stddef
o.iscntrl:      ^.stdh.h.stdio
o.iscntrl:      ^.stdh.h.stdarg
o.iscntrl:      ^.stdh.h.locale
o.iscntrl:      h.hostsys
o.iscntrl:      ^.stdh.h.stdio
o.iscntrl:      ^.stdh.h.time
o.iscntrl:      ^.h.rt
o.isalpha:      ^.c.ctype
o.isalpha:      ^.stdh.h.ctype
o.isalpha:      ^.h.internals
o.isalpha:      ^.stdh.h.stddef
o.isalpha:      ^.stdh.h.stdio
o.isalpha:      ^.stdh.h.stdarg
o.isalpha:      ^.stdh.h.locale
o.isalpha:      h.hostsys
o.isalpha:      ^.stdh.h.stdio
o.isalpha:      ^.stdh.h.time
o.isalpha:      ^.h.rt
o.isalnum:      ^.c.ctype
o.isalnum:      ^.stdh.h.ctype
o.isalnum:      ^.h.internals
o.isalnum:      ^.stdh.h.stddef
o.isalnum:      ^.stdh.h.stdio
o.isalnum:      ^.stdh.h.stdarg
o.isalnum:      ^.stdh.h.locale
o.isalnum:      h.hostsys
o.isalnum:      ^.stdh.h.stdio
o.isalnum:      ^.stdh.h.time
o.isalnum:      ^.h.rt
o.ctype:        ^.c.ctype
o.ctype:        ^.stdh.h.ctype
o.ctype:        ^.h.internals
o.ctype:        ^.stdh.h.stddef
o.ctype:        ^.stdh.h.stdio
o.ctype:        ^.stdh.h.stdarg
o.ctype:        ^.stdh.h.locale
o.ctype:        h.hostsys
o.ctype:        ^.stdh.h.stdio
o.ctype:        ^.stdh.h.time
o.ctype:        ^.h.rt
