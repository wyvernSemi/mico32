#/bin/bash
# -------------------------------------------------------------------
# runtest.sh
#
# Copyright (c) 2013 Simon Southwell. All rights reserved
#
# -------------------------------------------------------------------

#
# Remember the directory we started in
#
startdir=`pwd`

#
# List of test directories to be run. Expecting a test.s file in each one
#
dirlist="instructions/add \
         instructions/sub \
         instructions/cmp_e_ne \
         instructions/cmpg \
         instructions/cmpge \
         instructions/load \
         instructions/store \
         instructions/branch_cond \
         instructions/branch_uncond \
         instructions/and \
         instructions/or \
         instructions/xor \
         instructions/sext \
         instructions/sl \
         instructions/sr \
         instructions/csr \
         instructions/mul \
         instructions/div \
         exceptions/instruction \
         exceptions/external \
         exceptions/ibus_errors \
         exceptions/dbus_errors \
         exceptions/hw_debug \
         api/num_instr \
"

#
# File names involved with the test
#
srcfile=test.s
objfile=test.o
testfile=test.elf
execfile=cpumico32
inifile=test.ini

#
# Test result for a pass
#
passresult="0x0000900d"

# The MSVC built executable put ^M at the end of each line, so
# need to test for this possibility
passresult_win="$passresult"

#
# Statistics variables
#
num_run=0
num_failures=0
num_passed=0

# Cache config
maxsetsize=1024
setsize=128
linesize=4
tmpinifile=tmp.ini

if [ -n "$1" ]
then
    execfile=$1
fi

userargs=""
if [ -n "$CPUMICO32_ARGS" ]
then
  userargs="$CPUMICO32_ARGS"
fi

#
# Loop through each listed test...
#
for lm32_test in $dirlist
do
    echo "Running test " $lm32_test

    # Keep count of the number of tests run
    num_run=$(($num_run + 1))

    # Generate the .ini file with modified cache parameters
    cat $inifile | sed -e "s/cache_num_sets=.*$/cache_num_sets=$setsize/" | \
                   sed -e "s/cache_bytes_per_line=.*$/cache_bytes_per_line=$linesize/" > $lm32_test/$tmpinifile

    # Go to the directory of the current test
    cd $lm32_test

    # Compile the test code
    lm32-elf-as $srcfile -o $objfile
    lm32-elf-ld $objfile -o $testfile

    # Run the test and get the result
    result_str=`$execfile $userargs -i $tmpinifile -T -r 0xfffc -n10000 -f $testfile | tail -1`

    # Extract the relevant field from the result
    result_str=`echo $result_str | cut -d'=' -f2`

    # Process the cache variable. Double the set size until maximum, and then
    # double the line size until maximum. When both at maximum, reset to minimum
    if [ "$setsize" = "1024" ]
    then
	if [ "$linesize" = "16" ]
	then
            setsize=128
	    linesize=4
        else
	    linesize=$((linesize*2))
	fi
    else
        setsize=$(($setsize*2))
    fi

    # Check the result and indicate to stdout
    if [ $result_str = $passresult -o $result_str = $passresult_win ]
    then
        echo "  PASS"
	num_passed=$(($num_passed + 1))
    else
        echo "  *FAIL*"
	num_failures=$(($num_failures + 1))
    fi


    # Clean up temporary files
    rm $objfile $testfile $tmpinifile

    # Return to the main test directory
    cd $startdir

done


echo ""
echo "Tests run : $num_run"
echo "Tests pass: $num_passed"
echo "Tests fail: $num_failures"
