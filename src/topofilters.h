/*  topofilters.h
    
    Here are the "topological" filters, i.e., median and rank,
    also includes the sorting algorithms.
    The rank filter is implemented without recourse to sorting
    Magnasco and Schnitzer, 1993

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

#define OPER rk -= rank_cliplow ;\
             if (rk<0) rk=0; else if(rk>rank_cliphi) rk=rank_cliphi ;\
             k=((orig -rank_mvleft)*rk*rank_xpand)/(rank_cliphi*10); 

/* These are the parameters to the flexirank filter 
   They should be set through init_rank(); */

int rank_pixno   = -1;	/* width of the square */
int rank_power   = -1;	/* power to which to raise original pixel value */
int rank_cliplow = -1;	/* value below which rank->0 */
int rank_cliphi  = -1;	/* value above which rank->max*/
int rank_mvleft  = -1;	/* ??? */
int rank_xpand   = -1;	/* ??? */

void init_rank()
{

BGNPARAMETERS( " init_rank " )
PARAMETER(rank_pixno,  "rank_width")
PARAMETER(rank_power,  "rank_power")
PARAMETER(rank_cliplow,"rank_cliplow")
PARAMETER(rank_cliphi, "rank_cliphi")
PARAMETER(rank_mvleft, "rank_mvleft")
PARAMETER(rank_xpand,  "rank_xpand")
ENDPARAMETERS

if(rank_pixno   == -1) rank_pixno   =16;
if(rank_power   == -1) rank_power   =2;
if(rank_cliplow == -1) rank_cliplow =6;
if(rank_cliphi  == -1) rank_cliphi  =14;
if(rank_mvleft  == -1) rank_mvleft  =0;
if(rank_xpand   == -1) rank_xpand   =20;

/* Now we error check on all this */
if(rank_pixno < 1 ) 
{ printf("init_rank: wrong rank_width: %d \n",rank_pixno); exit(1); }
}


/* The filtering routine for the dynamic version of
   grabworms. The filter is a generalized version of Marcelo's
   rank25 filter - here the filter reads in the number of pixels
   to be used, the lower and upper clip points, expansion factor,
   the rank of the reference pixel (or center pixel can be specified),
   and the darkness shift in the brightness of the reference pixel. */

void flexi_rank(unsigned long m[480][640] ,	/* input bitmap */
                unsigned long n[480][640] )	/* output bitmap */
{
register int i, j, k, l, orig, cmp, rk, rad, rad2;

if(rank_pixno < 1) init_rank();

rad2=rank_pixno; rad=rint(sqrt(rad2+0.0));

for(i=rad;i<(480-rad);i++) for(j=rad;j<(640-rad);j++)
   if((orig=m[i][j]))
     {
     rk=0; for(k= -rad; k<=rad; k++) for(l= -rad;l<=rad;l++)
        if( ( (k*k+l*l) < rad2) && (cmp=m[i+k][j+l]) && (cmp<=orig) ) rk++;
     rk-=rank_cliplow; if(rk<0)rk=0; else if(rk>rank_cliphi) rk=rank_cliphi;
     switch(rank_power)
           {
     case 0: k=rk*rank_xpand/5; break;
     case 1: k=(sqrt(orig+0.0)-rank_mvleft)*rk*rank_xpand/rank_cliphi/10; break;
     case 2: k=((orig -rank_mvleft)*rk*rank_xpand)/(rank_cliphi*10); break;
     default: exit(1);
           }
     if(k<0)k=0;else if(k>255)k=255;
     n[i][j]=k;
     }
   else n[i][j]=0;
}


void refocusimage(unsigned long source[480][640],
                  unsigned long dest[480][640], 
                  unsigned long interm[480][640])
{
smooth5( source,interm);
flexi_rank( interm,dest);
}
