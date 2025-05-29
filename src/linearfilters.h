/***************************************************************************
  COPYRIGHT 2001 BioImaging Lab RWJMS/UMDNJ
****************************************************************************/
/* lienearfilters.h

 A smoothing filter
 M.O.M. 1994
 
 */

#define SWAP(a,b) { t=a; a=b; b=t; }
unsigned long t;
unsigned long scratch[300];
unsigned long vector[300];

void smooth5(unsigned long m[480][640],unsigned long n[480][640])
{ int i,j;

for(i=1;i<479;i++)
   for(j=1;j<639;j++)
      if(m[i][j])
        n[i][j]= ( 4*m[i][j] + 2*(m[i][j+1]+m[i][j-1]+m[i+1][j]+m[i-1][j])
         + m[i+1][j+1]+m[i+1][j-1]+m[i-1][j+1]+m[i-1][j-1])>>4;
      else n[i][j]=0;
}
