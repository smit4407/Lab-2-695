# Application-specific makefile.  This file only needs to
# set the APPROOT, SRCS, HDRS, and EXEC variables properly 
# (i.e. the location of the apps directory in relation to this Makefile), and
# then include the Makerules file from the main apps directory.
# All the real work in done in Makerules.  Things are setup
# this way because the build procedure for all apps is basically the same.


# SRCS=producer.c
# EXEC=producer.dlx.obj
# SRCS=n3_injector.c h2o_injector.c
# EXEC=n3_injector.dlx.obj h2o_injector.dlx.obj
SRCS=n3_injector.c
EXEC=n3_injector.dlx.obj

include ../Makerules

include $(APPROOT)/Makerules

