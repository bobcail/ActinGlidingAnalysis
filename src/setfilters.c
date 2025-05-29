/* setfilters.c

    Adjust threshholding and filters with OpenGL editing window.
    Open a IpLab B&W byte image stack, select a central image and
    test filter parameters on the image. Saves a ".Parameters" file
    for use by iplworms.

    Original ideas by M.O.M. NEC 1993
    Rewritten and updated by Donald Winkelmann, Feb. 2002

    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/
#ifdef LINUX
#include <GL/glut.h>
#else 
#include <GLUT/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <math.h>
#include <time.h>
#include <unistd.h>

#include "iplfile.h"
#include "setfilter.h"
#include "parameters.h"
#include "bitmaps.h"
#include "topofilters.h"
#include "linearfilters.h"

#define STATS 8
int pr[256], pg[256], pb[256];
int leftmouse=0, middlemouse=0, rightmouse=0;
short int nnz;
long int  nnx, nny;
int action_mode;
char mydir[128];

#define	OFFSET		1
#define	SDCOEFF		2
#define	WARPX		3
#define	WARPY		4
#define	SCALEX		5
#define	SCALEY		6
#define	UNFILTERED	7
#define	PARAMS		8
#define SETFILTER	9
#define	GRAPHICS	10
#define	EXPAND		11
#define	CLIPHI		12
#define	CLIPLOW		13
#define	SEPARATE	14

int histo[256];
double sum;
int num;
char *thefilename, *thecommandname;
int tiff=0;

unsigned long orig[480][640];
unsigned long disp[480][640];
unsigned long auxm[480][640];

#include "read_tiff.h"
#define M disp
#include "floodfill.h"

int orx=0, ory=0;
int setfilter=0;
int mx,my;
int ww=640, wh=700;
int active=1;

int pick(int, int);
void mydisplay();
void writeText (int, int, char* );
void screen_box(int, int , int, int );
void right_menu(int);
void myinit(void);
void mouse(int, int, int, int);
void closeAll(void);
void mydisplay(void);
void resetTextInfo();
void updateimg();
void showorig();

typedef struct textinfo {
char line[9][80];
}TEXTINFO;
#define WHITECOLOR glColor3f(1.0, 1.0, 1.0);
TEXTINFO newinfo;


void resetTextInfo (char *str)
{
	unsigned int i;
	for (i=0; i<strlen(str); i++)
		str[i] = '\0';
}


void writeText (int x, int y, char* str)	//write strings starting location (x, y)
{
	int shift = 0;
	unsigned int i;

	for (i=0; i<strlen(str); i++)
        {
            glRasterPos2i(x+shift, y);
            glutBitmapCharacter (GLUT_BITMAP_9_BY_15, str[i]);
            shift += glutBitmapWidth (GLUT_BITMAP_9_BY_15, str[i]);
	}
}

 
int pick(int x, int y)		//Coordinate definitions for control of setfilters
{
    y = wh - y;

    if(x<640 && y<480) return GRAPHICS;
    if (x<550) {
    if(x>420 && y > 620) return SETFILTER; 
    else if(x>380 && y > 600) return PARAMS;
    else if(x>380 && y > 585) return UNFILTERED;
    else if(x>380 && y > 570) {
        if (setfilter!=1) return SCALEY;
        else return EXPAND;}
    else if(x>380 && y > 555) {
        if (setfilter!=1) return SCALEX;
        else return CLIPHI;}
    else if(x>380 && y > 540) {
        if (setfilter!=1) return WARPY;
        else return CLIPLOW;}	
    else if(x>380 && y > 525) {
        if (setfilter!=1) return WARPX;
        else return 0;}
    else if(x>380 && y > 510) {
        if (setfilter!=1) return SDCOEFF;
        else return 0;}
    else if(x>380 && y > 495) {
        if (setfilter!=1) return OFFSET;
        else return SEPARATE;}
    else return 0;
    }
return 0;
}


void screen_box(int x, int y, int sx, int sy )
{
    glBegin(GL_QUADS);
      glVertex2i(x, y);
      glVertex2i(x+sx, y);
      glVertex2i(x+sx, y+sy);
      glVertex2i(x, y+sy);
    glEnd();
}


void right_menu(int id)
{
	if(id == 1) exit(0);
	else mydisplay();
}



void keyboard (unsigned char key, int xx, int yy)
{
    switch (key)
    {
        case 'q':
        case 'Q':
        exit(0);
    }
}


/*callback function, display on-screen menu and gate diagrams upon non-empty list */
void mydisplay()
{
	int i;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 0.0, 0.0);
    screen_box (0, 0, ww, wh);

