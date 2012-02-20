/*
 * Copyright (C) 1991-95 Advanced RISC Machines Limited. All rights reserved.
 */

/*
 * RCS $Revision: 1.1.2.1 $
 * Checkin $Date: 1997/08/14 11:30:50 $
 * Revising $Author: ijohnson $
 */


#include <stdio.h>

int getData(char *buffer,int length)
{
        int charsIn;
        int charRead;
        charsIn=0;
        printf("Please enter text to be uuencoded, ending input with RETURN: ");
        for (charsIn=0;charsIn<length;) {
                charRead=getchar();
                if (charRead == EOF) break;
                buffer[charsIn++]=charRead;
                if (charRead == '\n' ) break;
        }
        for (;(charsIn%3)!=0; ) buffer[charsIn++]='\0';
        return charsIn;
}
