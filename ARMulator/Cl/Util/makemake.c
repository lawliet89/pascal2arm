/* C Library makefile make utility
 * Copyright (C) 1991 Advanced Risc Machines Ltd. All rights reserved.
 */

/*
 * RCS $Revision: 1.24.4.1 $
 * Checkin $Date: 1998/03/16 19:14:32 $
 * Revising $Author: rivimey $
 */

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#ifdef __STDC__
#  include <stdlib.h>
#else
extern void exit();
extern char *malloc();
extern int system();
#endif

#ifdef unix
#define __unix 1
#endif

#ifdef __linux__
#undef false
#undef true
#endif

#ifdef HOST_DEFINED
#undef HOST_DEFINED
#endif

#ifdef __unix
#define MaxFilesPerDir      0
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              "/"
static char n_lib[] = "armlib.o";
#define MAKE_CMD            "make"
#define MAKEDEFS_FILE       "mdef_sun"
#define HOST_DEFINED 1
#endif

#ifdef __ZTC__
#define MaxFilesPerDir      0
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              "\\"
static char n_lib[] = "armlib.o";
#define MAKE_CMD            "make"
#define MAKEDEFS_FILE       "mdef_ztc"
#define HOST_DEFINED 1
#define DOUBLE_QUOTES 1
#endif

#ifdef __riscos
#define MaxFilesPerDir      77
#define FNAME_SUFFIXES      "c s h o o1 o2 o3 o4 o5 o6"
#define ObjectsInSeparateDirectory 1
#define DirSep              "."
static char n_lib[] = "lib.AnsiLib";
static char n_libdir[] = "lib";
#define MAKE_CMD            "amu"
#define MAKEDEFS_FILE       "mdef_ros"
#define HOST_DEFINED 1
#endif

#ifdef macintosh
#define MaxFilesPerDir      0
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              ":"
static char n_lib[] = "armlib.o";
#define MAKE_CMD            "make"
#define MAKEDEFS_FILE       "mdef_mac"
#define HOST_DEFINED 1
#define EXTEND_DEP          " ¶\n\t"
#define EXTEND2_DEP         " ¶\n"
#endif

#ifdef _MSC_VER
#define MaxFilesPerDir      0
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              "\\"
static char n_lib[] = "armlib.o";
#define MAKE_CMD            "armmake"
#define MAKEDEFS_FILE       "mdef_pc"
#define HOST_DEFINED 1
#define EXTEND_DEP          "\\\n\t"
#define EXTEND2_DEP         "\\\n"
#  ifndef WRITE_WINDOWS_APJ
#    define DOUBLE_QUOTES 1
#  endif
#endif

#ifdef __WATCOMC__
#define MaxFilesPerDir      0
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              "\\"
static char n_lib[] = "armlib.o";
#define MAKE_CMD            "wmake"
#define MAKEDEFS_FILE       "mdef_wat"
#define HOST_DEFINED 1
#define EXTEND_DEP          "&\n\t"
#define EXTEND2_DEP         "&\n"
#  ifndef WRITE_WINDOWS_APJ
#    define DOUBLE_QUOTES 1
#  endif
#endif

#ifdef WRITE_WINDOWS_APJ
#define FNAME_SUFFIXES      "c s h o"
#define DirSep              "\\"
#define HOST_DEFINED 1
#endif

