/*  speedpostscript.h

    The printing routines for the speeds.c program
    They work, but they only print to lpr, a network postscript printer

    Donald Winkelmann 3/08/02
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

FILE *psout;

#define PS fprintf(psout,
#define PSLINE(a,b,c,d) fprintf(psout,"%f %f %f %f mls\n",a,b,c,d);
#define PSMOVE(a,b) fprintf(psout,"%f %f moveto\n",a,b);
#define PSDRAW(a,b) fprintf(psout,"%f %f lineto\n",a,b);
#define PSRECT(a,b,c,d) \
        PSLINE(a,c,a,d) PSLINE(b,c,b,d) PSLINE(a,c,b,c) PSLINE(a,d,c,d)
#define STROKE fprintf(psout,"stroke\n");
        

void PSdisplaydata()
{
double xmin,xmax,ymin,ymax,scx, scy;
int i,totdata;

xmin=0; xmax=ndata;

ymin=ymax=xx[0];
for(i=0;i<ndata;i++)if(xx[i]<ymin)ymin=xx[i];
for(i=0;i<ndata;i++)if(xx[i]>ymax)ymax=xx[i];

maxpos=ymax; minpos=ymin;

scx=800.0/(xmax-xmin); scy=800.0/(ymax-ymin);

PS "%% comm 1\n");
PSRECT(0.0,800.0,0.0,800.0);

PS "%% comm 2\n");
PSRECT(0.0,scx*(from[0]-xmin),0.0,800.0)
for(i=1;i<nbreaks;i++)
   {
   PSRECT(scx*(to[i-1]-xmin)-0.3,scx*(from[i]-xmin)+0.3,0.0,800.0)
   }
PSRECT(scx*(to[nbreaks-1]-xmin),800.0,0.0,800.0)

PS "%% comm 3\n");
PS "5 setlinewidth 0.9 setgray\n");
for(i=0;i<nbreaks;i++)
   {
   PSLINE(scx*( from[i]-xmin), scy*(a[i]*from[i] +b[i] -ymin),
   scx*( to[i]  -xmin), scy*(a[i]*to[i]   +b[i]-ymin ));
   }

PS "0 setlinewidth 0 setgray\n");

PS "%% comm 4\n");
PSMOVE(scx*(0-xmin),scy*(xx[0]-ymin));
for(i=0;i<ndata;i++) PSDRAW(scx*(i-xmin),scy*(xx[i]-ymin));
STROKE;

PS "%% comm 5\n");
PSMOVE(scx*(0-xmin),scy*(xsmooth[0]-ymin));
for(i=0;i<ndata;i++) PSDRAW(scx*(i-xmin),scy*(xsmooth[i]-ymin));
STROKE;

totdata=0;
for(i=0;i<nbreaks;i++) totdata+=to[i]-from[i];
averageerror=totalerror/totdata;

ymax=ymin=0.0;
for(i=0;i<ndata;i++)if(ymax<xdiff[i])ymax=xdiff[i];
for(i=0;i<ndata;i++)if(ymax<-xdiff[i])ymax= -xdiff[i];

PSMOVE(scx*(0-xmin),xdiff[0]/ymax*40.0+850.0);
for(i=0;i<ndata;i++) PSDRAW(scx*(i-xmin),xdiff[i]/ymax*40.0+850.0);
STROKE;

PSLINE(scx*(hint-xmin),800.0,scx*(hint-xmin),  0.0);
}

void PSdisplaynumbers()
{
int i,totdata;
double toterrbars;

totdata=0;
for(i=0;i<nbreaks;i++) totdata+=to[i]-from[i];


PSMOVE(850.0,680.0); PS "(File: %s) show\n",theargv[currentfile]); 
PSMOVE(850.0,650.0); PS "(Segments: %d) show\n",nbreaks); 
PSMOVE(850.0,620.0); PS "(Data: %d frames) show\n",ndata); 
PSMOVE(850.0,590.0); PS "(Used: %d frames) show\n",totdata); 
PSMOVE(850.0,560.0); PS "(Range: %4.1f pixels) show\n",maxpos-minpos); 
PSMOVE(850.0,530.0); PS "(Kernel: %3.1f pixels) show\n",sqrt(kernelwidth)); 
PSMOVE(850.0,500.0); PS "(<E> : %f)  show\n",totalerror/totdata); 
PSMOVE(-10.0,-30.0); PS "(%2.0f) show\n",mintime);
PSMOVE(780.0,-30.0); PS "(%2.0f) show\n",maxtime);
PSMOVE(810.0,  0.0); PS "(%4.1f) show\n",minpos);
PSMOVE(810.0,790.0); PS "(%4.1f) show\n",maxpos);

toterrbars=0;
for(i=0;i<nbreaks;i++)toterrbars+=errspd[i];
}


void PSredisplayall()
{
printf("PRINTING!!!\n");
psout=popen("lpr -l","w");
//PS "%%%! PostScript dump by MOM\n");
PS "/mls { moveto lineto stroke } bind def\n");
PS "0 setlinewidth\n");
PS "90 rotate 50 -580 translate\n");
PS "0.63 0.63 scale\n");
PS "/Times-Roman findfont 30 scalefont setfont\n");
updatebreaks();
fitdata();
updatehint();
PSdisplaydata();
PSdisplaynumbers();
PS "showpage\n");
fclose(psout);
printf("FINISHED PRINTING\n");
}
