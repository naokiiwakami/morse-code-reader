# We will build the fft and
# run main which has test cases

read_wav : read_wav.o fft.o morse_reader.o default_decoder.o
	$(CXX) ${LDFLAGS} -o read_wav read_wav.o fft.o morse_reader.o default_decoder.o -lsndfile -lm

%.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -g -Wall -I /home/naoki/local/include -c $< -o $@

clean:
	rm -f *.o read_wav