#ifdef macintosh
#define ALL_DEP             "all Ä\t%s\n\n"
#define CLEAN_DEP           "clean Ä\t$OutOfDate\n\t{RM} %s "
#define OBJECT_DEP          "\n%s Ä\t"
#define LIB_DEP             "\n\t\t{RM} %s\n\t\t{ARMLIB} -co %s -v %s\n\n"
#define OFILE_DEP           "%s Ä\t %s\n"
#define NOT_OFILE_DEP       "%s Ä\t$OutOfDate%s\n"
#define DEPEND_DEP          "\ndepend Ä"
#define MAKEMAKE_DEP        "\n\t{MAKEMAKE} -edit Makefile {BAKFILE} {DEPLIST}\n\n"
#define D001_DEP            "d_%03x Ä\t%s\n"
#else
#define ALL_DEP             "all:\t%s\n\n"
#define CLEAN_DEP           "clean:;\t-$(RM) %s "
#define OBJECT_DEP          "\n%s:\t"
#define LIB_DEP             "\n\t\t$(RM) %s\n\t\t$(ARMLIB) -co %s -v %s\n\n"
#define OFILE_DEP           "%s: %s;"
#define NOT_OFILE_DEP       "%s:;%s"
#define DEPEND_DEP          "\ndepend:"
#define MAKEMAKE_DEP        "\n\t\t$(MAKEMAKE) -edit Makefile $(BAKFILE) $(DEPLIST)\n\n"
#define D001_DEP            "d_%03x:\t%s\n"
#endif

/* default macros */
#ifndef EXTEND_DEP
#  define EXTEND_DEP        " \\\n\t"
#endif
#ifndef EXTEND2_DEP
#  define EXTEND2_DEP       "\\\n"
#endif
#ifndef MAKEDEFS_FILE
#  define MAKEDEFS_FILE     "makedefs"
#endif


#ifdef HOST_DEFINED

/* also includes fname.h */
#include "fname.c"

#define MaxFName 512
#define MAXLINE  256

static char n_files[]    = "sources";
static char n_options[]  = "options";
static char n_makedefs[] = MAKEDEFS_FILE;
static char n_mfile[]    = "Makefile";
static char n_lfile[]    = "lfile";

typedef struct MakeListEntry {
    struct MakeListEntry *next;
    char *object;
    char *source;
    char *options;
    char *others;
    char type;
} MakeListEntry;

static char *self;

static char *sourcefile;
static int sourceline;

static void FatalError(message, arg)
char *message;
char *arg;
{
    fprintf(stderr, "%s: %s%s\n", self, message, arg);
    exit(1);
}

static void Warn(message, arg)
char *message;
char *arg;
{
    fprintf(stderr, "%s: %s line %d: warning: %s%s\n",
        self, sourcefile, sourceline, message, arg);
}

static char *HeapString(b)
char *b;
{
    int l = strlen(b);
    char *r = (char *)malloc(l+1);
    memcpy(r, b, l+1);
    return r;
}

static char *FileName(path, name)
char *path;
char *name;
{
    char b[256];
    strcpy(b, path);
    strcat(b, DirSep);
    strcat(b, name);
    return HeapString(b);
}

static int FileExists(path, name)
char *path;
char *name;
{
    FILE *f = fopen(FileName(path, name), "r");
    if (f == NULL) return 0;
    fclose(f);
    return 1;
}

#ifdef macintosh
# define CMakeRule "\t{CC} {CFLAGS} %s -o %s %s\n"
# define AMakeRule "\t{AS} {AFLAGS} %s -o %s %s\n"
# define OMakeRule "\t{CP} %s %s\n"
# define DCMakeRule \
    "\t{CC} {DCFLAGS} %s -o %s %s >>{DEPLIST}\n"
# define DAMakeRule \
    "\t{AS} {DAFLAGS} %s -o %s %s >>{DEPLIST}\n"
#else
# define CMakeRule "\t$(CC) $(CFLAGS) %s -o %s %s\n"
# define AMakeRule "\t$(AS) $(AFLAGS) %s -o %s %s\n"
# define OMakeRule "\t$(CP) %s %s\n"
# define DCMakeRule \
    "\t$(CC) $(DCFLAGS) %s -o %s %s >> $(DEPLIST)\n"
# define DAMakeRule \
    "\t$(AS) $(DAFLAGS) %s -o %s %s\n\t$(APPEND) $(MKTMP) >> $(DEPLIST)\n"
#endif
typedef enum {
    AFile,
    CFile,
    OFile,
    XFile
} FileType;

