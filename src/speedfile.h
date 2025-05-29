/* speedfile.h

    The format of the speed file is: (new and modified)

    from	to	speed	error	<len>	error	asciifile
    (%4d)	(%4d)	(%9.6f)	(%9.6f)	(%9.5f)	(%9.6f)	(%s)

    Revised 3/02/03 by Donald Winkelmann
    Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/

void getspeeds(char *asciifile, 
	int *nsegments,
	int *from,
	int *to)
{
FILE *speedfile;
char thisfile[10];
int ff, tt;
double spd, avl, erravl, esp;

*nsegments=0;
chdir (mydir); // the cwd kluge! DAW

if( (speedfile=fopen("data","r")) == NULL)
    {
    fprintf(stderr,"Speed file `data' does not exist\n");
    return;
    }

for(; fscanf(speedfile,"%d %d %lf %lf %lf %lf %s",
      &ff, &tt, &spd, &esp, &avl, &erravl, thisfile) == 7 ;)
    if( strcmp(asciifile,thisfile)==0)
    {
    from[*nsegments]=ff;
    to[*nsegments]  =tt;
    ++*nsegments;
    }
fclose(speedfile);
}

void putspeeds(
  	char *asciiname,
	int nsegments,
	int *from,
	int *to,
	double *spd,
	double avl)
{
FILE *speedfile;
char cmd[100];
int i,j; double len;

chdir (mydir); // the cwd kluge! DAW
system("touch data");
sprintf(cmd,"grep -v %s data > newdata",asciiname); system(cmd);
system("mv newdata data");

if( (speedfile=fopen("data","a")) == NULL)
    {
    fprintf(stderr,"Serious error: can't open data file for output\n");
    exit(1);
    }

for(i=0;i<nsegments;i++)
    {
    avl=erravl=0.0;
    for(j=from[i];j<to[i];j++)
        {
        len=fronttip[j]-backtip[j];
        avl+=len; erravl+=len*len;
        }
    avl/= to[i]-from[i];
    erravl= sqrt( erravl/(to[i]-from[i]) -avl*avl);
    if (to[i]-from[i]>4){ 		// Output filters to eliminate bad data points DAW
        if(erravl/avl<0.9)
            fprintf(speedfile,"%4d %4d %9.6f %9.6f %9.5f %9.6f %s\n",
            from[i],to[i],spd[i],errspd[i],avl,erravl,asciiname);
            }
    }
fclose(speedfile);   
}


void loadfile(char *fname)
{
int i,j;
double x,y,z;
FILE *fin;

chdir (mydir); // the cwd kluge! DAW
if( (fin=fopen(fname,"r")) == NULL)
    {
    fprintf(stderr,"I couldn't open file %s!\n",fname);
    exit(1);
    }

avl=0.0;
erravl=0.0;
for(i=0;fscanf(fin,"%d %lf %lf %lf",&j,&x,&y,&z)==4;i++)
    {
    xx[i]=x; fronttip[i]=y; backtip[i]=z;
    }
fclose(fin);
ndata=i;
smoothdata();
getspeeds(fname,&nbreaks,from,to);
if(nbreaks==0)
    {
    from[0]=1; to[0]=ndata-2;
    nbreaks=1;
    }
}


void savefile(char *fname)
{
    putspeeds(fname,nbreaks,from,to,a,avl);
}
