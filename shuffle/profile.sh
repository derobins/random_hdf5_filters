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
    # Loop over gzip compression levels
    gzip_level=0
#    for gzip_level in 0 5
#    do
        # Run the test program seven times (we'll discard the first)
        for i in 1 2 3 4 5 6 7
        do
            # Delete the file every time
            rm -rf *.h5

            # Profile the program
            # The I/O should dominate, so I'm just timing the entire program
            # instead of instrumenting the I/O calls.
            time -f "%e,%U,%S" ./shuffle_test_program $shuffle_filter_id $gzip_level
        done

        echo
        echo
#    done
done