static int Prefix(p, s)
char *p;
char *s;
{
    int i = 0;
    for (; *p != 0 && *p == s[i++]; p++)
        /* nothing */;
    return (*p == 0) ? i : 0;
}

static char *UpName(b, dir, old)
char *b;
char *dir;
char *old;
{
    UnparsedName unparse;
    char temp[MaxFName];

    strcpy(&temp[3], old);
    memcpy( &temp[0], "../", 3 );
    fname_parse( temp, FNAME_SUFFIXES, &unparse );
    fname_unparse( &unparse, FNAME_AS_NAME, b, MaxFName );
    return b;
}

static FileType FileTypeOf(name)
char *name;
{
    UnparsedName unparse;
    fname_parse(name, FNAME_SUFFIXES, &unparse);
    if (unparse.elen == 1) {
        char ch = *unparse.extn;
        if (isalpha(ch)) {
            ch = tolower(ch);
            if (ch == 'c') return CFile;
            if (ch == 's') return AFile;
            if (ch == 'o') return OFile;
        }
    }
    return XFile;
}

#ifdef ObjectsInSeparateDirectory

static void CreateDirectory(dir, name)
char *dir;
char *name;
{
    char command[256];
    sprintf(command, "cdir %s.%s", dir, name);
    system(command);
}

static int GetObjectDirName(b, no)
char *b;
int no;
{
    *b++ = 'o'; *b = 0;
    if (MaxFilesPerDir > 0) {
        int dirno = no / MaxFilesPerDir;
        no = no % MaxFilesPerDir;
        if (dirno > 0) {
            sprintf(b, "%d", dirno);
        }
    }
    return no;
}

#endif

static void GetObjectName(b, dir, file, no)
char *b;
char *dir;
char *file;
int no;
{
    char objdir[4];
    UnparsedName unparse;

    memset( &unparse, 0, sizeof( UnparsedName ) ); /* IMPORTANT */
#ifdef ObjectsInSeparateDirectory
    if (GetObjectDirName(objdir, no) == 0)
        CreateDirectory(dir, objdir);
#else
    objdir[0] = 'o'; objdir[1] = 0;
#endif
    unparse.root = file; unparse.rlen = strlen(file);
    unparse.extn = objdir; unparse.elen = strlen(objdir);
    unparse.path = NULL; unparse.plen = 0;
    unparse.type = FNAME_ACORN; unparse.pathsep = '.';
    fname_unparse(&unparse, FNAME_AS_NAME, b, MaxFName);
}

static int SkipToNL(f, ch)
FILE *f;
int ch;
{
    do ch = fgetc(f); while (ch != '\n' && ch != EOF);
    return ch;
}

static int GetCh(f)
FILE *f;
{
    int ch = fgetc(f);
    while (ch == '#')
    {
        ch = fgetc(f);
        if (ch == '#')
        {
            (void)SkipToNL(f, ch); /* returns newline character! */
            ch = fgetc(f);
        }
        else
        {
            ungetc(ch, f);
            return '#';
        }
    }
    return ch;
}

static int GetFileName(f, b, ch)
FILE *f;
char *b;
int ch;
{
    do {
        *b++ = ch;
        ch = GetCh(f);
    } while (!isspace(ch) && ch != EOF);
    *b = 0;
    return ch;
}

static int ReadWord(f, b, ch)
FILE *f;
char *b;
int ch;
{
    while (isalnum(ch) || ch == '_') {
        *b++ = ch;
        ch = GetCh(f);
    }
    *b = 0;
    return ch;
}

static int SkipSpaces(f, ch)
FILE *f;
int ch;
{
    while (ch == ' ' || ch == '\t')
        ch = GetCh(f);
    return ch;
}

static int ExpectNL(f, ch, key)
FILE *f;
int ch;
char *key;
{
    ch = SkipSpaces(f, ch);
    if (ch != '\n' && ch != EOF) {
        Warn("junk after #", key);
        ch = SkipToNL(f, ch);
    }
    return ch;
}

typedef enum { If, Else, Elif, End, Invalid } HashKey;

