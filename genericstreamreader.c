#undef NDEBUG
#include "genericstreamreader.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static const size_t INITIAL_CAPACITY = 1;
#ifndef MAXIMUM_CAPACITY
#define MAXIMUM_CAPACITY 1024
#endif

struct stream_reader {
  reader reader; // L'interface de lecture générique
  void **buffer; // Tampon circulaire
  size_t capacity;
  size_t head;
  size_t tail;
  size_t size;
  bool is_eof;
};

static bool
stream_resize (stream_reader *stream) {
  void *old_buffer = stream->buffer;
  size_t new_capacity = stream->capacity * 2;
  if (new_capacity > ((size_t)(MAXIMUM_CAPACITY)))
    return (errno = ENOMEM), false;
  else if (!((stream->buffer = realloc (stream->buffer, new_capacity * sizeof (*stream->buffer)))))
    return (stream->buffer = old_buffer), false;

  // Réarrangement
  if (stream->tail <= stream->head) {
    for (size_t i = stream->head; i < stream->capacity /* = old capacity */; i++)
      stream->buffer[i + stream->capacity] = stream->buffer[i];
    stream->head += stream->capacity;
  }

  stream->capacity = new_capacity;
  return true;
}

static bool
stream_fill (stream_reader *stream, size_t need_distance) {
  assert (need_distance);
  while (!stream->is_eof && stream->size < need_distance) {
    assert (stream->size <= stream->capacity);
    if (stream->size == stream->capacity) { // full
      if (!stream_resize (stream))
        break;
    }

    // Appel à l'interface générique (au lieu de fgetwc)
    void *c = stream->reader.read_fn (stream->reader.instance);
    if (c == 0) {
      stream->is_eof = true;
      break;
    }

    assert (stream->size < stream->capacity);
    stream->buffer[stream->tail] = c;
    stream->tail = (stream->tail + 1) % stream->capacity;
    stream->size++;
  } // while (!stream->is_eof && stream->size < need_distance)
  return stream->size >= need_distance;
}

static void
stream_drain (stream_reader *stream) {
  if (stream->reader.forget_read_fn)
    while (stream->size)
      stream->reader.forget_read_fn (stream_get (stream));
}

static void
_noop (void *p) {
  (void)p;
}

__attribute__ ((warn_unused_result)) stream_reader *
stream_create (reader r) {
  if (!r.read_fn || !r.instance)
    return (errno = ENODATA), NULL;

  stream_reader *stream = malloc (sizeof (*stream));
  if (!stream)
    return NULL;

  stream->reader = r;
  stream->capacity = INITIAL_CAPACITY ? INITIAL_CAPACITY : 1;
  stream->buffer = malloc (stream->capacity * sizeof (*stream->buffer));
  if (!stream->buffer) {
    free (stream);
    return NULL;
  }
  stream->head = 0;
  stream->tail = 0;
  stream->size = 0;
  stream->is_eof = false;

  return stream;
}

__attribute__ ((warn_unused_result)) void *
stream_get (stream_reader *stream) {
  if (!stream_fill (stream, 1))
    return 0;

  void *c = stream->buffer[stream->head];
  stream->head = (stream->head + 1) % stream->capacity;
  stream->size--;
  return c;
}

void *
stream_look_ahead (stream_reader *stream, size_t distance) {
  assert (distance > 0);
  if (!stream_fill (stream, distance))
    return 0;

  size_t index = (stream->head + distance - 1) % stream->capacity;
  return stream->buffer[index];
}

void
stream_destroy (stream_reader *stream) {
  stream_drain (stream);
  if (stream->reader.free_instance_fn)
    stream->reader.free_instance_fn (stream->reader.instance);
  free (stream->buffer);
  free (stream);
}

free_prototype
stream_forgetter (stream_reader *stream) {
  if (stream->reader.forget_read_fn)
    return stream->reader.forget_read_fn;
  else
    return _noop;
}

void
stream_forget (stream_reader *stream) { stream_forgetter (stream) (stream_get (stream)); }

//================== HELPERS ======================
static void *
ch_file_reader_adapter (void *f) {
  int ch = fgetc (f);
  if (ch == EOF)
    return 0;
  int *c = malloc (sizeof (c));
  if (!c)
    return 0;
  *c = ch;
  return c;
}

reader
ch_file_reader (FILE *f) {
  return (reader){ .instance = f, .read_fn = ch_file_reader_adapter, .forget_read_fn = free };
}

static void *
wc_file_reader_adapter (void *f) {
  wint_t wc = fgetwc (f);
  if (wc == WEOF)
    return 0;
  wint_t *c = malloc (sizeof (c));
  if (!c)
    return 0;
  *c = wc;
  return c;
}

reader
wc_file_reader (FILE *f) {
  return (reader){ .instance = f, .read_fn = wc_file_reader_adapter, .forget_read_fn = free };
}

struct StringReaderCtx {
  void *str;
  size_t cursor;
};

static void *
str_read_adapter (void *instance) {
  struct StringReaderCtx *ctx = instance;
  char *c = ctx->str;
  c += ctx->cursor;
  if (*c == '\0')
    return 0; // Fin de chaîne
  ctx->cursor++;
  return c;
}

reader
str_reader (char *source_str) {
  struct StringReaderCtx *ctx = malloc (sizeof (*ctx));
  if (ctx) {
    ctx->str = source_str;
    ctx->cursor = 0;
  }
  return (reader){ .instance = ctx, .read_fn = str_read_adapter, .free_instance_fn = free };
}

static void *
wcs_read_adapter (void *instance) {
  struct StringReaderCtx *ctx = instance;
  wchar_t *c = ctx->str;
  c += ctx->cursor;
  if (*c == L'\0')
    return 0; // Fin de chaîne
  ctx->cursor++;
  return c;
}

reader
wcs_reader (wchar_t *source_str) {
  struct StringReaderCtx *ctx = malloc (sizeof (*ctx));
  if (ctx) {
    ctx->str = source_str;
    ctx->cursor = 0;
  }
  return (reader){ .instance = ctx, .read_fn = wcs_read_adapter, .free_instance_fn = free };
}
