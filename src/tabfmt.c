/*
 * tabfmt - format tabular data
 * Copyright (c) 2005, 2006 Claudio Jolowicz <claudio@jolowicz.com>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * 
 * $Id: tabfmt.c,v 1.1.1.1 2009/07/16 02:17:25 reynolds Exp $
 */


#if HAVE_CONFIG_H
# include <config.h>
#endif
#include <stdio.h>
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#if STDC_HEADERS
# include <stdlib.h>
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif
#if HAVE_ERRNO_H
# include <errno.h>
#endif
#ifndef errno
extern int errno;
#endif
#if HAVE_LIMITS_H
# include <limits.h>
#endif
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_ERROR_H
# include <error.h>
#endif
#include "gettext.h"
#if HAVE_LOCALE_H
# include <locale.h>
#endif
#define _(String) gettext (String)
#define N_(String) gettext_noop (String)
#if !HAVE_BZERO && HAVE_MEMSET
# define bzero(buf, bytes) ((void) memset (buf, 0, bytes))
#endif
#if !HAVE_STRCHR && HAVE_INDEX
# define strchr index
#endif

/* Gnulib. */
#include "dirname.h"
#include "error.h"
#include "exit.h"
#include "exitfail.h"
#include "getdelim.h"
#include "getline.h"
#include "gettext.h"
#include "xalloc.h"
#include <stdbool.h>
#include <assert.h>
#include <getopt.h>


#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS     0
# define EXIT_FAILURE     1
#endif

#define DEFAULT_FIELDS    2
#define DEFAULT_PADDING   "0"
#define DEFAULT_DELIMITER "\t "
#define DEFAULT_OUTPUT_DELIMITER "\t"
#define DEFAULT_ALIGN     "L"
#define DEFAULT_PADDER    " "
#define DEFAULT_FILLER    " "

#define ALIGN_LEFT        0
#define ALIGN_CENTER      1
#define ALIGN_RIGHT       2

/* For long options that have no equivalent short option, use a
   non-character as a pseudo short option, starting with CHAR_MAX + 1.  */
#define OPTION_DEBUG      CHAR_MAX + 1


/*
 * usage and version
 */
const char *program_name;
const char *version;
const char *usage;


/* 
 * safe memory allocation
 */

inline void *
safe_malloc (size_t size)
{
  register void *p;

  if ((p = (void *) malloc (size)) == 0)
    error (EXIT_FAILURE, 0, _("out of memory"));

  return p;
}

inline void *
safe_realloc (void *p, size_t size)
{
  register void *r;

  if ((r = (void *) realloc (p, size)) == 0)
    if (size)
      error (EXIT_FAILURE, 0, _("out of memory"));

  return r;
}

inline void
safe_free (void *p)
{
  if (p)
    free (p);
}


/*
 * translating backslash escapes
 */

/* Copy S to T translating character escapes. Returns the number of
   chars written to T. If T is NULL, just count the chars to be
   written.  Thanks GNU coreutils 5.2.1 `echo.c' */
size_t
str_unesc (char *t, const char *s)
{
  register size_t n = 0;
  register int c;

  while ((c = *s++))
  {
    if (c == '\\' && *s)
    {
      switch (c = *s++)
      {
        case 'a':
          c = '\007'; 
          break;

        case 'b':
          c = '\b'; 
          break;

        case 'f':
          c = '\f'; 
          break;

        case 'n':
          c = '\n'; 
          break;

        case 'r':
          c = '\r'; 
          break;

        case 't':
          c = '\t'; 
          break;

        case 'v':
          c = (int) 0x0B; 
          break;

        case '0': case '1': case '2': case '3':
        case '4': case '5': case '6': case '7':
          c -= '0';

          if (*s >= '0' && *s <= '7')
            c = c * 8 + (*s++ - '0');

          if (*s >= '0' && *s <= '7')
            c = c * 8 + (*s++ - '0');

          break;

        case '\\': 
          break;

        default:
          if (t)
            t[n] = '\\';

          ++n;

          break;
      }
    }

    if (t)
      t[n] = c;

    ++n;
  }

  if (t)
    t[n] = '\0';

  return n;
}

