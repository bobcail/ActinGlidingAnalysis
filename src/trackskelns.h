/*  trackskelns.h

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

#define ITERS 15                  /* Number of iterations of the skinning */
#define SKRADI 3                  /* The `radius' of the skinning circle */
#define SIZEX 1000                /* Size of the bitmap in use */

int patt[2*SKRADI+1][2*SKRADI+1][2][2]; /* Overlay mask (circle) */
int pxs;                          /* Number of pixels */

/*----------------------------------------------------------- Setup Routines */

void construct_patt()            /* Construct the overlay circle */
{
int cx, cy,sprx,spry,spx,spy;

for(spx=0;spx<2;spx++)
for(spy=0;spy<2;spy++)
for(cx= -SKRADI;cx<=SKRADI;cx++)
for(cy= -SKRADI;cy<=SKRADI;cy++)
   {
   sprx=2*cx+spx;
   spry=2*cy+spy;
   patt[cx+SKRADI][cy+SKRADI][spx][spy]=
        ( (sprx*sprx+spry*spry) <= (4*SKRADI*SKRADI) );
   }
}

/************************ Serious Procedures *********************************/

/* ----------------------------------------------- Skeletonization procedure */

void skel()
{
int bx,by,cx,cy;              /* Basepoint, Circular displacement */
int tot;                      /* Total points inside circle */
int repeat;                   /* Looping variable for outmost loop */
int nx,ny;                    /* New point (rounded c.o.m.) */
double comx, comy;            /* c.o.m. in double */
int spx, spy;                 /* subpixel looping */


for(repeat=0;repeat<ITERS;repeat++)
   {
   for(bx=0;bx<650;bx++)         /* Loop over basepoints */
      for(by=0;by<500;by++)
         if(h[bx][by] & 01)                           /* If basepoint white */
           for(spx=0;spx<2;spx++) for(spy=0;spy<2;spy++)    /* loop subpixel */
              {
              comx=comy=0.0; tot=0;
              for(cx= -SKRADI;cx<=SKRADI;cx++)           /* Loop on the mask */
              for(cy= -SKRADI;cy<=SKRADI;cy++)
                 if(patt[cx+SKRADI][cy+SKRADI][spx][spy])   /* if mask white */
                   if(h[bx+cx][by+cy] & 01)              /* and pixel white */
                     {
                     comx+=bx+cx; comy+=by+cy; ++tot;     /* contrbte to COM */
                     }
              if(tot==0)printf("ERROR tot=0!!!\n");     /* tot>1 (assertion) */
              comx/=tot; comy/=tot;                               /* get COM */
              nx=rint(comx); ny=rint(comy);              /* round COM to pxl */
                           /* Assertion: COM can't lie outside of the circle */
              if(abs(nx-bx)>SKRADI)printf("Error nx=%d bx=%d\n",nx,bx);
              if(abs(ny-by)>SKRADI)printf("Error ny=%d by=%d\n",ny,by);
              h[nx][ny]+=4;                               /* Plot the point */
              }

   for(bx=0;bx<650;bx++)         /* Loop over basepoints */
      for(by=0;by<500;by++)
         h[bx][by]= (h[bx][by]>1);         /* shift down the pixmap */

   } /* End repeat */

}

/* ----------------------------------------------------------- Strip Doubles */

void stripdoubles()                       /* Strip doubles */
{
int i,j;           /* The looping variables */
int u,d,l,r;       /* up down left right */

for(i=0; i<=650; i++)                 /* loop over basepoints */
   for(j=0; j<500; j++)
      if(h[i][j]>0)                                         /* if white pxl */
        {
        if(h[i][j]!=1)                     /* Assertion: pixmap is b&w only */
          printf("ERROR@stripdoubles: h[%d][%d] != 1\n",i,j);
        r=h[i+1][j];                                      /* right left etc */
        l=h[i-1][j];
        u=h[i][j+1];
        d=h[i][j-1];
        if((r+l+u+d)==2)                          /* if two first neighbours */
          if( (r||l) && (u||d) )                      /* and at right angles */
            {
            h[i][j]=0;                                 /* then remove point */
            }
          else                                /* 2 f.n. but not right angles */
            {
            }
        else                                                    /* f.n. != 2 */
          {
          }
        }
}

