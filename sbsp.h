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
#ifndef SBSP_H
#define SBSP_H

#include <stdint.h>
#include <string.h>

#define BSP_WRITE_CHUNK (4)
#define BSP_MORE (0)
#define BSP_DONE (1)
#define BSP_ERR  (-1)

#define BSST_HEADER (0)
#define BSST_CTRL (1)
#define BSST_DIFF (2)
#define BSST_XTRA (3)

typedef int (*bsp_write_cb_t)(void *, int, uint8_t *, int);

struct bsp
{
  uint8_t buf[8+4];
  int32_t oldpos;
  int32_t newpos;
  int32_t newsize;
  int64_t ctrl[3];
  uint8_t state;
  int32_t spos;
  const uint8_t *old;
  int32_t oldsize;
  uint8_t out[BSP_WRITE_CHUNK];
  int16_t outfill;
  bsp_write_cb_t write;
  void *ud;
};

#endif

#ifdef SBSP_IMPLEMENTATION

static int64_t offtin(uint8_t *buf)
{
  int64_t y;

  y=buf[7]&0x7F;
  y=y*256;y+=buf[6];
  y=y*256;y+=buf[5];
  y=y*256;y+=buf[4];
  y=y*256;y+=buf[3];
  y=y*256;y+=buf[2];
  y=y*256;y+=buf[1];
  y=y*256;y+=buf[0];
  if(buf[7]&0x80) y=-y;
  return y;
}


int bspatch_init(struct bsp *bs, uint8_t *old, int32_t oldsize, bsp_write_cb_t write_cb, void *userdata)
{
  memset(bs, 0, sizeof(struct bsp));
  bs->old=old;
  bs->oldsize=oldsize;
  bs->write=write_cb;
  bs->ud=userdata;
  return(0);
}

int bspatch(struct bsp *bs, uint8_t *in, int32_t inlen)
{
  int32_t i,n=0;

  while(bs->newpos<bs->newsize || bs->state==BSST_HEADER)
  {
    // read header
    if(bs->state==BSST_HEADER)
    {
      for(; bs->spos<sizeof(bs->buf); n++)
      {
        // read more buf
        bs->buf[bs->spos]=in[n];
        bs->spos++;
        if(--inlen==0) return(BSP_MORE);
      }
      if(memcmp(bs->buf,"BSGG",4) != 0) return(BSP_ERR);
      bs->newsize=offtin(&bs->buf[4]);
      bs->spos=0;
      bs->state=BSST_CTRL;
    }

    // read ctrl block
    if(bs->state==BSST_CTRL)
    {
      for(; bs->spos<24; n++)
      {
        // read more buf
        i=bs->spos/8;
        int idx=bs->spos-(i*8);
        bs->buf[idx]=in[n];
        if(idx==7) bs->ctrl[i]=offtin(bs->buf);
        bs->spos++;
        if(--inlen==0) return(BSP_MORE);
      }
      bs->spos=0;
      bs->state=BSST_DIFF;
    }

    // diff
    if(bs->state==BSST_DIFF)
    {
      for(i=bs->spos; i<bs->ctrl[0]&&inlen>0; i++,n++,--inlen)
      {
        bs->out[bs->outfill] = in[n] + ((bs->oldpos>=0&&bs->oldpos<bs->oldsize) ? bs->old[bs->oldpos] : 0);
        bs->outfill++;
        ++bs->newpos;
        ++bs->oldpos;
        ++bs->spos;
        if(bs->outfill==BSP_WRITE_CHUNK)
        {
          bs->write(bs->ud, bs->newpos-BSP_WRITE_CHUNK, bs->out, BSP_WRITE_CHUNK);
          bs->outfill=0;
        }
      }
      if(i>=bs->ctrl[0])
      {
        bs->spos=0;
        bs->state=BSST_XTRA;
      }
      if(inlen<=0)
      {
        return(BSP_MORE);
      }
    }

    if(bs->state==BSST_XTRA)
    {
      for(i=bs->spos; i<bs->ctrl[1]&&inlen>0; i++,n++,--inlen)
      {
        bs->out[bs->outfill] = in[n];
        bs->outfill++;
        bs->spos++;
        ++bs->newpos;
        if(bs->outfill==BSP_WRITE_CHUNK)
        {
          bs->write(bs->ud, bs->newpos-BSP_WRITE_CHUNK, bs->out, BSP_WRITE_CHUNK);
          bs->outfill=0;
        }
      }
      if(i>=bs->ctrl[1])
      {
        bs->oldpos+=bs->ctrl[2];
        bs->spos=0;
        bs->state=BSST_CTRL;
      }
      if(bs->newpos >= bs->newsize) break;
      if(inlen<=0)
      {
        return(BSP_MORE);
      }
    }

  }

  if(bs->outfill>0)
  {
    bs->write(bs->ud, bs->newpos-bs->outfill, bs->out, bs->outfill);
  }

  return(BSP_DONE);
}
#endif
