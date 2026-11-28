#ifndef __GENERIC_ITERATOR_H__
#define __GENERIC_ITERATOR_H__

#include <stddef.h>
#include <stdio.h>

typedef void (*free_prototype) (void *);
typedef struct {
  void *instance;                          // Pointer to the concrete structure (FILE*, string, etc.)
  void *(*first_elem_fn) (void *instance); // Function to read the first element from the iterator (or 0 in the event of an end-of-iterator or error.)
  void *(*next_elem_fn) (void *instance);  // Function to read the next element from the iterator (or 0 in the event of an end-of-iterator or error.)
  free_prototype after_each_fn;            // Optional element deallocator to free memory allocated by next_elem_fn.
  free_prototype after_all_fn;             // Optional deallocator to free memory allocated to make the instance.
} iterator;

// The function `int (*on_each) (void *elem, void *arg)` is applied to each element consecutively until `on_each` returns `0`; `arg` is passed as second argument to `on_each`.
// Returns the number of elements which have been iterated on before `on_each` returns `0`.
size_t iterator_for_each (iterator iter, int (*on_each) (void *elem, void *arg), void *arg);

/* =========== HELPERS ============ */
iterator ch_file_iterator (FILE *f);
iterator wc_file_iterator (FILE *f);

#define ARRAY_ITERATOR(array) array_iterator (array, sizeof (array) / sizeof (*array), sizeof (*array))
iterator array_iterator (void *array, size_t length, size_t elem_size);

iterator str_iterator (char *source_str);
iterator wcs_iterator (wchar_t *source_str);

#endif