static char * HashKeyWords[] = { "if", "else", "elif", "end", 0 };

static HashKey HashKeyWord(b)
char *b;
{
    HashKey i;
    for (i = If; i != Invalid; i = (HashKey)(i+1))
        if (strcmp(b, HashKeyWords[i]) == 0) break;
    return i;
}

typedef struct Var {
    struct Var *next;
    char *name;
    char *val;
} Var;

static Var *vars;

static Var *LookUpWord(b)
char *b;
{
    Var *vp;
    for (vp = vars; vp != NULL; vp = vp->next)
        if (strcmp(b, vp->name) == 0) return vp;
    return NULL;
}

static void ReadOptions(dir)
char *dir;
{
    char *file = FileName(dir, n_options);
    FILE *opts = fopen(file, "r");
    char b[MaxFName];
    int ch;

    vars = NULL;
    if (opts == NULL) return;
    sourcefile = file;
    sourceline = 0;
    while ((ch = GetCh(opts)) != EOF) {
        sourceline++;
        ch = SkipSpaces(opts, ch);
        ch = ReadWord(opts, b, ch);
        {   Var *vp = LookUpWord(b);
            if (vp == NULL) {
                vp = (Var *)malloc(sizeof(*vp));
                vp->next = vars;
                vp->name = HeapString(b);
                vp->val = NULL;
                vars = vp;
            }
            ch = SkipSpaces(opts, ch);
            if (ch != '=') {
                Warn("Bad operator", "");
                ch = SkipToNL(opts, ch);
            } else {
                ch = SkipSpaces(opts, GetCh(opts));
                ch = ReadWord(opts, b, ch);
                vp->val = HeapString(b);
                ch = SkipSpaces(opts, ch);
                if (ch != '\n' && ch != EOF) {
                    Warn("junk at end of line", "");
                    ch = SkipToNL(opts, ch);
                }
            }
        }
    }
    fclose(opts);
}


typedef enum { false, true, bad } ExprVal;

typedef enum { equal, notequal, none } Comparison;

#ifdef __STDC__
static int ReadAndEvalExpr(FILE *f, int ch, ExprVal *val);
#else
static int ReadAndEvalExpr(/* FILE *f, int ch, ExprVal *val */);
#endif

static int ReadExprPrimary(f, ch, val)
FILE *f;
int ch;
ExprVal *val;
{
    char *emess = NULL;
    ch = SkipSpaces(f, ch);
    if (ch == '(') {
        ch = ReadAndEvalExpr(f, GetCh(f), val);
        if (*val == bad) return ch;
        ch = SkipSpaces(f, ch);
        if (ch != ')') { emess = "missing ')'"; goto Error; }
    } else {
        Comparison op = none;
        char b[MaxFName];
        Var *vp;
        ch = ReadWord(f, b, ch);
        vp = LookUpWord(b);
        ch = SkipSpaces(f, ch);
        if (ch == '=') {
            if ((ch = GetCh(f)) == '=') op = equal;
        } else if (ch == '!') {
            if ((ch = GetCh(f)) == '=') op = notequal;
        }
        if (op == none) { emess = "bad comparison operator"; goto Error; }
        ch = ReadWord(f, b, SkipSpaces(f, GetCh(f)));
        if (vp == NULL)
            *val = (ExprVal)(op == notequal);
        else
            *val = (ExprVal)((vp->val != NULL && strcmp(b, vp->val) == 0) ^
                             (op == notequal));
        return ch;
    }
    return ch;
Error:
    Warn(emess, " in expression");
    *val = bad;
    return ch;
}

