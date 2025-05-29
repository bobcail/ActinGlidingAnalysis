/***************************************************************************
  COPYRIGHT 2001 BioImaging Lab RWJMS/UMDNJ
****************************************************************************/
/*
   grabworms.c 

   This program grabs frames from an IpLab or Tiff byte format stack, 
   applies an appropriate pre-thresholding to the image, 
   median and rank filter in succession to refocus the image,
   thresholds image to separate foreground from background, 
   separates all connected domains by floodfill with jumps, 
   and dumps all resulting objects to files in cwb format.
   (FYI cwb == captured worm bitmaps, 100 frame packs for all worm
   coordinates for subsequent analysis) 
   Magnasco and Schnitzer, NEC, 1993-1994.
   Revised and ported to OS X by Donald Winkelmann, Feb. 2002.
   Tiff file support added Jan. 2005.
*/
/* -------------------------------------------------------- General includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "iplfile.h"

#ifdef LINUX
#include <tiffio.h>
#else 
#include </opt/local/include/tiffio.h>
#endif

/* --------------------------------------------------------------- Variables */

uint32 h, w;
uint32 rowsperstrip;
uint32 start;
uint32 end;
tsample_t samplesperpixel;
int frm = 0;
int dircount = 0;
char *FileOut;						/* Pointer for output cwb.* files */
int StartFrame;						/* First frame to grab */
int ThisFrame;						/* Frame we are currently pointing to*/
int FrameCount;						/* The number of frames collected*/
int object;							/* Number of objs in this frame (starts at 2)*/
int histo[256];						/* The histogram of pixel values */
unsigned long orig[480][640];		/* Monochrom screen used for tiff open routin*/
unsigned long monoscreen[480][640];	/* Monochrome screen we'll use */
unsigned long auxmscreen[480][640];	/* Auxiliary screen for smoothing */
int obx0[300], oby0[300], obx1[300], oby1[300]; /* object bounding rects */
int nnz, nnx, nny;
int tiff;
int frame=0;
FILE *fout;							/* The Output file */

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
void analyze_this();    /* Core of the frame analysis and worm extraction */
void TIFFReadData();	/* Tiff data analysis */
void open_tiff();		/* Open Tiff file format */

/* ----------------------------------------------------------------------*/

int main( int argc, char **argv )
{
    int				i, j, xcount, ycount;
    char			ifile[255];
    ipl_ds			*head;
    unsigned char	*bimptr;

/* Open the image stack containing the video sequence */

if(argc != 2) {
	fprintf(stderr,"USAGE: %s ifile\n",argv[0]);
	exit(1);
	}
strcpy(ifile,argv[1]);
	open_tiff(ifile);   // Try reading a Tiff file format first!
	if(tiff!=1)			// Try reading an IpLab file format if its not Tiff!
		{
		head = read_ipdataset(ifile);
		if(head == (ipl_ds*)NULL) {
		fprintf(stderr,"ERROR: Can't read dataset. You probably entered\n");
		fprintf(stderr,"       the wrong file name for the IpLab File.\n"); exit(1); }

		nnx = head->nx;
		nny = head->ny;
		nnz = head->nz;
		}

StartFrame = 0;				/* Convert the arguments to frames */
FrameCount = nnz/100;
FrameCount = (FrameCount * 100)-1;
init_bitmaps();
init_rank();
openfile(StartFrame);		/* Open the first cwb file */

/*------------------- The Main Loop ----------------------*/

if(tiff!=1)					/* Read each frame into an array from an IpLab file */
	{
		bimptr=(unsigned char *)head->data;
		for (ThisFrame=StartFrame; ThisFrame<=FrameCount; ThisFrame++)
		{
		for (ycount=0; ycount<nny; ycount++)
					{for (xcount=0; xcount<nnx; xcount++)
						{            
						j = *bimptr++;
						monoscreen[ycount][xcount] = j;
						}
					}
		fprintf(stderr,"  Frm #%d  ",ThisFrame);
		analyze_this();
		}					/* next frame please */
	}

if(tiff)					/* Read each frame into an array from a Tiff file */
	{
		TIFF* tif = TIFFOpen(ifile, "r");  /* Open the tiff file once*/
		for (ThisFrame=StartFrame; ThisFrame<=FrameCount; ThisFrame++)
		{
			if(TIFFSetDirectory(tif, ThisFrame))
			{
				start=end=0;
				TIFFReadData(tif);
			}	
		fprintf(stderr,"  Frm #%d  ",ThisFrame);
		analyze_this();	
		}					/* next frame please */

		TIFFClose(tif);		/* Close the tiff file when you're finished*/
	}
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
     if((maxx-minx)*(maxy-miny)<8)continue; // a minimum size of 14 is set here (was 50) DAW  testing 8
     oby0[object]=minx; oby1[object]=maxx;
     obx0[object]=miny; obx1[object]=maxy;
     object++;
     }
