/* shuffle_noduff_omp.c
 *
 * A clone of the official HDF5 shuffle filter, but with the Duff's device
 * replaced with a simple memory copy and OpenMP enabled.
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/* The HDF5 header */
#include <hdf5.h>

/* The HDF5 external plugin header */
#include <H5PLextern.h>

#include "shuffle.h"


/* Filter callback prototypes */
static herr_t set_local_shuffle(hid_t dcpl_id, hid_t type_id, hid_t space_id);
static size_t filter_shuffle(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf);


/* Information about this filter
 * H5Z_class2_t is defined in H5Zpublic.h
 */
const H5Z_class2_t SHUFFLE_CLASS[1] = {{
    H5Z_CLASS_T_VERS,                       /* Filter class version */
    SHUFFLE_NODUFF_OMP_ID,                  /* Filter id number */
    1,                                      /* encoder_present flag */
    1,                                      /* decoder_present flag */
    "shuffle_noduff_omp",                   /* Filter name for debugging */
    NULL,                                   /* The "can apply" callback */
    set_local_shuffle,                      /* The "set local" callback */
    (H5Z_func_t)filter_shuffle,             /* The actual filter function */
}};

/* Local macros */
#define SHUFFLE_PARM_SIZE       0   /* "Local" parameter for shuffling size */
#define SHUFFLE_USER_NPARMS     0   /* Number of parameters that users can set */
#define SHUFFLE_TOTAL_NPARMS    1   /* Total number of parameters for filter */



/* The plugin functions you must implement when you include H5PLextern.h */
H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }
const void *H5PLget_plugin_info(void) { return SHUFFLE_CLASS; }


static herr_t
set_local_shuffle(hid_t dcpl_id, hid_t type_id, hid_t space_id)
{
    unsigned flags;                             /* Filter flags */
    size_t type_size;                           /* Datatype size */
    size_t cd_nelmts = SHUFFLE_USER_NPARMS;     /* # of filter parameters */
    unsigned cd_values[SHUFFLE_TOTAL_NPARMS];   /* Filter parameters */

    /* Get the filter's current parameters */
    if (H5Pget_filter_by_id(dcpl_id, SHUFFLE_NODUFF_OMP_ID, &flags, &cd_nelmts, cd_values, (size_t)0, NULL, NULL) < 0)
        goto error;

    /* Get the type size */
    if (0 == (type_size = H5Tget_size(type_id)))
        goto error;

    /* Set "local" parameter for this dataset */
    cd_values[SHUFFLE_PARM_SIZE] = (unsigned)type_size;

    /* Modify the filter's parameters for this dataset */
    if(H5Pmodify_filter(dcpl_id, SHUFFLE_NODUFF_OMP_ID, flags, (size_t)SHUFFLE_TOTAL_NPARMS, cd_values) < 0)
        goto error;

    return 0;

error:
    return -1;
} /* end set_local_shuffle() */


static size_t
filter_shuffle(unsigned int flags, size_t cd_nelmts, const unsigned int cd_values[],
        size_t nbytes, size_t *buf_size, void **buf)
{
    unsigned bytes_per_elem;        /* Number of bytes per element */
    size_t n_elements;              /* Number of elements in buffer */
    size_t leftover;                /* Extra bytes at end of buffer */
    void *dest = NULL;              /* Buffer to deposit [un]shuffled bytes into */
    unsigned char *_src = NULL;     /* Alias for source buffer */
    unsigned char *_dest = NULL;    /* Alias for destination buffer */
    int i;

    /* Check arguments */
    if (cd_nelmts != SHUFFLE_TOTAL_NPARMS || cd_values[SHUFFLE_PARM_SIZE] == 0)
        goto error;

    /* Get the number of bytes per element from the parameter block */
    bytes_per_elem = cd_values[SHUFFLE_PARM_SIZE];

    /* Compute the number of elements in buffer */
    n_elements = nbytes / bytes_per_elem;

    /* If this is a single byte type or we have fractional elements, do nothing */
    if (bytes_per_elem <= 1 || n_elements <= 1)
        return *buf_size;

    /* Compute the leftover bytes if there are any */
    leftover = nbytes % bytes_per_elem;

    /* Allocate the destination buffer */
    if (NULL == (dest = malloc(nbytes)))
        goto error;

    if (flags & H5Z_FLAG_REVERSE) {

        /*************/
        /* UNSHUFFLE */
        /*************/

        /* Get a pointer to the source buffer */
        _src = (unsigned char *)(*buf);

        /* Input; unshuffle */
        for (i = 0; i < bytes_per_elem; i++) {

            int j = n_elements;

            _dest = ((unsigned char *)dest) + i;

            while(j > 0) {
                *_dest = *_src++;
                _dest += bytes_per_elem;

                j--;
            }
        }

        /* Add leftover to the end of data */
        if (leftover > 0) {
            /* Adjust back to end of shuffled bytes */
            _dest -= (bytes_per_elem - 1);
            memcpy((void *)_dest, (void *)_src, leftover);
        }

    } /* end unshuffle */
    else {

        /***********/
        /* SHUFFLE */
        /***********/

        /* Get a pointer to the destination buffer */
        _dest = (unsigned char *)dest;

        /* Output; shuffle */
        for (i = 0; i < bytes_per_elem; i++) {

            int j = n_elements;

            _src = ((unsigned char *)(*buf)) + i;

            while(j > 0) {
                *_dest++ = *_src;
                _src += bytes_per_elem;

                j--;
            }
        }

        /* Add leftover to the end of data */
        if(leftover > 0) {
            /* Adjust back to end of shuffled bytes */
            _src -= (bytes_per_elem - 1);
            memcpy((void*)_dest, (void*)_src, leftover);
        }

    } /* end shuffle */


    /* Free the input buffer */
    free(*buf);

    /* Set the buffer information to return */
    *buf = dest;

    return *buf_size;

error:
    free(dest);

    return 0;
} /* end filter_shuffle() */

