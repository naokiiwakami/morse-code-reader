# Morse Code Reader

This is experimental implementation of morse code reader.
The program, `read_morse` takes a PCM sound file (e.g. wav),
finds morse code, and decodes it.

## Dependencies
Following components are necessary to build this project:
- libsndfile v1.0.28 http://www.mega-nerd.com/libsndfile/
- pulseaudio library (libpulse, libpulse-simple)

## Getting Started
```
cd src
make
./read_morse <some_wav_file_containing_morse_signal>
```