fprintf(stderr,"%d Objects\n", object); 
}

void dumpobjects()
{
int i,j,k,cou; 

fprintf(fout,"FF");					/* Write Frame Header */
putw(ThisFrame,fout); putw(object-1,fout);	/* frame #, objects in frme */

for(i=2;i<object+1;i++)				/* Loop over objects found */
   {
   fprintf(fout,"RR");				/* Rectangle header */
   putw(obx0[i],fout); putw(oby0[i],fout);	/* boundaries */
   putw(obx1[i],fout); putw(oby1[i],fout);

   for(j=oby0[i];j<=oby1[i];j++)	/* Loop over lines */
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
      }								/* End loop lines */
   }								/* End loop objects */ 
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

void analyze_this () 
 {
        histogram(monoscreen,histo);	/* Histogram the image */
        prethreshold(monoscreen,histo);	/* Elliminate most of the background */
        expand(monoscreen,histo);		/* Expand to fill the range */
        refocusimage(monoscreen,monoscreen,auxmscreen); /* Apply rank */
        makeboolean(monoscreen,64);		/* Threshold at 64 */
        separate();						/* Floodfill monoimage into sep objects*/
        dumpobjects();					/* Write objects to output file*/
}
		
void open_tiff (char* filename)
{
	long flags = 0;
	int x;
	
    TIFF* tif = TIFFOpen(filename, "r");
    if (tif)
	{
		tiff++;
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		if(samplesperpixel>1)
			{
			printf("I'm Sorry, I can only process B&W image data!\n");
			TIFFClose(tif);
			exit (1);
			}		

        do {
            dircount++;
			} while (TIFFReadDirectory(tif));

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		printf("There are %d X %d pixels with %d rows/strip and %d frames in %s\n", w, h, rowsperstrip, dircount, filename);
				nnx=w;
				nny=h;
				nnz=dircount;
	TIFFClose(tif);
	}
}

static void
StripImage(unsigned char* pp, uint32 nrow, tsize_t scanline)
{
	register tsize_t xx;
	uint32 yy;
	int p;

		end = start + nrow;
        for (yy=start; yy<end; yy++)
		{
			for (xx = 0; xx < scanline; xx++) {            
				p = *pp++;
				monoscreen[yy][xx] = p;
				}
		}
	start += nrow;
}

void TIFFReadContigStripData(TIFF* tif)
{
	unsigned char *buf;
	tsize_t scanline = TIFFScanlineSize(tif);

	buf = (unsigned char *)_TIFFmalloc(TIFFStripSize(tif));
	if (buf) {
		uint32 row, h;
		uint32 rowsperstrip = (uint32)-1;

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		for (row = 0; row < h; row += rowsperstrip) {
			uint32 nrow = (row+rowsperstrip > h ?
			    h-row : rowsperstrip);
			tstrip_t strip = TIFFComputeStrip(tif, row, 0);
			if (TIFFReadEncodedStrip(tif, strip, buf, nrow*scanline) < 0) break;
			StripImage(buf, nrow, scanline);
		}
		_TIFFfree(buf);
	}
}

void TIFFReadSeparateStripData(TIFF* tif)
{
	unsigned char *buf;
	tsize_t scanline = TIFFScanlineSize(tif);

	buf = (unsigned char *)_TIFFmalloc(TIFFStripSize(tif));
	if (buf) {
		uint32 row, h;
		uint32 rowsperstrip = (uint32)-1;
		tsample_t s, samplesperpixel;

		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		TIFFGetField(tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);
		TIFFGetField(tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
		for (row = 0; row < h; row += rowsperstrip) {
			for (s = 0; s < samplesperpixel; s++) {
				uint32 nrow = (row+rowsperstrip > h ?
				    h-row : rowsperstrip);
				tstrip_t strip = TIFFComputeStrip(tif, row, s);
				if (TIFFReadEncodedStrip(tif, strip, buf, nrow*scanline) < 0) break;
				StripImage(buf, nrow, scanline);
			}
		}
		_TIFFfree(buf);
	}
}

void TIFFReadData(TIFF* tif)
{
	uint16 config;

	TIFFGetField(tif, TIFFTAG_PLANARCONFIG, &config);

		if (config == PLANARCONFIG_CONTIG)
			TIFFReadContigStripData(tif);
		else
			TIFFReadSeparateStripData(tif);
}

void quit()				/* This is where we exit */
{
	printf("\n\nSee you soon\n");
	exit(0);
}
