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

void _darray_destroy(void *array) {
	u64 *header = (u64 *)array - DARRAY_FIELD_LENGTH;
	//	u64 header_size = DARRAY_FIELD_LENGTH * sizeof(u64);
	//	u64 total_size =
	//		header_size + (header[DARRAY_LENGTH] * header[DARRAY_STRIDE]);
	free(header);
}

u64 _darray_field_get(void *array, u64 field) {
	u64 *header = (u64 *)array - DARRAY_FIELD_LENGTH;
	return header[field];
}

void _darray_field_set(void *array, u64 field, u64 value) {
	u64 *header = (u64 *)array - DARRAY_FIELD_LENGTH;
	header[field] = value;
}

void *_darray_resize(void *array) {
	u64 length = darray_length(array);
	u64 stride = darray_stride(array);
	void *temp =
		_darray_create((DARRAY_RESIZE_FACTOR * darray_capacity(array)), stride);
	memcpy(temp, array, length * stride);
	_darray_field_set(temp, DARRAY_LENGTH, length);
	_darray_destroy(array);
	return temp;
}

void *_darray_push(void *array, const void *value_ptr) {
	u64 length = darray_length(array);
	u64 stride = darray_stride(array);
	if (length >= darray_capacity(array)) {
		array = _darray_resize(array);
	}

	u64 addr = (u64)array;
	addr += length * stride;
	memcpy((void *)addr, value_ptr, stride);
	_darray_field_set(array, DARRAY_LENGTH, length + 1);
	return array;
}
