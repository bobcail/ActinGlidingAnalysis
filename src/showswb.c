/*  showswb.c

    Display and animate the swb file showing the track
    and movement along the track by the filament.
    
    Author: Qun Wang
    Ported to Mac OSX by Donald A. Winkelmann 
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/
#ifdef LINUX
#include <GL/glut.h>
#else 
#include <GLUT/glut.h>
#endif
 
#include <stdlib.h> 
#include <stdio.h> 
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
 
int ww = 640, wh =480; 
#ifdef LINUX
int sleep_time = 0;
#else 
int sleep_time = 8000;
#endif
 
void mydisplay(); 
void sp_sleep();

char mydir[128]; // declared for system (mydir) debugging DAW 
double trackx[1000], tracky[1000]; 
int trackpoints; 
int frm, liney, file, i; 
int x0, yy0, x1, yy1; 
int cnt, a1, a2, a3; 
FILE *fin; 
short vv[2]; 
char c1, c2; 
char str[80]; 
double vd[2]; 
char filename[500][10]; 
int SHOWWORM = 0;
int INDEX=1;
int file_num;
int FIRST = 0;

void writeText (int x, int y, char* str){ 
	int shift = 0; 
	int i; 
	for (i=0; i<strlen(str); i++){ 
		glRasterPos2i(x+shift, wh-y); 
		glutBitmapCharacter (GLUT_BITMAP_9_BY_15, str[i]); 
		shift += glutBitmapWidth (GLUT_BITMAP_9_BY_15, str[i]); 
	} 
} 

void message ()
{
	writeText (90, 60, "Welcome to the showswb program!");
	writeText (50, 90, "Use mouse or keyboard to control the process:");
	writeText (50, 120, "-- click left mouse to start program and continue");
	writeText (50, 140, "-- click right mouse to select operations from the menu");
	writeText (50, 180, "-- key N for next worm");
	writeText (50, 200, "-- key R for repeating the current worm");
	writeText (50, 220, "-- key P for previous worm");
	writeText (50, 240, "-- key Q to quit the program");

}
 
void gettrack(char *fname) 
{ 
	char tname[80]; 
	FILE *ftrk; 
	int i; 
 
	for(i=0;i<80;i++) 
		tname[i]=0; 
	for(i=0;fname[i];i++) 
		tname[i]=fname[i]; 
	tname[0]='t'; 
	tname[1]='r'; 
	tname[2]='k'; 
 
	if( (ftrk=fopen(tname,"r")) == NULL) 
	 { 
		printf("No track file for %s\n",fname); 
		trackpoints=0; 
		return; 
	 } 
 
	printf("Track file %s\n",tname); 
	for(i=0; fscanf(ftrk,"%lf %lf",trackx+i,tracky+i) == 2; i++); 
		trackpoints=i; 
	fclose(ftrk); 
} 
 
 
void do_swb()
{ 
	if (!SHOWWORM)
		return;
	FIRST = 0;
        chdir(mydir); // change current working directory Kluge DAW
	if(NULL==(fin=fopen(filename[INDEX],"r"))) 
	{
		printf("Cannot read %s\n", filename[INDEX]); 
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
	gettrack(filename[INDEX]); 
	frm=0; 
	for(;frm != -1;) 
	{ 
		c1=getc(fin); 
		c2=getc(fin); 
		if((c1!='F')||(c2!='F')) 
        	{ 
			printf("Wrong frame header: %c%c\n",c1,c2); 
			fclose(fin); 
			exit(1); 
        	} 

		glClearColor (0.1, 0.1, 0.1, 1.0);
		glClear (GL_COLOR_BUFFER_BIT);
		glFlush();
 
 		if(trackpoints) 
        	{ 
			glColor3f(1.0, 0.0, 0.0); 
			glBegin(GL_LINE_STRIP); 
			for(i=0;i<trackpoints;i++) 
			{ 
				glVertex2f (trackx[i], tracky[i]); 
			} 
			glEnd(); 
        	} 
    
		frm=getw(fin); 
		x0=getw(fin); 
		x1=getw(fin); 
		yy0=getw(fin); 
		yy1=getw(fin); 

/* draw the boundary of the worm */
		glColor3f(0.0, 0.0, 1.0); 
		glBegin(GL_LINE_LOOP); 
			glVertex2f (x0, yy0); 
			glVertex2f (x0, yy1); 
			glVertex2f (x1, yy1);
			glVertex2f (x1, yy0); 
		glEnd(); 
