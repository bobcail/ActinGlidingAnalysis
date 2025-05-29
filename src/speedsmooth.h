/*  speedsmooth.h
    
    The smoothing procedures for speed.c
    Revised 3/08/02 Donald Winkelmann
    
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/


void smoothdata()
{
int i,j,delt;
double kernel[100],norm;

for(i=-50;i<50;i++)
   kernel[i+50]= exp(-0.5*i*i/kernelwidth)/sqrt(M_PI*2.0*kernelwidth);

norm=0.0;
for(i=0;i<100;i++)norm+=kernel[i];
for(i=0;i<100;i++)kernel[i]/=norm;

for(i=0;i<ndata;i++)
   {
   xsmooth[i]=0.0;
   for(delt=-50;delt<50;delt++)
      {
      j=i+delt; if(j<0)j=0; else if(j>=ndata)j=ndata-1;
      xsmooth[i]+=xx[j]*kernel[delt+50];
      }
   }
}

void updatehint()
{
int i,j;
double ehint;

ehint=0.0;

for(i=0;i<ndata;i++)xdiff[i]=0.0;

for(i=0;i<nbreaks;i++)
   for(j=from[i];j<to[i];j++)
      xdiff[j]=xsmooth[j]-(a[i]*j+b[i]);

hint=0;
for(i=0;i<nbreaks;i++)
   {
   for(j=from[i]+20;j<(to[i]-20);j++)	//Was 30; Lower limits yields more breaks
      if( (xdiff[j]>xdiff[j+1]) && (xdiff[j]>xdiff[j-1])&& (xdiff[j]>ehint))
        /* Local maximum */
        { ehint=xdiff[j]; hint=j; }
      else
      if( (xdiff[j]<xdiff[j+1]) && (xdiff[j]<xdiff[j-1])&& ((-xdiff[j])>ehint))
        { ehint= -xdiff[j]; hint=j; }
   }
}


void fitdata()
{
int i,j;
double sx,sxx,sy,sxy,syy,s0;
double thiserr;
double uxx, uxy, uyy;
double speed1, speed2;
double *xf;

if(whichfit==0) xf=xx;
        else    xf=xsmooth;

totalerror=0.0;
for(i=0;i<nbreaks;i++)
   {
   sx=sxx=sy=sxy=syy=0.0; s0=to[i]-from[i]+1;
   for(j=from[i];j<=to[i];j++)
      {
      sx+=j; sxx+= j*j; sy+=xf[j]; sxy+= j*xf[j]; syy+=xf[j]*xf[j];
      }
   uxx=sxx-sx*sx/s0;
   uxy=sxy-sx*sy/s0;
   uyy=syy-sy*sy/s0;
   thiserr=uyy-uxy*uxy/uxx;
   speed1=uxy/uxx;	//This is the velocity calculation
   speed2=uyy/uxy;
   a[i]=speed1;
   b[i]=(sy-a[i]*sx)/s0;
   errspd[i]= sqrt(12.0*thiserr/s0)/s0;
   if(errspd[i]<0)errspd[i]= -errspd[i];
   totalerror+=thiserr;
   }
}