/* Return a copy of S with any character escapes translated. The
   returned string is allocated with malloc. */
inline char *
str_unesca (const char *s)
{
  char *t = malloc (strlen (s));
  (void) str_unesc (t, s);
  return t;
}


/*
 * sizevec data structure and functions
 */

/* A sizevec is an array of size_t with specified number of
   elements */
struct sizevec
{
  size_t *offv;
  size_t  offc;
};

/* initialise a sizevec vector */
inline void
sizevec_init (struct sizevec *sizevec, size_t n)
{
  sizevec->offv = (size_t *) safe_malloc (n * sizeof (size_t));
  bzero (sizevec->offv, n * sizeof (size_t));
  sizevec->offc = n;
}

/* resize a sizevec vector */
inline void
sizevec_resize (struct sizevec *sizevec, size_t n)
{
  sizevec->offv = (size_t *) safe_realloc (sizevec->offv, n * sizeof (size_t));
  if (n > sizevec->offc)
    bzero (&sizevec->offv[sizevec->offc], (n - sizevec->offc) * sizeof (size_t));
  sizevec->offc = n;
}

/* allocate a sizevec vector */
inline struct sizevec *
sizevec_alloc (size_t n)
{
  struct sizevec *sizevec = (struct sizevec *) safe_malloc (sizeof (struct sizevec));
  sizevec_init (sizevec, n);
  return sizevec;
}

/* copy a sizevec vector (no allocation) */
inline void
sizevec_copy (struct sizevec *target, const struct sizevec *source)
{
  size_t i, j;

  for (i = j = 0; i < target->offc; ++i, ++j)
  {
    if (j >= source->offc)
      --j;

    target->offv[i] = source->offv[j];
  }
}

/* clone a sizevec vector */
inline struct sizevec *
sizevec_clone (const struct sizevec *orig)
{
  struct sizevec *sizevec = (struct sizevec *) safe_malloc (sizeof (struct sizevec));
  sizevec_init (sizevec, orig->offc);
  sizevec_copy (sizevec, orig);
  return sizevec;
}

/* map min over a pair of sizevec's, modifying the first one */
void
sizevec_min (struct sizevec *sizevec, const struct sizevec *max)
{
  size_t i, j;

  for (i = j = 0; i < sizevec->offc; ++i, ++j)
  {
    if (j >= max->offc)
      --j;

    if (sizevec->offv[i] > max->offv[j])
      sizevec->offv[i] = max->offv[j];
  }
}

/* print a sizevec to a stream */
int
sizevec_print (FILE *stream, const struct sizevec *sizevec)
{
  size_t i;

  for (i = 0; i < sizevec->offc - 1; ++i)
    if (fprintf (stream, "%d, ", sizevec->offv[i]) == EOF)
      return EOF;

  return fprintf (stream, "%d\n", sizevec->offv[i]);
}

/* free a sizevec vector */
inline void
sizevec_free (struct sizevec * sizevec)
{
  safe_free (sizevec->offv);
  safe_free (sizevec);
}


/* 
 * entry data structure and functions
 */

/* An entry describes a series of consecutive strings delimited by a
   single stop character. All entries are linked in a simply-linked
   list. The strings may contain NUL characters. */
struct entry
{
  char          *line;   /* line buffer */
  struct sizevec fields; /* field sizes */
  struct entry  *next;   /* pointer to next entry */
};

/* initialise an entry */
inline void
entry_init (struct entry *entry, size_t n)
{
  entry->line = NULL;
  sizevec_init (&entry->fields, n);
  entry->next = NULL;
}

/* allocate and initialise an entry */
inline struct entry *
entry_alloc (size_t n)
{
  struct entry *entry = (struct entry *) safe_malloc (sizeof (struct entry));
  entry_init (entry, n);
  return entry;
}

/* free an entry */
inline void
entry_free (struct entry *entry)
{
  safe_free (entry->line);
  safe_free (entry);
}

/* free all entries starting at entry */
inline void
entry_free_all (struct entry *entry)
{
  struct entry *cur;
  struct entry *next;

  for (cur = entry; cur; cur = next)
  {
    next = cur->next;
    entry_free (cur);
  }
}

