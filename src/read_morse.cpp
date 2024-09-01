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

#include "decoder.h"
#include "default_decoder.h"
#include "fft.h"
#include "monitor.h"
#include "morse_reader.h"
#include "morse_signal_detector.h"

#define BUFFER_SIZE 256
#define WINDOW_SIZE (BUFFER_SIZE * 2)

namespace morse {

class StubMorseDecoder : public Decoder {
private:
  bool silent_;

public:
  StubMorseDecoder(bool silent) : silent_(silent) {}

  void Dit() {
    if (!silent_) {
      printf("(dit)");
    }
  }

  void Dah() {
    if (!silent_) {
      printf("(dah)");
    }
  }

  void Break() {
    if (!silent_) {
      printf("( )");
    }
  }

  void Space() {
    if (!silent_) {
      printf("(/)");
    }
  }
};

} // namespace morse

void ReadFile(int fd, ::morse::MorseReader *reader) {
  // curses setup
  /*
  int row, col;
  initscr();
  getmaxyx(stdscr, row, col);
  curs_set(0);
  WINDOW *local_win = newwin(row - 12, col, 12, 0);
  */
  auto *monitor = new morse::Monitor();

  const size_t kBufSize = 1024;
  char buffer[kBufSize];

  // int prev_level = 0;
  int irow = 0;
  int icol = 1;
  int length;
  uint8_t prev_level = 0;
  while ((length = read(fd, buffer, sizeof(buffer))) > 0) {
    for (int i = 0; i < length; ++i) {
      usleep(10000);
      char value = buffer[i];
      // printf("%c", value);
      // fflush(stdout);
      /*
      mvprintw(irow, icol++, "%c", value == '^' ? '^' : ' ');
      refresh();
      if (icol == col - 1) {
        ++irow;
        icol = 1;
      }
      */
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
        /*
          wclear(local_win);
          reader->Dumpw(col, row, local_win);
          wrefresh(local_win);
          */
      }
      prev_level = level;
      /*
      timing_tracker->Proceed();
      char value = buffer[i];
      int level;
      switch (value) {
      case '^':
        level = 1;
        if (prev_level == 0) {
          timing_tracker->Rise();
        }
        break;
      case '_':
        level = 0;
        if (prev_level == 1) {
          timing_tracker->Fall();
        }
        break;
      default:
        // ignore
        continue;
      }
      prev_level = level;
      */
    }
  }
  monitor->Dump(reader);
  /*
  wclear(local_win);
  reader->Dumpw(col, row, local_win);
  wrefresh(local_win);
  */

  // printf("\n");
  // reader->Dump();
  /*
  mvprintw(row - 1, 0, "press any key to exit");
  getch();
  endwin();
  */
  delete monitor;
}

int main(int argc, char *argv[]) {

  // read arguments
  std::string pattern_file_name{};
  bool verbose = false;
  while (true) {
    static struct option long_options[] = {
        {"record", required_argument, nullptr, 'r'}, {0, 0, 0, 0}};
    int c = getopt_long(argc, argv, "r:v", long_options, nullptr);
    if (c == -1) {
      break;
    }
    switch (c) {
    case 'r':
      pattern_file_name = optarg;
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
  auto *morse_decoder = new ::morse::DefaultDecoder{};
  morse_decoder->Subscribe(new ::morse::DefaultEventListener{});
  auto *morse_reader = new ::morse::MorseReader(morse_decoder);

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
  short buffers[2][BUFFER_SIZE];
  short *current_buffer;
  short *prev_buffer;
  prev_buffer = buffers[0];
  current_buffer = buffers[1];
  memset(prev_buffer, 0, sizeof(short));

  // setup morse reader
  auto *signal_detector =
      new ::morse::MorseSignalDetector(morse_reader, BUFFER_SIZE);
  signal_detector->Verbose(verbose);
  if (!pattern_file_name.empty()) {
    if (signal_detector->SetDumpFile(pattern_file_name) < 0) {
      fprintf(stderr, "File open failed: %s (%s)\n", pattern_file_name.c_str(),
              strerror(errno));
      exit(-1);
    }
  }

  /*
    int row, col;
    initscr();
    getmaxyx(stdscr, row, col);
    curs_set(0);
    WINDOW *local_win = newwin(row - 12, col, 12, 0);
    wprintw(local_win, "AAAAAAA\n");
    wrefresh(local_win);
    */
  auto *monitor = new morse::Monitor();

  // read and process data of approximately 6ms for each in the loop
  bool first = true;
  sf_count_t num_samples;
  do {
    num_samples = sf_read_short(sndfile, current_buffer, BUFFER_SIZE);

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

    if (!first) {
      signal_detector->Process(prev_buffer, current_buffer, num_samples,
                               monitor);
    }
    first = false;
    short *temp = prev_buffer;
    prev_buffer = current_buffer;
    current_buffer = temp;

  } while (num_samples == BUFFER_SIZE);
  monitor->Dump(morse_reader);

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