/* getbraided.h

 This unit reads braided worm numbering used to display
 worm tracks
 revised DAW 2/28/02
 
 */

int braidedinit = 0;
FILE *braidfile;

void initbraids()
{
char c1, c2;

if( (braidfile=fopen("braid","r")) == NULL)
  { fprintf(stderr,"Could not open the braid file\n"); exit(1); }

c1=getc(braidfile); c2=getc(braidfile);
if( (c1!='_') || (c2!='B') )
  { fprintf(stderr,"Wrong header for a braid file!\n"); exit(1); }

fprintf(stderr,"Opening the braid file.\n");

braidedinit=1;
}

int getframe(int frame, int *x)
{
int i,frm,obs;
char c;

if(!braidedinit) initbraids();

c=getc(braidfile);
if(c!='F') { fprintf(stderr,"Wrong frame header in braid file\n"); 
		exit(1); }
frm=getw(braidfile); obs=getw(braidfile);
if(frm == -1) return(0);
for(i=0;i<obs;i++) { x[i]=getw(braidfile); }
x[obs]=0;

if(frm>frame) return(0);
if(frm==frame) return(1);

fprintf(stderr,"Skipping %5d -> %5d: ",frm,frame);
for(; frm<frame;)
   {
   c=getc(braidfile); 
   if(c!='F') { fprintf(stderr,"Wrong frame header in braid\n"); exit(1); }
   frm=getw(braidfile); obs=getw(braidfile);
   if(frm == -1)return(0);
   for(i=0;i<obs;i++) { x[i]=getw(braidfile); }
   x[obs]=0;
   if(frm%100==0){fprintf(stderr,"."); fflush(stderr); }
   }
    fprintf(stderr,"\n");

    if(frm>frame) return(0);
    if(frm==frame) return(1);
    return 0;             /* ANSI C requires main to return int. */
}

int getnextframe(int *frame, int *objects, int *x)
{
int i,frm,obs;
char c;

if(!braidedinit) initbraids();

c=getc(braidfile);
if(c!='F') { fprintf(stderr,"Wrong frame header\n"); exit(1); }
frm=getw(braidfile); obs=getw(braidfile);
if(frm == -1)return(0);
*objects=obs;
for(i=0;i<obs;i++) { x[i]=getw(braidfile); }

*frame=frm;
return(1);
}
