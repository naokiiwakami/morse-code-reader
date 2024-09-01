PROGRAM = read_morse

OBJECT_FILES = \
	$(PROGRAM).o \
	fft.o \
	monitor.o \
	morse_reader.o \
	morse_signal_detector.o \
	world_line.o

LIBS = -lsndfile -lm -lpulse -lpulse-simple -lncurses

$(PROGRAM) : $(OBJECT_FILES)
	$(CXX) ${LDFLAGS} -o $(PROGRAM) $(OBJECT_FILES) $(LIBS)

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

clean:
	rm -f *.o $(PROGRAM)
