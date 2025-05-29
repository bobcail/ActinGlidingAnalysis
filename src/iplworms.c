/***************************************************************************
  COPYRIGHT 2001 BioImaging Lab RWJMS/UMDNJ
****************************************************************************/
/*
   iplworms.c 

   This program grabs video frames from an IpLab byte format stack, 
   applies an appropriate pre-thresholding to the image, 
   median and rank filter in succession to refocus the image,
   thresholds image to separate foreground from background, 
   separates all connected domains by floodfill with jumps, 
   and dumps all resulting objects to files in cwb format.
   (FYI cwb == captured worm bitmaps, 100 frame packs for all worm
   coordinates for subsequent analysis) 
   Magnasco and Schnitzer, NEC, 1993-1994.
   Revised and ported to OS X by Donald Winkelmann, Feb. 2002.
*/
/* -------------------------------------------------------- General includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "iplfile.h"

/* --------------------------------------------------------------- Variables */

char *FileOut;				/* do I want this? */
int StartFrame;				/* First frame to grab */
int ThisFrame;				/* Frame we are currently pointing to*/
int FrameCount;				/* The number of frames collected*/
int object;				/* Number of objs in this frame (starts at 2)*/
int histo[256];				/* The histogram of pixel values */
unsigned long monoscreen[480][640];	/* Monochrome screen we'll use */
unsigned long auxmscreen[480][640];	/* Auxiliary screen for smoothing */
int obx0[100], oby0[100], obx1[100], oby1[100]; /* object bounding rects */
int nnz, nnx, nny;

FILE *fout;		/* The Output file */

/* -------------------------------------------------------------- Procedures */

#define M monoscreen
#include "floodfill.h"
#include "parameters.h"
#include "linearfilters.h"
#include "topofilters.h"
#include "bitmaps.h"

void quit();			/* Turn the lights off when leaving */
void separate();		/* Floodfill thresholded image */
void dumpobjects();		/* Explore monobuffer and dump packed binary */
void openfile(int);		/* Open cwb.frame file */

/* -------------------------------------------------------------- The MAIN() */

main( int argc, char **argv )
{
    int			i, j, xcount, ycount;
    char 		ifile[255];
    ipl_ds 		*head;
    unsigned char	*bimptr;

/* Open the IpLab image stack containing the video sequence */

if(argc != 2) {
	fprintf(stderr,"USAGE: %s ifile\n",argv[0]);
	exit(1);
	}
strcpy(ifile,argv[1]);
head = read_ipdataset(ifile);

if(head == (ipl_ds*)NULL) {
  fprintf(stderr,"ERROR: Can't read dataset. Make sure data\n");
  fprintf(stderr,"       has a valid WASHU header.\n"); exit(1); }

    nnx = head->nx;
    nny = head->ny;
    nnz = head->nz;

StartFrame = 0;           /* Convert the arguments to frames */
FrameCount = nnz/100;
FrameCount = (FrameCount * 100)-1;
init_bitmaps();
init_rank();
openfile(StartFrame);      /* Open the first cwb file */

/*------------------- The Main Loop ----------------------*/

    /* Read each frame into an array m[i][j] from the ipl_ds */

    bimptr=(unsigned char *)head->data;

    for (ThisFrame=StartFrame; ThisFrame<=FrameCount; ThisFrame++)

    {
        for (ycount=0; ycount<nny; ycount++)
        {

        /* copy along an x segment and then jump to the next row */

            for (xcount=0; xcount<nnx; xcount++)
            {            
            j = *bimptr++; 			//This line works
            monoscreen[ycount][xcount] = j;
            }
	 }
         
    fprintf(stderr,"  Frm #%d  ",ThisFrame);
                
		/* This now is the core of the analysis. This sequence
		   of steps defines what is being done to the image */
 
        histogram(monoscreen,histo);	/* Histogram the image */
        prethreshold(monoscreen,histo);	/* Elliminate most of the background */
        expand(monoscreen,histo);	/* Expand to fill the range */
        refocusimage(monoscreen,monoscreen,auxmscreen); /* Apply rank */
        makeboolean(monoscreen,64);	/* Threshold at 64 */
        separate();			/* Floodfill monoimage into sep objects*/
        dumpobjects();			/* Write objects to output file*/

    }		/* next frame please */

quit();
}

/*---------------- Subroutines --------------------*/

void separate()
{
    int i,j,k,l; 

object=2;
for(j=0;j<nny;j++)for(i=0;i<nnx;i++)
   if(monoscreen[j][i]==1)
     {
     floodfill(j,i,1,object);
     if((maxx-minx)*(maxy-miny)<14)continue; // a minimum size of 14 is set here (was 50) DAW
     oby0[object]=minx; oby1[object]=maxx;
     obx0[object]=miny; obx1[object]=maxy;
     object++;
     }
fprintf(stderr,"%d Objects\n", object); 
}

void dumpobjects()
{
int i,j,k,cou; 

fprintf(fout,"FF");				/* Write Frame Header */
putw(ThisFrame,fout); putw(object-1,fout);	/* frame #, objects in frme */

for(i=2;i<object+1;i++)				/* Loop over objects found */
   {
   fprintf(fout,"RR");				/* Rectangle header */
   putw(obx0[i],fout); putw(oby0[i],fout);	/* boundaries */
   putw(obx1[i],fout); putw(oby1[i],fout);

   for(j=oby0[i];j<=oby1[i];j++)		/* Loop over lines */
      {
      fprintf(fout,"LL");			/* Line header */
      k=obx0[i];
      while(k<obx1[i])
        {
        cou=0; while((monoscreen[j][k]!=i)&&(k<obx1[i]))cou++,k++;
        if(cou<256)putc(cou,fout);
        else { putc(255,fout); putc(0,fout); putc(cou-255,fout); }
        cou=0; while((monoscreen[j][k]==i)&&(k<obx1[i]))cou++,k++;
        if(cou<256)putc(cou,fout);
        else { putc(255,fout); putc(0,fout); putc(cou-255,fout); }
        }
      }						/* End loop lines */
   }						/* End loop objects */ 
if(ThisFrame%100==99)
  {
  fclose(fout);
  if(FrameCount>ThisFrame)openfile(ThisFrame+1);
  }
}

void openfile(int fr)
{
char name[120];
sprintf(name,"cwb.%05d",fr);
if(NULL==(fout=fopen(name,"w")))
  { printf("Couldn't open %s!\n",name); quit();}
fprintf(fout,"_B");
putw(fr,fout); putw(fr+99,fout);
}

void quit() 		/* This is where we come when we exit */
{
printf("\n\nSee you soon\n");
exit(0);
}
