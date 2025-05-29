/*  showswb_sp.h
    
    Animate a single swb file in a GL window as part of the Speed Editor
    
    Author: Qun Wang
    Ported to Mac OSX by Donald A. Winkelmann 
    Copyright (c) 2020 Rutgers-RWJMS. All rights reserved.
*/ 
#include <sys/time.h>
#include <unistd.h>
 
int iw = 640, ih =480; 

#ifdef LINUX
int sleep_time = 0;
#else 
int sleep_time = 8000;
#endif

double trackx[1000], tracky[1000]; 
int trackpoints; 
int frm, liney, file, i; 
int x0, yy0, x1, yy1; 
int cnt, a1, a2, a3; 
FILE *swbfin, *trkfin; 
short vv[2]; 
char c1, c2; 
char str1[80]; 
double vd[2]; 
int SHOWWORM = 0;
int file_num;
int FIRST = 0;


void swbText (int x, int y, char* str1){ 
	int shift = 0; 
	int i; 
	for (i=0; i<strlen(str1); i++){ 
		glRasterPos2i(x+shift, ih-y); 
		glutBitmapCharacter (GLUT_BITMAP_9_BY_15, str1[i]); 
		shift += glutBitmapWidth (GLUT_BITMAP_9_BY_15, str1[i]); 
	} 
} 
 
void do_swb(){ 
	if (!SHOWWORM)
		return;
	FIRST = 0;

if(NULL==(swbfin=fopen(swbfile,"r")))
    {
    printf("%s does not exist\n",swbfile);
    return;
    }
	c1=getc(swbfin); 
	c2=getc(swbfin); 
	if((c1!='_') || (c2!='S')) 
    	{ 
		printf("wrong swb header: %c%c\n",c1,c2); 
		fclose(swbfin); 
		exit(1); 
    	} 
	frm=0; 
	for(;frm != -1;) 
	{ 
		c1=getc(swbfin); 
		c2=getc(swbfin); 
		if((c1!='F')||(c2!='F')) 
        	{ 
			printf("Wrong frame header: %c%c\n",c1,c2); 
			fclose(swbfin); 
			exit(1); 
        	} 

                glColor3f(0.0, 0.0, 0.0);   // Clear the swb box between frames
                screen_box (0, 0, 640, 480);
                glColor3f(1.0, 1.0, 1.0); 
		glBegin(GL_LINE_LOOP); 
			glVertex2f (0, 0); 
			glVertex2f (640, 0); 
			glVertex2f (640, 480);
			glVertex2f (0, 480); 
		glEnd();

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
    
		frm=getw(swbfin); 
		x0=getw(swbfin); 
		x1=getw(swbfin); 
		yy0=getw(swbfin); 
		yy1=getw(swbfin); 

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
			c1=getc(swbfin); 
			c2=getc(swbfin); 
			if((c1!='L')||(c2!='L')) 
			{ 
				printf("Wrong line header: %c%c\n",c1,c2); 
				fclose(swbfin); 
				exit(1); 
			} 
			cnt=x0; 
			while(cnt<x1) 
            		{ 
				a1=getc(swbfin); 
				a2=getc(swbfin); 
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
 
		sprintf(str1,"%s (file %d/%d)      %05d       %02d' %02d\" %02df", swbfile, Thisfile, file_num, frm, a3, a2, a1);

		glColor3f (0.0, 1.0, 0.0); 
		swbText (20.0, 20.0, str1); 
                sp_sleep(sleep_time); /* 8000 µsec sleep daw*/
                glPopAttrib(); // kludge
                glFlush(); // kludge
                //glutSwapBuffers();
                 
      	}// end for (frame) 

	fclose(swbfin);
	SHOWWORM = 0; 
} 
 
void left_menu(int id) 
{ 
	if(id == 1) return;
        if(id == 2) SHOWWORM = 1; 
} 
 
void swbdisplay() 
{ 
	glPushAttrib(GL_ALL_ATTRIB_BITS); 
	glClearColor (0.1, 0.1, 0.1, 1.0); 
    	glClear(GL_COLOR_BUFFER_BIT); 
	glFlush();
	glutSwapBuffers();  
    	glPopAttrib(); 
} 

void swbinit(void)
{
	glViewport(0,0,iw,ih);

/* Pick 2D clipping window to match size of X window 
This choice avoids having to scale object coordinates
each time window is resized */

    	glMatrixMode(GL_PROJECTION);
    	glLoadIdentity(); 
    	glOrtho(0.0, (GLdouble) iw , 0.0, (GLdouble) ih , -1.0, 1.0);

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
