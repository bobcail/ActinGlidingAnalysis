/* floodfill.h

 Cluster analysis algorithms
 Version 1.0, nov 90, m.o.m.
 Modified 1993 Marcello O. Magnasco
 Rockefeller Univ. 1994
 
*/


/* Defines and constants */
#define NPTRS 300		/* The size of the stack. */

/* Variables */
int minx, miny, maxx, maxy;	/* The bounding rectangle */

int pointerx[NPTRS], pointery[NPTRS];	/* The Stack */

/*------------------------ The Code --------------------------------*/

void initpointers()	/* Clear the stack */
{ int i; for(i=0;i<NPTRS;i++)pointerx[i]= (-5); }

int findpointer()	/* Pop a pointer from the stack */
{ int i;
for(i=0;(pointerx[i]==-5)&(i<NPTRS);i++) ;
if(i<NPTRS) return(i); else return(-1); }

void createpointer(int x, int y)	/* push a pointer on the stack */
{ int i;
for(i=0;(pointerx[i]!=-5)&(i<NPTRS);i++) ; 
if(i==NPTRS){printf("Not enough pointers!\n");exit(1);}
pointerx[i]=x; pointery[i]=y;
}

void fillpointer(int ptr, int c1, int c2)
				/* Fill connected horizontal segment 
				containing the pointer ptr; only fill points
				color c1, fill them with color c2 */
{
int upptrnd, dnptrnd, x,y ;

x=pointerx[ptr];
y=pointery[ptr];
pointerx[ptr]=(-5); 		/* Invalidate current pointer */
if(M[x][y] != c1) return;	/* This pointer is no longer pointing to c1 */

for(;M[x][y] == c1;x--);	/* find leftmost edge of conn.segment */
x++;
if(x<minx)minx=x;		/* Extend the bounding rectngle if necessary */

/* JUMPING ADDITIONS GO HERE (LEFT SIDE) */



upptrnd=dnptrnd=1;		/* Upper and lower pointers needed */
for(;M[x][y]==c1;x++){		/* Sweep right */
   M[x][y]=c2;			/* Fill with c2's */
   if(upptrnd==1){		/* Have I yet put a pointer up there? */
     if(M[x][y+1] == c1){	/* There's c1 up there; put a pointer! */
       createpointer(x,y+1);	/* put pointer */
       upptrnd=0;}}		/* no longer need up there until !c1 */
   else	{			/* I put something up there already */
     if(M[x][y+1] != c1) 	/* No longer c1 up there */
       upptrnd=1; }		/* Gotta keep looking */

if(dnptrnd==1) { 		/* Same thing, all for looking down */
     if(M[x][y-1] == c1) { 
       createpointer(x,y-1); 
       dnptrnd=0; } } 
   else { 
     if(M[x][y-1] != c1)  
       dnptrnd=1; } }

if( (x-1) > maxx) maxx=x-1;	/* Extend rectangle boundaries if necessary */

/* JUMPING ADDITIONS GO HERE (RIGHT SIDE) */

}

void floodfill(x,y,c1,c2)	/* Floodfill main routine */
int x,y,c1,c2;
{ int ptr;

minx=maxx=x; miny=maxy=y;	/* initialize rectangle to single point */
initpointers();
pointerx[0]=x;
pointery[0]=y;
for(;(ptr=findpointer()) != -1;)	/* repeat until no more pointers in
					the stack */
   {
   if(pointery[ptr]>maxy)maxy=pointery[ptr];	/* Adjust boundaries of */
   if(pointery[ptr]<miny)miny=pointery[ptr];	/* rectangle if necessary */
   fillpointer(ptr,c1,c2);	/* fill the horizontal segment */
   }

} 				/* That's it */
