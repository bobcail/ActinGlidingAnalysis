/* makewormlist.c

   Make a list of all objects in cwb files; explicitly count the number of
   frames in which each object is found to get the lifetime of each 
   object, and output all worms living for more than argument frames.
   Conceptually to be used with output piped to sort, and then a shell
   running the output
   
   Marcello O. Magnasco, Rockefeller 1994
 */
#include <stdlib.h>
#include <stdio.h>
#include "getbraided.h"
#define TOTNAMES 80000
#define LOCNAMES 10000

int lifetime[TOTNAMES];
int lastseen[TOTNAMES];

int names[LOCNAMES];

int main(int argc, char **argv)
{
int i,j,k, frm, objects, threshold;

threshold=0;
if(argc==2) threshold=atoi(argv[1]);
if(threshold==0) threshold=30;

for(i=0;i<TOTNAMES;i++)lastseen[i]=lifetime[i]=0;

initbraids();

getnextframe(&frm,&objects,names);

for(; getnextframe(&frm,&objects,names);)
   {
   if(objects>LOCNAMES)
     {
     fprintf(stderr,"objects=%d > LOCNAMES=%d\nRecompile\n",objects,LOCNAMES);
     exit(1);
     }
   for(i=0;i<objects;i++)
      {
      if(names[i] > TOTNAMES)
        {
        fprintf(stderr,"names=%d > TOTNAMES=%d\nRecompile!\n",*names,TOTNAMES);
        exit(1);
        }
       
      lifetime[names[i]]++;
      lastseen[names[i]]=frm;
      }
   }

fclose(braidfile);

for(i=0;i<TOTNAMES;i++) 
   if(lifetime[i]>threshold) 
     printf("extractworm %6d %6d %6d %6d\n",
          lifetime[i],i,lastseen[i]-lifetime[i],lastseen[i]);
}
