CC = g++-4.9
CFLAGS = -std=c++11 -O3 -Wall
OBJS = ibstream.o obstream.o gps_point.o huffman.o plt_reader.o \
len_freq_div.o linear_predictor.o predictive_compressor.o illinois_reader.o \
util.o csv_reader.o dynamic_encoder.o
STATS_OBJS = delta_compressor.o dp_compressor.o dummy_compressor.o \
squish_compressor.o

default: trajic clean

all: trajic test experiments clean

test: $(OBJS) $(STATS_OBJS)
	$(CC) $(CFLAGS) -Isrc test/test.cpp -lcpptest $(OBJS) $(STATS_OBJS) -o test/test
	test/test

trajic: $(OBJS) main.o
	$(CC) $(CFLAGS) $(OBJS) main.o -o bin/trajic

%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o

doc:
	doxygen

experiments: $(OBJS) $(STATS_OBJS) stats.o
	$(CC) $(CFLAGS) $(OBJS) $(STATS_OBJS) stats.o -o experiments/stats