/* ------------------------------------------------------------- Get Skinned */

int getskinned() /* Get the appropiate neighs */
{
int i,j;                   /* baseline loop */
int k,l,t;                   /* first neighbour loop */
int cero,one,two,three,more;     /* pixels having $1 neighbours */
int nei;                   /* neighbours of this pixel */
int nx,ny;
int t1i, t1j, t2i, t2j;


/*
   First stage: checking for a kosher topology. In current incarnation,
   all non kosher topologies are discarded. A kosher topology is defined as
   that for which the filament is a homeomorphic image of [0,1]: i.e., only
   two tips, and all other points have precisely two neighbours; no isolated
   points or triple connections yet.
*/

pxs=cero=one=two=three=more=nei=0;

for(i=0;i<650;i++) 				    /* loop over base point */
   for(j=0;j<500;j++)
      if(h[i][j])                                               /* if white */
        {
        if(h[i][j]!=1)                              /* assert pixmap is b&w */
          printf("Error@getskinned h[%d][%d] != 1\n",i,j);
        pxs++;
        nei=0;
        for(k= -1;k<2;k++)                           /* loop over neighbours */
           for(l= -1;l<2;l++)
              nei+=h[i+k][j+l];                     /* count how many white */
        nei--;                                        /* we counted this one */

        if(nei==1)
          {
          if(one==0) { t1i=i; t1j=j; }
          else       { t2i=i; t2j=j; }
          } /* if this is a tip make a note of it */

/* histogram number of neighbours */
        if(nei==0) { pxs--; h[i][j]=0; }
        else if(nei==1) {  one++; }
        else if(nei==2) {  two++; }
        else if(nei==3) {three++; }
        else            { more++; }

        }

printf("Topology of this track (as far as I can see):\n");
printf("0=%d    1=%d    2=%d    3=%d    more=%d\n",cero,one,two,three,more);

if(   (one!=2)                 /* the appropriate thing is to have two tips, */
    ||(cero>0)                                        /* no isolated points, */
    ||(three>0)                                      /* and no triple points */
    ||(more>0))                                      /* and no ntuple points */
  {                                         /* so if worm is not appropriate */
  printf("I'm awfully sorry, but I can't do this yet!\n");          /* shout */
  return(0);                                       /* and come back tomorrow */
  }
  
if( pxs<15) // This sets a minimum track length of 30 pixels and could be decreased. lets try 15 DAW
  {
  printf("Track is too short to be of use: %d pixels\n",pxs);
  return(0);
  }

/*
   Second stage: getting the actual filament out. We proceed from one tip
   (which one is immaterial, supposedly; actual one is the last tip scanned)
   and keep outputting data until we hit the other tip. We mark points already
   used with a 2, so as not to lose the bitmap, just in case.
*/

if((one+two)!=pxs) /* Assertion: total length equals bound plus middle */
  { printf("Logic@getskinned: %d %d %d\n",one,two,pxs); exit(1);} /* barf! */ 

printf("OK, this is a kosher track topology, I think\n");
if( (h[t1i][t1j] != 1) || (h[t2i][t2j] != 1) )
  { printf("Logic@getskinned: tip outside the skeleton\n"); }
if( (m[t1i][t1j] == 0) || (m[t2i][t2j] == 0) )
  { printf("Logic@getskinned: tip outside the track\n"); }

if( m[t1i][t1j] < m[t2i][t2j] )
  { /* Tip one came first */
  i=t1i; j=t1j;
  }
else 
  { /* Tip two came first */
  i=t2i; j=t2j;
  }

printf("Tips: (%d,%d):%d (%d,%d):%d; (%d,%d) is first\n",
t1i,t1j,m[t1i][t1j], t2i,t2j,m[t2i][t2j], i,j); 

fx[0]=i; fy[0]=j;

for(t=1;t<pxs;t++)
   {
   h[i][j]=2;
   nx= -1075;
   for(k= -1;k<2;k++)for(l= -1;l<2;l++) /* loop over neighbours */
      if(h[i+k][j+l]==1) /* found the next guy */
        {
        nx=i+k; ny=j+l; 
        }
   if(nx== -1075) /* one neighbour should have been found! */ 
     {
     printf("Logic@getskinned: no more neighbours!\n");
     printf("***** This is truly not supposed to happen!\n");
     len=t+1; return(0); /*len=t+1; sleep(5); return(0); Deleted sleep!*/
     } /* Die Kunst der Fugue */
   i=nx; j=ny;
   fx[t]=i; fy[t]=j;
   }
len=pxs;
return(1);
}

