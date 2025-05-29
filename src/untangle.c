/* untangle.c

   This program will follow the thread file and produce an explicit 
   renumbering of the objects in the thread file

   The format of the thread file as defined in this program is:
   _T           (header)
   F%w%w%w      (frame number, number of objects,  and number of "edges")
   [%w%w]       (new number, old number)
                (frame number = -1 to end file)

   The output of this program is a braid file, whose format is
   _B		(header)
   F%w%w	(frame number and number of objects)
   [%w]		(object renumbering)
		(frame number = -1 to end file)

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

#include <stdlib.h>
#include <stdio.h>
#define MAXO 10000
#define MAXL 30000
#define TALK if(0)
#define WW getw(ff)

int main()
{
int i,j,k,l, frm, lnks, obs, assigned;
char c1,c2;
FILE *ff, *fo;

int oldnumber[MAXO], newnumber[MAXO], linksto[MAXO];
int link0[MAXL], link1[MAXL];

int oldname, newname, oldobs;


if( (ff=fopen("thread","r")) == NULL)
  { printf("Sorry, can't open the thread file\n"); exit(1); }

if( (fo=fopen("braid","w")) == NULL)
  { printf("Sorry, can't open the braid file\n"); exit(1); }

TALK printf("Starting to untangle\n");

c1=getc(ff); c2=getc(ff); 
if( (c1 != '_') || (c2!='T'))
  { printf("Wrong file header for a thread file\n"); exit(1); }

frm=0;

c1=getc(ff); if(c1!='F') { printf("Wrong frame header\n"); exit(1); }
frm=WW; obs=WW-1; lnks=WW;
TALK printf("Begin: frame %d, %d objects with %d links\n", frm,obs,lnks);

putc('_',fo);
putc('B',fo);
putc('F',fo);
putw(frm,fo);
putw(obs,fo);

for(i=0;i<obs;i++)oldnumber[i]=i+1000;
for(i=0;i<obs;i++)putw(oldnumber[i],fo);
assigned=obs+1000;
oldobs=obs;


for(;;)
   {
   c1=getc(ff); if(c1!='F') { printf("Wrong frame header\n"); break; }
   frm=WW; obs=WW-1; lnks=WW; 
   if(frm%300==0)printf("%d\n",frm);
   putc('F',fo); putw(frm,fo); putw(obs,fo);
   if(frm== -1) break;
   TALK printf("\nFRAME %d has %d objects, %d links\n",frm,obs,lnks);
   for(i=0;i<lnks;i++)
      { link1[i]=WW-2; link0[i]=WW-2;
      TALK printf("%5d -> %5d\n",link0[i],link1[i]);
      }

   for(i=0;i<obs; i++) newnumber[i]=0;

   /* Now we verify for each of the old numbers how many links go out of
      that vertex; if it's
      0, then no further action taken,
      1, then check how many to target
         1, then assign new number = old number
         other, then if target not numbered, make new number
      more, if targets not numbered make new numbers
   */
   for(oldname=0; oldname<oldobs; oldname++)
      {
      TALK printf("checking on %d\n",oldname);
      for(i=l=0; i<lnks; i++) 
         if(link0[i]==oldname) { j=i; l++; }
      TALK printf("%d has %d links going out (one at %d)\n",oldname,l,j);
      if(l==0)
        {
        TALK printf("No further action\n"); 
        } 
      else if(l==1)
        {
	newname=link1[j];
        for(l=i=0;i<lnks;i++)
           if(link1[i]==(newname)) l++;
        TALK printf("target has %d links\n",l);
        if(l==1) 
          {
          newnumber[newname]=oldnumber[oldname];
        TALK printf("We propagate the name forward (%d)\n",newnumber[newname]);
          }
        else if(newnumber[newname]==0) newnumber[newname]=assigned++;
        }
      else 
        {
        for(i=0;i<lnks;i++)if(link0[i]==oldname)
           {
           newname=link1[i];
           if(newnumber[newname]==0)newnumber[newname]=assigned++;
           }
        }
      }
   /* OK, we have done everything that we were told to do. Now we have to 
      check whether there are any genuinely new objects in there! */

   for(newname=0;newname<obs;newname++)
      if(newnumber[newname]==0)newnumber[newname]=assigned++;

   /* So the new roster is complete! */
   for(i=0;i<obs;i++)putw(newnumber[i],fo);
   /* Output here whatever has to go into the untangle file */
   TALK printf("Frame %d, %d objects\n",frm,obs);
   TALK for(i=0;i<obs;i++) printf("%4d", newnumber[i]-1000);	   
   TALK printf("\n\n---------------------------------------------\n\n");

   /* And now copy stuff into the old thing */
   for(i=0;i<obs;i++)oldnumber[i]=newnumber[i];
   oldobs=obs;
   }

fclose(ff);
fclose(fo);
}