/* fill an entry from a string with the given field separators */
size_t
entry_fill (struct entry *entry, char *s, size_t len, const char *stopset)
{
  size_t n, i;

  entry->line = s;

  for (i = n = 0; i < len; ++i, ++n)
  {
    if (n >= entry->fields.offc)
      sizevec_resize (&entry->fields, n * 2);

    assert (entry->fields.offv[n] == 0);

    for (; i < len; ++i)
    {
      char *p = strchr (stopset, s[i]);

      if (p && *p) /* NUL chars are never delimiters */
        break;

      entry->fields.offv[n] ++;
    }
  }

  if (n != entry->fields.offc)
    sizevec_resize (&entry->fields, n);

  return n;
}

/* update field widths from the given entry */
void
entry_update_widths (const struct entry *entry, struct sizevec *widths)
{
  int i, j;

  for (i = j = 0; i < entry->fields.offc; ++i, ++j)
  {
    if (j >= widths->offc)
      --j;

    if (entry->fields.offv[i] > widths->offv[j])
      widths->offv[j] = entry->fields.offv[i];
  }
}

/* print an entry field to a stream, given the field width, padding
   and alignment */
int
entry_print_field (FILE *output, const char *s, size_t len, size_t width, 
                   char filler, size_t lpadding, size_t rpadding, char padder, 
                   size_t align)
{
  size_t i, j;
  int fill;

  /* print the left padding */
  for (i = 0; i < lpadding; ++i)
    if (putc (padder, output) == EOF)
      return EOF;

  /* print the field */
  switch (align)
  {
    case ALIGN_LEFT:
      for (i = 0; i < width; ++i)
        if (putc (i < len ? s[i] : filler, output) == EOF)
          return EOF;

      break;

    case ALIGN_RIGHT:
      fill = width - len;
      i = 0;

      if (fill > 0)
        for (; i < fill; ++i)
          if (putc (filler, output) == EOF)
            return EOF;

      for (j = 0; i < width; ++i, ++j)
        if (putc (s[j], output) == EOF)
          return EOF;

      break;

    case ALIGN_CENTER:
      fill = width - len;
      i = 0;

      if (fill > 0)
        for (; i < fill / 2; ++i)
          if (putc (filler, output) == EOF)
            return EOF;

      for (j = 0; i < width; ++i, ++j)
        if (putc (j < len ? s[j] : filler, output) == EOF)
          return EOF;

      break;

    default:
      abort ();
  }


  /* print the right padding */
  for (i = 0; i < rpadding; ++i)
    if (putc (padder, output) == EOF)
      return EOF;

  return 0;
}

/* print an entry to a stream, given the field widths, paddings and
   alignments */
int
entry_print (FILE *output, const struct entry *entry, 
             const char *output_delimiter, const struct sizevec *widths, 
             char filler, const struct sizevec *lpadding, 
             const struct sizevec *rpadding, char padder, 
             const struct sizevec *align)
{
  char *s = entry->line;
  int e, w, lp, rp, a;

  for (e = w = lp = rp = a = 0; e < entry->fields.offc; ++e, ++w, ++lp, ++rp, ++a)
  {
    if (w >= widths->offc)
      --w;

    if (lp >= lpadding->offc)
      --lp;

    if (rp >= rpadding->offc)
      --rp;

    if (a >= align->offc)
      --a;

    if (entry_print_field (output, s, entry->fields.offv[e], widths->offv[w], 
                           filler, lpadding->offv[lp], rpadding->offv[rp], padder,
                           align->offv[a]) == EOF)
      return EOF;

    if (e < entry->fields.offc - 1)
      fputs (output_delimiter, output);

    s += entry->fields.offv[e] + 1;
  }

  return putc ('\n', output);
}


/*
 * input/output filter
 */

/* build entries */
struct entry *
process_input (FILE *input, const char *delimiter, struct entry *cur, 
               size_t *max_fields)
{
  char   *buf = NULL;  /* buffer for getline */
  size_t  bufsize = 0; /* size of buffer */
  ssize_t nread;       /* number of chars read */

  while ((nread = getline (&buf, &bufsize, input)) > 0)
  {
    size_t n = 0; /* number of fields in last line read */

    buf[--nread] = '\0'; /* erase newline */

    cur->next = entry_alloc (*max_fields);
    cur = cur->next;

    if ((n = entry_fill (cur, buf, nread, delimiter)) > *max_fields)
      *max_fields = n;

    buf = NULL;
  }

  return cur;
}

