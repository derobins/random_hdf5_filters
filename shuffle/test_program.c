/* filter_test_program.c
 *
 * This is test program for the do-nothing filter for HDF5.
 *
 * Copy the files into another directory when you start a new filter.
 *
 *
 * Copyright (C) 2019 Dana Robinson <dana.e.robinson@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <hdf5.h>

#include "shuffle.h"

/* Names */
#define TEST_FILE_NAME  "test_file.h5"
#define DSET_NAME       "filtered data"

/* Dataset and chunk sizes
 * Note that the sizes are in elements, not bytes
 */
#define NDIMS           1                       /* 1-dimensional */
#define DSET_DIMS       (5 * 1024 * 1024)       /* 20 MiB w/ 32-bit ints */
#define CHUNK_DIMS      (128 * 1024)            /* 512 KiB w/ 32-bit ints */

/* I/O size */
#define ELEMS_PER_IO    (64 * 1024)             /* 1/2 chunk to force partial chunk writes */

/* Some error macros */
#define PRINT_ERROR_MSG         do {fprintf(stderr, "***ERROR*** at line %d...\n", __LINE__);} while (0)
#define HDF5_ERROR              do {PRINT_ERROR_MSG; goto error;} while (0)
#define PROGRAM_ERROR(s)        do {PRINT_ERROR_MSG; fprintf(stderr, ": %s\n", (s)); goto error;} while (0)


