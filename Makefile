SMSOURCES  = spheretest.cpp sphereModels.cpp math3d.cpp utilities.cpp
TXSOURCES  = texturetest.cpp textureData.cpp sphereModels.cpp math3d.cpp utilities.cpp
SHSOURCES  = shadertest.cpp textureData.cpp sphereModels.cpp math3d.cpp utilities.cpp
TX2SOURCES = texturetest2.cpp textureData.cpp sphereModels.cpp math3d.cpp utilities.cpp
NBSOURCES  = nbtest.cpp nbody.cpp sphereModels.cpp math3d.cpp nb_creators.cpp utilities.cpp
TESTSOURCES = tritest.cpp utilities.cpp

CC = g++
LIBDIRS = -L/usr/X11R6/lib -L/usr/X11R6/lib64 -L/usr/local/lib
LIBS    = -lX11 -lglut -lGL -lGLU -lm -lGLEW
INCDIRS = -I/usr/include -I/usr/local/include -I/usr/include/GL

CFLAGS  = -c -Wall -g $(INCDIRS)
LDFLAGS = $(LIBDIRS) $(LIBS)

all: spheretest nbtest tritest

spheretest: $(SMSOURCES:.cpp=.o)
	$(CC) -o $@  $(SMSOURCES:.cpp=.o) $(LDFLAGS)

#texturetest: $(TXSOURCES:.cpp=.o)
#	$(CC) -o $@  $(TXSOURCES:.cpp=.o) $(LDFLAGS)

#shadertest: $(SHSOURCES:.cpp=.o)
#	$(CC) -o $@  $(SHSOURCES:.cpp=.o) $(LDFLAGS)

#texturetest2: $(TX2SOURCES:.cpp=.o)
#	$(CC) -o $@  $(TX2SOURCES:.cpp=.o) $(LDFLAGS)

nbtest: $(NBSOURCES:.cpp=.o)
	$(CC) -o $@  $(NBSOURCES:.cpp=.o) $(LDFLAGS)

tritest: $(TESTSOURCES:.cpp=.o)
	$(CC) -o $@  $(TESTSOURCES:.cpp=.o) $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f *.o tides spheretest nbtest texturetest texturetest2 tritest
