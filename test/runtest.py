#!/usr/bin/env python
# -------------------------------------------------------------------
# runtest.py
#
# Copyright (c) 2017 Simon Southwell. All rights reserved
#
# -------------------------------------------------------------------
import os
import subprocess
import argparse
import platform

# --------------------------------------------------------------
# Utility to take an environment variable name, and return a 
# system variable format, applicable to the OS on which the 
# script is being run.
#
# Args:
#  var_str    : String containing the name of the environment variable
#  is_windows : Flag indicating whether running in windows or not
#
def envVar (var_str, is_windows=False) :
  if is_windows :
    return '%' + var_str + '%'
  else :
    return '$' + var_str  

# --------------------------------------------------------------
# Parse the command line arguments
#
def processCmdLine () :

  # Create a parser object
  parser = argparse.ArgumentParser(description='Process command line options.')
  
  # Command line options added here
  parser.add_argument('-p', '--print_only', dest='printOnly', default=False,       action='store_true', help='Print commands only---don\'t execute')
  parser.add_argument('-e', '--execFile',   dest='execFile',  default='cpumico32', action='store',      help='Execution file')
  parser.add_argument('-s', '--sim_tests',  dest='simTests',  default=False,       action='store_true', help='Run simulation tests')
  parser.add_argument('-H', '--hw_tests',   dest='hwTests',   default=False,       action='store_true', help='Run H/W platform tests')
  
  # Examples of options with other types of arguments
  # parser.add_argument('-t', '--test',       dest='testVar',   default='UNSET', action='store',  help='Test argument')
  # parser.add_argument('-a', '--add_test',   dest='testList',  default=[],      action='append', help=argparse.SUPPRESS)
  # parser.add_argument('-f', '--func_names', dest='funcNames', nargs='+',                        help='List of funcs')

  
  # Parse arguments on command line and return namespace containing option values
  return parser.parse_args()

# --------------------------------------------------------------
# Run a command in a shell using subprocess
#
# Args:
#  cmd_str    : String containing command to be run in OS
#  curr_dir   : String indicating directory from which command is to be run
#  print_only : Flag indicating to print commands rather than execute
#  cd         : Flag indicating, if printing, whether to output a cd command
#  background : Flag indicating to run command in the background
#
def shellCmd (cmd_str, curr_dir = None, print_only=False, cd=True, background=False) :

  rtn_status = 0
  useShell   = True  # subprocess commands default to using shell, in case a built-in is used
  
  # If only printing...
  if print_only :
  
    # If required to print a 'change directory' command, and not the default...
    if cd and curr_dir != None :
      print ('cd ' + curr_dir)
    # Print the command
    print (cmd_str)
	
  else :
  
    # If requested to run in the background, use Popen without assigning its output
    # to anything. 
    if background :
      subprocess.Popen(cmd_str, shell=useShell, cwd=curr_dir)
    # Run subprocess.call() and wait for a return status
    else :
      rtn_status = subprocess.check_output(cmd_str, shell=useShell, cwd=curr_dir)
    return rtn_status
    

# --------------------------------------------------------------
# Run the test on the model

def runTestsModel(execfile, userargs, inifile, tmpinifile, setsize, linesize, testfile, lm32_test, printonly) :

  shellCmd('cat ' + inifile + '| sed -e "s/cache_num_sets=.*$/cache_num_sets=' + str(setsize) + '/" | ' +
                                'sed -e "s/cache_bytes_per_line=.*$/cache_bytes_per_line=' +  str(linesize) + '/" > ' +
                                 lm32_test + '/' + tmpinifile)
                                 
  # Run the test and get the result
  result_str = shellCmd(execfile + ' ' + userargs + ' -i ' + tmpinifile + ' -T -r 0xfffc -n10000 -f ' + testfile, 
                        lm32_test, printonly, False)
  
  # Extract the relevant field from the result
  result_str = result_str.decode('UTF-8')
  result_str = result_str.split('=')
  
  return int(result_str[1], 16)

# --------------------------------------------------------------
# Run tests on simulator

def runTestsSim (lm32_test, is_windows, printonly) :

  hdl_test_dir = '../HDL/test'
  
  systype = platform.system().lower()
  if systype.find('cygwin') != -1:
    is_cygwin = True
  else :
    is_cygwin = False

  # Generate test .hex file
  if is_windows :
    # This assumes that Quartus II is installed (likely if using Altera
    # Version of ModelSim) and QUARTUS_ROOTDIR defined. If not, point
    # to another version of objcopy 
    if is_cygwin :
      OBJCOPY = 'objcopy'
    else :
      OBJCOPY = os.path.abspath(os.environ['QUARTUS_ROOTDIR'] + '/bin/cygwin/bin/objcopy')
    PLI = './pli/lib/de1_pli.dll'
  else :
    OBJCOPY = 'objcopy' 
    PLI = './pli/lib/de1_pli.so'

  # Create a verilog hex dump from the .elf file, and place in the HDL test dir
  shellCmd(OBJCOPY + ' -O verilog test.elf ../../' + hdl_test_dir + '/test_elf.hex', lm32_test, printonly)
  
  # Run the simulation
  result_str = shellCmd('vsim -c -pli ' + PLI + ' test -suppress 3017,3722 +FINISH=1 +DISABLE_GUI=1 -quiet -do "run -all"', 
                         hdl_test_dir, printonly, False)
  
  result_str = (result_str.decode('UTF-8')).split('\n')
  result_str = [ s for s in result_str if any(xs in s for xs in ['RAM'])]
  result_str = result_str[0].split('=')
  
  return int(result_str[1], 16)

