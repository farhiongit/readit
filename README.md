> This tiny project brings a generic stream reader with look-ahead capacities, and a generic iterator.

# Generic stream reader with look-ahead capacities

The [interface](genericstreamreader.h) permits:

- the creation of a `stream_reader` from a provided [reader](#readers) with `stream_create`;
- reading and consuming the next "character" with `stream_get`;
- cleaning up the last consumed "character" by applying the function returned by `stream_forgetter` to it.
- reading ahead[^2] the *n*<sup>th</sup> "character" (*n* > 0) not consumed yet with `stream_look_ahead` (up to 1024[^1] or exhaustion of memory, which comes first);
- ignoring and consuming[^2] the next "character" with `stream_forget`;
- closing the stream with `stream_destroy`.

[^1]: This limit can be modified at compile-time with the macro `MAXIMUM_CAPACITY` (i.e. `-DMAXIMUM_CAPACITY=8`).

[^2]: This generic reader is suitable for use with a lexer for instance.

## Readers

`stream_create` requires a `reader`, user-defined or provided, composed of:

| Field | Description |
| - | - |
| `void *instance`     | Pointer to the concrete structure (FILE*, string, etc.) to read. |
| `free_instance_fn`   | Optional function to free memory allocated by the instance. |
| `read_fn`            | Function to read the next character from the instance (or 0 in the event of an end-of-stream or error.) |
| `forget_read_fn`     | Optional function to free memory allocated by read_fn. |

Provided predefined readers are:

| Reader | Description |
| - | - |
| `ch_file_reader` | To read characters `char` from a file `FILE *`. |
| `wc_file_reader` | To read wide characters `wchar_t` from a file `FILE *`. |
| `str_reader`     | To read characters `char` from a string `char *`. |
| `wcs_reader`     | To read wide characters `wchar_t` from a string `wchar_t *`. |

## Files

| File | Description |
| - | - |
| [`genericstreamreader.h`](genericstreamreader.h) | Interface |
| [`genericstreamreader.c`](genericstreamreader.c) | Implementation |
| [`test_streamreader.c`](test_streamreader.c)     | Tests and examples (type [`make`](Makefile)) |

# Generic iterator

## Iterators

The user provides a user-defined or predefined `iterator` composed of:

| Field | Description |
| - | - |
| `void *instance`     | Pointer to the concrete structure (FILE*, string, etc.) to iterate on. |
| `free_instance_fn`   | Optional function to free memory allocated by the instance. |
| `first_elem_fn`      | Optional function to get the first element from the instance (or 0 if none.). `next_elem_fn` is used by default. |
| `next_elem_fn`       | Function to get the next element from the instance (or 0 if none.) |
| `free_elem_fn`       | Optional function to free memory allocated by an element. |

Provided predefined iterators are:

| Reader | Description |
| - | - |
| `ch_file_iterator` | To iterate on characters `char` from a file `FILE *` |
| `wc_file_iterator` | To iterate on wide characters `wchar_t` from a file `FILE *` |
| `str_iterator`     | To iterate on a string `char *` |
| `wcs_iterator`     | To iterate on a wide characters string `wchar_t *` |
| `array_iterator`   | To iterate on elements of an array. |

## Iteration

Once the [iterator](#iterators) is defined, its components are used to iterate on data with a call to `iterator_for_each`:

- Its signature is `size_t iterator_for_each (iterator iter, int (*actor) (void *elem, void *arg), void *arg)`;
- A function `int (*actor) (void *elem, void *arg)` is applied on each character sucessively until this function returns `0`;
- `arg` is passed as second argument to `actor`;
- It returns the number of elements wich have been iterated on.

## Files

| File | Description |
| - | - |
| [`genericiter.h`](genericiter.h) | Interface |
| [`genericiter.c`](genericiter.c) | Implementation |
| [`test_iter.c`](test_iter.c)     | Tests and examples (type [`make`](Makefile)) |

