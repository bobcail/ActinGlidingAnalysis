/*  speeddisplay.h
    The display routines for the speed.c program
    
    Revised DAW 3/08/02
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

void updatebreaks()
{
int	i, j, alt;

alt=1;
for(;alt>0;)
   {
   alt=0;
   for(i=0;i<nbreaks;i++)
      if(from[i]>to[i])
        {
        alt++;
        for(j=i+1;j<nbreaks;j++)
           {
           from[j-1]=from[j]; to[j-1]=to[j];
           }
        i--; nbreaks--;
        }

   for(i=1;i<nbreaks;i++)
      if(from[i]<to[i-1])
        {
        alt++;
        for(j=i;j<nbreaks;j++) to[j-1]=to[j];
        for(j=i;j<(nbreaks-1);j++) from[j]=from[j+1];
        nbreaks--;
        }
   }
}


void displaydata()
{
double	xmin, xmax, ymin, ymax, scx, scy;
int	i,totdata; 

xmin=0; xmax=ndata;
ymin=ymax=xx[0];

for(i=0;i<ndata;i++)if(xx[i]<ymin)ymin=xx[i];
for(i=0;i<ndata;i++)if(xx[i]>ymax)ymax=xx[i];

maxpos=ymax; minpos=ymin;
mintime=xmin; maxtime=xmax;

scx=(4*ww/5-5)/(xmax-xmin); scy=(14*boxheight)/(ymax-ymin);

FITCOLOR;
glLineWidth(3.0);
    for(i=0;i<nbreaks;i++)
    {
        glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(from[i]-xmin), scy*(a[i]*from[i]+b[i]-ymin));
        glVertex2f(scx*(to[i]-xmin), scy*(a[i]*to[i]+b[i]-ymin));
        }
        glEnd();
    }

DATACOLOR;
glLineWidth(2.0);
    for(i=0;i<ndata-1;i++)
    {
        glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(i-xmin),scy*(xx[i]-ymin));
        glVertex2f(scx*(i+1-xmin), scy*(xx[i+1]-ymin));
        }
        glEnd();
    }

SMTHCOLOR;
glLineWidth(1.0);
    for(i=0;i<ndata-1;i++)
    {
        glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(i-xmin), scy*(xsmooth[i]-ymin));
        glVertex2f(scx*(i+1-xmin), scy*(xsmooth[i+1]-ymin));
        }
        glEnd();
    }

    totdata=0;
    for(i=0;i<nbreaks;i++) totdata+=to[i]-from[i];
    averageerror=totalerror/totdata;

    ymax=ymin=0.0;
    for(i=0;i<ndata;i++)if(ymax<xdiff[i])ymax=xdiff[i];
    for(i=0;i<ndata;i++)if(ymax<-xdiff[i])ymax= -xdiff[i];

    glColor3f(1.0,0.5,0.5);
    for(i=0;i<ndata-1;i++)
    {
        glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(i-xmin), xdiff[i]/ymax*30.0+15*boxheight);
        glVertex2f(scx*(i+1-xmin), xdiff[i+1]/ymax*30.0+15*boxheight);
        }
        glEnd();
    }
    
glLineWidth(2.0);	//The yellow (left) break marker
    glColor3f(1.0,1.0,0.0);
    for(i=0;i<nbreaks;i++)
    {
       glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(from[i]-xmin),14*boxheight+1);
        glVertex2f(scx*(from[i]-xmin),14*boxheight+8);
        }
        glEnd();
    }

glLineWidth(2.0);	//The cyan (right) break marker
    glColor3f(0.0,0.6,1.0);
    for(i=0;i<nbreaks;i++)
    {
        glBegin(GL_LINE_STRIP);
        {
        glVertex2f(scx*(to[i]-xmin),14*boxheight+1);
        glVertex2f(scx*(to[i]-xmin),14*boxheight+8);
        }
        glEnd();
    }

glLineWidth(1.0);	//The hint line
    glColor3f(1.0,0.4,0.4);
    glBegin(GL_LINE_STRIP);
    {
    glVertex2f(scx*(hint-xmin),14*boxheight);
    glVertex2f(scx*(hint-xmin),0);
    }
    glEnd();
}


void displaynumbers()
{
int	i, totdata;
double	toterrbars;

totdata=0; 
for(i=0;i<nbreaks;i++) totdata+=to[i]-from[i];
toterrbars=0;
for(i=0;i<nbreaks;i++)toterrbars+=errspd[i];
    
for (i=0; i<9; i++) resetTextInfo (newinfo.line[i]);
    sprintf (newinfo.line[0], "File: %s", theargv[Thisfile]);
    sprintf (newinfo.line[1], "Segm: %3d",nbreaks);
    sprintf (newinfo.line[2], "Data: %3d frames",ndata);
    sprintf (newinfo.line[3], "Used: %3d frames",totdata);
    sprintf (newinfo.line[4], "Rang: %5.1f pixels",maxpos-minpos);
    sprintf (newinfo.line[5], "<E> : %5.1f ", totalerror/totdata);
    sprintf (newinfo.line[6], "KWid: %5.1f pixels",sqrt(kernelwidth));
    sprintf (newinfo.line[7], "TotE: %8.4f",toterrbars);
    sprintf (newinfo.line[8], "File: %3d of %3d",Thisfile, file_num); //file_num here

    glColor3f(0.0,0.0,0.0);
    screen_box(ww*4/5,wh-5*boxheight,ww,wh); /*clear the screen before data update*/

for (i=0; i<9; i++)
    {
    if (i==4) glColor3f(1.0, 0.5, 0.0);
    else if (i==7) glColor3f(1.0, 0.0, 1.0);
    else glColor3f(1.0, 1.0, 1.0);
    writeText (boxleft, wh-(i+1)*texth, newinfo.line[i]);
    }
}


void redisplayall()
{
    glColor3f(0.0, 0.0, 0.0);
    screen_box (0, 0, 4*ww/5, wh);
    glColor3f(0.0, 0.0, 0.6);
    screen_box (0, 0, 4*ww/5-5, 14*boxheight);
    updatebreaks();
    fitdata();
    updatehint();
    displaydata();
    displaynumbers();
}
