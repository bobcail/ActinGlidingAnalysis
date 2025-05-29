/*  extractworm.c

    Extract swb timelines from the cwb files
    Marcello O. Magnasco Rockefeller 1994

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

#define TALK if(1)

#include <stdlib.h>
#include <stdio.h>
#include "getbraided.h"
#include "getcwb.h"

int currentframe= -1;
int currentbgnframe= -1;
int currentendframe= -1;
int objects;
int objectnumber;

int objectname;
int startframe, endframe;

char thebitmap[30000];
int bitmaplength;
int xx0,xx1,yy0,yy1;
char cwbfname[80];
FILE *cwbfin;
FILE *swbout;
int names[300];


void gotoframe(int frm)
{

if(currentframe== -1) 	/* This means we have not initialized anything */
  {
  sprintf(cwbfname,"cwb.%05d",frm-frm%100);
  cwb_getheader(&cwbfin,cwbfname,&currentbgnframe,&currentendframe);
  if( (frm<currentbgnframe) || (frm>currentendframe))
    { 
    printf("Error: requested %d is not in %s [%d,%d]\n",
          frm, cwbfname, currentbgnframe, currentendframe);
    exit(1);
    }
  }

if(frm>currentendframe) /* This means we need a new file */
  {
  cwb_close();		/* Let's be as tidy as possible */
  sprintf(cwbfname,"cwb.%05d",frm-frm%100);
  cwb_getheader(&cwbfin,cwbfname,&currentbgnframe,&currentendframe);
  if( (frm<currentbgnframe) || (frm>currentendframe))
    { 
    printf("Error: requested %d is not in %s [%d,%d]\n",
          frm, cwbfname, currentbgnframe, currentendframe);
    exit(1);
    }
  }

/* Therefore, we are in the right file! */
/* Advance the cwb file until we are in the right frame */

cwb_getframe(&currentframe,&objects);

for(;currentframe<frm;) 
   {
   cwb_flushframe(objects); 
   cwb_getframe(&currentframe,&objects);
   }

if(currentframe != frm) 
  {
  printf("Error: it looks like frame %d is missing from %s\n", frm,cwbfname);
  exit(1);
  }

/* Now advance the braid file until it is int the right frame */

getframe(frm,names);

/* We are now in the right frame both in cwb and braid */
}


void getobject() 	/* Go to the appropriate object number within frame */
{
int i,j,k;

objectnumber= -1;
for(i=0;i<objects;i++)
   if(names[i]==objectname) objectnumber=i;

if(objectnumber == -1)
  {
  printf("getobject(): object %d does not exist in frame %d\nListing:\n",
        objectname,currentframe);
  for(i=0;i<objects;i++)
     printf("%8d",names[i]);
  printf("\n");
  exit(1);
  }
    
for(i=0;i<objects;i++)
   if(names[i]==objectname)
     {
     cwb_getrect(&xx0,&yy0,&xx1,&yy1);
     bitmaplength=cwb_getbitmap(thebitmap,xx0,yy0,xx1,yy1) - 2;
     }
   else
     cwb_flushobject();

}


int main(int argc, char **argv)
{
int frame,i;

if(argc != 5) 
  {
  printf("Usage: %s lifetime braidnumber startframe endframe\n",argv[0]);
  exit(1);
  }

objectname=atoi(argv[2]);
startframe=atoi(argv[3])+1;
  endframe=atoi(argv[4]);

printf("\n\nExtracting worm %d in frames [%d,%d]\n",
      objectname,startframe,endframe);

sprintf(cwbfname,"swb.%05d",objectname);
if( (swbout=fopen(cwbfname,"w")) == NULL)
  {
  printf("Could not open %s for writing\n",cwbfname);
  exit(1);
  }

printf("Writing single worm file %s\n",cwbfname);
fputc('_',swbout);
fputc('S',swbout);

for(frame=startframe;frame<=endframe;frame++)
   {
   gotoframe(frame);
   fputc('F',swbout); fputc('F',swbout);
   putw(frame,swbout);
   getobject();
   putw(xx0,swbout); putw(xx1,swbout);
   putw(yy0,swbout); putw(yy1,swbout);
   for(i=0;i<bitmaplength;i++) fputc(thebitmap[i],swbout);
   }

fputc('F',swbout); fputc('F',swbout);
putw( -1 ,swbout);
putw(xx0,swbout); putw(xx1,swbout);
putw(yy0,swbout); putw(yy1,swbout);
fclose(swbout);
}
