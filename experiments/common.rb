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

DATA_BASE_PATH = "/home/aiden/Data/Trajectories/"

def data_files path, extension, number=:all
  absolute_path = DATA_BASE_PATH + path
  files = []
  Find.find(absolute_path) do |path|
    if FileTest.file? path and path.end_with? ".#{extension}"
      files << path
      break if number.is_a? Fixnum and files.size >= number
    end
  end
  files
end
