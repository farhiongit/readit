#undef NDEBUG
#include "genericiter.h"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void *
start (void *instance) {
  return instance;
}

static void *
incr (void *instance) {
  int *c = instance;
  (*c)++;
  return instance;
}

static int
if_lt_do (void *elem, void *arg) {
  int *c = elem;
  int *upper = arg;
  return *c < *upper ? fprintf (stderr, "%+i ", *c), 1 : 0;
}

static int
lower (void *elem, void *arg) {
  char *c = elem;
  *c = (char)tolower (*c);
  if (arg) {
    void (*f) (char *) = arg;
    f (c);
  }
  return 1;
}

static void
printc (char *c) {
  fprintf (stderr, "%c", *c);
}

struct ll_elem {
  char val;
  struct ll_elem *next;
};

struct ll_iter {
  struct ll_elem *list;
  struct ll_elem *current;
};

static void *
ll_begin (void *instance) { return ((struct ll_iter *)instance)->current = ((struct ll_iter *)instance)->list; }
static void *
ll_next (void *instance) { return ((struct ll_iter *)instance)->current = ((struct ll_iter *)instance)->current->next; }

int
main (void) {
  { // Home-made loop in integers
    int counter = -5;
    int upper = 5;
    assert (if_lt_do (&counter, &upper));
    // the operator "==" does not guarantee the order in which operands are evaluated.
    size_t nb = (size_t)(upper - counter);
    // counter is modified by the iterations.
    assert (nb == iterator_for_each ((iterator){ .instance = &counter, .first_elem_fn = start, .next_elem_fn = incr }, if_lt_do, &upper));
    fprintf (stderr, "\n");
    assert (!if_lt_do (&counter, &upper));
  }
  { // Loop on an array
    char alphabet['Z' - 'A' + 1];
    for (size_t i = 0; i < sizeof (alphabet) / sizeof (*alphabet); i++)
      fprintf (stderr, "%c", alphabet[i] = (char)('A' + i));
    fprintf (stderr, "\n");
    assert (iterator_for_each (ARRAY_ITERATOR (alphabet), lower, printc) == sizeof (alphabet) / sizeof (*alphabet));
    fprintf (stderr, "\n");
  }
  { // Loop on a FILE
    char *const s = "ITERATE ON ME!";
    FILE *f = fmemopen (s, strlen (s), "r");
    assert (iterator_for_each (ch_file_iterator (f), lower, printc) == strlen (s));
    fclose (f);
    fprintf (stderr, "\n");
  }
  { // Loop on a string
    char s[] = "ITERATE ON YOU!";
    fprintf (stderr, "%s\n", s);
    assert (iterator_for_each (str_iterator (s), lower, printc) == strlen (s));
    fprintf (stderr, "\n");
  }
  { // Loop on a linked list.
    struct ll_elem *head = 0;
    for (size_t i = 0; i < 10; i++) {
      struct ll_elem *new = malloc (sizeof (*new));
      assert (new);
      fprintf (stderr, "%c", new->val = (char)('A' + i));
      new->next = head;
      head = new;
    }
    fprintf (stderr, "\n");

    assert (10 == iterator_for_each ((iterator){ .instance = &((struct ll_iter){ .list = head }), .first_elem_fn = ll_begin, .next_elem_fn = ll_next }, lower, printc));

    struct ll_elem *n;
    for (struct ll_elem *e = head; e; e = n) {
      n = e->next;
      free (e);
    }
    fprintf (stderr, "\n");
  }
}