/* draw the worm */
		glColor3f(1.0, 1.0, 1.0); 
		if( frm == -1 )  
			break; 
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
			cnt=x0; 
			while(cnt<x1) 
            		{ 
				a1=getc(fin); 
				a2=getc(fin); 
				cnt+=a1; 
				vv[0]=cnt; 
				vv[1]=liney; 
				if(a2>0) 
       	        	 	{ 
					glBegin (GL_LINES); 
						glVertex2f (cnt, liney); 
						glVertex2f (cnt+a2+1, liney); 
					glEnd(); 
                		} // end if
				cnt+=a2; 
			} //end while
		} //end for (liney)
		a3=frm; 
		a1=a3%30; 
		a3/=30; 
 
		a2=a3%60; 
		a3/=60; 
 
		sprintf(str,"%s (file %d/%d)      %05d       %02d' %02d\" %02df", filename[INDEX], INDEX, file_num, frm, a3, a2, a1);

		glColor3f (0.0, 1.0, 0.0); 
		writeText (20.0, 20.0, str); 
                sp_sleep(sleep_time); /* if its OSX ~8000 µsec sleep daw*/
                glutSwapBuffers();
                 
      	}// end for (frame) 

	fclose(fin);
	SHOWWORM = 0;

    if (INDEX == 1) INDEX++;
} 
 

void mydisplay() 
{ 
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glClearColor (0.1, 0.1, 0.1, 1.0); 
    	glClear(GL_COLOR_BUFFER_BIT); 
	if (FIRST){
		glColor3f(1.0, 1.0, 1.0);
		writeText (90, 90, "Already first worm!");
		writeText (90, 130, "Press mouse button to show first worm");
		writeText (90, 170, "or press key R to show the first worm");
	}

	else if (!SHOWWORM)
                message();

	glFlush();
	glutSwapBuffers();  
    	glPopAttrib(); 
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

    	glClearColor (0.1, 0.1, 0.1, 1.0);
    	glClear(GL_COLOR_BUFFER_BIT);
    	glFlush();
} 

void mouse(int btn, int state, int x, int y)
{
    	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN && SHOWWORM == 0){

                if (INDEX == 1) {
			SHOWWORM = 1;
		}
		else if (INDEX == file_num){
			SHOWWORM = 0;
		}
		else { INDEX++;
			SHOWWORM = 1;
		}
    	}
        else if (btn == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
                myinit (); // clear first
                message ();
        	glFlush();
                glutSwapBuffers();
    	}
        
       	glPopAttrib();
       	glFlush();
}

void keys (unsigned char key, int x, int y)
{
	if (key == 'q' || key == 'Q')
		exit(0);
	if (key == 'n' || key == 'N'){
		if (INDEX == file_num){
			SHOWWORM = 0;

		}
		else if (INDEX == 1){
			SHOWWORM = 0;
			FIRST = 1;
			mydisplay();
		}
                else{	INDEX++;
			SHOWWORM = 1;
		}
	}
	if (key == 'p' || key == 'P'){
		if (INDEX == 1){
			SHOWWORM = 0;
			FIRST = 1;
			mydisplay();
		}
		else {
			INDEX = INDEX - 1;
			SHOWWORM = 1;
		}
	}
	if (key == 'r' || key == 'R'){
		SHOWWORM = 1;
	}

}

void sp_sleep(interval) /*Somu's microsecond sleep()*/
int interval;
{
   struct timeval initial_time,final_time;
   struct timezone *zone;
   extern int abs();

   zone = (struct timezone *) malloc(sizeof(struct timezone));
   gettimeofday(&initial_time,zone);
   gettimeofday(&final_time,zone);
   while (abs((final_time.tv_usec - initial_time.tv_usec)) < interval)
      gettimeofday(&final_time,zone);
   return;
}


int main(int argc, char** argv)
{ 
	int i;
        size_t	length=128;
    	if (argc < 2){
		printf ("Usage: %s swb*\n", argv[0]);
		exit(1);
    	}
	file_num = argc-1;
	for (i=1; i<argc; i++){
		if (strncmp (argv[i], "swb.", 4) != 0){
			printf("Usage: showswb swb*\n");
			exit (1);
		}
		else
    			strcpy (filename[i], argv[i]);
	}
        getcwd(mydir, length); // the cwb kluge that works! DAW
      	glutInit(&argc,argv);
    	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(ww, wh);
    	glutCreateWindow("showswb");
    	glutDisplayFunc(mydisplay);	
    	myinit (); 
	glutKeyboardFunc(keys); 
    	glutMouseFunc (mouse); 
	glutIdleFunc (do_swb);
    	glutMainLoop();
    return(1);
}
