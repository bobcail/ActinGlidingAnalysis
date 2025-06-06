#----------------------------------------------------------------------------------
# COPYRIGHT 2005 Robert Wood Johnson Medical School
#-------------------------------------------------------------------------------
#  Makefile for Motility Tracking programs.  This is v1.5 
#
#  If you want to change the default compiler, change the 'CC' variable 
#  (e.g. SunOS machines may need /usr/ucb/cc for BSD compatibility and linux gcc)
#
#  Author:
#		Donald A. Winkelmann
#		Division of Medical Informatics
#		Department of Pathology and Laboratory Medicine
#		Robert Wood Johnson Medical School - UMDNJ
#----------------------------------------------------------------------------------
# If you are compiling on a system other that Mac OSX, there are 4 items that need your
# attention listed below
#
# (1)
# CC_FLAGS: Enter the appropriate compiler flags here. 
#           Beware of over-optimizing the compile. On OSX O3 is the maximum!
#           Also added an architecture flag for intel (-arch i386) vs. ppc (-arch ppc) processor
#			If you change to -arch ppc, you will need to change the value of need_swab=1 to =0 in
#			read_ipdataset.c line 17 in order to read iVision (IpLab) file formats. 
#
CC_FLAGS = -O3 -arch x86_64
#
#(2)
# L_LIBS:   The linking libraries depend on the OS of the installation. The default are for
#			Mac OSX.  An example for Linux is given. If you want to port to other systems,
#           this line will need checking.
#MacOSX 
L_LIBS = -framework GLUT -framework OpenGL -lobjc.A -L/opt/local/lib
#
#Linux
#L_LIBS = -L/usr/lib -lm -lGL -lGLU -lglut -ltiff -L/usr/X11R6/lib -lXmu -lX11 
#
#(3)
# DEFS:    One other problem, there is a little difference in the location of the GLUT include
#          files between MacOSX and Linux. So DEFS = -DLinux sets <GLUT/glut.h> to <GL/glut.h>
#		   for Linux. DEFS = <null> uses the default Mac OSX definitions
#MacOSX
DEFS =
#
#Linux
#DEFS = -DLINUX
#
#(4)
# INST_DIR:  The directory to install all tracking programs.  Each user that
#            uses the programs must have this directory in their $PATH.  Also, whoever
#            compiles the programs must have write permission to this directory, and it
#            must already exist. For example you can use the /bin directory shipped with
#            the source code:
#
INST_DIR = ../bin 
###################################################################################
#                    No editing beyond this point                                 #
###################################################################################
#
CC	 = cc
CFLAGS   = $(CC_FLAGS) $(DEFS)
LIBS     = $(L_LIBS)
INSTALL  = mv -f

all:	speed setfilters showcwb showswb grabworms asciibatchns threadworms untangle makewormlist extractworm install

speed: speed.o
	$(CC) $(CFLAGS) -o speed speed.o $(LIBS)

setfilters: setfilters.o read_ipdataset.o
	$(CC) $(CFLAGS) -o setfilters setfilters.o read_ipdataset.o $(LIBS) -ltiff

showcwb: showcwb.o
	$(CC) $(CFLAGS) -o showcwb showcwb.o $(LIBS)

showswb: showswb.o
	$(CC) $(CFLAGS) -o showswb showswb.o $(LIBS)

grabworms: grabworms.o read_ipdataset.o
	$(CC) $(CFLAGS) -o grabworms grabworms.o read_ipdataset.o $(LIBS) -ltiff

asciibatchns: asciibatchns.o
	$(CC) $(CFLAGS) -o asciibatchns asciibatchns.o $(LIBS)

threadworms: threadworms.o
	$(CC) $(CFLAGS) -o threadworms threadworms.o $(LIBS)

untangle: untangle.o
	$(CC) $(CFLAGS) -o untangle untangle.o $(LIBS)
        
makewormlist: makewormlist.o
	$(CC) $(CFLAGS) -o makewormlist makewormlist.o $(LIBS)
        
extractworm: extractworm.o
	$(CC) $(CFLAGS) -o extractworm extractworm.o $(LIBS)
        
install:
	$(INSTALL) speed $(INST_DIR)
	$(INSTALL) setfilters $(INST_DIR)
	$(INSTALL) showcwb $(INST_DIR)
	$(INSTALL) showswb $(INST_DIR)
	$(INSTALL) grabworms $(INST_DIR)
	$(INSTALL) asciibatchns $(INST_DIR)
	$(INSTALL) threadworms $(INST_DIR)
	$(INSTALL) untangle $(INST_DIR)
	$(INSTALL) makewormlist $(INST_DIR)
	$(INSTALL) extractworm $(INST_DIR)
	cp -f analyze $(INST_DIR)

clean:
	rm -f *.o

tar:
	(cd ..; tar -cvf tracks_v1.5.tar bin src ; compress -f tracks_v1.5.tar)

#
# Dependencies
#
speed.o : speedclicks.h speedfile.h speedsmooth.h speeddisplay.h speedpostscript.h showswb_sp.h
setfilters.o : bitmaps.h linearfilters.h setfilter.h topofilters.h floodfill.h iplfile.h parameters.h read_tiff.h
showcwb.o : getbraided.h 
grabworms : bitmaps.h iplfile.h parameters.h floodfill.h linearfilters.h topofilters.h