/* -------------------------------------------- Reparametrization procedures */

int findnext(int i, double xx0, double yy0) 
/* given xx0 yy0, get first pointer exceeding r */
{
double yy1,xx1,d2; int j;

for(j=i+1;j<len;j++)
   {
   xx1=fx[j]-xx0; yy1=fy[j]-yy0; 
   d2=xx1*xx1+yy1*yy1;
   if(d2 >= r2)
     {
     return(j);
     }
   }
return(0);
}

void findinte(               /* Find the intersection of circle and segment */
double xx0, double yy0,      /* origin of segment */
double xx1, double yy1,      /* end of segment */
double xc, double yc,        /* center of circle (radius ra) */
double *xi, double *yi)      /* intersection (output) */
{
double a,b,c,t,xx0c,yy0c,xx01,yy01,d0c;

xx0c=xx0-xc; yy0c=yy0-yc;
xx01=xx1-xx0; yy01=yy1-yy0;
d0c=xx0c*xx0c+yy0c*yy0c;
a=xx01*xx01+yy01*yy01;
b=2.0*(xx0c*xx01+yy0c*yy01);
c=d0c-r2; 
t=(-b+sqrt(b*b-4.0*a*c))/(2.0*a);
if((t<0.0)||(t>1.0))
  {
  t=(-b-sqrt(b*b-4.0*a*c))/(2.0*a);
  if((t<0.0)||(t>1.0))
    {
    printf("Logic@findinte: couldn't get t: %f %f %f\n",a,b,c);
    exit(1);
    }
  }
*xi=xx0+t*(xx1-xx0);
*yi=yy0+t*(yy1-yy0);
xx01= *xi-xc;
yy01= *yi-yc;
a=xx01*xx01+yy01*yy01;
if(fabs(a-r2)>1e-9)
  {
  printf("Num@findinte: wrong distance(%f-%f=%e):\n",a,r2,a-r2);
  printf("              between %f %f and %f %f\n",*xi,*yi,xc,yc);
  exit(1);
  }
return;
}

/* ---------------------------------- The reparametrization procedure proper */

int coarsegrain()
{
int ci, i,j;
double cx,cy,nx,ny;

ci=0; cx=fx[0]; cy=fy[0]; j=1;
x[1]=cx; y[1]=cy;
for(;(i=findnext(ci,cx,cy));) /* added parentheses*/
   {
   findinte(fx[i-1],fy[i-1], fx[i],fy[i], cx,cy, &nx,&ny);
   j++; cx=nx; cy=ny; ci=i;
   x[j]=cx; y[j]=cy;
   }
for(i=1;i<j;i++){dx[i]=x[i+1]-x[i]; dy[i]=y[i+1]-y[i];}


x[0]=x[1]-dx[1];
y[0]=y[1]-dy[1];
x[j+1]=x[j]+dx[j-1];
y[j+1]=y[j]+dy[j-1];

return(j+2);
}