int
create_file(void)
{
    hid_t fid       = H5I_INVALID_HID;
    hid_t sid       = H5I_INVALID_HID;
    hid_t dcpl_id   = H5I_INVALID_HID;
    hid_t did       = H5I_INVALID_HID;
    hsize_t dset_dims   = DSET_DIMS;
    hsize_t chunk_dims  = CHUNK_DIMS;

    /* Create the test file */
    if (H5I_INVALID_HID == (fid = H5Fcreate(TEST_FILE_NAME, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Create a simple dataspace to describe the dataset's size */
    if (H5I_INVALID_HID == (sid = H5Screate_simple(NDIMS, &dset_dims, NULL)))
        HDF5_ERROR;

    /* Create a dataset creation property list and turn chunking on */
    if (H5I_INVALID_HID == (dcpl_id = H5Pcreate(H5P_DATASET_CREATE)))
        HDF5_ERROR;
    if (H5Pset_chunk(dcpl_id, NDIMS, &chunk_dims) < 0)
        HDF5_ERROR;

    /* Set the custom filter */
    if (H5Pset_filter(dcpl_id, SHUFFLE_ID, H5Z_FLAG_MANDATORY, 0, NULL))
        HDF5_ERROR;

    /* Create the dataset (in the root group).
     *
     * The datatype is 32-bit little-endian integer, which will conveniently
     * allow the use of simple int buffers on LP64 machines without type
     * conversion.
     *
     * Always be explicit about your types when creating datasets. Don't
     * use the NATIVE type aliases. Those are best used for reads and writes.
     */
    if (H5I_INVALID_HID == (did = H5Dcreate(fid, DSET_NAME, H5T_STD_I32LE, sid, H5P_DEFAULT, dcpl_id, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Close everything */
    if (H5Fclose(fid) < 0)
        HDF5_ERROR;
    if (H5Sclose(sid) < 0)
        HDF5_ERROR;
    if (H5Pclose(dcpl_id) < 0)
        HDF5_ERROR;
    if (H5Dclose(did) < 0)
        HDF5_ERROR;

    return 0;

error:
    /* Error case clean up */
    H5E_BEGIN_TRY {
        H5Fclose(fid);
        H5Sclose(sid);
        H5Pclose(dcpl_id);
        H5Dclose(did);
    } H5E_END_TRY;

    return -1;
} /* end create_file() */

int
write_to_file(void)
{
    hid_t fid       = H5I_INVALID_HID;
    hid_t msid      = H5I_INVALID_HID;
    hid_t fsid      = H5I_INVALID_HID;
    hid_t did       = H5I_INVALID_HID;
    int *buf        = NULL;
    hsize_t msid_dims       = ELEMS_PER_IO;
    hsize_t n_elems_written = 0;
    int i;

    /* Open the test file */
    if (H5I_INVALID_HID == (fid = H5Fopen(TEST_FILE_NAME, H5F_ACC_RDWR, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Open the dataset */
    if (H5I_INVALID_HID == (did = H5Dopen(fid, DSET_NAME, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Set up a dataspace to represent the in-memory data */
    if (H5I_INVALID_HID == (msid = H5Screate_simple(NDIMS, &msid_dims, NULL)))
        HDF5_ERROR;

    /* Set up a dataspace to represent a subset of the dataset */
    if (H5I_INVALID_HID == (fsid = H5Dget_space(did)))
        HDF5_ERROR;

    /* Allocate the buffer and fill it with something we can check later */
    if (NULL == (buf = (int *)calloc(ELEMS_PER_IO, sizeof(int))))
        PROGRAM_ERROR("memory allocation for buf failed");
    for (i = 0; i < ELEMS_PER_IO; i++)
        buf[i] = i;

    /* Write data to the file */
    while (n_elems_written < DSET_DIMS) {
        hsize_t start   = n_elems_written;
        hsize_t stride  = 1;
        hsize_t count   = 1;
        hsize_t block   = ELEMS_PER_IO;

        /* Adjust the dataset dataspace's hyperslab */
        if (H5Sselect_hyperslab(fsid, H5S_SELECT_SET, &start, &stride, &count, &block) < 0)
            HDF5_ERROR;

        /* Write the data */
        if (H5Dwrite(did, H5T_NATIVE_INT, msid, fsid, H5P_DEFAULT, buf) < 0)
            HDF5_ERROR;

        /* Update the count */
        n_elems_written += ELEMS_PER_IO;
    }

    /* Close everything */
    if (H5Fclose(fid) < 0)
        HDF5_ERROR;
    if (H5Sclose(msid) < 0)
        HDF5_ERROR;
    if (H5Sclose(fsid) < 0)
        HDF5_ERROR;
    if (H5Dclose(did) < 0)
        HDF5_ERROR;
    free(buf);

    return 0;

error:
    /* Error case clean up */
    H5E_BEGIN_TRY {
        H5Fclose(fid);
        H5Sclose(msid);
        H5Sclose(fsid);
        H5Dclose(did);
    } H5E_END_TRY;

    free(buf);

    return -1;
} /* end write_to_file() */

int
read_from_file(void)
{
    hid_t fid       = H5I_INVALID_HID;
    hid_t msid      = H5I_INVALID_HID;
    hid_t fsid      = H5I_INVALID_HID;
    hid_t did       = H5I_INVALID_HID;
    int *buf        = NULL;
    hsize_t msid_dims       = ELEMS_PER_IO;
    hsize_t n_elems_read    = 0;
    int i;

    /* Open the test file (read-only) */
    if (H5I_INVALID_HID == (fid = H5Fopen(TEST_FILE_NAME, H5F_ACC_RDONLY, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Open the dataset */
    if (H5I_INVALID_HID == (did = H5Dopen(fid, DSET_NAME, H5P_DEFAULT)))
        HDF5_ERROR;

    /* Set up a dataspace to represent the in-memory data */
    if (H5I_INVALID_HID == (msid = H5Screate_simple(NDIMS, &msid_dims, NULL)))
        HDF5_ERROR;

    /* Set up a dataspace to represent a subset of the dataset */
    if (H5I_INVALID_HID == (fsid = H5Dget_space(did)))
        HDF5_ERROR;

    /* Allocate the buffer */
    if (NULL == (buf = (int *)calloc(ELEMS_PER_IO, sizeof(int))))
        PROGRAM_ERROR("memory allocation for buf failed");

    /* Read the data from the file */
    while (n_elems_read < DSET_DIMS) {
        hsize_t start   = n_elems_read;
        hsize_t stride  = 1;
        hsize_t count   = 1;
        hsize_t block   = ELEMS_PER_IO;

        /* Adjust the dataset dataspace's hyperslab */
        if (H5Sselect_hyperslab(fsid, H5S_SELECT_SET, &start, &stride, &count, &block) < 0)
            HDF5_ERROR;

        /* Read the data */
        if (H5Dread(did, H5T_NATIVE_INT, msid, fsid, H5P_DEFAULT, buf) < 0)
            HDF5_ERROR;

        /* Verify the data and reset the buffer */
        for (i = 0; i < ELEMS_PER_IO; i++) {
            if (buf[i] != i)
                PROGRAM_ERROR("invalid data read from dataset");
        }
        memset(buf, 0, (size_t)(ELEMS_PER_IO * sizeof(int)));

        /* Update the count */
        n_elems_read += ELEMS_PER_IO;
    }

    /* Close everything */
    if (H5Fclose(fid) < 0)
        HDF5_ERROR;
    if (H5Sclose(msid) < 0)
        HDF5_ERROR;
    if (H5Sclose(fsid) < 0)
        HDF5_ERROR;
    if (H5Dclose(did) < 0)
        HDF5_ERROR;
    free(buf);

    return 0;

error:
    /* Error case clean up */
    H5E_BEGIN_TRY {
        H5Fclose(fid);
        H5Sclose(msid);
        H5Sclose(fsid);
        H5Dclose(did);
    } H5E_END_TRY;

    free(buf);

    return -1;
} /* end read_from_file() */


int
main(void)
{
    if (create_file() < 0)
        PROGRAM_ERROR("Unable to create file");

    if (write_to_file() < 0)
        PROGRAM_ERROR("Unable to write to file");

    if (read_from_file() < 0)
        PROGRAM_ERROR("Unable to read from file");

    return EXIT_SUCCESS;

error:
    return EXIT_FAILURE;
} /* end main */
