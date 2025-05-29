/***************************************************************************
  COPYRIGHT 2001 BioImaging Lab RWJMS/UMDNJ
****************************************************************************/
#include <stdlib.h>
#define CheckNull(dat) if(dat == NULL) { \
	fprintf(stderr,"ERROR: Out of memory.\n"); exit(1); }

/*
 Data types for 'mode' field
*/
#define IP_BYTE 0
#define	IP_SHORT 1
#define IP_FLOAT 3
#define IP_USHORT 6
#define	IP_INT 2

#define IP_MAXDIM	10000

/* used to manipulate the dirty bit */

#define STAT_FLAG_SET	0x00000001	
#define STAT_FLAG_RESET	0xFFFFFFFE	

/*   
     The ipl_ds is a structure which contains all the elements found in an
     IPLab image file. The first section of the structure is the 2120 byte header.
     The remaining fields allow the image file data to be included in the
     OSM dataset structure. It is assumed that no footer data is included.
*/

typedef struct _iphdr
{

/******     IPLab_HEADER FORMAT BEGINS HERE    *****/

/****** start fields used by IPLab programs ******/

   char 		ver[4];		/* iplab version=3.1a */
   char 		format;		/* IPLab File format data=0 */
   char 		  mode;		/* data type */
   short int		nn;		/* dummy data for first two bytes */
   short int		nx;		/* data width, Note: Its swabbed! */
   short int		mm;		/* dummy data for first two bytes */
   short int 		ny;		/* data height, Note: Its swabbed! */
   char 		resrvd[6];	/* 6 byte reserved section */
   short int 		nz;		/* number of frames, Note: Its swabbed! */
   char 		resrvd2[50];	/* 50 byte reserved section */
   char			clut[2048];	/* 2048 clut array */

/****** IPL HEADER FORMAT ENDS HERE (2120 bytes) *****/
   char			*data;		/* pointer to the data */
/****** end   fields used by IPLab programs ******/
} ipl_ds;

/* header procedure prototypes*/ 

extern ipl_ds *read_ipdataset(char* filename);
extern ipl_ds *read_ipheader(char *fname);