/* build entries, reading input from filename */
struct entry *
process_file (const char *filename, const char *delimiter, struct entry *cur, 
              size_t *max_fields)
{
  FILE *input = NULL;

  /* special case stdin */
  if (strcmp (filename, "-") == 0)
    return process_input (stdin, delimiter, cur, max_fields);

  /* open the file, process it, and close it */
  if ((input = fopen (filename, "r")) == NULL)
    error (EXIT_FAILURE, errno, "%s", filename);

  cur = process_input (input, delimiter, cur, max_fields);

  if (ferror (input))
    error (EXIT_FAILURE, errno, "%s", filename);

  if (fclose (input) == EOF)
    error (EXIT_FAILURE, errno, _("closing %s"), filename);

  return cur;
}

/* print the entries with appropriate spacing */
int
produce_output (FILE *output, struct entry *head, size_t max_fields, 
                const char *output_delimiter, int equal_width, 
                struct sizevec *min_widths, struct sizevec *max_widths, char filler,
                struct sizevec *lpadding, struct sizevec *rpadding, char padder,
                struct sizevec *align, int debug)
{
  struct sizevec *widths; /* vector of field widths */
  struct entry   *cur;    /* current entry */

  /* compute the field widths */
  widths = sizevec_alloc (equal_width ? 1 : max_fields);

  if (min_widths)
    sizevec_copy (widths, min_widths);

  for (cur = head->next; cur; cur = cur->next)
    entry_update_widths (cur, widths);

  if (max_widths)
    sizevec_min (widths, max_widths);

  /* print debugging information */
  if (debug)
  {
    if (fprintf (stderr, "widths = ") == EOF ||
        sizevec_print (stderr, widths) == EOF)
      error (EXIT_FAILURE, 0, _("write error"));
  }

  /* print the entries */
  for (cur = head->next; cur; cur = cur->next)
    if ((entry_print (output, cur, output_delimiter, widths, filler, 
                      lpadding, rpadding, padder, align)) == EOF)
      error (EXIT_FAILURE, 0, _("write error"));

  sizevec_free (widths);

  return 0;
}


/*
 * main program
 */

/* this function is called from main, with the non-option arguments
   and options passed as parameters */
int
do_main (int argc, char **argv, const char *output, const char *delimiter,
         const char *output_delimiter, int equal_width, 
         struct sizevec *min_widths, struct sizevec *max_widths, char filler,
         struct sizevec *lpadding, struct sizevec *rpadding, char padder, 
         struct sizevec *align, int debug)
{
  size_t        max_fields = DEFAULT_FIELDS; /* greatest number of fields */
  struct entry *head;                        /* list head for entries */
  FILE         *stream;

  /* initialise */
  head = entry_alloc (0); 

  if (output == NULL || (strcmp (output, "-")) == 0 )
    stream = stdout;
  else if ((stream = fopen (output, "w")) == NULL)
    error (EXIT_FAILURE, errno, "%s", output);

  /* process input */
  if (argc)
  {
    int i = 0;
    struct entry *cur = head;
    do 
    {
      cur = process_file (argv[i++], delimiter, cur, &max_fields);
    } while (i < argc);
  }
  else
  {
    (void) process_input (stdin, delimiter, head, &max_fields);

    if (ferror (stdin))
      error (EXIT_FAILURE, errno, _("read error"));
  }

  /* produce output */
  (void) produce_output (stream, head, max_fields, output_delimiter, 
                         equal_width, min_widths, max_widths, filler,
                         lpadding, rpadding, padder, 
                         align, debug);

  /* clean up */
  if (stream != stdout)
    if (fclose (stream) == EOF)
      error (EXIT_FAILURE, errno, _("closing %s"), output);

  entry_free_all (head);

  return 0;
}

/* Parse ALIGN_SPEC and return the result in a sizevec, allocated with
   malloc. */