static int ReadAndEvalExpr(f, ch, val)
FILE *f;
int ch;
ExprVal *val;
{
    ch = ReadExprPrimary(f, ch, val);
    if (*val == bad) return ch;
    for (;;) {
        ch = SkipSpaces(f, ch);
        if (ch == '&') {
            ch = GetCh(f);
            if (ch == '&') {
                ExprVal val2;
                ch = ReadExprPrimary(f, GetCh(f), &val2);
                if (val2 == bad) {
                    *val = bad;
                    return ch;
                }
                *val = (ExprVal)(*val & val2);
                continue;
            }
        }
        else if (ch == '|') {
            ch = GetCh(f);
            if (ch == '|') {
                ExprVal val2;
                ch = ReadExprPrimary(f, fgetc(f), &val2);
                if (val2 == bad) {
                    *val = bad;
                    return ch;
                }
                *val = (ExprVal)(*val | val2);
                continue;
            }
        }
        else if (ch == '\n' || ch == EOF)
            return ch;

        Warn("Bad operator", "");
        *val = bad;
        return ch;
    }
}

typedef enum { off, toelse, toend } Skip;

static MakeListEntry *ReadSources(dir)
char *dir;
{
    MakeListEntry *makelist = NULL;
    MakeListEntry **makelistp = &makelist;

    char *filesname = FileName(dir, n_files);
    FILE *source = fopen(filesname, "r");
    int ch;
    char b[MaxFName*2];
    Skip skipping = off;
    int skipdepth = 0, ifdepth = 0;
    if (source == NULL) FatalError("can't read ", filesname);

    sourcefile = filesname;
    sourceline = 0;
    while ((ch = GetCh(source)) != EOF) {
        sourceline++;
        if (ch == '#') {
            ch = SkipSpaces(source, GetCh(source));
            ch = ReadWord(source, b, ch);
            switch (HashKeyWord(b)) {
            case If:
                ifdepth++;
                if (skipping != off) {
                    skipdepth++;
                    ch = SkipToNL(source, ch);
                } else {
                    ExprVal val;
                    ch = ReadAndEvalExpr(source, ch, &val);
                    if (val != true) skipping = toelse; /* bad as well */
                    if (val == bad)
                        ch = SkipToNL(source, ch);
                    else
                        ch = ExpectNL(source, ch, "if");
                }
                break;
            case Else:
                if (skipping == off)
                    skipping = toend;
                else if (skipdepth == 0 && skipping == toelse)
                    skipping = off;
                ch = ExpectNL(source, ch, "else");
                break;
            case Elif:
                if (skipping == off)
                    skipping = toend;
                else if (skipdepth == 0 && skipping == toelse) {
                    ExprVal val;
                    ch = ReadAndEvalExpr(source, ch, &val);
                    if (val == true) skipping = off;
                    if (val == bad)
                        ch = SkipToNL(source, ch);
                    else
                        ch = ExpectNL(source, ch, "elif");
                }
                break;
            case End:
                if (--ifdepth < 0) Warn("unmatched #end", "");
                if (skipdepth == 0)
                    skipping = off;
                else
                    skipdepth--;
                ch = ExpectNL(source, ch, "end");
                break;
            default:
                Warn("unexpected # keyword", "");
                ch = SkipToNL(source, ch);
                break;
            }
        } else if (skipping != off) {
            ch = SkipToNL(source, ch);
        } else {
            MakeListEntry *e = (MakeListEntry *)malloc(sizeof(*e));
            *makelistp = e;
            makelistp = &e->next;
            e->next = 0; e->options = ""; e->others = NULL;
            ch = GetFileName(source, b, ch);
            e->object = HeapString(b);
            if (ch == EOF) FatalError("premature EOF at ", e->object);

            ch = GetFileName(source, b, SkipSpaces(source, ch));
            e->source = HeapString(b);

            ch = SkipSpaces(source, ch);
            if (ch == '[') {
                int i = 0;
                while ((ch = GetCh(source)) != ']' && ch != EOF)
                {
#ifdef macintosh
                    if (ch == '\\') ch = '¶';
#endif
#ifdef DOUBLE_QUOTES
                    if (ch == '\"') b[i++] = '\\'; /* add \ before " */
                    if (ch == '\'') ch = '\"';
#endif
                    b[i++] = ch;
                }
                b[i] = 0;
                ch = SkipSpaces(source, GetCh(source));
                e->options = HeapString(b);
            }
            if (ch != '\n' && ch != EOF) {
                int i = 0;
                do {
                    b[i++] = ch;
                    ch = GetCh(source);
                } while (ch != '\n' && ch != EOF);
                b[i] = 0;
                e->others = HeapString(b);
            }
        }
        if (ch == EOF) break;
    }
    if (ifdepth != 0) Warn("missing #end", "");
    fclose(source);
    return makelist;
}

