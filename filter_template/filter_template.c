/* filter_template.c
 *
 * This is template that creates a do-nothing filter for HDF5.
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

/* The HDF5 header */
#include <hdf5.h>

/* The HDF5 external plugin header */
#include <H5PLextern.h>

#include "filter_template.h"


/* The data conversion function for this filter */
static size_t filter_template_function(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf);


/* Information about this filter
 * H5Z_class2_t is defined in H5Zpublic.h
 */
const H5Z_class2_t FILTER_TEMPLATE_CLASS[1] = {{
    H5Z_CLASS_T_VERS,                       /* Filter class version */
    FILTER_TEMPLATE_ID,                     /* Filter id number */
    1,                                      /* encoder_present flag */
    1,                                      /* decoder_present flag */
    "HDF5 template filter",                 /* Filter name for debugging */
    NULL,                                   /* The "can apply" callback */
    NULL,                                   /* The "set local" callback */
    (H5Z_func_t)filter_template_function,   /* The actual filter function */
}};


/* The plugin functions you must implement when you include H5PLextern.h */
H5PL_type_t H5PLget_plugin_type(void) { return H5PL_TYPE_FILTER; }
const void *H5PLget_plugin_info(void) { return FILTER_TEMPLATE_CLASS; }


/* The filter's data transformation goes here */
static size_t
filter_template_function(unsigned int flags, size_t cd_nelmts,
        const unsigned int cd_values[], size_t nbytes, size_t *buf_size,
        void **buf)
{
    /* This is just a shell and does nothing */

    if (flags & H5Z_FLAG_REVERSE) {
        /* Decompress data */
    }
    else {
        /* Compress data */
    }

    return *buf_size;
} /* end filter_template_function() */

