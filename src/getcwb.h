/* getcwb.h
Handle reading of CWB files.
Marcello O. Magnasco 1993
*/

FILE *cwb_file;   /* the file handle for CWB in */
char c1, c2;      /* two chars used in reading headers */

/* Here come the CWB input routines. The format of the CWB is:
Compressed Worm Bitmap: cwb.framenumb
Header: _B%w%w          begin frame, end frame;
Frame: FF%w%w           frame #, number of rectangles in frame
Rect: RR%w%w%w%w        x0,yy0,x1,yy1 (bounding box)followed by(yy1-yy0+1)lines:
Line: LL[%c]            an arbitrary number of chars, saying how many
                        (alternatively black and white) pixels to fill in.
*/

/* ------------------------------------------ Open CWB for input, get header */

void cwb_getheader(FILE **filein,   /* output: file handle */
                   char *fn,        /* input: file name */
                   int *beginframe, /* output: starting frame */
                   int *endframe)   /*         and end frame */
{
if(NULL==(cwb_file=fopen(fn,"r")))                   /* Try to open the file */
  { printf("Cant read %s\n",fn); exit(1); }              /* couldn't open it */
*filein=cwb_file;                                      /* export file handle */
c1=getc(cwb_file); c2=getc(cwb_file);                   /* Get the header _B */
if((c1!='_') || (c2!='B'))                           /* not the right header */
  {
  printf("cwb_getheader: wrong header: %c%c\n",c1,c2);
  fclose(cwb_file); exit(1);
  }

*beginframe=getw(cwb_file);
*endframe=getw(cwb_file);                         /* Get startframe endframe */
TALK printf("Opened cwb file %s for reading\n",fn);     /* spread the gossip */
}

/* ---------------------------------------------------- Get CWB frame header */

void cwb_getframe(int *frm, int *rcts)   /* return frame and number of rects */
{
c1=getc(cwb_file); c2=getc(cwb_file);             /* Get the frame header FF */
if((c1!='F')||(c2!='F'))                                         /* Check it */
  { 						      /* wrong; gettahellout */
  printf("cwb_getframe: wrong frame header: %c%c\n",c1,c2);
  fclose(cwb_file);
  exit(1);
  }
*frm  =getw(cwb_file);                               /* Get the frame number */
*rcts=getw(cwb_file);                 /* Get the number of totrects in frame */
}

/* ------------------------------------------------ Get CWB rectangle header */

void cwb_getrect(int *x0, int *yy0, int *x1, int *yy1)   /* output rect bounds */
{
c1=getc(cwb_file); c2=getc(cwb_file);                /* Get rectangle header */
if((c1!='R')||(c2!='R'))                                         /* Check it */
  { printf("Wrong rect header: %c%c\n",c1,c2); /* wrong; exit parsimoniously */
  fclose(cwb_file); exit(1); 
  }

*x0=getw(cwb_file); *yy0=getw(cwb_file);         /* Get the boundaries of the */
*x1=getw(cwb_file); *yy1=getw(cwb_file);                         /* rectangle */
}

/* ----------------------------------------------------- Get CWB line header */

void cwb_getline()
{
c1=getc(cwb_file); c2=getc(cwb_file);                  /* get line header LL */
if((c1!='L')||(c2!='L'))                                         /* Check it */
  { printf("Wrong line header GETLINE: %c%c\n",c1,c2); /* wrong; can you believe it? */
    fclose(cwb_file); exit(1);
  }
}

/* ---------------------------------------- Get entire object into a string */

int cwb_getbitmap(char *bitmap, int xx0,int yy0,int xx1,int yy1)
{
int line; char c1, c2;

int bp, cnt;

int area;

area=0;
bp=0;

for(line=yy0;line<=yy1;line++)
   {
   c1=getc(cwb_file); c2=getc(cwb_file);
   if((c1!='L')||(c2!='L'))
   { printf("Wrong line header GETBITMAP: %c%c\n",c1,c2); fclose(cwb_file); exit(1);}
   bitmap[bp++]='L'; bitmap[bp++]='L';
   cnt=xx0;
   while(cnt<xx1)
        {
        c1=getc(cwb_file); c2=getc(cwb_file);
        bitmap[bp++]=c1; bitmap[bp++]=c2;
        cnt+= (c1+c2);
        area+=c2;
        }
   }

bitmap[bp++]=0; bitmap[bp++]=0; /* Double terminator just for fun */

return(bp);
}

/* ------------------------------------------- Two flushing routines now: */
void cwb_flushbitmap(int xx0,int yy0,int xx1,int yy1)
{
int line; char c1, c2;

int cnt;


for(line=yy0;line<=yy1;line++)
   {
   c1=getc(cwb_file); c2=getc(cwb_file);
   if((c1!='L')||(c2!='L'))
   { printf("Wrong line header FLUSHBITMAP: %c%c\n",c1,c2); fclose(cwb_file); exit(1);}
   cnt=xx0;
   while(cnt<xx1)
        {
        c1=getc(cwb_file); c2=getc(cwb_file);
        cnt+=c1+c2;
        }
   }
}

/* -------------------------------------- Flush the current object */

void cwb_flushobject()
{
int a,b,c,d;

cwb_getrect(&a,&b,&c,&d);
cwb_flushbitmap(a,b,c,d);
}

void cwb_flushframe(int obj)
{
int i;

for(i=0;i<obj;i++) cwb_flushobject();
}
/* ------------------------------------------------------ Close the CWB file */

void cwb_close()
{
fclose(cwb_file);
}