#ifndef WRITE_WINDOWS_APJ

static char *start_deps =
    "# START OF AUTO-GENERATED DEPENDENCIES\n";
static char *end_deps =
    "# END OF AUTO-GENERATED DEPENDENCIES\n";

static void edit_dependencies(n_makef, n_backf, n_deplist)
char *n_makef;
char *n_backf;
char *n_deplist;
{
    FILE *makef, *backf, *depf;
    int ch, copy;
    char line[MAXLINE];
    char last[MAXLINE];

    if ((makef = fopen(n_makef, "r")) == NULL)
        FatalError("Can't open file ", n_makef);

    if ((backf = fopen(n_backf, "w")) == NULL)
        FatalError("Can't create file ", n_backf);

    if ((depf = fopen(n_deplist, "r")) == NULL)
        FatalError("Can't open file ", n_deplist);

    for (;;)
    {   ch = getc(makef);
        if (ch == EOF) break;
        putc(ch, backf);
    }

    if ((backf = freopen(n_backf, "r", backf)) == NULL)
        FatalError("Can't re-read file ", n_backf);

    if ((makef = freopen(n_makef, "w", makef)) == NULL)
        FatalError("Can't re-write file ", n_makef);

    copy = 1;
    for (;;)
    {   if (fgets(line, MAXLINE, backf) == NULL ||
            strcmp(line, end_deps) == 0) break;
        if (copy) fputs(line, makef);
        if (strcmp(line, start_deps) == 0) copy = 0;
    }

#ifdef macintosh
    while( fgets( line, MAXLINE, depf ) != NULL )
        {
                fputs( line, makef );
        }
#else
    strcpy(last, ":");
    copy = 0;
    for (;;)
    {   char *s;
        if (fgets(line, MAXLINE, depf) == NULL) break;
        if (strstr(line, last) == line)
        {   int l;
            s = line + strlen(last);
            if (s[0] == '\t') s[0] = ' ';
            l = strlen(s);
            if ((l + copy) < 72)
            {   s[l-1] = 0;
                fputs(s, makef);
                copy += l;
                continue;
            }
        }
        fputc('\n', makef);
        copy = strlen(line)-1;
        line[copy] = 0;
        fputs(line, makef);
        if ((s = strchr(line, ':')) != NULL)
        {   s[1] = 0;
            strcpy(last, line);
        }
        else
            strcpy(last, ":");
    }
    fputs("\n\n", makef);
#endif

    fputs(end_deps, makef);

    for (;;)
    {   if (fgets(line, MAXLINE, backf) == NULL) break;
        fputs(line, makef);
    }

    fclose(makef);
    fclose(backf);
    fclose(depf);
}

