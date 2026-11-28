> This tiny project brings a generic stream reader with look-ahead capacities, and a generic iterator.

# Generic stream reader with look-ahead capacities

The [interface](genericstreamreader.h) permits:

- the creation of a `stream_reader` from a provided [reader](#readers) with `stream_create`;
- to read and consume, with `stream_get`, the next "character";
- to apply the function returned by `stream_forgetter` to the last "character" consumed by `stream_get` (to avoid memory leaks);
- to look ahead[^1], with `stream_look_ahead`, the *n*<sup>th</sup> "character" not consumed yet, *without consuming it*
  (*n* > 0, up to 1024[^2] or exhaustion of memory, which comes first);
- to ignore and consume[^1], with `stream_forget`, the next "character";
- to close the stream with `stream_destroy` (to avoid memory leaks).

[^1]: This generic reader is suitable for use with a lexer for instance.

[^2]: This limit can be modified at compile-time with the macro `MAXIMUM_CAPACITY` (i.e. `-DMAXIMUM_CAPACITY=8`).

## Readers

`stream_create` requires a `reader`, user-defined or provided, composed of:

| Field | Description |
| - | - |
| `void *instance`     | Pointer to the concrete structure (FILE*, string, etc.) to read. |
| `free_instance_fn`   | Optional function to free memory allocated by the instance. |
| `read_fn`            | Function to read the next character from the instance (or 0 in the event of an end-of-stream or error.) |
| `forget_read_fn`     | Optional function to free memory allocated by read_fn. |

### Provided predefined readers are:

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
| [`test_streamreader.c`](test_streamreader.c)     | Test and example (type [`make`](Makefile)) |

# Generic iterator

## Iterators

The user provides a user-defined or predefined `iterator` composed of:

| Field | Description |
| - | - |
| `void *instance`     | Pointer to the concrete structure (FILE*, string, etc.) to iterate on. |
| `first_elem_fn`      | Optional function to get the first element from the instance (should return `0` if none.). `next_elem_fn` is used by default. |
| `next_elem_fn`       | Function to get the next element from the instance (should return `0` if none.) |
| `after_each_fn`      | Optional function called after each iteration[^3], especially to free memory allocated by (when defined) `first_elem_fn` and `next_elem_fn`. |
| `after_all_fn`       | Optional function called after all iterations, especially to free memory allocated by the instance. |

[^3]: Even if `on_each` returns `0`.

### Provided predefined iterators are:

| Reader | Description |
| - | - |
| `ch_file_iterator` | To iterate on characters `char` from a file `FILE *` |
| `wc_file_iterator` | To iterate on wide characters `wchar_t` from a file `FILE *` |
| `str_iterator`     | To iterate on a string `char *` |
| `wcs_iterator`     | To iterate on a wide characters string `wchar_t *` |
| `array_iterator`   | To iterate on elements of an array. |

## Iteration

Once the [iterator](#iterators) is defined, its components are used to iterate on data with a call to `iterator_for_each`:

- Its signature is `size_t iterator_for_each (iterator iter, int (*on_each) (void *elem, void *arg), void *arg)`;
- A function `int (*on_each) (void *elem, void *arg)` is applied to each element consecutively until this function `on_each` returns `0`;
- `arg` is passed as second argument to `on_each`;
- It returns the number of elements which have been iterated on *before* `on_each` returns `0`.

> The same `iterator` can be reused several times with diffrent functions `on_each`.

## Files

| File | Description |
| - | - |
| [`genericiter.h`](genericiter.h) | Interface |
| [`genericiter.c`](genericiter.c) | Implementation |
| [`test_iter.c`](test_iter.c)     | Tests and examples (type [`make`](Makefile)) |

