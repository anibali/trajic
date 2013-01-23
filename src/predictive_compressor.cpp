#include "predictive_compressor.h"
#include "len_freq_div.h"
#include "huffman.h"

#include <cmath>
#include <algorithm>
using namespace std;

Predictor* default_predictor = new LinearPredictor();

int PredictiveCompressor::calculate_discarded_bits(double max_value,
  double error_bound)
{
  // Get exponent field from maxValue
  long x = (bits64{dbl: max_value}.lng >> 52) & 0x7ff;
  // Calculate number of bits to discard
  return min((int)log2(error_bound * pow(2, 1075 - x) + 1), 52);
}

void PredictiveCompressor::compress(obstream& obs, vector<GPSPoint> points)
{
  double max_time = 0, max_coord = 0;
  for(GPSPoint point : points)
  {
    max_time = max(max_time, abs(point.get_time()));
    max_coord = max(max_coord, abs(point.get_latitude()));
    max_coord = max(max_coord, abs(point.get_longitude()));
  }
  
  int discard[3];
  discard[0] = calculate_discarded_bits(max_time, max_temporal_err);
  discard[1] = calculate_discarded_bits(max_coord, max_spatial_err);
  discard[2] = discard[1];
  
  obs.write_int(discard[0], 8);
  obs.write_int(discard[1], 8);
  
  obs.write_int(points.size(), 32);
  
  obs.write_double(points[0].get_time());
  obs.write_double(points[0].get_latitude());
  obs.write_double(points[0].get_longitude());
  
  bits64 tuples[points.size()][3];
  for(int i = 0; i < points.size(); ++i)
  {
    tuples[i][0].dbl = points[i].get_time();
    tuples[i][1].dbl = points[i].get_latitude();
    tuples[i][2].dbl = points[i].get_longitude();
  }
  
  uint64_t residuals[points.size() - 1][3];
  for(int i = 0; i < points.size() - 1; ++i)
  {
    if(discard[0] > 0)
    {
      uint64_t pred_time = predictor->predict_time(tuples, i + 1).lng;
      uint64_t residual = tuples[i + 1][0].lng ^ pred_time;
      residual >>= discard[0];
      residual <<= discard[0];
      residuals[i][0] = residual;
      tuples[i + 1][0].lng = pred_time ^ residual;
    }
    
    bits64 pred[3];
    predictor->predict_coords(tuples, i + 1, pred);
    
    for(int j = 0; j < 3; ++j)
    {
      uint64_t residual = tuples[i + 1][j].lng ^ pred[j].lng;
      residual = (residual >> discard[j]) << discard[j];
      residuals[i][j] = residual;
      tuples[i + 1][j].lng = pred[j].lng ^ residual;
    }
  }

  encode_residuals(residuals, points.size() - 1, obs, discard);
}