static void WriteMakeFile(makelist, dir)
MakeListEntry *makelist;
char *dir;
{
    char b[MaxFName*2];

    char *makefilename = FileName(dir, n_mfile);
    char *libfilename = FileName(dir, n_lfile);

    FILE *makefile = fopen(makefilename, "w");
    FILE *libfile = fopen(libfilename, "w");

    MakeListEntry *e;
    int objectno;
    int l;

    if (makefile == NULL) FatalError("can't write ", makefilename);
    if (libfile == NULL) FatalError("can't write ", libfilename);

#ifdef ObjectsInSeparateDirectory
    CreateDirectory(dir, n_libdir);
#endif
    {   char *makedef_fn = FileName(dir, n_makedefs);
        FILE *makedef_f = fopen(makedef_fn, "r");
        if (makedef_f == NULL) FatalError("can't read ", makedef_fn);
        for (;;) {
            int ch = GetCh(makedef_f);
            if (ch == EOF) break;
            fputc(ch, makefile);
        }
        fclose(makedef_f);
    }

#ifdef __ZTC__
    fputs(start_deps, makefile);
    fputs(end_deps, makefile);
#endif

    fprintf(makefile, ALL_DEP, n_lib);

    fprintf(makefile, CLEAN_DEP, n_lib);
#ifdef ObjectsInSeparateDirectory
    fputs("o*.*\n", makefile);
#else
    fputs("*.o\n", makefile);
#endif

    fprintf(makefile, OBJECT_DEP, n_lib);
    l = 100;

    for (e = makelist, objectno = 0; e != NULL; e = e->next, objectno++) {
        int n;
        e->type = FileTypeOf(e->source);
        if (e->type == OFile) {
            objectno--;    /* one fewer to make */
        }
        GetObjectName(b, dir, e->object, objectno);
        n = strlen(b);
        if (l + n + 1 > 80) {
            fputs( EXTEND_DEP, makefile);
            l = 8;
        } else {
            fputc(' ', makefile);
        }
        l += n+1;
        fputs(b, makefile);
    }
    fprintf(makefile,
         LIB_DEP,
         n_lib, n_lib, n_lfile);

    for (e = makelist, objectno = 0; e != NULL; e = e->next, objectno++) {
        int objlen;
        if (e->type == OFile) objectno--;
        GetObjectName(b, dir, e->object, objectno);
        objlen = strlen(b)+1;
        fprintf(libfile, "%s\n", b);
        (void)UpName(&b[objlen], dir, e->source);
        if (e->type == OFile) {
            fprintf(makefile, OFILE_DEP, b, &b[objlen]);
        } else {
            fprintf(makefile, NOT_OFILE_DEP, b, (objlen > 6 ? "" : "\t"));
        }
        if (e->type == CFile)
            fprintf(makefile, CMakeRule, e->options, b, &b[objlen]);
        else if (e->type == AFile)
            fprintf(makefile, AMakeRule, e->options, b, &b[objlen]);
        else if (e->type == OFile)
            fprintf(makefile, OMakeRule, &b[objlen], b);
    }

    {   int i; int l;
        fprintf(makefile, DEPEND_DEP);
        for (l = 15, i = 0; i < objectno; i++, l += 6) {
            if (l >= 80) { fputs(EXTEND2_DEP, makefile); l = 15; }
            fputc((l == 15 ? '\t' : ' '), makefile);
            fprintf(makefile, "d_%03x", i);
        }
        fputs(MAKEMAKE_DEP, makefile);
    }
    for (e = makelist, objectno = 0; e != NULL; e = e->next, objectno++)
        if (e->type == OFile)
            objectno--;
        else {
            int objlen;
            GetObjectName(b, dir, e->object, objectno);
            objlen = strlen(b)+1;
            fprintf(makefile, D001_DEP,
                     objectno, UpName(&b[objlen], dir, e->source));
            if (e->type == CFile)
                fprintf(makefile, DCMakeRule, e->options, b, &b[objlen]);
            else if (e->type == AFile)
                fprintf(makefile, DAMakeRule, e->options, b, &b[objlen]);
        }
#ifndef __ZTC__
    fputc('\n', makefile);
    fputs(start_deps, makefile);
    fputs(end_deps, makefile);
#endif

    fclose(makefile);
    fclose(libfile);

}

#else /* WRITE_WINDOWS_APJ */

static void WriteCountedString(fp, s)
FILE *fp;
char *s;
{
  fputc((unsigned char)strlen(s), fp);
  fputs(s, fp);
}  

static void WriteDWORD(fp, n)
FILE *fp;
unsigned int n;
{
  fputc(n & 0xFF, fp);
  n >>= 8;
  fputc(n & 0xFF, fp);
  n >>= 8;
  fputc(n & 0xFF, fp);
  n >>= 8;
  fputc(n & 0xFF, fp);
}  