struct sizevec *
get_align (char *align_spec)
{
  char *s = align_spec;
  struct sizevec *align;
  size_t n;

  /* allocate an array one bigger than the number of separators */
  for (n = 0; (s = strchr (s, ',')) != NULL; ++n, ++s)
    ;
  align = sizevec_alloc (n + 1);

  /* parse the width spec */
  n = 0;
  for (s = align_spec; *s; ++s)
  {
    switch (*s)
    {
      case 'l': case 'L':
        align->offv[n++] = ALIGN_LEFT;
        break;

      case 'c': case 'C':
        align->offv[n++] = ALIGN_CENTER;
        break;

      case 'r': case 'R':
        align->offv[n++] = ALIGN_RIGHT;
        break;

      case ',':
        /* skip delimiter */
        break;

      default:
        error (EXIT_FAILURE, 0, 
               _("invalid alignment: value #%d, char %d in `%s'"), 
               n + 1, (s - align_spec), align_spec);
    }
  }

  return align;
}

/* Parse S as a size_t, assigning NEXT to first invalid
   character. Return the size_t, or -1 on error. */
long
get_size (char *s, char **next)
{
  long rv;

  *next = s;
  errno = 0;
  rv = strtol (s, next, 0);

  if ((rv == 0 && s == *next) /* no valid characters */
     || (errno) /* underflow/overflow */
     || (rv < 0)) /* negative */
    return -1;

  return rv;
}

/* Parse SVREP and return a sizevec allocated with malloc. */
struct sizevec *
get_sizevec (char *svrep)
{
  char *s = svrep;
  char *next = s;
  struct sizevec *widths;
  size_t n;

  /* allocate an array one bigger than the number of separators */
  for (n = 0; (s = strchr (s, ',')) != NULL; ++n, ++s)
    ;
  widths = sizevec_alloc (n + 1);

  /* parse the width spec */
  n = 0;
  for (s = svrep; ; s = ++next)
  {
    if ((widths->offv[n++] = get_size (s, &next)) == -1)
      error (EXIT_FAILURE, errno, 
             _("invalid width: value #%d, char %d in `%s'"), 
             n + 1, (s - svrep), svrep);

    /* done */
    if (*next == '\0')
      break;

    /* invalid separator */
    if (*next != ',')
      error (EXIT_FAILURE, 0, _("invalid width: value #%d, char %d in `%s'"), 
             n + 1, (next - svrep), svrep);
  } 

  return widths;
}

/* Parse WIDTHS_SPEC and return the result in PMINV and PMAXV.

   width_spec  ::=  width_item ( `,' width_item )*       
   width_item  ::=  ( num )? ( `-' | `-' num )? 
*/
void
get_widths (char *widths_spec, struct sizevec **pminv, struct sizevec **pmaxv)
{
  struct sizevec *minv, *maxv;
  char *s = widths_spec;
  char *next = s;
  size_t n;

  /* allocate the sizevecs */
  for (n = 0; (s = strchr (s, ',')) != NULL; ++n, ++s)
    ;

  minv = *pminv = sizevec_alloc (n + 1);
  maxv = *pmaxv = sizevec_alloc (n + 1);

  for (n = 0, s = widths_spec; ; ++n, ++s)
  {
    maxv->offv[n] = SIZE_MAX;

    /* read LHS */
    if (*s != ',' && *s != '\0' && *s != '-')
    {
      if ((minv->offv[n] = maxv->offv[n] = get_size (s, &next)) == -1)
        error (EXIT_FAILURE, errno, 
               _("invalid width: value #%d, char %d in `%s'"), 
               n + 1, (s - widths_spec), widths_spec);

      s = next;
    }

    /* read RHS */
    if (*s == '-')
    {
      ++s;

      if (*s == ',' || *s == '\0')
      {
        maxv->offv[n] = SIZE_MAX;
      }
      else
      {
        if ((maxv->offv[n] = get_size (s, &next)) == -1)
          error (EXIT_FAILURE, errno, 
                 _("invalid width: value #%d, char %d in `%s'"), 
                 n + 1, (s - widths_spec), widths_spec);

        s = next;
      }
    }

    /* done */
    if (*s == '\0')
      break;

    /* invalid separator */
    if (*s != ',')
      error (EXIT_FAILURE, 0, _("invalid width: value #%d, char %d in `%s'"), 
             n + 1, (s - widths_spec), widths_spec);
  } 
}