# --------------------------------------------------------------
# Run tests on hardware

def runTestsHw (lm32_test, test_file, is_windows, printonly) :

  # Location of the driver
  driver_dir = '../../../HDL/driver'

  if is_windows :
    driver = os.path.relpath(driver_dir + '/msvc/Release/lm32_driver.exe')
  else :
    driver = os.path.relpath(driver_dir + '/lm32_driver')

  result_str = shellCmd(driver + ' -f ' + test_file + ' -d 0', lm32_test, printonly)
  
  result_str = (result_str.decode('UTF-8')).split('\n')
  result_str = [ s for s in result_str if any(xs in s for xs in ['RAM'])]
  result_str = result_str[0].split('=')
  
  return int(result_str[1], 16)
  
# --------------------------------------------------------------
# Run all the tests

def runtest() :

  # Determine whether on windows or not, as this will make some differences
  if platform.system() == 'Linux' :
    is_windows = False 
  else :
    is_windows = True

  # Process the command line options	
  args = processCmdLine()

  #
  # Remember the directory we started in
  #
  startdir=os.getcwd()

  #
  # List of test directories to be run, common to all tests. Expecting a 
  # test.s file in each one
  #
  dirlist = [
             'instructions/add', 
             'instructions/sub',
             'instructions/cmp_e_ne',
             'instructions/cmpg',
             'instructions/cmpge',
             'instructions/load',
             'instructions/store',
             'instructions/branch_cond',
             'instructions/branch_uncond',
             'instructions/and',
             'instructions/or',
             'instructions/xor',
             'instructions/sext',
             'instructions/sl',
             'instructions/sr',
             'instructions/mul',
             'instructions/div',
             'exceptions/instruction',
             'mmu/tlb',
            ] 
 
  # List of tests just for the C model
  mdllist = ['instructions/csr',
             'exceptions/external',
             'exceptions/ibus_errors',
             'exceptions/dbus_errors',
             'exceptions/hw_debug',
             'api/num_instr']

  # If the C model is to be run (and not the simulation or platform), add the model specific tests
  if not args.simTests and not args.hwTests:
    dirlist += mdllist

  #
  # File names involved with the test
  #
  srcfile  = 'test.s'
  objfile  = 'test.o'
  testfile = 'test.elf'
  inifile  = 'test.ini'

  #
  # Test result for a pass
  #
  passresult = 0x900d

  #
  # Statistics variables
  #
  num_run      = 0
  num_failures = 0
  num_passed   = 0

  # Cache config
  maxsetsize = 1024
  setsize    = 128
  linesize   = 4
  tmpinifile = 'tmp.ini'

  # Get the exec path (relative to a test dir) and normalise it to the local OS convention.
  execfile = os.path.relpath(args.execFile)

  userargs = ''
  
  if 'CPUMICO32_ARGS' in os.environ :
    userargs = os.environ['CPUMICO32_ARGS"']

  #
  # Loop through each listed test...
  #
  for lm32_test in dirlist :
  
    print ('Running test ' + lm32_test)

    # Keep count of the number of tests run
    num_run = num_run + 1

    # Compile the test code
    shellCmd('lm32-elf-cpp ' + srcfile + ' ' + srcfile + '.tmp',  lm32_test, args.printOnly, True)
    shellCmd('lm32-elf-as '  + srcfile + '.tmp -o ' + objfile,  lm32_test, args.printOnly, True)
    shellCmd('lm32-elf-ld '  + objfile + ' -o ' + testfile, lm32_test, args.printOnly, False)
    
    # Run the test and get the result
    if args.simTests :
      result = runTestsSim(lm32_test, is_windows, args.printOnly)
    elif  args.hwTests :
      result = runTestsHw (lm32_test, testfile, is_windows, args.printOnly)
    else :
      result = runTestsModel(execfile, userargs, inifile, tmpinifile, setsize, linesize, testfile, lm32_test, args.printOnly)
 
    # Process the cache variable. Double the set size until maximum, and then
    # double the line size until maximum. When both at maximum, reset to minimum
    if setsize == 1024 :
      if linesize == 16 :
        setsize  = 128
        linesize = 4
      else :
        linesize *= 2
    else :
      setsize *= 2    
    
    # Check the result and indicate to stdout
    if result == passresult :
      print ('  PASS')
      num_passed += 1
    else :
      print ('  FAIL')
      num_failures += 1
    
    # Clean up temporary files
    os.chdir(lm32_test)
    os.remove(objfile)
    os.remove(testfile)
    #os.remove(tmpinifile)
    os.chdir(startdir)
    
  # Print out test summary
  print ('')
  print ('Tests run : ' + str(num_run))
  print ('Tests pass: ' + str(num_passed))
  print ('Tests fail: ' + str(num_failures))

# --------------------------------------------------------------
# Entry point if run from command line (i.e. not imported)
#
def __main () :

  runtest()
  
  
# ###############################################################
# Only run __main() if not imported
#
if __name__ == "__main__" :
  __main()
