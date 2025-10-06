# Application-specific makefile.  This file only needs to
# set the APPROOT, SRCS, HDRS, and EXEC variables properly 
# (i.e. the location of the apps directory in relation to this Makefile), and
# then include the Makerules file from the main apps directory.
# All the real work in done in Makerules.  Things are setup
# this way because the build procedure for all apps is basically the same.


# SRCS=consumer.c
# EXEC=consumer.dlx.obj
# SRCS=reaction1.c reaction2.c reaction3.c
# EXEC=reaction1.dlx.obj reaction2.dlx.obj reaction3.dlx.obj
SRCS=reaction1.c
EXEC=reaction1.dlx.obj

include ../Makerules

include $(APPROOT)/Makerules