/* The main function parses the command line and calls do_main. */
int
main (int argc, char **argv)
{
  int             opt_debug = 0;         /* debug flag */
  struct sizevec *opt_align = NULL;      /* alignment of fields */
  struct sizevec *opt_lpadding = NULL;   /* left padding */
  struct sizevec *opt_rpadding = NULL;   /* right padding */
  char           *opt_padder = NULL;     /* padding character */
  struct sizevec *opt_min_widths = NULL; /* set of minimum widths */
  struct sizevec *opt_max_widths = NULL; /* set of maximum widths */
  int             opt_equal_width = 0;   /* use equal width for all columns */
  char           *opt_filler = NULL;     /* fill character */
  const char     *opt_output = NULL;     /* output file */
  char           *opt_delimiter = NULL;  /* set of field delimiters */
  char           *opt_output_delimiter = NULL; /* output field delimiter */
  int             free_delimiter = 0;
  int             free_output_delimiter = 0;
  int             free_filler = 0;
  int             free_padder = 0;
  int             exit_status = EXIT_SUCCESS;

  /* initialise */
  program_name = base_name (argv[0]);
  version = _("\
%s (%s) %s - format tabular data\n\
Written by Claudio Jolowicz <claudio@jolowicz.com>.\n\
\n\
Copyright (c) 2005 Claudio Jolowicz.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\
");
  usage = _("\
Usage: %s [OPTION]... [FILE]...\n\
Format tabular data producing constant-width columns.\n\
If no FILE or if FILE is `-', read standard input.\n\
\n\
Mandatory arguments to long options are mandatory for short options too.\n\
  -w, --widths=WLIST            use WLIST as field widths\n\
  -e, --equal-width             use equal width for all fields\n\
  -f, --filler=CHAR             use CHAR to fill fields [SPC]\n\
  -a, --align=ALIST             align fields according to LIST\n\
  -p, --padding=LIST            use LIST as padding [none]\n\
  -l, --left-padding=PLIST      use PLIST as left padding\n\
  -r, --right-padding=PLIST     use PLIST as right padding\n\
  -P, --padder=CHAR             use CHAR as padding character [SPC]\n\
  -d, --delimiter=DELIM         set of field delimiters [TAB,SPC]\n\
  -D, --output-delimiter=DELIM  output field delimiter [TAB]\n\
  -o, --output=FILE             write output to FILE [stdout]\n\
      --debug                   print debugging information\n\
  -h, --help                    display this help and exit\n\
  -V, --version                 output version information and exit\n\
\n\
WLIST is a comma-separated list of nonnegative numbers or ranges. More\n\
precisely, an item can be one of `NUM', `NUM-', `-NUM', and `NUM-NUM',\n\
where NUM is a nonnegative number, or it can be empty.  ALIST is a\n\
comma-separated list of alignments, which may be one of `l' (left),\n\
`r' (right) and `c' (center).  PLIST is a comma-separated list of\n\
nonnegative numbers.  If the number of fields exceeds the number of\n\
list elements, the last element counts for all remaining fields.\n\
\n\
The following escape sequences are recognized for the -dfPD options:\n\
\n\
  \\NNN   the character whose ASCII code is NNN (octal)\n\
  \\\\     backslash\n\
  \\a     alert (BEL)\n\
  \\b     backspace\n\
  \\f     form feed\n\
  \\n     new line\n\
  \\r     carriage return\n\
  \\t     horizontal tab\n\
  \\v     vertical tab\n\
\n\
If the IFS environment variable is set, its value will be used as the\n\
set of input field separators, unless overridden with the -d option.\n\
\n\
Report bugs to %s.\n\
");

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE_NAME, LOCALEDIR);
  textdomain (PACKAGE_NAME);
