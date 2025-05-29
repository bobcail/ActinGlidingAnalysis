/*  speed.c

    This program uses the glut library to analyze motility data.
    OpenGL routines written by Qun Wang, June, 2000
    Speed routine implemented by Donald Winkelmann, Feb 2002
    This version revised 11/25/03
    Movie implementation was updated 12/07/03 DAW
    Added automatic hint acceptance to speed analysis 03/09/18 DAW
    
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/
#ifdef LINUX
#include <GL/glut.h>
#else 
#include <GLUT/glut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define	MAXBRKS		100
#define	MAXDATA 	50000

char theargv[1000][10];
char mydir[128];
char swbfile[100];

int theargc;
int windowname;
int currentfile, Thisfile;
int nbreaks;
int ndata;
int whichfit=0;
int from[MAXBRKS], to[MAXBRKS];
int hint;
int file_num;
int pick(int, int);
int ww=900, wh=650;
int texth, boxheight, hh ;
int boxwidth, boxleft;

double a[MAXBRKS], b[MAXBRKS];
double errspd[MAXBRKS]; 
double xx[MAXDATA], xsmooth[MAXDATA], xdiff[MAXDATA];
double avl, erravl;
double totalerror, averageerror;
double fronttip[MAXDATA], backtip[MAXDATA];
double maxpos, minpos, maxtime,mintime;
double kernelwidth=10.0;


#define	FITCOLOR	glColor3f(1.0,0.3,0.3);
#define	DATACOLOR	glColor3f(1.0,1.0,1.0);
#define SMTHCOLOR	glColor3f(1.0,1.0,0.0);
#define WHITECOLOR 	glColor3f(1.0, 1.0, 1.0);
#define FIT		1
#define SHOWMOVIE	2
#define PRINT		3
#define SMOOTHER	4
#define LESS_SMO	5
#define HINT		6
#define NEXTFILE	7
#define LASTFILE	8
#define DELETE		9
#define BLANK		10
#define QUIT		11
#define GRAPHICS	12

void updatebreaks();
void displaydata();
void smoothdata();
void fitdata();
void updatehint();
void callshowswb();
void do_hint();
void mydisplay();
void writeText (int, int, char* );
void screen_box(int, int , int, int );
void myinit(void);
void mouse(int, int, int, int);
void closeAll(void);
void mydisplay(void);
void resetTextInfo();
void deleteasc();
void sp_sleep();
void do_swb();

typedef struct textinfo {
        char line[9][80];
        }TEXTINFO;

TEXTINFO newinfo;

#include "speedfile.h"
#include "speeddisplay.h"
#include "speedsmooth.h"
#include "speedclicks.h"
#include "speedpostscript.h"
#include "showswb_sp.h"

void resetTextInfo (char *str)
{
	unsigned int i;
	for (i=0; i<strlen(str); i++)
		str[i] = '\0';
}


/*write strings starting location (x, y)*/
void writeText (int x, int y, char* str)
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


