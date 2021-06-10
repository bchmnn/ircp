#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>
#include <stdbool.h>

#define rb_init(size, buf_type, strategy) __rb_init(size, sizeof(buf_type), strategy)

typedef enum {
	NULL_TERMINATED = 0,
	SINGLE_ELEMENT = 1
} rb_strategy;

typedef struct {
	size_t size;
	size_t free;
	size_t buf_type;
	void* buf;
	void* head;
	void* tail;
	rb_strategy strategy;
} rb_t;

typedef struct {
	size_t len;
	void*  elems;
} rb_elems_t;

/**
 * Allocates memory for rb_t and the
 * underlying buffer.
 * @param size      size of buffer
 * @param buf_type  sizeof(type)
 * @param strategy  null_terminated or single_element
 * @return ringbuffer_t pointer
 */
rb_t* __rb_init(size_t size, size_t buf_type, rb_strategy strategy);

/**
 * @param rb  rb_t pointer
 * @return    bool
 */
bool rb_empty(rb_t* rb);

/**
 * @param rb  rb_t pointer
 * @return    bool
 */
bool rb_full(rb_t* rb);

/**
 * @param rb  rb_t pointer
 * @return    free space (#elements)
 */
size_t rb_free(rb_t* rb);

/**
 * Appends elems to buffer.
 * @param rb       rb_t pointer
 * @param elems    elements to append
 * @param size     number of elements
 *                 special case 0:
 *                     * NULL_TERMINATED: extracts size with arrlen
 *                     * SINGLE_ELEMENT: size will be interpreted as 1
 * @return zero on success, greater zero on error
 */
int rb_push(rb_t* rb, void* elems, size_t size);

/**
 * Pops elements.
 * @param rb  rb_t pointer
 * @return    rb_elems_t pointer
 *            null on error
 */
rb_elems_t* rb_pop(rb_t* rb);

/**
 * Frees all allocated memory of rb_t.
 * @param rb  rb_t pointer
 */
void free_rb(rb_t* rb);

/**
 * Frees all allocated memory of rb_elems_t.
 * @param rb_elems  rb_elems_t pointer
 */
void free_rb_elems(rb_elems_t* rb_elems);

/**____ ____ _  _ _  _ ____ _  _ ____ 
 * |    |  | |\/| |\/| |  | |\ | [__  
 * |___ |__| |  | |  | |__| | \| ___]
 */

/**
 * Appends str to buffer.
 * Checks if rb strategy is NULL_TERMINATED
 * and if rb buf_type is 1 (sizeof(char)).
 * @param rb   rb_t pointer
 * @param str  string to append
 * @return     zero on success, greater zero on error
 */
int rb_push_str(rb_t* rb, char* str);

/**
 * Pops str.
 * Checks if rb strategy is NULL_TERMINATED
 * and if rb buf_type is 1 (sizeof(char)).
 * @param rb  rb_t pointer
 * @return    char pointer, null on error
 */
char* rb_pop_str(rb_t* rb);

#endif //RINGBUFFER_H