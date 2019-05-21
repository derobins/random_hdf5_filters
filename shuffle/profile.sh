#!/bin/bash
#

# Set up the plugin path
export HDF5_PLUGIN_PATH="."

# Make sure we have a fresh build
make clean
make

echo "Times are REAL,USER,SYS in seconds"

# Loop over shuffle filters
for shuffle_filter_id in 0 1 315 316
do
    # Set the gzip level
    gzip_level=0

    # Run the test program multiple times (we'll discard the first)
    for i in {1..21}
    do
        # Profile the program
        #
        # The I/O should dominate, so I'm just timing the entire program
        # instead of instrumenting the I/O calls.
        #
        # Since we're using bash, we need the full path to time to
        # avoid the built-in.
        /usr/bin/time -f "%e,%U,%S" ./shuffle_test_program $shuffle_filter_id $gzip_level
    done

    echo
    echo
done
