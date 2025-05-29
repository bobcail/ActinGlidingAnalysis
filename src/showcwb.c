/*  showcwb.c

    Display and animate cwb.* files. This requires a braid (untangle)
    derived from the thread file (threadworms) as well as the cwb files.
    Author: Qun Wang 5/00; 
    Ported to MacOSX: DAW 01/02, Updated 12/03 DAW 
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
#include "getbraided.h"

#ifdef LINUX
int sleep_time = 0;
#else 
int sleep_time = 400000;
#endif
 
int ww = 640, wh =480; 
char filename[100][10];
char mydir[128]; // declared for system (mydir) debugging DAW 
int file_num;
int lef, mid, rig;
int thenumbers[500];
int frm,frmcmp,rct,liney,file; int bf, tf, ef; int rects;
int x0,yy0,x1,yy1; int bl,el,cnt,a1,a2,a3,cc1;
int totalobjects;
FILE *fin; short vv[2];
char c1,c2;
char str[80];
int dum;
int START = 0;
int rerun = 0;

void writeText (int x, int y, char* str)
{
	int shift = 0; 
	int i; 
	for (i=0; i<strlen(str); i++)
        { 
		glRasterPos2i(x+shift, wh-y); 
		glutBitmapCharacter (GLUT_BITMAP_9_BY_15, str[i]); 
		shift += glutBitmapWidth (GLUT_BITMAP_9_BY_15, str[i]); 
	} 
} 

void color(int id)
{
    if(id==1) glColor3f(1.0, 0.0, 0.0);
    else if(id==2) glColor3f(0.0, 1.0, 0.0);
    else if(id==3) glColor3f(0.0, 0.0, 1.0);
    else if(id==4) glColor3f(1.0, 1.0, 0.0);
    else if(id==5) glColor3f(1.0, 0.0, 1.0);
    else if(id==6) glColor3f(0.0, 1.0, 1.0);
    else if(id==7) glColor3f(1.0, 1.0, 1.0);
    else if(id==8) glColor3f(0.5, 0.5, 0.0);
    else if(id==9) glColor3f(0.0, 0.5, 0.5);
    else if(id==10) glColor3f(0.5, 0.5, 0.5);
    else if(id==11) glColor3f(1.0, 0.5, 0.0);
    else if(id==12) glColor3f(0.0, 0.4, 0.2);
    else if(id==13) glColor3f(0.0, 0.2, 0.8);
    else if(id==14) glColor3f(0.2, 0.5, 0.0);
    else glColor3f(1.0, 1.0, 1.0);
}
 
void mydisplay() 
{
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glClearColor (0.1, 0.1, 0.1, 1.0); 
    	glClear(GL_COLOR_BUFFER_BIT); 

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

void do_cwb (){
	if (!START) return;
	totalobjects=0;
        chdir(mydir);
	for(file=1; file<file_num+1; file++){
	/* Read the header */

		if(NULL==(fin=fopen(filename[file],"r")))
		{
			printf("Cannot read %s\n",filename[file]);
	        	 exit(1);
		}
printf("File %s opened\n", filename[file]);
		c1=getc(fin); 
		c2=getc(fin);
		if((c1!='_') || (c2!='B'))
		{ 
			printf("Wrong header: %c%c \n",c1,c2); 
			fclose(fin); 
			exit(1);
		}

		bf=getw(fin); 
		ef=getw(fin);

		for(frm=bf;frm<=ef;frm++)
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
	   		frmcmp=getw(fin); 
	   		if(frmcmp!=frm) 
				printf("Frame number wrong: %d\n",frmcmp);
	   		rects=getw(fin);
	   		if(!getframe(frm,thenumbers))
	   		{ 
				printf("There is some problem with the braid, exiting\n"); 
				exit(1);
			}

	   		for(rct=0;rct<rects;rct++)
	      		{
	      			c1=getc(fin); 
				c2=getc(fin);
	      			if((c1!='R')||(c2!='R'))
	      			{ 
					printf("Wrong rect header: %c%c\n",c1,c2); 
					fclose(fin); 
					exit(1);
				}

	      			if(thenumbers[rct] > totalobjects) 
					totalobjects=thenumbers[rct];
	      			x0=getw(fin); 
				yy0=getw(fin); 
				x1=getw(fin); 
				yy1=getw(fin);
	      			color(thenumbers[rct]%14+1);
                                sp_sleep(sleep_time/totalobjects); /* variable µsec sleep daw*/

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
	              				if(a2>0){
                                                   	glBegin(GL_LINES);
								glVertex2i(vv[0], vv[1]);
								vv[0] += a2+1;
								glVertex2i(vv[0], vv[1]);
							glEnd();
                                                } //end if
	        	      			cnt+=a2;
	         			}//end while
	      			}//end  for (liney)
	   		}//end for (rct)
	   		a3=frm;
	   		a1=a3%30; 
			a3/=30;
	   		a2=a3%60; 
			a3/=60;
	   		sprintf(str,"%s      %05d       %02d' %02d\" %02df     %7d",
                filename[file],frm,a3,a2,a1, totalobjects-1000);
			color(2); 
                        writeText (20, 460, str);
		   	glutSwapBuffers();
		}//end for frm

		fclose(fin);
	} //end for file
	printf("Bye bye!\n");
	fclose (braidfile);
       	glPopAttrib();
       	glFlush();
	START = 0;
}


void keys (unsigned char key, int x, int y)
{
	if (key == 'q' || key == 'Q')
		exit(0);
}

void left_menu(int id)
{
	if (id == 1 && rerun == 1)
            {
                START=1;
                initbraids();   // reopen the braid file!
                do_cwb ();
            }      // repeat the movie!
        else if(id == 1 && rerun == 0)
            {
                START=1; // show the movie the first time
                rerun=1; // every run after that is a rerun!
            }
        else if(id==2)
		exit(0); 
} 


int main(int argc, char** argv)
{ 
	int i;
        size_t	length=128;
                
    	if (argc < 2){
		printf ("Usage: %s cwb*\n", argv[0]);
		exit(1);
    	}
	file_num = argc-1;
	for (i=1; i<argc; i++){
		if (strncmp (argv[i], "cwb.", 4) != 0){
			printf("Usage: showcwb cwb*\n");
			exit (1);
		}
		else
    			strcpy (filename[i], argv[i]);
	}

        getcwd(mydir, length);
      	glutInit(&argc,argv);
    	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(ww, wh);
    	glutCreateWindow("showcwb");
    	glutDisplayFunc(mydisplay); 
    	glutCreateMenu(left_menu);
    	glutAddMenuEntry("Run",1);
    	glutAddMenuEntry("Quit",2);
    	glutAttachMenu(GLUT_LEFT_BUTTON); 
        myinit ();
	glutKeyboardFunc(keys); 
	glutIdleFunc (do_cwb);
    	glutMainLoop();
    return 0;             /* ANSI C requires main to return int. */
}
