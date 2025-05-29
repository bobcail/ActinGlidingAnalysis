Readme
ls 
Tracking v1.6

March 9, 2018

Tracking v1.6 contains the March 2018 changes to 'speed' that initiated auto selection of void do_hint. The auto mode was set to run 10 times before manual mode kicked in. This is a bit of a Kluge, but it works. These changes to the 'speed' program have been thoroughly tested and are no longer maintained separate from the other programs. So, the Speed_SA directory has been discarded.

Nov. 12, 2020

In MacOSX Catalina the top directory "/" is read-only so putting /sw in that directory is difficult. /sw is needed for the Tiff libraries. To work around this restriction a set of sw directories were placed in /use/local/sw and the Makefile was rebuilt to reflect this new location. This works and uses the Tiff Libraries on a run-time basis, so any computer running these programs will require /usr/local/sw libraries saved in this location.

