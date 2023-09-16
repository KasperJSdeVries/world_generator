#include "darray.h"

#include <stdlib.h>
#include <string.h>

void *_darray_create(u64 length, u64 stride) {
	u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
	u64 array_size = length * stride;
	u64 *new_array = malloc(header_size + array_size);
	memset(new_array, 0, header_size + array_size);
	new_array[DARRAY_CAPACITY] = length;
	new_array[DARRAY_LENGTH] = 0;
	new_array[DARRAY_STRIDE] = stride;
	return (void *)(new_array + DARRAY_FIELD_LENGTH);
}
