This is a simple filter template for creating HDF5 filters. You can copy the
files and modify them to quickly create a working filter.

To build, just run ccmake or whatnot, point it at your HDF5 install if it can't
find it, generate, and run 'make' to build the filter plugin and a small
test program which uses the plugin.

To test, just run the runme.sh script. This runs the sample program and also
performs the key step of setting the HDF5 plugin path, which does not include
'.' by default.

The dummy filter literally does nothing to the data.

The test program simply creates a file + dataset using the filter and then
writes integer data to it and reads it back. For non-lossy filters, you won't
need to update this program to perform a basic test of your filter.
