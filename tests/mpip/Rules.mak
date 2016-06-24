# Makefile for MPIP	-*-Makefile-*-
# Please see license in doc/UserGuide.html
# $Id: Rules.mak 369 2006-10-05 19:21:12Z chcham $

.c.o:
	${CC} ${CFLAGS} ${CPPFLAGS} -c $< -o $@

.f.o:
	${FC} ${CFLAGS} -c $< -o $@

clean::
	-rm -f $(OBJS) *.mpiP *.log core *~ ${C_TARGET} TAGS

##### EOF
