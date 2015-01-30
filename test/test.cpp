#include "test.h"

void IbstreamTestSuite::test_read_functions()
{
  char data[] = {0x41, (char)0x0F};
  stream<basic_array_source<char>> buffer(data, sizeof(data));
  ibstream ibs(&buffer);
  TEST_ASSERT(ibs.read_byte() == 'A');
  TEST_ASSERT(ibs.read_int(4) == 15);
  TEST_ASSERT(ibs.read_bit() == false);
}

void ObstreamTestSuite::test_write_double()
{
  char buffer[128] = {0};
  stream<basic_array<char> > bas(buffer, 128);
  obstream obs(&bas);

  // This double has a binary representation of
  // 0x4141414141414141 (note that 0x41 corresponds
  // to the ASCII character 'A').
  obs.write_double(2.2616345098039215e+06);
  obs.close();
  for(int i = 0; i < 8; ++i)
    TEST_ASSERT(buffer[i] == 'A');
}

void ObstreamTestSuite::test_write_int()
{
  char buffer[128] = {0};
  stream<basic_array<char> > bas(buffer, 128);
  obstream obs(&bas);

  obs.write_int(0x41, 7);
  obs.close();
  TEST_ASSERT(buffer[0] == 'A');
}

void GPSPointTestSuite::test_get_time()
{
  GPSPoint point(123, 50.7, 63.7);
  TEST_ASSERT(point.get_time() == 123);
}

void GPSPointTestSuite::test_distance()
{
  GPSPoint p1(123, 50.7, 63.7);
  GPSPoint p2(234, 53.7, 67.7);

  TEST_ASSERT_DELTA(p1.distance(p2), 5, 0.0000001);
}

void HuffmanTestSuite::test_node()
{
  char c = 'X';
  Huffman::Node<char> n1(c, 0.4);
  Huffman::Node<char> n2(c, 0);
  Huffman::Node<char> n3(c, 0.2);
  Huffman::Node<char> n4(c, 0.3);
  n2.left = &n3;
  n2.right = &n4;
  TEST_ASSERT(n1 < n2)
}

void HuffmanTestSuite::test_codebook()
{
  vector<char> alphabet;
  alphabet.push_back('A');
  alphabet.push_back('N');
  alphabet.push_back('T');
  char data[] = {0x02, (char)0xE9, (char)0xD2};
  stream<basic_array_source<char>> buffer(data, sizeof(data));
  ibstream ibs(&buffer);
  Huffman::Codebook<char> cb(alphabet, ibs);
  string msg = "";
  for(int i = 0; i < 6; ++i)
    msg += cb.lookup(ibs);
  TEST_ASSERT(msg == "TANANT")
}

void HuffmanTestSuite::test_create_codewords()
{
  double freqs[] = {0.3, 0.6, 0.1};
  vector<string> codewords = Huffman::create_codewords(freqs, 3);
  TEST_ASSERT(codewords[0] == "10")
  TEST_ASSERT(codewords[1] == "0")
  TEST_ASSERT(codewords[2] == "11")
}

void LenFreqDivTestSuite::test_len_freq_div()
{
  double freqs[] = {0, 0.1, 0.6, 0.3, 0};
  LengthFrequencyDivider inst(freqs, 5, 5);
  inst.calculate();

  int expected[] = {2, 3};
  int actual[2];
  inst.get_dividers(actual, 2);
  bool same = true;
  for(int i = 0; same and i < 2; ++i)
    if(expected[i] != actual[i]) same = false;
  TEST_ASSERT(same)

  TEST_ASSERT(inst.get_cost(2) == 1.1)
  TEST_ASSERT(inst.get_cost(4) == 2)
}

void DeltaCompressorTestSuite::test_all()
{
  DeltaCompressor c;

  vector<GPSPoint> points;
  points.push_back(GPSPoint(1, 100, 200));
  points.push_back(GPSPoint(2, 103, 207));
  points.push_back(GPSPoint(2, 108, 206));

  char buffer[128] = {0};
  stream<basic_array<char> > bas(buffer, 128);
  obstream obs(&bas);
  c.compress(obs, points);
  obs.close();

  boost::iostreams::seek(bas, 0, std::ios_base::beg);
  ibstream ibs(&bas);
  vector<GPSPoint> new_points = c.decompress(ibs);

  for(int i = 0; i < points.size(); ++i)
    TEST_ASSERT(points[i].distance(new_points[i]) < 0.00000001);
}

void SquishCompressorTestSuite::test_all()
{
  SquishCompressor c(0.5);

  vector<GPSPoint> points;
  points.push_back(GPSPoint(1, 0, 0));
  points.push_back(GPSPoint(2, 1, 1));
  points.push_back(GPSPoint(3, 2, 2));
  points.push_back(GPSPoint(4, 3, 3));
  points.push_back(GPSPoint(5, 2, 2));
  points.push_back(GPSPoint(6, 1, 1));

  char buffer[1024] = {0};
  stream<basic_array<char> > bas(buffer, 1024);
  obstream obs(&bas);
  c.compress(obs, points);
  obs.close();

  boost::iostreams::seek(bas, 0, std::ios_base::beg);
  ibstream ibs(&bas);
  vector<GPSPoint> new_points = c.decompress(ibs);

  TEST_ASSERT(new_points.size() == 3);
  TEST_ASSERT(points[0].distance(new_points[0]) < 0.00000001);
  TEST_ASSERT(points[3].distance(new_points[1]) < 0.00000001);
  TEST_ASSERT(points[5].distance(new_points[2]) < 0.00000001);
}

void EncoderTestSuite::test_dynamic()
{
  uint64_t nums[] = {2, 1, 5, 67, 68, 4, 3, 2, 1, 73, 0, 2};

  char buffer[128] = {0};
  stream<basic_array<char> > bas(buffer, 128);
  obstream obs(&bas);

  DynamicEncoder enc(obs, nums, 12);
  for(int i = 0; i < 12; ++i)
    enc.encode(obs, nums[i]);

  obs.close();

  boost::iostreams::seek(bas, 0, std::ios_base::beg);
  ibstream ibs(&bas);

  DynamicEncoder dec(ibs);

  for(int i = 0; i < 12; ++i)
  {
    uint64_t num = dec.decode(ibs);
    TEST_ASSERT(num == nums[i]);
  }

}

int main()
{
  Test::Suite ts;
  ts.add(auto_ptr<Test::Suite>(new IbstreamTestSuite));
  ts.add(auto_ptr<Test::Suite>(new ObstreamTestSuite));
  ts.add(auto_ptr<Test::Suite>(new GPSPointTestSuite));
  ts.add(auto_ptr<Test::Suite>(new HuffmanTestSuite));
  ts.add(auto_ptr<Test::Suite>(new LenFreqDivTestSuite));
  ts.add(auto_ptr<Test::Suite>(new DeltaCompressorTestSuite));
  ts.add(auto_ptr<Test::Suite>(new SquishCompressorTestSuite));
  ts.add(auto_ptr<Test::Suite>(new EncoderTestSuite));

  Test::TextOutput output(Test::TextOutput::Terse);
  ts.run(output);

  return 0;
}

