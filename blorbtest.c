/*
 * Loading sounds from a Blorb file
 *
 * Using libsndfile to load Oggv chunks.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <sndfile.h>

#ifdef WITH_AO
  #include <ao/ao.h>
  ao_device *device;
  ao_sample_format format;
#endif

#include "blorb.h"
#include "blorblow.h"

#define BUFFSIZE 4096

int playsample(FILE *, bb_result_t);
int playogg(FILE *, bb_result_t);


int main(int argc, char *argv[])
{
    FILE *blorbFile;
    int number;
    bb_map_t *blorbMap;
    bb_result_t resource;
    bb_err_t	bb_err;

    if (argc == 3) {
	number = atoi(argv[2]);
    } else {
	printf("usage: blorbtest <blorbfile> <resource_number>\n");
	exit(1);
    }

    blorbFile = fopen(argv[1], "rb");
    if (blorbFile == NULL) {
	printf("Can't open %s\n", argv[1]);
	exit(1);
    }
    bb_err = bb_create_map(blorbFile, &blorbMap);
    if (bb_err != bb_err_None) {
	printf("Unable to build Blorb map: %s\n", bb_err_to_string(bb_err));
	exit(1);
    }

    if (bb_load_resource(blorbMap, bb_method_FilePos, &resource, bb_ID_Snd, number) == bb_err_None) {
	printf("Sound resource %d is ",number);
	if (blorbMap->chunks[resource.chunknum].type == bb_make_id('F','O','R','M')) {
	    printf("an AIFF sample.\n");
	    printf("Size: %zu bytes.\n", (size_t) resource.length);
	    printf("Loading and playing with libsndfile\n");
	    playsample(blorbFile, resource);
	} else if (blorbMap->chunks[resource.chunknum].type == bb_make_id('M','O','D',' ')) {
	    printf("a MOD file.\n");
	    printf("Size: %zu bytes.\n", (size_t) resource.length);
	    printf("  -- This test doesn't involve MOD.\n");
	} else if (blorbMap->chunks[resource.chunknum].type == bb_make_id('O','G','G','V')) {
	    printf("an OGG compressed sample.\n");
	    printf("Size: %zu bytes.\n", (size_t) resource.length);
	    printf("Loading and playing with libsndfile\n");
	    playsample(blorbFile, resource);
	} else
	    printf("something else.  This should not happen.\n");
    } else
	printf("Sound resource %d not found.\n", number);

    return 0;
}


int playsample(FILE *fp, bb_result_t result)
{
    int frames_read;
    int frames_written;
    int total_read = 0;
    int total_written = 0;

    int count;
    int toread;
    int *buffer;
    long filestart;

    SNDFILE     *sndfile = NULL;
    SF_INFO     sf_info;

    SNDFILE	*outfile = NULL;
    SF_INFO	outfile_info;

#ifdef WITH_AO
    int default_driver;
    ao_device *device;
    ao_sample_format format;

    ao_initialize();
    default_driver = ao_default_driver_id();
    memset(&format, 0, sizeof(ao_sample_format));
#endif

    memset(&sf_info, 0, sizeof(SF_INFO));
    memset(&outfile_info, 0, sizeof(SF_INFO));

    filestart = ftell(fp);

    lseek(fileno(fp), result.data.startpos, SEEK_SET);
    sndfile = sf_open_fd(fileno(fp), SFM_READ, &sf_info, 0);

    outfile_info.format = SF_FORMAT_AIFF | SF_FORMAT_PCM_S8;
    outfile_info.channels = sf_info.channels;
    outfile_info.samplerate = sf_info.samplerate;

    outfile = sf_open("blorbout.aiff", SFM_WRITE, &outfile_info);

    printf("  Channels:       %d\n", sf_info.channels);
    printf("  Rate:           %d\n", sf_info.samplerate);
    printf("  Frames:         %d\n", (int) sf_info.frames);
    printf("  Samples:        %d\n", (int) sf_info.frames * sf_info.channels);
    printf("Processing\n");

#ifdef WITH_AO
    format.byte_format = AO_FMT_NATIVE;
    format.bits = 32;
    format.channels = sf_info.channels;
    format.rate = sf_info.samplerate;
    device = ao_open_live(default_driver, &format, NULL /* no options */);

    if (device == NULL) {
        printf("Error opening sound device.\n");
        return 1;
    }
#endif

    buffer = malloc(BUFFSIZE * sf_info.channels * sizeof(int));
    frames_read = 0;
    toread = sf_info.frames * sf_info.channels;

    while (toread > 0) {
	if (toread < BUFFSIZE * sf_info.channels)
	    count = toread;
	else
	    count = BUFFSIZE * sf_info.channels;

	frames_read = sf_read_int(sndfile, buffer, count);
	frames_written = sf_write_int(outfile, buffer, count);
	toread = toread - frames_read;
	total_read += frames_read;
	total_written += frames_written;
	printf("  Reading:        %d\n", frames_read);

#ifdef WITH_AO
	ao_play(device, (char *)buffer, frames_read * sizeof(int));
#endif
    }
    printf("Finished\n");
    printf("  Total read:     %d\n", total_read);
    printf("  Total writ:     %d\n", total_written);

    free(buffer);
    fseek(fp, filestart, SEEK_SET);
    sf_close(sndfile);

#ifdef WITH_AO
    ao_close(device);
    ao_shutdown();
#endif

    return 0;
}
