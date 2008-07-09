/* $Id: f2p.c,v 1.1.1.1 2004/09/13 21:07:48 mstrout Exp $ */

#include <stdio.h>
#include <unistd.h>
#ifndef WIN32
#include <sys/wait.h>
#endif
#include <petit/definitions.h>
#include <petit/petit_args.h>
#include <petit/Exit.h>

namespace omega {
void convert_to_petit()
    {
#ifndef WIN32
    if(petit_args.FileName)
        {
        int i;
        for (i=0; petit_args.FileName[i]; i++);

        if (i>=2&&petit_args.FileName[i-1]=='f'&&petit_args.FileName[i-2]=='.')
            {
            int pid;
            FILE *fd;
            printf("+ Translating %s to petit\n", petit_args.FileName);

            /* Check if .t file already exists */
            petit_args.FileName[i-1] = 't';
            if((fd=fopen(petit_args.FileName,"r"))!=NULL)
                {
                fclose(fd);
                fprintf(stderr,
                        "File %s already exists. "
                        "Do you want to overwrite it (y, [n]) ?", 
			petit_args.FileName);
                fflush(stderr);
                }
            petit_args.FileName[i-1] = 'f';

            pid=fork();
            Assert(pid>=0, "can not fork f2p");
            if (pid==0)
                {
                Assert(execlp("f2p", "f2p", petit_args.FileName, 
		       (char*)NULL) >= 0,
                       "Can not execute f2p");
                }
            else
                {
                int c, status;
                while((c=wait(&status))!=pid)
                    Assert(c!=-1, "wait f2p error");
                UserAssert(status==0,
                           "Translation to petit was not successful");
                }
            petit_args.FileName[i-1] = 't';
            }
        }
#else
    assert(0 && "Convert to Petit not yet supported under Visual C++");
#endif
    } /* convert_to_petit */

} // end omega namespace
