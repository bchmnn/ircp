#include "util/ringbuffer.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "util/arrutils.h"

void* rb_ptr_mod(rb_t* rb, void* p) {
	return (void*) rb->buf + ((p - rb->buf) % (rb->size * rb->buf_type));
}

rb_t* __rb_init(size_t size, size_t buf_type, rb_strategy strategy) {
	rb_t* rb = malloc(sizeof(rb_t));
	rb->size = size;
	rb->free = size;
	rb->buf_type = buf_type;
	rb->buf = malloc(size * buf_type);
	rb->head = rb->buf;
	rb->tail = rb->buf;
	rb->strategy = (size_t) strategy;
	return rb;
}

bool rb_empty(rb_t* rb) {
	return rb->size == rb->free;
}

bool rb_full(rb_t* rb) {
	return !rb->free;
}

size_t rb_free(rb_t* rb) {
	return rb->free;
}

size_t rb_next_size(rb_t* rb) {

	if (rb_empty(rb))
		return 0;

	if (rb->strategy == SINGLE_ELEMENT)
		return 1;
	
	size_t size = 0;

	u_int8_t* tail_elem = (u_int8_t*) rb->head;
	bool is_null = false;

	while (!is_null) {
		is_null = true;
		for (size_t i = 0; i < rb->buf_type; i++) {
			if (*tail_elem++ != 0) {
				is_null = false;
				break;
			}
		}
		tail_elem = (u_int8_t*) rb_ptr_mod(rb, (void*) tail_elem);
		size++;
	}

	return size;
}

int rb_push(rb_t* rb, void* elems, size_t size) {

	if (!size) {
		if (rb->strategy == NULL_TERMINATED)
			size = _arrlen(elems, rb->buf_type);
		else // SINGLE_ELEMENT
			size = 1;
	}

	size_t sep = rb->strategy == NULL_TERMINATED;

	if (size + sep > rb_free(rb)) // not enough space
		return 1;

	for (size_t i = 0; i < size; i++) {
		memcpy(rb->tail, elems, rb->buf_type);
		rb->tail = rb_ptr_mod(rb, rb->tail + rb->buf_type);
		elems += rb->buf_type;
		rb->free--;
	}

	if (rb->strategy == NULL_TERMINATED) {
		for (size_t i = 0; i < rb->buf_type; i++)
			*((u_int8_t*) rb->tail++) = 0;
		rb->tail = rb_ptr_mod(rb, rb->tail);
		rb->free--;
	}

	return 0;
}

rb_elems_t* rb_pop(rb_t* rb) {

	if (rb_empty(rb))
		return NULL;

	size_t size = rb_next_size(rb);

	rb_elems_t* rb_elem = malloc(sizeof(rb_elems_t));
	rb_elem->elems = malloc(size * rb->buf_type);

	void* dest = rb_elem->elems;
	for (size_t i = 0; i < size; i++) {

		memcpy(dest, rb->head, rb->buf_type);
		dest += rb->buf_type;
		rb->head = rb_ptr_mod(rb, rb->head + rb->buf_type);

	}

	rb->free += size;

	rb_elem->len = size;
	return rb_elem;

}

void free_rb(rb_t* rb) {
	free(rb->buf);
	free(rb);
}

void free_rb_deep(rb_t* rb, void(*free_elem)(void*) ){
	while (!rb_empty(rb) ){
		free_elem(rb_pop(rb));
	}
	free_rb(rb);
	
}


void free_rb_elems(rb_elems_t* rb_elems) {
	free(rb_elems->elems);
	free(rb_elems);
}

int rb_push_str(rb_t* rb, char* str) {
	if (rb->strategy != NULL_TERMINATED || rb->buf_type != sizeof(char))
		return 2;
	return rb_push(rb, (void*) str, 0);
}

char* rb_pop_str(rb_t* rb) {
	if (rb->strategy != NULL_TERMINATED || rb->buf_type != sizeof(char))
		return NULL;
	rb_elems_t* rb_elems = rb_pop(rb);
	if (rb_elems == NULL)
		return NULL;
	
	char* str = (char*) rb_elems->elems;
	free(rb_elems);
	return str;
}