glColor3f(1.0,0.2,0.2);
screen_box(380,480,220,180); /*clear the screen before data update*/

for (i=0; i<8; i++) resetTextInfo (newinfo.line[i]);
    sprintf (newinfo.line[0], "offset   ");
    sprintf (newinfo.line[1], "sdcoef   ");
    sprintf (newinfo.line[2], "warp x   ");
    sprintf (newinfo.line[3], "warp y   ");
    sprintf (newinfo.line[4], "scale x  ");
    sprintf (newinfo.line[5], "scale y   ");
    sprintf (newinfo.line[6], "Show Unfiltered");
    sprintf (newinfo.line[7], "-> .Parameters");

for (i=0; i<8; i++)
    {
    glColor3f(1.0, 1.0, 1.0);
    writeText (380, 480+(i+1)*15, newinfo.line[i]);
    }

    glColor3f(0.2, 0.2, 1.0); 
        screen_box(420, 620, 140, 30);
	WHITECOLOR;
	writeText (420+9, 630, "Set Filter");
        
    glFlush();
    glPopAttrib();
}


void myReshape (int w, int h)
{
	/* adjusting clipping box */
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
//	gluOrtho2D(0.0, (GLdouble) w, 0.0, (GLdouble) h);
	glOrtho(0.0, (GLdouble)w, 0.0, (GLdouble)h, -1.0, 1.0);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();

	/* adjusting viewport and clear */
	glViewport (0, 0, w, h);
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glFlush();
	/* set global size for use by drawing routine */
	ww = w;
	wh = h;
}