void PredictiveCompressor::encode_residuals(uint64_t residuals[][3],
  int n_residuals, obstream& obs, int* discard)
{
  double freqs[3][65];
  int n_freqs[3];
  for(int i = 0; i < 3; ++i)
  {
    n_freqs[i] = 65 - discard[i];
    for(int j = 0; j < n_freqs[i]; ++j)
      freqs[i][j] = 0;
  }
  double step = 1.0 / n_residuals;
  
  for(int i = 0; i < n_residuals; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      int min_len = 0;
      if(residuals[i][j] > 0)
        min_len = (int)log2(residuals[i][j]) + 1 - discard[j];
      freqs[j][min_len] += step;
    }
  }
  
  Huffman::Codebook<int>* codebooks[3];
  for(int i = 0; i < 3; ++i)
  {
    // Predict the number of dividers beyond which there will be no compression
    // gain.
    int max_divs = 0;
    for(int j = 0; j < n_freqs[i]; ++j)
    {
      if(freqs[i][j] > 0.02) max_divs += 1;
    }
    if(max_divs < 4) max_divs = 4;
    if(max_divs > 32) max_divs = 32;
  
    int dividers[max_divs];
    double min_cost = numeric_limits<double>::max();
    LengthFrequencyDivider lfd(freqs[i], n_freqs[i], max_divs);
    lfd.calculate();
    
    int n_dividers = max_divs;
    
    for(int n_codewords = 2; n_codewords <= max_divs; ++n_codewords)
    {
      double cost = lfd.get_cost(n_codewords) +
        7.0 * n_codewords / n_residuals;
      if(cost < min_cost)
      {
         min_cost = cost;
         n_dividers = n_codewords;
         lfd.get_dividers(dividers, n_dividers);
      }
      else
      {
        // Breaking seems to work here, but can't prove why.
        // break;
      }
    }
    
    double clumped_freqs[n_dividers];
    int b = 0;
    for(int j = 0; j < n_freqs[i] and b < n_dividers; ++j)
    {
      clumped_freqs[b] += freqs[i][j];
      if(j == dividers[b]) ++b;
    }
    
    vector<int> div_vec;
    div_vec.assign(dividers, dividers + n_dividers);
    codebooks[i] = new Huffman::Codebook<int>(div_vec,
      Huffman::create_codewords(clumped_freqs, n_dividers));
  }
  
  // Write out alphabets and codebooks
  for(int j = 0; j < 3; ++j)
  {
     obs.write_int(codebooks[j]->get_alphabet().size(), 8);
     for(int symbol : codebooks[j]->get_alphabet())
        obs.write_int(symbol, 8);
     codebooks[j]->encode(obs);
  }
  
  // Write residuals
  for(int i = 0; i < n_residuals; ++i)
  {
    for(int j = 0; j < 3; ++j)
    {
      vector<int> dividers = codebooks[j]->get_alphabet();
      int min_len = 0;
      if(residuals[i][j] > 0)
        min_len = (int)log2(residuals[i][j]) + 1 - discard[j];
      
      int index = 0;
      while(dividers[index] < min_len)
        ++index;
      for(char c : codebooks[j]->get_codewords()[index])
        obs.write_bit(c != '0');
      obs.write_int(residuals[i][j] >> discard[j], dividers[index]);
    }
  }
  
  for(int j = 0; j < 3; ++j) delete codebooks[j];
}

vector<GPSPoint> PredictiveCompressor::decompress(ibstream& ibs)
{
  int discard[3];
  discard[0] = ibs.read_byte();
  discard[1] = ibs.read_byte();
  discard[2] = discard[1];
  
  int n_points = ibs.read_int(32);
  bits64 tuples[n_points][3];
  
  for(int i = 0; i < 3; ++i)
    tuples[0][i].lng = ibs.read_int(64);
  
  Huffman::Codebook<int>* codebooks[3];
  for(int j = 0; j < 3; ++j)
  {
    int alphabet_len = ibs.read_byte();
    vector<int> alphabet(alphabet_len);
    for(int i = 0; i < alphabet_len; ++i)
    {
      alphabet[i] = ibs.read_byte();
    }
    codebooks[j] = new Huffman::Codebook<int>(alphabet, ibs);
  }
  
  for(int i = 1; i < n_points; ++i)
  {
    uint64_t residuals[3];
    
    for(int j = 0; j < 3; ++j)
    {
      int residual_len = codebooks[j]->lookup(ibs);
      residuals[j] = ibs.read_int(residual_len) << discard[j];
    }
    
    uint64_t time = predictor->predict_time(tuples, i).lng ^ residuals[0];
    tuples[i][0].lng = time;
    bits64 pred[3];
    predictor->predict_coords(tuples, i, pred);

    for(int j = 0; j < 3; ++j)
      tuples[i][j].lng = pred[j].lng ^ residuals[j];
  }
  
  vector<GPSPoint> points;
  
  for(int i = 0; i < n_points; ++i)
  {
    GPSPoint point(tuples[i][0].dbl, tuples[i][1].dbl, tuples[i][2].dbl);
    points.push_back(point);
  }
  
  for(int j = 0; j < 3; ++j) delete codebooks[j];
  
  return points;
}

