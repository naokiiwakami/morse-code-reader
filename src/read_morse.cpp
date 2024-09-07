#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <ncurses.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include <pulse/error.h>
#include <pulse/simple.h>

#include "fft.h"
#include "monitor.h"
#include "morse_reader.h"
#include "morse_signal_detector.h"

#define BUFFER_SIZE 256
#define NUM_BUFFERS 4

/**
 * Read morse signals from pattern file instead of analysing wav.
 */
void ReadFile(int fd, ::morse::MorseReader *reader) {
  auto *monitor = new morse::Monitor();

  const size_t kBufSize = 1024;
  char buffer[kBufSize];

  int length;
  uint8_t prev_level = 0;
  while ((length = read(fd, buffer, sizeof(buffer))) > 0) {
    for (int i = 0; i < length; ++i) {
      usleep(10000);
      char value = buffer[i];
      monitor->AddSignal(value);
      uint8_t level;
      switch (value) {
      case '^':
        level = 1;
        break;
      case '_':
        level = 0;
        break;
      default:
        // ignore
        continue;
      }
      reader->Update(level);
      if (prev_level != level) {
        monitor->Dump(reader);
      }
      prev_level = level;
    }
  }
  monitor->Dump(reader);
  delete monitor;
}

int main(int argc, char *argv[]) {
  // read arguments
  std::string pattern_file_name{};
  std::string analysis_file_name{};
  bool verbose = false;
  int mute = 0;
  while (true) {
    static struct option long_options[] = {
        {"record", required_argument, nullptr, 'r'},
        {"analyze", required_argument, nullptr, 'a'},
        {"mute", no_argument, &mute, 1},
        {0, 0, 0, 0},
    };
    int c = getopt_long(argc, argv, "r:a:v", long_options, nullptr);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 'r':
      pattern_file_name = optarg;
      break;
    case 'a':
      analysis_file_name = optarg;
      break;
    case 'v':
      verbose = true;
      break;
    }
  }

  if (optind >= argc) {
    fprintf(stderr, "Usage: %s [options] <morse_audio_wav>\n",
            basename(argv[0]));
    fprintf(stderr, "options:\n");
    fprintf(stderr,
            "  --record <pattern_file> : Dump signal pattern to file\n");
    exit(1);
  }

  auto input_file_name = argv[optind++];

  // make morse timing tracker
  auto *morse_reader = new ::morse::MorseReader();

  // setup input file
  SF_INFO sf_info = {0};
  SNDFILE *sndfile;
  sndfile = sf_open(input_file_name, SFM_READ, &sf_info);
  if (sndfile == nullptr) {
    /*
    int sf_errno = sf_error(sndfile);
    if (sf_errno != SF_ERR_SYSTEM && sf_errno != SF_ERR_UNRECOGNISED_FORMAT) {
      fprintf(stderr, "File error: %s: %s\n", input_file_name,
              sf_error_number(sf_errno));
      return 1;
    }
    */
    int fd = open(input_file_name, 0);
    if (fd < 0) {
      fprintf(stderr, "File error: %s: %s\n", input_file_name, strerror(errno));
      return 1;
    }
    ReadFile(fd, morse_reader);
    close(fd);
    return 0;
  }

  if (verbose) {
    fprintf(stderr, "file       = %s\n", input_file_name);
    fprintf(stderr, "channels   = %d\n", sf_info.channels);
    fprintf(stderr, "samplerate = %d\n", sf_info.samplerate);
    fprintf(stderr, "channels   = %d\n", sf_info.channels);
    fprintf(stderr, "format     = 0x%x\n", sf_info.format);
  }

  printf("\n");

  // setup playback environment
  pa_sample_spec ss;
  memset(&ss, 0, sizeof(ss));
  ss.format = PA_SAMPLE_S16LE;
  ss.rate = sf_info.samplerate;
  ss.channels = sf_info.channels;

  int error;
  pa_simple *pa = pa_simple_new(NULL, argv[0], PA_STREAM_PLAYBACK, NULL,
                                "playback", &ss, NULL, NULL, &error);
  if (!pa) {
    fprintf(stderr, ": pa_simple_new() failed: %s\n", pa_strerror(error));
    sf_close(sndfile);
    return -1;
  }

  // setup buffers
  short buffers_mem[NUM_BUFFERS][BUFFER_SIZE];
  short *buffers[NUM_BUFFERS];
  for (int i = 0; i < NUM_BUFFERS; ++i) {
    buffers[i] = buffers_mem[i];
    memset(buffers[i], 0, sizeof(short) * BUFFER_SIZE);
  }

  short *current_buffer = buffers[NUM_BUFFERS - 1];

  // setup morse reader
  auto *signal_detector =
      new ::morse::MorseSignalDetector(morse_reader, NUM_BUFFERS, BUFFER_SIZE);
  signal_detector->Verbose(verbose);
  if (!pattern_file_name.empty() &&
      signal_detector->SetDumpFile(pattern_file_name) < 0) {
    fprintf(stderr, "File open failed: %s (%s)\n", pattern_file_name.c_str(),
            strerror(errno));
    exit(-1);
  }
  if (!analysis_file_name.empty() &&
      signal_detector->SetAnalysisFile(analysis_file_name) < 0) {
    fprintf(stderr, "File open failed: %s (%s)\n", analysis_file_name.c_str(),
            strerror(errno));
    exit(-1);
  }

  morse::Monitor *monitor = nullptr;
  if (analysis_file_name.empty()) {
    monitor = new morse::Monitor();
  }

  // read and process data of approximately 6ms for each in the loop
  int num_windows = 0;
  sf_count_t num_samples;
  do {
    num_samples = sf_read_short(sndfile, current_buffer, BUFFER_SIZE);

    if (!mute) {
      if (pa_simple_write(pa, current_buffer,
                          (size_t)(num_samples * sizeof(*current_buffer)),
                          &error) < 0) {
        fprintf(stderr, __FILE__ ": pa_simple_write() failed: %s\n",
                pa_strerror(error));
        if (pa) {
          pa_simple_free(pa);
        }

        return -1;
      }
    }

    if (++num_windows >= NUM_BUFFERS) {
      signal_detector->Process(buffers, num_samples, monitor);
    }

    short *temp = buffers[0];
    for (int i = 1; i < NUM_BUFFERS; ++i) {
      buffers[i - 1] = buffers[i];
    }
    buffers[NUM_BUFFERS - 1] = temp;
    current_buffer = temp;

  } while (num_samples == BUFFER_SIZE);

  /*
    if (monitor != nullptr) {
      monitor->Dump(morse_reader);
    }
    */

  delete monitor;

  if (pa_simple_drain(pa, &error) < 0) {
    fprintf(stderr, __FILE__ ": pa_simple_drain() failed: %s\n",
            pa_strerror(error));
  }

  // shutdown
  if (pa) {
    pa_simple_free(pa);
  }
  delete signal_detector;
  sf_close(sndfile);

  return 0;
}