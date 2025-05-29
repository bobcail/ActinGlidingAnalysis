/* bitmaps.h
 
This contains utilities for dealing with bitmaps.
Originally written by Magnasco and Schnitzer 1993
Revised by Donald Winkelmann, Feb. 2002

Copyright (c) 2001 UMDNJ-RWJMS. All rights reserved.
*/


/* The parameters to the routines */
int bitmap_bottom = 0;		/* This is the rectangle where some */
int bitmap_top    = 480;	/* operations will take place, such as */
int bitmap_left   = 0;		/* histogramming or computing stats */
int bitmap_right  = 640;

int bitmap_offset = 0;		/* Threshold will be offset by this much */
int bitmap_sdcoeff= 10;		/* how many stds up from the mode to thrshold*/
int bitmap_warpx  = 0;		/* quadratic background level in x */
int bitmap_warpy  = 0;		/* quadratic background level in y */
int bitmap_scalex = 0; 		/* quad foreground adjustment in x */
int bitmap_scaley = 0; 		/* quad foreground adjustment in y */

/* The routines themselves are:
void init_bitmaps()			initialize and read parameters
void copyto( bitmap, bitmap )		transfer a->b
void clipboundaries( bitmap )		borders to black
void histogram( bitmap, histo )		histogram image
void bitmapstats(histo, *double, *double, *int)) get stats
void offsetdown( bitmap, int) 		offset and clamp to zero
void makeboolean( bitmap, int)		threshold and make boolean
void prethreshold( bitmap, histo)	
void expand( bitmap, histo )		rescale maxpixvalue to 255
void screentomono( bitmap )		from screen to bw bitmap
void threshold(bitmap )			make boolean based on whatever
*/


void init_bitmaps() 	/* Read the parameter block for the routines */
{
BGNPARAMETERS( " init_bitmaps() " )
PARAMETER(bitmap_bottom,"bitmap_bottom")
PARAMETER(bitmap_top,   "bitmap_top")
PARAMETER(bitmap_left,  "bitmap_left")
PARAMETER(bitmap_right, "bitmap_right")
PARAMETER(bitmap_sdcoeff,"bitmap_sdcoeff")
PARAMETER(bitmap_warpx, "bitmap_warpx")
PARAMETER(bitmap_warpy, "bitmap_warpy")
PARAMETER(bitmap_scalex,"bitmap_scalex")
PARAMETER(bitmap_scaley,"bitmap_scaley")
PARAMETER(bitmap_offset,"bitmap_offset")
ENDPARAMETERS
}

void copyto(				/* Just copy the bitmaps */
     unsigned long m[480][640],		/* from m */
     unsigned long n[480][640])		/* to n */
{
register int i,j;
for(i=0;i<480;i++)for(j=0;j<640;j++)n[i][j]=m[i][j];
}

void clipboundaries(unsigned long m[480][640])	/* Clip boundaries to zero */
{
register int i,j;
for(i=0;i<480;i++) for(j=   0;j<bitmap_left;j++) m[i][j]=0;
for(i=0;i<480;i++) for(j=bitmap_right;j<640;j++) m[i][j]=0;
for(i=0;i<bitmap_bottom;i++) for(j=0;j<640;j++) m[i][j]=0;
for(i=bitmap_top;i<480;i++)  for(j=0;j<640;j++) m[i][j]=0;
}


void histogram(				/* Make a histogram of pixel values */
     unsigned long m[480][640],		/* bitmap to be histogrammed */
     int histo[256])			/* histogram where to accumulate */
{
register int i,j;
for(i=0;i<256;i++)histo[i]=0;

for(i=bitmap_bottom;i<bitmap_top;i++)for(j=bitmap_left;j<bitmap_right;j++)
   histo[m[i][j]]++;
}


void bitmapstats(int histo[256], 	/* Get mean, sd, mode */
     double *mean,
     double *sd,
     int *mode)
{
int i,m;
double total;
total=0.0; for(i=1;i<256;i++) total+=histo[i];       /* Get the total counts */
*mean=0.0; for(i=1;i<256;i++) *mean+=i*histo[i]/total;       /* Get the mean */
*sd=0.0;   for(i=1;i<256;i++) *sd+=(i-*mean)*(i-*mean)*histo[i]/total;
*sd=sqrt(*sd);                                 /* Get the standard deviation */
for(m=i=0;i<256;i++) if(histo[i]>m)m=histo[i];
for(i=0;i<256;i++)if(histo[i]==m)*mode=i;        /* Get the mode */
}

void offsetdown(			/* Offset by a constant and clamp->0 */
     unsigned long m[480][640],
     int off)
{
register int i,j;
for(i=0;i<640;i++)for(j=0;j<480;j++)
   if(m[j][i]<off) m[j][i]=0;
   else            m[j][i]-=off;
}

void makeboolean(                        /* Threshold to 0-1 at off */
     unsigned long m[480][640],
     int off)
{
register int i,j;
for(i=0;i<640;i++)for(j=0;j<480;j++) m[j][i]=(m[j][i]>off);
}


/** This is the pre-thresholding routine essential to the well-being of
    the rank filter and so on and so forth */

void prethreshold(unsigned long m[480][640],  /* Pre-threshold an image */
    int histo[256])
{
int	i, j, mode, k, i2, j2;
double	mean, sd, off, wx, wy, sx, sy;

clipboundaries(m);		  /* Clip to zero the appropriate boundaries */
bitmapstats(histo,&mean,&sd,&mode); /* Get the statistics of the image */

wx=bitmap_warpx*0.00002;  	/* Set the background variation was 0.00001*/
wy=bitmap_warpy*0.00002;
sx=bitmap_scalex*0.000002;	/* Set the foreground variation was 0.000001*/
sy=bitmap_scaley*0.000002;

off=mode+0.1*bitmap_sdcoeff*sd+bitmap_offset; /* raw offset is the mode,
			plus sdcoeff times standard dev, plus offset */

for(i=bitmap_bottom;i<bitmap_top;i++)    /* Explore the image */
   for(j=bitmap_left;j<bitmap_right;j++)
      {
      i2=(i-(bitmap_top/2))*(i-(bitmap_top/2)); j2=(j-(bitmap_right/2))*(j-(bitmap_right/2));
                /* The quadratic terms */
      k=rint( ( m[i][j] - off + wx*j2 + wy*i2 ) * (1.0 + sx*j2+sy*i2) );
		/* This offsets by off plus quadratic terms, and then 
		   renormalizes by multiplying by one plus quad terms */

      if(k<0)k=0; else if(k>255)k=255; /* Clip to the range */
      m[i][j]=k;
      }

}

void expand(unsigned long m[480][640],  /* Expand an image to fill range */
    int histo[256])
{
int i,j,max,sum;

for(max=255,sum=0; (sum<100) && (max>4) ; max--)
   sum+=histo[max];		/* Get that pixel value for which 100 pixels 
				   are above it. */

for(i=0;i<480;i++)for(j=0;j<640;j++)
   if(m[i][j])
   {
     if(m[i][j]<max)
       m[i][j]= rint( (255.0*m[i][j])/max);
     else (m[i][j]=255);
   }
}
    
