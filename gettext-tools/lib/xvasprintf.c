/* vasprintf and asprintf with out-of-memory checking.
   Copyright (C) 1999, 2002-2004, 2006 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* Specification.  */
#include "xvasprintf.h"

#include <errno.h>
#include <limits.h>
#include <string.h>

#include "vasprintf.h"
#include "xalloc.h"

/* Checked size_t computations.  */
#include "xsize.h"

/* Some systems, like OSF/1 4.0 and Woe32, don't have EOVERFLOW.  */
#ifndef EOVERFLOW
# define EOVERFLOW E2BIG
#endif

static inline char *
xstrcat (size_t argcount, va_list args)
{
  char *result;
  va_list ap;
  size_t totalsize;
  size_t i;
  char *p;

  /* Determine the total size.  */
  totalsize = 0;
  va_copy (ap, args);
  for (i = argcount; i > 0; i--)
    {
      const char *next = va_arg (ap, const char *);
      totalsize = xsum (totalsize, strlen (next));
    }

  /* Don't return a string longer than INT_MAX, for consistency with
     vasprintf().  */
  if (totalsize > INT_MAX)
    {
      errno = EOVERFLOW;
      return NULL;
    }

  /* Allocate and fill the result string.  */
  result = (char *) xmalloc (totalsize + 1);
  p = result;
  va_copy (ap, args);
  for (i = argcount; i > 0; i--)
    {
      const char *next = va_arg (ap, const char *);
      size_t len = strlen (next);
      memcpy (p, next, len);
      p += len;
    }
  *p = '\0';

  return result;
}

char *
xvasprintf (const char *format, va_list args)
{
  char *result;

  /* Recognize the special case format = "%s...%s".  It is a frequently used
     idiom for string concatenation and needs to be fast.  We don't want to
     have a separate function xstrcat() for this purpose.  */
  {
    size_t argcount = 0;
    const char *f;

    for (f = format;;)
      {
	if (*f == '\0')
	  /* Recognized the special case of string concatenation.  */
	  return xstrcat (argcount, args);
	if (*f != '%')
	  break;
	f++;
	if (*f != 's')
	  break;
	f++;
	argcount++;
      }
  }

  if (vasprintf (&result, format, args) < 0)
    {
      if (errno == ENOMEM)
	xalloc_die ();
      return NULL;
    }

  return result;
}