/* speedclicks.h

What the keyclicks are supposed to do
Revised Donald Winkelmann 3/08/02

Copyright (c) 2002 UMDNJ-RWJMS. All rights reserved.
*/


void do_hint()
{
int where, i,j;

updatebreaks();
where=hint;
for(i=0;i<nbreaks;i++)
    {
    if( (where>from[i]) && (where<to[i]))
        {
    /* We have to create a break; */
    for(j=nbreaks;j>i;j--) to[j]=to[j-1];
    for(j=nbreaks;j>i;j--) from[j]=from[j-1];
    to[i]=where; from[i+1]=where+1;
    nbreaks++;
    redisplayall();
        }
    }
    updatebreaks();
}


void gplclick(int mx, int my) /* Moves the nearest left edge here */
{
int where, i,j, mindist, mini, dist;

updatebreaks();
j=0; mini=-1; mindist=60;
for(i=0;i<nbreaks;i++)
    {
    where=rint((4*ww/5)*from[i]/ndata);
    dist=mx-where; if(dist<0)dist= -dist;
    if(dist<mindist) { mindist=dist; mini=i; }
    }
if(mini != -1)
    {
    from[mini] = rint(mx*ndata/(4*ww/5));
    redisplayall();
    }
    updatebreaks();
}


void gprclick(int mx, int my)
{
int where, i,j, mindist, mini, dist;

updatebreaks();
j=0; mini=-1; mindist=60; // mindist was 40 DAW
for(i=0;i<nbreaks;i++)
    {
    where=rint((4*ww/5)*to[i]/ndata);
    dist=mx-where; if(dist<0)dist= -dist;
    if(dist<mindist) { mindist=dist; mini=i; }
    }

if(mini != -1)
    {
    to[mini] = rint(mx*ndata/(4*ww/5));
    redisplayall();
    }
    updatebreaks();
}


void gpmclick(int mx, int my)
{
int where, i,j;

updatebreaks();
where=rint(mx*ndata/(4*ww/5));
for(i=0;i<nbreaks;i++)
    {
    if( (where>from[i]) && (where<to[i]))
        {
    /* We have to create a break; */
    for(j=nbreaks;j>i;j--) to[j]=to[j-1];
    for(j=nbreaks;j>i;j--) from[j]=from[j-1];
    to[i]=where; from[i+1]=where+1;
    nbreaks++;
    redisplayall();
        }
    }
    updatebreaks();
}
