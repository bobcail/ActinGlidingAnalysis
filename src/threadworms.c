/*  threadworms.c
    
    This program will create the thread graph which tracks the continuity in
    time of filaments

    The format of the thread file as defined in this program is:
    _T 		(header)
    F%w%w%w     (frame number, number of objects,  and number of "edges")
    [%w%w]	(new number, old number)
                (frame number = -1 to end file)

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/
   
#define MAXOBJ 300
#define MEM 2
#define TALK if(0)
#include <stdlib.h>
#include <stdio.h>
#include "getcwb.h"

int img[640][480][MEM];

int hits[MAXOBJ][MAXOBJ];

int frm, prevfrm;
FILE *filein, *fileout;
int rects;

void compareframes()
{
int i,j,k, nf,of, no, oo, edges;

putc('F',fileout); putw(frm,fileout); putw(rects,fileout);
if(prevfrm<0)
  {
  printf("-"); fflush(stdout); putw(0,fileout);
  TALK printf("old frame=%d, exiting cmp\n",prevfrm);
  return; }
if( (frm-prevfrm)!=1)
  { putw(0,fileout);
  printf("\n\nThere is a missing frame: prev=%d, current=%d\n\n",prevfrm,frm);
  return;
  }

TALK printf("Entering compareframes() for %d %d \n",frm,prevfrm);
nf=frm%MEM; of=prevfrm%MEM;
for(i=0;i<MAXOBJ;i++)for(j=0;j<MAXOBJ;j++)hits[i][j]=0;

for(i=0;i<640;i++)for(j=0;j<480;j++)
   if( (no=img[i][j][nf]) )
     if( (oo=img[i][j][of]) ) hits[no][oo]++;
TALK printf("Done scanning\n");

TALK printf("Frame %d\n",frm);
edges=0;
for(i=0;i<MAXOBJ;i++)for(j=0;j<MAXOBJ;j++) 
   if(hits[i][j])edges++;

putw(edges,fileout);
for(i=0;i<MAXOBJ;i++)for(j=0;j<MAXOBJ;j++) 
   if((k=hits[i][j]))
     { putw(i,fileout);putw(j,fileout); }

TALK for(i=0;i<MAXOBJ;i++)for(j=0;j<MAXOBJ;j++) 
   if((k=hits[i][j]))printf("%d %d %d\n",i,j,k);

}


int main(int argc, char **argv)
{
int i,j,k,l,ff,file, bf,ef, xx0,xx1,yy0,yy1, width, height, b,w, rect;

prevfrm=-5;

if( (fileout=fopen("thread","w"))==NULL)
  {
  printf("Can't create thread file for writing, exiting\n");
  exit(1);
  }
putc('_',fileout);
putc('T',fileout);

for(file=1; file<argc; file++)
   {
   cwb_getheader(&filein,argv[file],&bf,&ef);
   printf("\n%s:",argv[file]); fflush(stdout);
   for(frm=bf;frm<=ef;frm++)
      {
      for(i=0;i<640;i++)for(j=0;j<480;j++)img[i][j][frm%MEM]=0;
      cwb_getframe(&ff,&rects);
      if(frm%2==0){printf("+"); fflush(stdout); }
      TALK printf("Reading frame %d (%d rects) in %s\n",frm,rects,argv[file]);
      if((rects+2) > MAXOBJ)
        { printf("Too many rectangles, MAXOBJ=%d\nRecompile thread.c\n",rects);
        exit(1); }
        
      if(ff!=frm)
        printf("Frame # wrong in %s: %d should be %d?\n",argv[file],frm,ff);
      for(rect=0;rect<rects;rect++)
         {
         cwb_getrect(&xx0,&yy0,&xx1,&yy1);
         width=xx1-xx0; height=yy1-yy0;
         for(j=yy0;j<=yy1;j++)
            {
            cwb_getline();
            i=xx0;
            while(i<xx1)
                 {
                 b=getc(filein); w=getc(filein);
                 if(w>0)
                   for(l=i+b;l<=(i+b+w);l++)img[l][j][frm%MEM]=rect+2;
                 i+= b+w;
                 }
            } /* done with this object */
         }    /* done with the frame */

      compareframes();
      prevfrm=frm;

      } /* continue on the next file */

    cwb_close();
    } /* done with the thread */

putc('F',fileout);
putw(-1,fileout);
putw(0,fileout);
fclose(fileout);
}
