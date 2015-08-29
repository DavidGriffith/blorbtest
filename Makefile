CC = gcc
FLAGS = -g -ggdb -Wall
LIBS = -lsndfile
#AUDIO_LIBS = -lao -DWITH_AO


all:	blorblib blorbtest sampletest

blorbtest:	blorbtest.c blorblib
	$(CC) -o blorbtest blorbtest.c blorblib.a $(LIBS) $(AUDIO_LIBS) $(FLAGS) 

blorblib:
	$(CC) -o blorblib.o -c blorblib.c
	ar rc blorblib.a blorblib.o
	ranlib blorblib.a

sampletest:	sampletest.c
	$(CC) -o sampletest sampletest.c $(LIBS) $(FLAGS) $(AUDIO_LIBS)
	
clean:
	rm -f *.o *.a sampletest blorbtest sampleout.aiff blorbout.aiff 
