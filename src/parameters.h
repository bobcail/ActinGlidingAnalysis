/* parameters.h

 Read parameters from a .Parameter file.                 
 Magnasco and Schnitzer, 1993                            
 Format of the call:                                     

 BGNPARAMETERS( "routine name " )                        
 PARAMETER( variable_name , "parameter_name_string" )    
 ENDPARAMETERS                                           

 Yes, like that, no ';'s , no nothing else. 
 Please realize that every call to these macros open the .Parameters 
 file, and close it. Hence you can call it from several different 
 subroutines, but never call it inside a loop or something. 
 Also, the bracketing must be as shown, or else ...

 Comments not included in the file for obvious reasons.

*/


#define BGNPARAMETERS( a ) { \
FILE *par; char parname[100]; char scratch[120]; int parvalue; \
printf("\n---------- Parameters for %s ---------\n",a);\
if( (par=fopen(".Parameters","r")) == NULL) \
  printf("\n\nWARNING: Could not open parameter file\n"); \
else { \
  for(;fgets(scratch,120,par)!=NULL;) {\
     sscanf(scratch,"%s %d",parname,&parvalue);\
     if(0);

#define PARAMETER(a,b) else if( strcmp(b,parname) == 0)\
    { a=parvalue; printf("Parameter %s=%d\n",parname,parvalue); }

#define ENDPARAMETERS else ;} \
 fclose(par); printf("---------- End Parameter Block ------------\n\n");} }
