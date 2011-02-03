SMSOURCES = spheretest.cpp sphereModels.cpp math3d.cpp
NBSOURCES = nbtest.cpp nbody.cpp sphereModels.cpp math3d.cpp nb_creators.cpp

CC = g++
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
LIBS    = -lX11 -lglut -lGL -lGLU -lm
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL

CFLAGS  = -c -Wall -g $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LIBS)

all: spheretest nbtest

spheretest: $(SMSOURCES:.cpp=.o)
	$(CC) -o $@  $(SMSOURCES:.cpp=.o) $(LDFLAGS)
	
nbtest: $(NBSOURCES:.cpp=.o)
	$(CC) -o $@  $(NBSOURCES:.cpp=.o) $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o tides spheretest nbtest
