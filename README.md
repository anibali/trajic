Trajic
======

Synopsis
--------

Trajic is an algorithm for compressing GPS trajectory data. For a more in-depth
academic explanation of how Trajic works, be sure to read the
[research paper](https://raw.githubusercontent.com/anibali/trajic/gh-pages/trajic_paper.pdf)
I wrote with Dr Zhen He.

This project contains a reference implementation of Trajic along with
implementations of various other GPS trajectory compression schemes which were
used for benchmarks and experiments.

Development dependencies
------------------------

* [libcpptest-dev](http://cpptest.sourceforge.net/)
* [libboost-iostreams-dev](http://www.boost.org/doc/libs/1_54_0/libs/iostreams/doc/index.html)

Experiments also require

* [gnuplot](http://www.gnuplot.info/)

Usage
-----

Compress a trajectory losslessly:

    trajic c traj.plt

Compress a trajectory with max errors of 0.1 s temporally and 0.001 degrees
spatially:

    trajic c traj.plt 0.1 0.001

Decompress a trajectory:

    trajic d traj.tjc

Important classes
-----------------

* @ref PredictiveCompressor
* @ref ibstream
* @ref obstream
* @ref GPSPoint