void myinit(void)
{
    glViewport(0,0,ww,wh);

/* Pick 2D clipping window to match size of X window 
This choice avoids having to scale object coordinates
each time window is resized */

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity(); 
    glOrtho(0.0, (GLdouble) ww , 0.0, (GLdouble) wh , -1.0, 1.0);

/* set clear color to black and clear window */

    glClearColor (0.8, 0.8, 0.8, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glFlush();
}


void mouse(int btn, int state, int x, int y)
{
    int where;
    
        where = pick(x,y);

	if(state==GLUT_DOWN) 
        {
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		where = pick(x,y);

		if(where != 0)		//selection mode, select object type to draw
		{
                    action_mode = where;
                    
                    switch(action_mode){

                    case SETFILTER:
                            if (setfilter) setfilter=0;
                            else setfilter=1;
                            displaydata();
                            break;
                    case GRAPHICS:
                            if (btn==GLUT_LEFT_BUTTON) displaydata();
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            break;
                    case OFFSET:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_offset++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_offset--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case SEPARATE:                            
                            updateimg();
                            analyze();
                            displaydata();
                            break;
                    case SDCOEFF:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_sdcoeff++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_sdcoeff--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case WARPX:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_warpx++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_warpx--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case CLIPLOW:
                            if(btn==GLUT_RIGHT_BUTTON) rank_cliplow++;
                            if(btn==GLUT_MIDDLE_BUTTON){updateimg();
                            refocusimage(disp,disp,auxm);
                            refresh(); clearhisto();}
                            if(btn==GLUT_LEFT_BUTTON) rank_cliplow--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case WARPY:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_warpy++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_warpy--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case SCALEX:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_scalex++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_scalex--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case CLIPHI:
                            if(btn==GLUT_RIGHT_BUTTON) rank_cliphi++;
                            if(btn==GLUT_MIDDLE_BUTTON){updateimg();
                            refocusimage(disp,disp,auxm);
                            refresh(); clearhisto();}
                            if(btn==GLUT_LEFT_BUTTON) rank_cliphi--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case SCALEY:
                            if(btn==GLUT_RIGHT_BUTTON) bitmap_scaley++;
                            if(btn==GLUT_MIDDLE_BUTTON) updateimg();
                            if(btn==GLUT_LEFT_BUTTON) bitmap_scaley--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case EXPAND:
                            if(btn==GLUT_RIGHT_BUTTON) rank_xpand++;
                            if(btn==GLUT_MIDDLE_BUTTON){updateimg();
                            refocusimage(disp,disp,auxm);
                            refresh(); clearhisto();}
                            if(btn==GLUT_LEFT_BUTTON) rank_xpand--;
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                           break;
                    case UNFILTERED:
                            if(btn==GLUT_MIDDLE_BUTTON) showorig();
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            displaydata();
                            break;
                    case PARAMS:
                            if (btn==GLUT_LEFT_BUTTON) displaydata();
                            else if (btn==GLUT_MIDDLE_BUTTON)	{
                            dumpnewparms();
                            glColor3f(0.0,0.0,0.0);
                            screen_box(10,wh-40,630,40);
                            glColor3f(1.0,1.0,1.0);
                            writeText (10, wh-20, "'.Parameters' File Saved!"); break;
                            }
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            break;
                    }
                }
       glPopAttrib();
       glFlush();
     }
}


void status(char *m)
{
glColor3f(0.0,0.0,0.0);
screen_box(10,wh-40,630,40);
glColor3f(1.0,1.0,1.0);
writeText (10, wh-20, m);
}

void makepalette()
{
int k, r, j;

for(r=0;r<256;r++)
   { 
   k=(r/1)*1 % 256;
   pr[r]=k;
   pb[r]=255-k;
   if(k<128) pg[r]=2*k;
   else      pg[r]=511-2*k;
   }

pr[0]=pg[0]=pb[0]=0;

for(k=0;k<255;k++)
   {
   glColor3ub(pr[k],pg[k],pb[k]);
glBegin(GL_POINTS);
    for(j=485;j<495;j++) glVertex2s(k+3,j);
glEnd();
   }
}

void refresh()
{
int	i, j, k;

glColor3f(0.0,0.0,0.0);
screen_box(0,0,640,480);

if (nnx<640) orx=(640-nnx)/2;
if (nny<480) ory=(480-nny)/2;

for(j=0;j<nny;j++)for(i=0;i<nnx;i++)	//draw theshholded image
   {
   k=disp[j][i];
   glColor3ub(pr[k],pg[k],pb[k]);
        glBegin(GL_POINTS);
            glVertex2s(i+orx,j+ory);
        glEnd();
   }
}


void clearhisto()
{ int i, j, m, lim;

histogram(disp,histo);
m=0;for(i=1;i<256;i++)if(histo[i]>m)m=histo[i];

glColor3f(0.0,0.0,0.0);
screen_box(0,495,360,180);

for(i=1;i<256;i++)
   {
   glColor3ub(pr[i],pg[i],pb[i]);
   lim=100.0*log(histo[i]+1.0)/log(m+0.0);
    glBegin(GL_POINTS);
        for(j=496;j<495+lim;j++) glVertex2s(i+2,j);
    glEnd();
   }
}


void displaydata()
{
int i;

glColor3f(1.0,0.2,0.2);
screen_box(380,480,220,180); /*clear the screen before data update*/

if (setfilter!=1)
{
for (i=0; i<8; i++) resetTextInfo (newinfo.line[i]);
    sprintf (newinfo.line[0], "offset   %d",bitmap_offset);
    sprintf (newinfo.line[1], "sdcoef   %d",bitmap_sdcoeff);
    sprintf (newinfo.line[2], "warp x   %d",bitmap_warpx);
    sprintf (newinfo.line[3], "warp y   %d",bitmap_warpy);
    sprintf (newinfo.line[4], "scale x  %d",bitmap_scalex);
    sprintf (newinfo.line[5], "scale y  %d",bitmap_scaley);
    sprintf (newinfo.line[6], "Show Unfiltered");
    sprintf (newinfo.line[7], "-> .Parameters");

for (i=0; i<8; i++)
    {
    glColor3f(1.0, 1.0, 1.0);
    writeText (380, 480+(i+1)*15, newinfo.line[i]);
    }

    glColor3f(0.2, 0.2, 1.0); 
        screen_box(420, 620, 140, 30);
	WHITECOLOR;
	writeText (420+9, 630, "Set Filter");

}

else if (setfilter) {
for (i=0; i<8; i++) resetTextInfo (newinfo.line[i]);
    sprintf (newinfo.line[0], "Threshhold and Separate");
    sprintf (newinfo.line[1], "  ");
    sprintf (newinfo.line[2], "  ");
    sprintf (newinfo.line[3], "clipLow  %d",rank_cliplow);
    sprintf (newinfo.line[4], "clipHi   %d",rank_cliphi);
    sprintf (newinfo.line[5], "expand   %d",rank_xpand);
    sprintf (newinfo.line[6], "Show Unfiltered");
    sprintf (newinfo.line[7], "-> .Parameters");

for (i=0; i<8; i++)
    {
    glColor3f(1.0, 1.0, 1.0);
    writeText (380, 480+(i+1)*15, newinfo.line[i]);
    }

    glColor3f(1.0, 1.0, 1.0);
    screen_box(420, 620, 140, 30);
    glColor3f(0.0,0.0,1.0);
	writeText (420+5, 630, "Set Threshhold");
}


glColor3f(0.0,0.0,0.0);
screen_box(10,wh-40,630,40); //clear the screen before data update*

glColor3f(1.0,1.0,1.0);
switch(action_mode)
  {
    case GRAPHICS: writeText (10, wh-20, "Press 'q' or 'Q' to QUIT");break;
    case UNFILTERED: writeText (10, wh-20, "Original Unfiltered Image  "); break;
    case PARAMS: writeText (10, wh-20, "Middle button Saves '.Parameters' File"); break;
    case SEPARATE: writeText(10, wh-20, newinfo.line[8]); break;
    default: writeText (10, wh-20, "left: --               middle: Threshold                right: ++");
  }

makepalette();          // setup Display palette
}


int trackstatus(int mx, int my)
{
int newactive;

newactive=(my-500)/15;
if(newactive<0) newactive=0;
if( (newactive<0) || (newactive>STATS) ) return(0);
if(newactive!=active)
    {
    active=newactive;
    displaydata();
    }
return 0;
}

       
void updateimg()	//Now we are ready to do the stuff   
{
    clearhisto();
    copyto(orig,disp);
    histogram(disp,histo);
    prethreshold(disp,histo);
    histogram(disp,histo);
    expand(disp,histo);
    refresh();
    clearhisto();
}


void showorig()
{                    
copyto(orig,disp);
refresh();
clearhisto();
}


void analyze()
{
int i,j,k,l;
int object;

glColor3f(0.0,0.0,0.0);
screen_box(0,0,640,480);

for(j=0;j<nny;j++)for(i=0;i<nnx;i++) //Make image binary
    {
     if(disp[j][i] > 64){ disp[j][i]=1;
        glColor3ub(255,255,255);
        glBegin(GL_POINTS);
            glVertex2s(i+orx,j+ory);
        glEnd();}
     else disp[j][i]=0;
    }

object=2;
for(i=0;i<nny;i++)for(j=0;j<nnx;j++)
   if(disp[i][j]==1)
    {
    floodfill(i,j,1,object);
    glColor3ub((object*375+123)%255,(object*99+23)%255,(object*785+1)%255);
    for(k=minx;k<=maxx;k++)for(l=miny;l<=maxy;l++)
        if(disp[k][l]==object)	{
            glBegin(GL_POINTS);
                glVertex2s(l+orx,k+ory);
            glEnd();
                                }
        object++;
    }
    sprintf (newinfo.line[8], " %d objects in this frame",object);

}


void dumpnewparms()
{

FILE	*pp;
time_t	clck, te;
char	*timestring;

te=time(&clck);
timestring=ctime(&clck);
chdir(mydir);       //the cwd kluge! Very necessary but I'm not sure why. DAW

status("Opening .newps");
if( (pp=fopen(".newps","w")) == NULL)
  { status("ERROR: Could not open .newps! "); ;return;}

status("Dumping stuff to .newps");
fprintf(pp,"bitmap_ 1 Edited by Setfilters \n");
fprintf(pp,"bitmap_ 2 from Iplab file %s \n",thefilename);
fprintf(pp,"bitmap_ 3 on %s",timestring);
fprintf(pp,"bitmap_top     %d\n",bitmap_top     );
fprintf(pp,"bitmap_bottom  %d\n",bitmap_bottom  );
fprintf(pp,"bitmap_left    %d\n",bitmap_left    );
fprintf(pp,"bitmap_right   %d\n",bitmap_right   );
fprintf(pp,"bitmap_offset  %d\n",bitmap_offset  );
fprintf(pp,"bitmap_sdcoeff %d\n",bitmap_sdcoeff );
fprintf(pp,"bitmap_warpx   %d\n",bitmap_warpx   );
fprintf(pp,"bitmap_warpy   %d\n",bitmap_warpy   );
fprintf(pp,"bitmap_scalex  %d\n",bitmap_scalex  );
fprintf(pp,"bitmap_scaley  %d\n",bitmap_scaley  );
fprintf(pp,"rank_ 1   Edited by Setfilters \n");
fprintf(pp,"rank_ 2   from IpLab file %s \n",thefilename);
fprintf(pp,"rank_ 3   on the %s",timestring);
fprintf(pp,"rank_width   %d\n",rank_pixno);
fprintf(pp,"rank_power   %d\n",rank_power);
fprintf(pp,"rank_cliplow %d\n",rank_cliplow);
fprintf(pp,"rank_cliphi  %d\n",rank_cliphi);
fprintf(pp,"rank_mvleft  %d\n",rank_mvleft);
fprintf(pp,"rank_xpand   %d\n",rank_xpand);
fclose(pp);

system("mv .newps .Parameters");
status("Dumped .Parameters");
system("sleep 1");
status(" ");
}

/*******************************/

int main(int argc, char **argv)
{
    int				j, xcount, ycount;
    char			ifile[255];
    ipl_ds			*head;
    unsigned char	*bimptr;
    FILE			*fin;
    size_t			length=128;

if(argc!=2) { printf("Usage: Setfilters file \n"); exit(1); }
thefilename=argv[1];

if( (fin=fopen(argv[1],"r"))==NULL)
  { printf("Couldn't open %s\n",argv[1]); exit(1); }

if(argc != 2) {			//Check for # of arguments
	fprintf(stderr,"USAGE: %s ifile\n",argv[0]);
	exit(1);
	}

strcpy(ifile,argv[1]);

	read_tiff(ifile);   // Try reading a Tiff file format first!

	if(tiff!=1)			// Try reading an IpLab file format if its not Tiff!
	{
	head = read_ipdataset(ifile);

	if(head == (ipl_ds*)NULL) {
		fprintf(stderr,"ERROR: Can't read dataset. Make sure data\n");
		fprintf(stderr,"       is a valid iVision/IpLab file.\n"); exit(1); }

		nnx = head->nx;
		nny = head->ny;
		nnz = head->nz;

    bimptr=(unsigned char *)head->data;	//Read the 1st frame into an array orig[i][j]

        for (ycount=0; ycount<nny; ycount++)
        {
        /* copy along an x segment and then jump to the next row */
            for (xcount=0; xcount<nnx; xcount++)
            {            
            j = *bimptr++;
            orig[ycount][xcount] = j;
            }
		}
		fclose(fin);	// close open IpLab file only
	}
	
    init_bitmaps();
    init_rank();
    bitmap_top = nny;
    bitmap_right= nnx;
    getcwd(mydir, length);
    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(ww, wh);
    glutCreateWindow("Set Filters");
    glutDisplayFunc(mydisplay);
    myinit ();
    glutReshapeFunc (myReshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc (mouse);
    glutMainLoop();

return(0);
}
