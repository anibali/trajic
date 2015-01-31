require 'mkmf'
require 'find'
require 'fileutils'
include FileUtils

# Disable mkmf log files
module Logging
  @logfile = File::NULL
end

def check_for_executables!
  executables_missing = false

  %w[gnuplot].each do |executable|
    executables_missing ||= !find_executable(executable)
  end

  print "checking for Trajic stats binary..."
  stats_binary_exists = File.exists?("./stats")
  executables_missing ||= !stats_binary_exists
  puts stats_binary_exists ? " yes" : " no"

  if executables_missing
    STDERR.puts "[ERROR] Executable(s) missing"
    exit
  end
end
