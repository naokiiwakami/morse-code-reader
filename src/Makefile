PROGRAM = read_morse

OBJECT_FILES = \
	$(PROGRAM).o \
	default_decoder.o \
	fft.o \
	morse_reader.o \
	morse_signal_detector.o

$(PROGRAM) : $(OBJECT_FILES)
	$(CXX) ${LDFLAGS} -o $(PROGRAM) $(OBJECT_FILES) -lsndfile -lm -lpulse -lpulse-simple

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

clean:
	rm -f *.o $(PROGRAM)