#endif

  /* command line */
  while (1)
  {
    static const char *short_options = "a:l:r:p:P:w:ef:o:d:D:hV";
    static struct option long_options[] = 
    {
      { "align", required_argument, 0, 'a' },
      { "left-padding", required_argument, 0, 'l' },
      { "right-padding", required_argument, 0, 'r' },
      { "padding", required_argument, 0, 'p' },
      { "padder", required_argument, 0, 'P' },
      { "widths", required_argument, 0, 'w' },
      { "equal-width", no_argument, 0, 'e' },
      { "filler", required_argument, 0, 'f' },
      { "output", required_argument, 0, 'o' },
      { "delimiter", required_argument, 0, 'd' },
      { "output-delimiter", required_argument, 0, 'D' },
      { "debug", no_argument, 0, OPTION_DEBUG },
      { "help", no_argument, 0, 'h' },
      { "version", no_argument, 0, 'V' },
      { 0, 0, 0, 0 }
    };
    int option_index = 0;
    int c = getopt_long (argc, argv, short_options, long_options, &option_index);

    if (c == -1)
      break;

    switch (c)
    {
      case 'a':
        opt_align = get_align (optarg);
        break;

      case 'l':
        opt_lpadding = get_sizevec (optarg);
        break;

      case 'r':
        opt_rpadding = get_sizevec (optarg);
        break;

      case 'p':
        opt_lpadding = get_sizevec (optarg);
        opt_rpadding = sizevec_clone (opt_lpadding);
        break;

      case 'P':
        opt_padder = str_unesca (optarg);
        free_padder = 1;
        break;

      case 'w':
        get_widths (optarg, &opt_min_widths, &opt_max_widths);
        break;

      case 'e':
        opt_equal_width = 1;
        break;

      case 'f':
        opt_filler = str_unesca (optarg);
        free_filler = 1;
        break;

      case 'o':
        opt_output = optarg;
        break;

      case 'd':
        opt_delimiter = str_unesca (optarg);
        free_delimiter = 1;
        break;

      case 'D':
        opt_output_delimiter = str_unesca (optarg);
        free_output_delimiter = 1;
        break;

      case OPTION_DEBUG:
        opt_debug = 1;
        break;

      case 'h':
        printf (usage, program_name, PACKAGE_BUGREPORT);
        exit (0);

      case 'V':
        printf (version, program_name, PACKAGE_NAME, VERSION);
        exit (0);

      case '?':
        fprintf (stderr, _("Try `%s --help' for more information.\n"), program_name);
        exit (EXIT_FAILURE);

      default:
        abort ();
    }
  }

  argc -= optind;
  argv += optind;

  /* apply defaults */
  if (! opt_delimiter)
    if ((opt_delimiter = getenv ("IFS")) == NULL)
      opt_delimiter = DEFAULT_DELIMITER;

  if (! opt_output_delimiter)
    opt_output_delimiter = DEFAULT_OUTPUT_DELIMITER;
    
  if (! opt_filler)
    opt_filler = DEFAULT_FILLER;

  if (! opt_lpadding)
    opt_lpadding = get_sizevec (DEFAULT_PADDING);

  if (! opt_rpadding)
    opt_rpadding = get_sizevec (DEFAULT_PADDING);

  if (! opt_padder || ! *opt_padder)
    opt_padder = DEFAULT_PADDER;

  if (! opt_align)
    opt_align = get_align (DEFAULT_ALIGN);

  /* do the actual work */
  exit_status = do_main (argc, argv, opt_output, opt_delimiter, 
                         opt_output_delimiter, opt_equal_width, 
                         opt_min_widths, opt_max_widths, opt_filler[0], 
                         opt_lpadding, opt_rpadding, opt_padder[0], opt_align, 
                         opt_debug);

  /* clean up */
  if (free_padder)
    safe_free (opt_padder);

  if (free_filler)
    safe_free (opt_filler);

  if (free_delimiter)
    safe_free (opt_delimiter);

  if (free_output_delimiter)
    safe_free (opt_output_delimiter);

  if (opt_min_widths)
    sizevec_free (opt_min_widths);

  if (opt_max_widths)
    sizevec_free (opt_max_widths);

  sizevec_free (opt_lpadding);
  sizevec_free (opt_rpadding);

  return exit_status;
}


/*
 * Local Variables:
 * mode:c
 * tab-width:2
 * indent-tabs-mode:nil
 * End:
 */