double nearestpoint(int u, int v)
{
int i;
double t,d, dx, dy;
double x1,y1,x0,y0;
double smdist, smarcl;

x0=u-x[0]; y0=v-y[0]; smdist=sqrt(x0*x0+y0*y0); smarcl=0.0;
for(i=1;i<lre;i++)
   {
   x0=x[i-1]; y0=y[i-1]; x1=x[i]; y1=y[i];
   t= ((u-x0)*(x1-x0)+(v-y0)*(y1-y0)) / ((x1-x0)*(x1-x0)+(y1-y0)*(y1-y0));
   if(t<0.0) t=0.0;
   else if (t>1.0) t=1.0;
   dx= x0+t*(x1-x0) - u; dy=y0+t*(y1-y0)-v;
   d=sqrt(dx*dx+dy*dy);
   if(d<smdist)
     {
     smdist=d;
     smarcl=ra*((i-1)+t);
     }
   }
return(smarcl);
}

void projectfile(char *fname)
{
int frm,liney;
int x0,y0,x1,y1;
int cnt,a1,a2;
FILE *fin;
FILE *fout;
char c1,c2;
char str[80];
int i;
double front, back, average;

if(NULL==(fin=fopen(fname,"r")))
  {
  printf("Cant read %s\n",fname);
  exit(1);
  }

for(i=0;i<80;i++)str[i]=0;
for(i=0;fname[i];i++)str[i]=fname[i];
str[0]='a'; str[1]='s'; str[2]='c';
if(NULL==(fout=fopen(str,"w")))
  {
  printf("Cant write %s\n",str);
  exit(1);
  }
printf("Opened ascii file %s for writing\n",str);

c1=getc(fin);
c2=getc(fin);
if((c1!='_') || (c2!='S'))
  {
  printf("wrong swb header: %c%c\n",c1,c2);
  fclose(fin);
  exit(1);
  }
frm=0;
for(;frm != -1;)
   {
   c1=getc(fin); c2=getc(fin);
   if((c1!='F')||(c2!='F'))
     {
     printf("Wrong frame header: %c%c\n",c1,c2);
     fclose(fin);
     exit(1);
     }
   points=0;
   frm=getw(fin);
   x0=getw(fin);
   x1=getw(fin);
   y0=getw(fin);
   y1=getw(fin);
   if( frm == -1 ) break;
   a1 = frm % COLS;
   
   for(liney=y0;liney<=y1;liney++)
      {
      c1=getc(fin);
      c2=getc(fin);
      if((c1!='L')||(c2!='L'))
        {
        printf("Wrong line header: %c%c\n",c1,c2);
        fclose(fin);
        exit(1);
        }
      cnt=x0;
      while(cnt<x1)
           {
           a1=getc(fin);
           a2=getc(fin);
           cnt+=a1;
           if(a2>0)
             for(i=cnt;i<=(cnt+a2);i++)
                arclengths[points++]=nearestpoint(i,liney);
           cnt+=a2;
           }
      }
   /* Now we have in arclength a vector with all the relevant points */
   front=back=arclengths[0];
   average=0;
   for(i=0;i<points;i++)
      {
      if(arclengths[i]>front)front=arclengths[i];
      if(arclengths[i]<back )back =arclengths[i];
      average+=arclengths[i];
      }
   average/=points;
   fprintf(fout,"%5d %7.1f %7.1f %7.1f\n",frm,average,front,back);   
   if(frm%100==0)printf("%5d %7.1f %7.1f %7.1f\n",frm,average,front,back);
   }
printf("Done with the ascii file\n\n\n");
fclose(fin);
fclose(fout);
}

void dumptrack(char *fname)
{
FILE *fout;
char tname[80];
int i,j;

for(i=0;i<80;i++)tname[i]=0;
for(i=0;fname[i];i++)tname[i]=fname[i];

tname[0]='t'; tname[1]='r'; tname[2]='k';

if( (fout=fopen(tname,"w")) == NULL)
  {
  printf("Sorry, can't open trackfile %s\n",tname);
  return;
  }

for(i=0;i<lre;i++)
   fprintf(fout,"%4.1f %4.1f\n",x[i],y[i]);
fclose(fout);
}
