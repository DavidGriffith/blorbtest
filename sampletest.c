/*
 * A simple program that loads a file with libsndfile and plays it.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sndfile.h>

#define BUFFSIZE 4096

int playfile(FILE *);

int main(int argc, char *argv[])
{
    FILE *fp;

    if (argc < 2) {
	printf("usage: %s <filename>\n", argv[0]);
	exit(1);
    }

    fp = fopen(argv[1], "rb");
    if (fp == NULL) {
	printf("Cannot open %s.\n", argv[1]);
	exit(2);
    }

    playfile(fp);
    fclose(fp);

    return 0;
}

int playfile(FILE *fp)
{
    int frames_read;
    int frames_written;
    int total_read = 0;
    int total_written = 0;
    int count;
    int toread;
    int *buffer;
    long filestart;

    SNDFILE     *sndfile;
    SF_INFO     sf_info;

    SNDFILE     *outfile;
    SF_INFO     outfile_info;

    filestart = ftell(fp);

    memset(&sf_info, 0, sizeof(SF_INFO));
    memset(&outfile_info, 0, sizeof(SF_INFO));

    sndfile = sf_open_fd(fileno(fp), SFM_READ, &sf_info, 0);

    outfile_info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_S8;
    outfile_info.channels = sf_info.channels;
    outfile_info.samplerate = sf_info.samplerate;

    outfile = sf_open("sampleout.aiff", SFM_WRITE, &outfile_info);

    buffer = malloc(BUFFSIZE * sf_info.channels * sizeof(int));
    frames_read = 0;
    toread = sf_info.frames * sf_info.channels;

    printf("Total frames:     %d\n\n", toread);

    while (toread > 0) {
	if (toread < BUFFSIZE * sf_info.channels)
	    count = toread;
	else
	    count = BUFFSIZE * sf_info.channels;

	printf("Frames attempted: %d\n", count);

        frames_read = sf_read_int(sndfile, buffer, count);
	frames_written = sf_write_int(outfile, buffer, count);
	toread = toread - frames_read;
        total_written += frames_written;
        total_read += frames_read;
	printf("frames read:      %d\n", frames_read);
	printf("frames remaining: %d\n\n", toread);
    }

    fseek(fp, filestart, SEEK_SET);
    sf_close(sndfile);
    free(buffer);

    printf("Read:    %d\n", total_read);
    printf("Written: %d\n", total_written);
    printf("Finished\n");

    return 0;
}
