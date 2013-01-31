#include <iostream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <typeinfo>
using namespace std;

#include <boost/lexical_cast.hpp>

#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>
using boost::iostreams::basic_array;
using boost::iostreams::stream;

#include "boost/date_time/posix_time/posix_time.hpp"
using boost::posix_time::microsec_clock;
using boost::posix_time::ptime;

#include "util.h"
#include "read_points.h"
#include "predictive_compressor.h"
#include "delta_compressor.h"
#include "dummy_compressor.h"
#include "dp_compressor.h"

void stats(string filename, Compressor *c)
{
  ptime start_time;
  long compr_time, decompr_time;
  
  vector<GPSPoint> points = read_points(filename);
  
  char* buffer = new char[999999]; 
  stream<basic_array<char> > bas(buffer, 999999);
  
  obstream obs(&bas);
  
  start_time = microsec_clock::universal_time();
  c->compress(obs, points);
  compr_time = (microsec_clock::universal_time() -
                start_time).total_microseconds();
  obs.close();
  
  boost::iostreams::seek(bas, 0, std::ios_base::beg);
  ibstream ibs(&bas);
  start_time = microsec_clock::universal_time();
  vector<GPSPoint> new_points = c->decompress(ibs);
  decompr_time = (microsec_clock::universal_time() -
                  start_time).total_microseconds();
  delete[] buffer;
  
  double max_km = -1;
  if(typeid(*c) == typeid(DPCompressor))
  {
    max_km = dynamic_cast<DPCompressor*>(c)->max_error_kms;
  }
  else
  {
    for(int i = 0; i < points.size(); ++i)
    {
      max_km = max(max_km, points[i].distance_kms(new_points[i]));
    }
  }
  
  cout << "raw_size=" << points.size() * 8 * 3 << endl;
  cout << "compr_size=" << boost::iostreams::seek(bas, 0, std::ios_base::cur)
       << endl;
  cout << "max_error_kms=" << setprecision(10) << showpoint << max_km << endl;
  cout << "compr_time=" << compr_time << endl;
  cout << "decompr_time=" << decompr_time << endl;
  
  // Check for "pure" lossless (point for point correspondence)
  bool lossless = true;
  for(int i = 0; lossless and i < points.size(); ++i)
  {
    lossless = points[i].distance(new_points[i]) < 0.000000001;
  }
  cout << "lossless=" << boolalpha << lossless << endl;
}

int main(int argc, char** args)
{
  if(argc < 3)
  {
    cout << "stats [trajic|delta|dp] <infile>"
         << " <max_temporal error=0> <max_spatial_error=0>" << endl;
  }
  else
  {
    string alg = args[1];
    string infile = args[2];
    double mte, mse;
    
    if(argc > 3)
    {
      try
      {
        mte = boost::lexical_cast<double>(args[3]);
      }
      catch(boost::bad_lexical_cast const&)
      {
        mte = 0;
      }
    }
    
    if(argc > 4)
    {
      try
      {
        mse = boost::lexical_cast<double>(args[4]);
      }
      catch(boost::bad_lexical_cast const&)
      {
        mse = 0;
      }
    }
    
    if(alg == "trajic")
    {
      PredictiveCompressor c(mte, mse);
      stats(infile, &c);
    }
    else if(alg == "dp")
    {
      DummyCompressor dc;
      DPCompressor c(&dc, mse);
      stats(infile, &c);
    }
    else if(alg == "dpd")
    {
      DeltaCompressor dc;
      DPCompressor c(&dc, mse);
      stats(infile, &c);
    }
    else if(alg == "delta")
    {
      DeltaCompressor c;
      stats(infile, &c);
    }
    else
    {
      cerr << "Invalid algorithm specified" << endl;
      return 1;
    }
  }
  
  return 0;
}