int pick(int x, int y)
{
    y = wh - y;
    if(x < boxleft && y<14*boxheight) return GRAPHICS;
    if(x < boxleft || y>11*boxheight) return 0;
    else if(y > 10*boxheight) return FIT;
    else if(y > 9*boxheight) return SHOWMOVIE;
    else if(y > 8*boxheight) return PRINT;
    else if(y > 7*boxheight) return SMOOTHER;
    else if(y > 6*boxheight) return LESS_SMO;
    else if(y > 5*boxheight) return HINT;
    else if(y > 4*boxheight) return NEXTFILE;
    else if(y > 3*boxheight) return LASTFILE;
    else if(y > 2*boxheight) return DELETE;
    else if(y > boxheight) return BLANK;
    else if(y > 0) return QUIT;	
    else return 0;
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


void keyboard (unsigned char key, int xx, int yy)
{
    switch (key)
    {
        case 'q':
        case 'Q':
            savefile(theargv[Thisfile]);
            exit(0);
    }
}


/*callback function, display on-screen menu and gate diagrams upon non-empty list */
void mydisplay()
{
	int i;
	texth=wh/30;
	boxheight=wh/16; 
	hh=wh/52 ;
	boxwidth=ww/5;
	boxleft=ww*4/5;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glClearColor (0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 0.0, 0.6);
    screen_box (0, 0, 4*ww/5-5, 14*boxheight);

    glColor3f(1.0, 0.0, 0.0);
    screen_box(boxleft,10*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 10*boxheight + hh, "Fit Smoothed Data");

    glColor3f(0.0, 0.5, 0.0);
    screen_box(boxleft,9*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 9*boxheight + hh, "Show Movie");

    glColor3f(0.0, 0.0, 1.0);
    screen_box(boxleft,8*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 8*boxheight + hh, "PRINT");

    glColor3f(0.5, 0.2, 0.0);
    screen_box(boxleft,7*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 7*boxheight + hh, "Smoother");

    glColor3f(0.5, 0.6, 05);
    screen_box(boxleft,6*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 6*boxheight + hh, "Less Smooth");

    glColor3f(0.0, 0.4, 0.4);
    screen_box(boxleft,5*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 5*boxheight + hh, "DO HINT");

    glColor3f(0.9, 0.4, 0.0);
    screen_box(boxleft,4*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 4*boxheight + hh, "First File");

    glColor3f(0.2, 0.5, 0.0);
    screen_box(boxleft,3*boxheight, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, 3*boxheight + hh, "Last File");

    glColor3f(0.5, 0.5, 1.0);
    screen_box(boxleft,2*boxheight, boxwidth, boxheight);
	glColor3f(0.0, 0.0, 0.0);
	writeText (boxleft+5, 2*boxheight + hh, "Delete");
 
    glColor3f(0.2, 1.0, 0.0);
    screen_box(boxleft,1*boxheight, boxwidth, boxheight);
    
    glColor3f(0.2, 0.2, 0.2);
    screen_box(boxleft, 0, boxwidth, boxheight);
	WHITECOLOR;
	writeText (boxleft+5, hh, "QUIT");

	if(1){
                for (i=0; i<9; i++)
                {
                if (i==4) glColor3f(1.0, 0.5, 0.0);
                else if (i==7) glColor3f(1.0, 0.0, 1.0);
                else glColor3f(1.0, 1.0, 1.0);
                writeText (boxleft, wh - (i+1)*texth, newinfo.line[i]);
                }
            }

    glFlush();
    glPopAttrib();
}


void myReshape (int w, int h)
{
	/* adjusting clipping box */
	glViewport (0, 0, w, h);
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity();
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
    int action_mode;
    int takehint = 1;

    if(state==GLUT_DOWN)
    {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
       
		where = pick(x,y);

		if(where != 0)		//selection mode, select object type to draw
		{
                    action_mode = where;
                    
                    switch(action_mode){
                    case FIT:
                            if(btn==GLUT_LEFT_BUTTON){
                            if(whichfit==0) {whichfit=1;
                            glColor3f(0.8, 0.0, 0.0);
                            screen_box(boxleft,10*boxheight, boxwidth, boxheight);
                            WHITECOLOR;
                            writeText (boxleft+5, 10*boxheight + hh, "Fit Original Data");}
                            else {whichfit=0;
                            glColor3f(1.0, 0.0, 0.0);
                            screen_box(boxleft,10*boxheight, boxwidth, boxheight);
                            WHITECOLOR;
                            writeText (boxleft+5, 10*boxheight + hh, "Fit Smoothed Data");}
                            redisplayall();}
                            break;
                    case GRAPHICS:
                            if (btn==GLUT_LEFT_BUTTON && state==GLUT_DOWN) gplclick(x,y);
                            else if(btn==GLUT_MIDDLE_BUTTON && state==GLUT_DOWN) gpmclick(x,y);
                            else if (btn==GLUT_RIGHT_BUTTON && state==GLUT_DOWN) gprclick(x,y);
                            savefile(theargv[Thisfile]);
                            glPushAttrib(GL_ALL_ATTRIB_BITS);
                            break;
                    case SHOWMOVIE:
                            if(btn==GLUT_LEFT_BUTTON) callshowswb();
                            redisplayall();
                            break;
                    case PRINT:
                            if(btn==GLUT_LEFT_BUTTON) PSredisplayall(); 
                            redisplayall();
                            break;
                    case SMOOTHER:
                            if(btn==GLUT_LEFT_BUTTON) kernelwidth*=1.2; smoothdata();
                            redisplayall();
                            break;
                    case LESS_SMO:
                            if(btn==GLUT_LEFT_BUTTON){kernelwidth/=1.2;
                            smoothdata();}
                            redisplayall();
                            break;
                    case HINT:
                            if(btn==GLUT_LEFT_BUTTON) {do_hint();
                            savefile(theargv[Thisfile]);}
                            break;
                    case NEXTFILE:
                            if(btn==GLUT_LEFT_BUTTON)
                            {
                            if(currentfile<file_num+1){
                                loadfile(theargv[currentfile]);
                                Thisfile=currentfile;
                                redisplayall();
                                while (takehint <= 10){
                                    do_hint();
                                    ++takehint;}    /* semi-auto hint acceptance DAW*/
                                savefile(theargv[currentfile]);
                                glColor3f(1.0, 0.5, 0.0);
                                screen_box(boxleft,4*boxheight, boxwidth, boxheight);
                                WHITECOLOR;
                                writeText (boxleft+5, 4*boxheight + hh, "Next File");
                                currentfile++;}
                            else
                                {glColor3f(1.0, 0.6, 0.1);
                                screen_box(boxleft,4*boxheight, boxwidth, boxheight);
                                WHITECOLOR;
                                writeText (boxleft+5, 4*boxheight + hh, "That's all!");}
                            }
                            break;
                    case LASTFILE:
                            if(btn==GLUT_LEFT_BUTTON){
                            if(currentfile>2){
                            loadfile(theargv[currentfile-2]);
                            Thisfile=currentfile-2;
                            redisplayall();
                            currentfile--;                                
                            glColor3f(1.0, 0.5, 0.0);
                            screen_box(boxleft,4*boxheight, boxwidth, boxheight);
                            WHITECOLOR;
                            writeText (boxleft+5, 4*boxheight + hh, "Next File");}
                            }
                            break;
                    case DELETE:
                            if(btn==GLUT_LEFT_BUTTON) deleteasc();
                            while (takehint <= 10){
                                do_hint();
                                ++takehint;}    /* semi-auto hint acceptance DAW*/
                            break;
                    case BLANK:
                            break;
                    case QUIT:
                            if(btn==GLUT_LEFT_BUTTON){
                            printf ("leaving the program\n");
                            exit(0);}
                    }
		}
       glPopAttrib();
       glFlush();
     }
}


void callshowswb()
{
char trkfile[100];
int i;

chdir(mydir); // the cwd kluge! DAW

for(i=0;theargv[Thisfile][i];i++)trkfile[i]=swbfile[i]=theargv[Thisfile][i];
trkfile[i]=swbfile[i]=0;
swbfile[0]='s'; swbfile[1]='w'; swbfile[2]='b';
trkfile[0]='t'; trkfile[1]='r'; trkfile[2]='k';

if( (trkfin=fopen(trkfile,"r")) == NULL)
  {
  printf("%s does not exist\n",trkfile);
  return;
  }
else
for(i=0; fscanf(trkfin,"%lf %lf",trackx+i,tracky+i) == 2; i++); 
		trackpoints=i; 
fclose(trkfin);

    SHOWWORM=1;

    do_swb();
    sp_sleep (200000);
    mydisplay();
    glColor3f(1.0, 0.5, 0.0);
    screen_box(boxleft,4*boxheight, boxwidth, boxheight);
    WHITECOLOR;
    writeText (boxleft+5, 4*boxheight + hh, "Next File");
    glPopAttrib();
    glFlush();

    return;
}


void deleteasc() //delete the current asc file from dataset
{
char comm[128], ascfile[100];
char newList[1000][20];
int i, j;

    nbreaks=0; // if nbreaks=0, nothing is written to the file
    savefile(theargv[Thisfile]);
    
    chdir(mydir); // the cwd kluge! DAW
    strcpy (ascfile, theargv[Thisfile]);
    sprintf(comm,"rm %s",ascfile);
    printf("%s deleted\n", ascfile);
    system(comm);  //Delete the asc file but leave the swb and trk files

    for (i=1; i<Thisfile; i++) strcpy (newList[i], theargv[i]);
    for (i=Thisfile; i<file_num+1; i++){
        j = i+1; strcpy (newList[i], theargv[j]);
        }
    for (i=1; i<file_num; i++) strcpy( theargv[i], newList[i]);

    if(currentfile>1) currentfile = currentfile-1;
    else currentfile = currentfile + 1;
    file_num = file_num-1;

// Now load the next file since you've deleted the current file
    if(currentfile < file_num+1)
        {
        loadfile(theargv[currentfile]);
        Thisfile=currentfile;
        redisplayall();                                
        savefile(theargv[currentfile]);
        glColor3f(1.0, 0.5, 0.0);
        screen_box(boxleft,4*boxheight, boxwidth, boxheight);
        WHITECOLOR;
        writeText (boxleft+5, 4*boxheight + hh, "Next File");
        currentfile++;
        }
    else{
        glColor3f(1.0, 0.6, 0.1);
        screen_box(boxleft,4*boxheight, boxwidth, boxheight);
        WHITECOLOR;
        writeText (boxleft+5, 4*boxheight + hh, "That's all!");
        }
}


int main(int argc, char** argv)
{
    int i;
    int oldcurrentfile=0;
    size_t length=128;
    theargc=argc;
    currentfile=1;
    texth=wh/26;
    boxheight=wh/13; 
    hh=wh/52 ;
    boxwidth=ww/5;
    boxleft=ww*4/5;

    	if (argc < 2){
		printf ("Usage: %s asc*\n", argv[0]);
		exit(1);
    	}
	file_num = argc-1;
	for (i=1; i<argc; i++){
		if (strncmp (argv[i], "asc.", 4) != 0){
			printf("Usage: speed asc*\n");
			exit (1);
		}
		else
    			strcpy (theargv[i], argv[i]);
	}
    

    for (i=0; i<9; i++)
    resetTextInfo (newinfo.line[i]);
    sprintf (newinfo.line[0], "File:   ");
    sprintf (newinfo.line[1], "SegM:   ");
    sprintf (newinfo.line[2], "Data:  ");
    sprintf (newinfo.line[3], "Used:   ");
    sprintf (newinfo.line[4], "Rang:   ");
    sprintf (newinfo.line[5], "<E> :  ");
    sprintf (newinfo.line[6], "KWid:   ");
    sprintf (newinfo.line[7], "TotE:   ");
    sprintf (newinfo.line[8], "File:   ");
   
    if(currentfile!=oldcurrentfile)
        {
     if(oldcurrentfile!=0)
       savefile( argv[oldcurrentfile] );
        }
    getcwd(mydir, length); // the cwd kluge! DAW
    glutInit(&argc,argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(ww, wh);
    glutCreateWindow("Speed Semi-Auto");
    glutDisplayFunc(mydisplay);
    myinit ();
    glutReshapeFunc (myReshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc (mouse);
    glutMainLoop();

return(0);
}