static void WriteWORD(fp, n)
FILE *fp;
unsigned int n;
{
  fputc(n & 0xFF, fp);
  n >>= 8;
  fputc(n & 0xFF, fp);
}  

static void WriteAPJ(makelist, dir)
MakeListEntry *makelist;
char *dir;
{
    char *apjname = FileName(dir, "CLIB.APJ");
    FILE *fp = fopen(apjname, "wb");
    MakeListEntry *e;
        int count;

    if (fp == NULL) FatalError("can't write ", apjname);
         
        count = 0;
        for (e = makelist; e != NULL; e = e->next)
            count++;

        /* Write out APJ version string */
    WriteCountedString(fp, "0003");

        /* Write source filename list */
    WriteWORD(fp, count);
        for (e = makelist; e != NULL; e = e->next)
        {
            char buf[256];

        UpName(&buf, dir, e->source);
            WriteCountedString(fp, buf);
        }

    /* Write params list */
    WriteWORD(fp, count); 
        for (e = makelist; e != NULL; e = e->next)
        {
            char buf[256];

                strcpy(buf, "-o ");
        GetObjectName(&buf[3], dir, e->object, 0 /* object number used for o1 o2 etc */);
            strcat(buf, " ");
            strcat(buf, e->options);
            WriteCountedString(fp, buf);
        }

    /* armcc, armasm, armlink flags */
        WriteDWORD(fp, 0);
    WriteDWORD(fp, 0);
        WriteDWORD(fp, 0);

    /* armcc, armasm, armlink global options */
        WriteCountedString(fp, "-I.. -I. -DHOSTSEX_l -I..\\fplib");
        WriteCountedString(fp, "-I.. -I.");
        WriteCountedString(fp, "");

    WriteWORD(fp, 0);   /* pcc bool flag */
    WriteWORD(fp, 0);   /* ??? bool flag */
    WriteWORD(fp, 0);   /* nocache bool flag */
    WriteWORD(fp, 0);   /* noesc bool flag */
    WriteWORD(fp, 0);   /* nowarn bool flag */
    WriteWORD(fp, 0);   /* 32bit bool flag */
    WriteWORD(fp, 0);   /* compiler flags */
    WriteWORD(fp, 0);   /* processor type number */
        WriteCountedString(fp, "");       /* command line */
        WriteWORD(fp, 0);   /* full dependency scan */
        WriteWORD(fp, 1);   /* little endian */
        WriteWORD(fp, 0);   /* debug */
        WriteCountedString(fp, "ARM Object Library");   /* project type */
        WriteWORD(fp, 0);   /* build all flag */
        WriteWORD(fp, 0);   /* no optimise flag */
        WriteWORD(fp, 2);   /* optimisation type */

    fclose(fp);
}

#endif


int main(argc, argv)
int argc;
char **argv;
{
    char *host = NULL, *target = NULL;

    self = argv[0];

    if (argc < 2) FatalError("what target?", "");

#ifndef WRITE_WINDOWS_APJ
    if (argc == 5 && strcmp(argv[1], "-edit") == 0)
    {   edit_dependencies(argv[2], argv[3], argv[4]);
        return 0;
    }
#endif

    target = argv[1];
    host = argv[(argc < 3) ? 1 : 2];

    ReadOptions(FileExists(host, n_options) ? host : target);

    {   MakeListEntry *makelist = ReadSources(target);

#ifdef WRITE_WINDOWS_APJ
        WriteAPJ(makelist, host);
#else
        WriteMakeFile(makelist, host);
#endif
    }
    return 0;
}

#else /* HOST_DEFINED */

int main(argc, argv)
int argc;
char **argv;
{
    fprintf(stderr, "\nmakemake: host system not recognised.\n");
    fprintf(stderr, "Please examine makemake.c and adapt it as necessary.\n\n");
    return 1;
}

#endif /* HOST_DEFINED */
