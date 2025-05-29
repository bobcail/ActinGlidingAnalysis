/* asciibatchns.c

   Now, this is the real stuff. Move through the swb.*
   files and analyze to create the asc.* files
   Procedures:
   skeletonize the track,
   stripdoubles,
   arclength parametrize,
   reopen the file, 
   go frame by frame and compute distances to the track,
   accumulate positions in a vector,
   sort the vector, 
   output in an ascii.* file the arclength position as a function of time.

   Marcello O. Magnasco, Rockefeller Univ. 1994
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define COLS 1000
#define RADIUS 11.287364234

int red[COLS], grn[COLS], blu[COLS];
int m[700][500], h[700][500];

int len;
int lre;
double fx[2000], fy[2000];   /* The position vectors as digitized */
double dx[2000], dy[2000];   /* The tangent vectors after reparam */
double x[2000], y[2000];     /* The position vectors after reparam */
double ra, r2;

double arclengths[1000];
int points;


#include "trackskelns.h"

/***************************************************************************/

int fileexists(char *fname)
{
FILE *ff;

if( (ff=fopen(fname,"r")) == NULL)
  return(0);
fclose(ff);
return(1);
}

int asciiexists(char *fname)
{
int i; char aname[80];
for(i=0;i<80;i++)aname[i]=0;
for(i=0;fname[i];i++)aname[i]=fname[i];
aname[0]='a'; aname[1]='s'; aname[2]='c';
return(fileexists(aname));
}


void makepalette()
{
int i;
double x;

for(i=0;i<COLS;i++)
   {
   x=0.5*(cos(2*M_PI*(i - 000.0)/COLS) + 1.0); red[i]=rint(255.0*x*x);
   x=0.5*(cos(2*M_PI*(i - 333.3)/COLS) + 1.0); grn[i]=rint(255.0*x*x);
   x=0.5*(cos(2*M_PI*(i - 666.6)/COLS) + 1.0); blu[i]=rint(255.0*x*x);
   }
}

void displayfile(char *fname)
{
int frm,liney;
int xx0,yy0,xx1,yy1;
int cnt,a1,a2;
FILE *fin;
short vv[2];
char c1,c2;
char str[80];
int i;

for(yy0=0;yy0<500;yy0++)for(xx0=0;xx0<700;xx0++)m[xx0][yy0]=0;

if(NULL==(fin=fopen(fname,"r")))
  {
  printf("Cant read %s\n",fname);
  exit(1);
  }

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
   frm=getw(fin);
   xx0=getw(fin);
   xx1=getw(fin);
   yy0=getw(fin);
   yy1=getw(fin);
   if( frm == -1 ) break;
   a1 = frm % COLS;
   for(liney=yy0;liney<=yy1;liney++)
      {
      c1=getc(fin);
      c2=getc(fin);
      if((c1!='L')||(c2!='L'))
        {
        printf("Wrong line header: %c%c\n",c1,c2);
        fclose(fin);
        exit(1);
        }
      cnt=xx0;
      while(cnt<xx1)
           {
           a1=getc(fin);
           a2=getc(fin);
           cnt+=a1;
           vv[0]=cnt;
           vv[1]=liney;
           if(a2>0)
              {
              for(i=cnt;i<=(cnt+a2);i++)m[i][liney]=frm;
              }
           cnt+=a2;
           }
      }
   }
fclose(fin);
}

void analyze(char *fname)
{
int i,j,k;
char command[300];

construct_patt();

for(i=0;i<3;i++)for(j=0;j<500;j++)m[i][j]=0;
for(i=0;i<650;i++)for(j=0;j<3;j++)m[i][j]=0;

for(i=0;i<650;i++) for(j=0;j<500;j++)
   if(m[i][j]) h[i][j]=1;
   else        h[i][j]=0;

for(i=0;i<650;i++) for(j=0;j<500;j++)
   if((k=m[i][j]))
     {
     k %= COLS;
     }

skel();
stripdoubles();

if(getskinned())
  {
  ra=RADIUS; r2=ra*ra;
  lre=coarsegrain();
  printf("There are %d segments in the reparametrization\n",lre);
  dumptrack(fname);
  projectfile(fname);
  }
else
  {
  sprintf(command,"rm %s",fname);
  printf("File %s fails. I will delete it.\nCOMMAND= %s\n",fname,command);
  system(command);
  }

}


int main(int argc, char **argv)
{
int file;
char comm[100];

if(argc<2)
  {
  printf("Usage: %s files\n",argv[0]);
  exit(1);
  }

for(file=1;file<argc;file++)
   {
   if(asciiexists(argv[file])) continue;
   printf("\n\n\nFILE %s \n",argv[file]);
   displayfile(argv[file]);
   analyze(argv[file]);
   }

printf("Bye bye!\n");
exit(0);
}
