#include "genericiter.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

// Loop on a iterator
size_t
iterator_for_each (iterator g, int (*actor) (void *elem, void *arg), void *arg) {
  int go = 1;
  size_t ret = 0;
  if (g.instance && g.next_elem_fn)
    for (void *elem = (g.first_elem_fn ? g.first_elem_fn : g.next_elem_fn) (g.instance);
         elem;
         elem = g.next_elem_fn (g.instance)) {
      if ((go = actor (elem, arg)))
        ret++;
      if (g.free_elem_fn)
        g.free_elem_fn (elem);
      if (!go)
        break;
    }
  if (g.free_instance_fn)
    g.free_instance_fn (g.instance);
  return ret;
}

/* =========== HELPERS ============ */
static void *
next_char (void *instance) {
  int c = fgetc (instance);
  if (c == EOF)
    return 0;
  char *p = malloc (sizeof (*p));
  if (!p)
    return 0;
  *p = (char)c;
  return p;
}

iterator
ch_file_iterator (FILE *f) { return ((iterator){ .instance = f, .next_elem_fn = next_char, .free_elem_fn = free }); }

static void *
next_wc (void *instance) {
  wint_t c = fgetwc (instance);
  if (c == WEOF)
    return 0;
  wchar_t *p = malloc (sizeof (*p));
  if (!p)
    return 0;
  *p = (wchar_t)c;
  return p;
}

iterator
wc_file_iterator (FILE *f) { return (iterator){ .instance = f, .next_elem_fn = next_wc, .free_elem_fn = free }; }

struct array {
  void *array;
  size_t length;
  size_t size;
  void *cursor;
};

static void *
first_elem (void *instance) {
  struct array *ctx = instance;
  return ctx->length ? ctx->cursor = ctx->array : 0;
}

static void *
next_elem (void *instance) {
  struct array *ctx = instance;
  ctx->cursor += ctx->size;
  if (ctx->cursor < ctx->array + ctx->length * ctx->size)
    return ctx->cursor;
  else
    return 0;
}

iterator
array_iterator (void *array, size_t length, size_t elem_size) {
  struct array *instance = malloc (sizeof (*instance));
  if (instance)
    *instance = (struct array){ .array = array, .length = length, .size = elem_size };
  return (iterator){ .instance = instance, .first_elem_fn = first_elem, .next_elem_fn = next_elem, .free_instance_fn = free };
}

struct string {
  void *str;
  size_t cursor;
};

static void *
str_iterator_adapter (void *instance) {
  struct string *ctx = instance;
  char *c = ctx->str;
  c += ctx->cursor;
  if (*c == '\0')
    return 0; // Fin de chaîne
  ctx->cursor++;
  return c;
}

iterator
str_iterator (char *source_str) {
  struct string *ctx = malloc (sizeof (*ctx));
  if (ctx) {
    ctx->str = source_str;
    ctx->cursor = 0;
  }
  return (iterator){ .instance = ctx, .next_elem_fn = str_iterator_adapter, .free_instance_fn = free };
}

static void *
wcs_iterator_adapter (void *instance) {
  struct string *ctx = instance;
  wchar_t *c = ctx->str;
  c += ctx->cursor;
  if (*c == L'\0')
    return 0; // Fin de chaîne
  ctx->cursor++;
  return c;
}

iterator
wcs_iterator (wchar_t *source_str) {
  struct string *ctx = malloc (sizeof (*ctx));
  if (ctx) {
    ctx->str = source_str;
    ctx->cursor = 0;
  }
  return (iterator){ .instance = ctx, .next_elem_fn = wcs_iterator_adapter, .free_instance_fn = free };
}
