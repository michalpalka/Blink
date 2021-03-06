/* 
   Copyright (c) Microsoft Corporation
   All rights reserved. 

   Licensed under the Apache License, Version 2.0 (the ""License""); you
   may not use this file except in compliance with the License. You may
   obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
   LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
   A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

   See the Apache Version 2.0 License for specific language governing
   permissions and limitations under the License.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "params.h"
#include "types.h"



char * try_alloc_bytes(unsigned int siz) 
{
  char *buf = (char *) malloc(siz);
  if (buf == NULL) {
    fprintf(stderr, "Error: could not allocate buffer of size %ul\n", siz);
    exit(1); }
  return buf;
}


/* DV: This is really the smallest and simplest implementation.
   Allocate memory for all threads on the same global heap. 
   Most likely this will have to change at some point (move to TLS).
 ******************************************************************/

static void * wpl_heap = NULL;
static unsigned int wpl_free_idx;
static unsigned int wpl_heap_siz;

#define CALIGN16(x) (((((x) + 15) >> 4) << 4))


void wpl_init_heap(unsigned int max_heap_size)
{
  // if (num_of_allocs == 0) return; // we need no heap!
  
  wpl_heap_siz = max_heap_size;
  wpl_heap     = try_alloc_bytes(max_heap_size);
  wpl_free_idx = CALIGN16((unsigned int) wpl_heap) - (unsigned int) wpl_heap;
}


unsigned int wpl_get_free_idx() 
{ 
  return wpl_free_idx; 
}

// precondition: 16-aligned
void wpl_restore_free_idx(unsigned int idx) 
{ 
  wpl_free_idx = idx;  
}


void * wpl_alloca(unsigned int bytes)
{
  unsigned int allocunit = CALIGN16(bytes);

  /* printf("Allocating: %d bytes\n", bytes); */
  /* printf("wpl_heap_siz: %d bytes\n", wpl_heap_siz); */

  if (wpl_free_idx + allocunit >= wpl_heap_siz) {
    fprintf(stderr, "WPL allocator out of memory, try increasing heap size!\n");
    exit(-1);
  }

  void * ret = (void *) ((unsigned int) wpl_heap + wpl_free_idx);
  
  wpl_free_idx += allocunit;

  return ret;

}
