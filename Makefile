STMAIN    = spheretest
STSOURCES = spheretest.cpp sphereModels.cpp math3d.cpp

CC = g++
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
LIBS    = -lX11 -lglut -lGL -lGLU -lm
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL

STOBJECTS = $(STSOURCES:.cpp=.o)
CFLAGS  = -c -Wall -g $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LIBS)



$(STMAIN) : $(STOBJECTS)
	$(CC) -o $@  $(STOBJECTS) $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o tides spheretest
