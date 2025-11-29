/*
    BSD 2-Clause License

    Copyright (c) 2025, Gergely Gati

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, this
       list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
    FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
    SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define SBSP_WRITE_CHUNK (4)
#define SBSP_IMPLEMENTATION
#include "sbsp.h"

static int wnew(void  *ud, int addr, uint8_t *buf, int len)
{
  int ret=-1;
  FILE *f=(FILE *)ud;
  ret=fwrite(buf,1,len,f);
  return(ret);
}

int main(int argc, char **argv)
{
  uint8_t *old;
  struct sbsp sbs={0};

  if(argc<3)
  {
    printf("Usage: %s oldfile patchfile newfile [buffer-length]\n",argv[0]);
    return(0);
  }
  int buffer_len=256;
  if(argc>4) buffer_len=atoi(argv[4]);
  if(buffer_len<5||buffer_len>2000000)
  {
    fprintf(stderr,"buffer-length out of range!\n");
    exit(1);
  }
  FILE *fo=fopen(argv[1],"rb");
  if(NULL!=fo)
  {
    fseek(fo,0,SEEK_END);
    int osiz=ftell(fo);
    old=malloc(osiz);
    fseek(fo,0,SEEK_SET);
    fread(old,1,osiz,fo);
    fclose(fo);
    FILE *fn=fopen(argv[3],"wb");
    sbsp_init(&sbs, old, osiz, wnew, fn);
    uint8_t *wbuf=malloc(buffer_len);
    FILE *fp=fopen(argv[2],"rb");
    int st;
    do
    {
      int rl=fread(wbuf,1,buffer_len,fp);
      st=sbsp_patch(&sbs,wbuf,rl);
    } while(st==SBSP_MORE);
    free(wbuf);
    fclose(fn);
    fclose(fp);
  }
  return(0);
}
