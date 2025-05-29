/***************************************************************************
  COPYRIGHT 2001 BioImaging Lab RWJMS/UMDNJ
****************************************************************************/
/* 
 read_ipdataset.c 

	Reads a 3-D volume file that has IPLab header on it.
	Performs byte-swapping of header and data if complied as -arch i386
    for running on intel Mac.
	DAW 4/29/01, updated to run on intel Mac 10/12/09
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include "iplfile.h"
int		need_swab=1;	/* if -arch i386 then need_swab = 1, otherwise change value to 0*/

#define swap(a,b) tmp = a; a = (char)b; b = (char)tmp;

void SwabShort(dat)
short *dat;
{
	register char tmp;
	register char *ptr = (char*)dat;
	swap(*ptr,*(ptr+1));
}

ipl_ds *read_ipheader(char *filename)
{
   FILE *fp;
   ipl_ds *ds;

   if((ds = (ipl_ds *)malloc(sizeof(ipl_ds)))==(ipl_ds*)NULL) {
	fprintf(stderr,"ERROR: Out of memory in read_dataset()\n");
	return((ipl_ds*)NULL);
	}

   if((fp=fopen(filename,"rb"))==(FILE*)NULL) {
	fprintf(stderr,"ERROR: Cant open `%s' for read\n",filename);
	return((ipl_ds*)NULL);
	}
   if(fread((void*)ds,1,2120,fp) != 2120) {
	fprintf(stderr,"ERROR: Incomplete read of header in `%s'\n",filename);
	free((char*)ds);
	return((ipl_ds*)NULL);
	}
   return(ds);
}

ipl_ds *read_ipdataset (char *filename)
{
FILE		*fp;		/* FILE pointer */
int		data_size;	    /* size of the data portion of the file */
ipl_ds		*ds;		/* dataset processing structure */
int		vol_size;

   if((ds = (ipl_ds *)malloc(sizeof(ipl_ds)))==(ipl_ds*)NULL) {
	fprintf(stderr,"ERROR: Out of memory in read_dataset()\n");
	return((ipl_ds*)NULL);
	}

   if((fp=fopen(filename,"rb"))==(FILE*)NULL) {
	fprintf(stderr,"ERROR: Cant open `%s' for read\n",filename);
	return((ipl_ds*)NULL);
	}

   if(fread((void*)ds,1,2120,fp) != 2120) {
	fprintf(stderr,"ERROR: Incomplete read of header in `%s'\n",filename);
	free((char*)ds);
	return((ipl_ds*)NULL);
	}
  
   if (need_swab)
   {
	SwabShort(&(ds->nx)); /* Swap byte routines for header file dimensions          */
	SwabShort(&(ds->ny)); /* These are required for intel Mac's, need_swab=1 is set */
	SwabShort(&(ds->nz)); /* at top of this file. DAW 10/12/09                      */
   }
	
   if((ds->nx < 0)||(ds->nx > IP_MAXDIM)||(ds->ny < 0)||(ds->ny > IP_MAXDIM)||
      (ds->nz < 0)||(ds->nz > IP_MAXDIM)) {
	   fprintf(stderr,"ERROR: Data dimensions in header invalid.\n");
	   fprintf(stderr,"       Each dimension must be in range [0..%d]\n",
						IP_MAXDIM);
	   fprintf(stderr,"        nx dimension is [%d]\n",
						ds->nx);
	   fprintf(stderr,"        ny dimension is [%d]\n",
						ds->ny);
	   fprintf(stderr,"        nz dimension is [%d]\n",
						ds->nz);
           fprintf(stderr,"  Sorry, It is not my fault! This does not seem to be an IPLab file.\n"); 
	   free((char*)ds);
	   return((ipl_ds*)NULL);
	   }
           
        fprintf(stderr,"  Opening IPLab File with x,y,z dimensions of %d x %d x %d\n",
                            ds->nx, ds->ny, ds->nz);
        fprintf(stderr,"  The IPLab data type is %d.\n",ds->mode); 
        
   data_size = ds->nx * ds->ny * ds->nz;
   vol_size = data_size;
   switch (ds->mode) {
      case IP_BYTE: break;
      case IP_SHORT:  data_size *= sizeof(short); ds->mode=4; break;  
      case IP_USHORT: data_size *= sizeof(short); ds->mode=4; break;
      case IP_FLOAT:  data_size *= sizeof(float); ds->mode=2; break;
      case IP_INT:    data_size *= sizeof(int);   ds->mode=5; break;
     default: 
	fprintf(stderr,"ERROR: Invalid data_type field in `%s' header\n",
			filename);
	return((ipl_ds*)NULL);
   }

/* Allocate image data array */

   if((ds->data=(char*)malloc(data_size))==(char*)NULL) {
	fprintf(stderr,"ERROR: Out of memory in read_dataset()\n");
	free((char*)ds);
	return((ipl_ds*)NULL);
	}


   if(fread((void*)ds->data,1,data_size,fp) != data_size) {
	fprintf(stderr,
	"WARNING: Incomplete read of image data in read_dataset()\n");
	fprintf(stderr,"         Data may be invalid (file=`%s')\n",filename);
	}

   fclose(fp);
   return (ds);
}
