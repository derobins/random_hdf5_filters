#!/bin/sh
#
# This really isn't necessary, but it makes it obvious that you need to set
# the plugin path in order to find your fancy new filter plugin.
export HDF5_PLUGIN_PATH="."
./filter_test_program
