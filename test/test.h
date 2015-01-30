#include "gps_point.h"
#include "huffman.h"
#include "len_freq_div.h"
#include "delta_compressor.h"
#include "squish_compressor.h"
#include "dynamic_encoder.h"

#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
using namespace std;

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
using boost::iostreams::basic_array;
using boost::iostreams::basic_array_source;
using boost::iostreams::stream;

#include <cpptest.h>

class IbstreamTestSuite : public Test::Suite
{
public:
  IbstreamTestSuite()
  {
    TEST_ADD(IbstreamTestSuite::test_read_functions)
  }

private:
  void test_read_functions();
};

class ObstreamTestSuite : public Test::Suite
{
public:
  ObstreamTestSuite()
  {
    TEST_ADD(ObstreamTestSuite::test_write_double)
    TEST_ADD(ObstreamTestSuite::test_write_int)
  }

private:
  void test_write_double();
  void test_write_int();
};

class GPSPointTestSuite : public Test::Suite
{
public:
  GPSPointTestSuite()
  {
    TEST_ADD(GPSPointTestSuite::test_get_time)
    TEST_ADD(GPSPointTestSuite::test_distance)
  }

private:
  void test_get_time();
  void test_distance();
};

class HuffmanTestSuite : public Test::Suite
{
public:
  HuffmanTestSuite()
  {
    TEST_ADD(HuffmanTestSuite::test_node)
    TEST_ADD(HuffmanTestSuite::test_codebook)
    TEST_ADD(HuffmanTestSuite::test_create_codewords)
  }

private:
  void test_node();
  void test_codebook();
  void test_create_codewords();
};

class LenFreqDivTestSuite : public Test::Suite
{
public:
  LenFreqDivTestSuite()
  {
    TEST_ADD(LenFreqDivTestSuite::test_len_freq_div)
  }

private:
  void test_len_freq_div();
};

class DeltaCompressorTestSuite : public Test::Suite
{
public:
  DeltaCompressorTestSuite()
  {
    TEST_ADD(DeltaCompressorTestSuite::test_all)
  }

private:
  void test_all();
};

class SquishCompressorTestSuite : public Test::Suite
{
public:
  SquishCompressorTestSuite()
  {
    TEST_ADD(SquishCompressorTestSuite::test_all)
  }

private:
  void test_all();
};

class EncoderTestSuite : public Test::Suite
{
public:
  EncoderTestSuite()
  {
    TEST_ADD(EncoderTestSuite::test_dynamic)
  }

private:
  void test_dynamic();
};

