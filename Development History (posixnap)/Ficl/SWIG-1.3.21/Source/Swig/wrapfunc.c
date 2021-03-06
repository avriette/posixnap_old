/* ----------------------------------------------------------------------------- 
 * wrapfunc.c
 *
 *     This file defines a object for creating wrapper functions.  Primarily
 *     this is used for convenience since it allows pieces of a wrapper function
 *     to be created in a piecemeal manner.
 * 
 * Author(s) : David Beazley (beazley@cs.uchicago.edu)
 *
 * Copyright (C) 1998-2000.  The University of Chicago
 * Copyright (C) 1995-1998.  The University of Utah and The Regents of the
 *                           University of California.
 *
 * See the file LICENSE for information on usage and redistribution.	
 * ----------------------------------------------------------------------------- */

char cvsroot_wrapfunc_c[] = "$Header: /home/jabba/convert/cvs/Ficl/SWIG-1.3.21/Source/Swig/wrapfunc.c,v 1.1 2004-02-01 04:05:41 alex Exp $";

#include "swig.h"
#include <ctype.h>

static int Compact_mode = 0;   /* set to 0 on default */
static int Max_line_size = 128;

/* -----------------------------------------------------------------------------
 * NewWrapper()
 *
 * Create a new wrapper function object.
 * ----------------------------------------------------------------------------- */

Wrapper *
NewWrapper() {
  Wrapper *w;
  w = (Wrapper *) malloc(sizeof(Wrapper));
  w->localh = NewHash();
  w->locals = NewString("");
  w->code = NewString("");
  w->def = NewString("");
  return w;
}

/* -----------------------------------------------------------------------------
 * DelWrapper()
 *
 * Delete a wrapper function object.
 * ----------------------------------------------------------------------------- */

void
DelWrapper(Wrapper *w) {
  Delete(w->localh);
  Delete(w->locals);
  Delete(w->code);
  Delete(w->def);
  free(w);
}

/* -----------------------------------------------------------------------------
 * Wrapper_compact_print_mode_set()
 *
 * Set compact_mode.
 * ----------------------------------------------------------------------------- */

void 
Wrapper_compact_print_mode_set(int flag) {
  Compact_mode = flag;
}

/* -----------------------------------------------------------------------------
 * Wrapper_pretty_print()
 *
 * Formats a wrapper function and fixes up the indentation.
 * ----------------------------------------------------------------------------- */

void 
Wrapper_pretty_print(String *str, File *f) {
  String *ts;
  int level = 0;
  int c, i;
  int empty = 1;

  ts = NewString("");
  Seek(str,0, SEEK_SET);
  Clear(ts);
  while ((c = Getc(str)) != EOF) {
    if (c == '\"') {
      Putc(c,ts);
      while ((c = Getc(str)) != EOF) {
	if (c == '\\') {
	  Putc(c,ts);
	  c = Getc(str);
	}
	Putc(c,ts);
	if (c == '\"') break;
      }
      empty = 0;
    } else if (c == '\'') {
      Putc(c,ts);
      while ((c = Getc(str)) != EOF) {
	if (c == '\\') {
	  Putc(c,ts);
	  c = Getc(str);
	}
	Putc(c,ts);
	if (c == '\'') break;
      }
      empty = 0;
    } else if (c == '{') {
      Putc(c,ts);
      Putc('\n',ts);
      for (i = 0; i < level; i++) 
	Putc(' ',f);
      Printf(f,"%s", ts);
      Clear(ts);
      level+=4;
      while ((c = Getc(str)) != EOF) {
	if (!isspace(c)) {
	  Ungetc(c,str);
	  break;
	}
      }
      empty = 0;
    } else if (c == '}') {
      if (!empty) {
	Putc('\n',ts);
	for (i = 0; i < level; i++)
	  Putc(' ',f);
	Printf(f,"%s",ts);
	Clear(ts);
      }
      level-=4;
      Putc(c,ts);
      empty = 0;
    } else if (c == '\n') {
      Putc(c,ts);
      empty = 0;
      if (!empty) {
	if ((Char(ts))[0] != '#') {
	  for (i = 0; i < level; i++)
	    Putc(' ',f);
	}
	Printf(f,"%s",ts);
      }
      Clear(ts);
      empty = 1;
    } else if (c == '/') {
      empty = 0;
      Putc(c,ts);
      c = Getc(str);
      if (c != EOF) {
	Putc(c,ts);
	if (c == '/') {         /* C++ comment */
	  while ((c = Getc(str)) != EOF) {
	    if (c == '\n') {
	      Ungetc(c,str);
	      break;
	    }
	    Putc(c,ts);
	  }
	} else if (c == '*') {  /* C comment */
          int endstar = 0;
	  while ((c = Getc(str)) != EOF) {
	    if (endstar && c == '/') {  /* end of C comment */
	      Putc(c,ts);
	      break;
	    }
            endstar = (c == '*');
	    Putc(c,ts);
            if (c == '\n') { /* multi-line C comment. Could be improved slightly. */
              for (i = 0; i < level; i++)
	        Putc(' ',ts);
            }
	  }
        }
      }
    } else {
      if (!empty || !isspace(c)) {
	Putc(c,ts);
	empty = 0;
      }
    }
  }
  if (!empty) Printf(f,"%s",ts);
  Delete(ts);
  Printf(f,"\n");
}

/* -----------------------------------------------------------------------------
 * Wrapper_compact_print()
 *
 * Formats a wrapper function and fixes up the indentation.
 * Print out in compact format, with Compact enabled.
 * ----------------------------------------------------------------------------- */

void 
Wrapper_compact_print(String *str, File *f) {
  String *ts, *tf; /*temp string & temp file */
  int level = 0;
  int c, i;
  int empty = 1;

  ts = NewString("");
  tf = NewString("");
  Seek(str,0, SEEK_SET);
  Clear(ts);
  Clear(tf);

  while ((c = Getc(str)) != EOF) {
    if (c == '\"') { /* string 1 */
      empty = 0;
      Putc(c,ts);
      while ((c = Getc(str)) != EOF) {
	if (c == '\\') {
	  Putc(c,ts);
	  c = Getc(str);
	}
	Putc(c,ts);
	if (c == '\"') break;
      }
    } else if (c == '\'') { /* string 2 */
      empty = 0;
      Putc(c,ts);
      while ((c = Getc(str)) != EOF) {
	if (c == '\\') {
	  Putc(c,ts);
	  c = Getc(str);
	}
	Putc(c,ts);
	if (c == '\'') break;
      }
    } else if (c == '{') { /* start of {...} */
      empty = 0;
      Putc(c,ts);
      if (Len(tf) == 0) {
	for (i = 0; i < level; i++) 
	  Putc(' ',tf);
      } else if ((Len(tf) + Len(ts)) < Max_line_size) {
	Putc(' ',tf);
      } else {
	Putc('\n',tf);
	Printf(f,"%s", tf);
	Clear(tf);
	for (i = 0; i < level; i++) 
	  Putc(' ',tf);
      }
      Printf(tf,"%s",ts);
      Clear(ts);
      level+=4;
      while ((c = Getc(str)) != EOF) {
	if (!isspace(c)) {
	  Ungetc(c,str);
	  break;
	}
      }
    } else if (c == '}') { /* end of {...} */
      empty = 0;
      if (Len(tf) == 0) {
	for (i = 0; i < level; i++) 
	  Putc(' ',tf);
      } else if ((Len(tf) + Len(ts)) < Max_line_size) {
	Putc(' ',tf);
      } else {
	Putc('\n',tf);
	Printf(f,"%s", tf);
	Clear(tf);
	for (i = 0; i < level; i++) 
	  Putc(' ',tf);
      }
      Printf(tf, "%s", ts);
      Putc(c, tf);
      Clear(ts);
      level-=4;
    } else if (c == '\n') { /* line end */
      while ((c = Getc(str)) != EOF) {
	if (!isspace(c))
	  break;
      }
      if (c == '#'){
	Putc('\n',ts);
      } else if (c == '}') {
	Putc(' ',ts);
      } else if ( (c != EOF) || (Len(ts)!=0) ){
	if (Len(tf) == 0) {
	  for (i = 0; i < level; i++) 
	    Putc(' ',tf);
	} else if ((Len(tf) + Len(ts)) < Max_line_size) {
	  Putc(' ',tf);
	} else {
	  Putc('\n',tf);
	  Printf(f,"%s", tf);
	  Clear(tf);
	  for (i = 0; i < level; i++) 
	    Putc(' ',tf);
	}
	Printf(tf,"%s",ts);
	Clear(ts);
      }
      Ungetc(c,str);

      empty = 1;
    } else if (c == '/') { /* comment */
      empty = 0;
      c = Getc(str);
      if (c != EOF) {
	if (c == '/') {         /* C++ comment */
	  while ((c = Getc(str)) != EOF) {
	    if (c == '\n') {
	      Ungetc(c,str);
	      break;
	    }
	  }
	} else if (c == '*') {  /* C comment */
	  int endstar = 0;
	  while ((c = Getc(str)) != EOF) {
	    if (endstar && c == '/') {  /* end of C comment */
	      break;
	    }
            endstar = (c == '*');
	  }
        } else {
	  Putc('/',ts);
	  Putc(c,ts);
	}
      }
    } else if (c == '#') { /* Preprocessor line */
      Putc('#', ts);
      while ((c = Getc(str)) != EOF) {
	Putc(c, ts);
	if (c == '\\') { /* Continued line of the same PP */
	  c = Getc(str);
	  if (c == '\n')
	    Putc(c, ts);
	  else
	    Ungetc(c, str);
	} else if (c == '\n') 
	  break;
      }
      if (!empty) {
	Printf(tf,"\n");
      }
      Printf(tf,"%s",ts);
      Printf(f, "%s", tf);
      Clear(tf);
      Clear(ts);
      for (i = 0; i < level; i++) 
        Putc(' ',tf);
      empty = 1;
    } else {
      if (!empty || !isspace(c)) {
	Putc(c,ts);
	empty = 0;
      }
    }
  }
  if (!empty) {
    Printf(tf,"%s",ts);
  }
  if (Len(tf) != 0)
    Printf(f,"%s",tf);
  Delete(ts);
  Delete(tf);
  Printf(f,"\n");
}

/* -----------------------------------------------------------------------------
 * Wrapper_print()
 *
 * Print out a wrapper function.  Does pretty or compact printing as well.
 * ----------------------------------------------------------------------------- */

void 
Wrapper_print(Wrapper *w, File *f) {
  String *str;

  str = NewString("");
  Printf(str,"%s\n", w->def);
  Printf(str,"%s\n", w->locals);
  Printf(str,"%s\n", w->code);
  if (Compact_mode == 1)
    Wrapper_compact_print(str,f);
  else
    Wrapper_pretty_print(str,f);
}

/* -----------------------------------------------------------------------------
 * Wrapper_add_local()
 *
 * Adds a new local variable declaration to a function. Returns -1 if already
 * present (which may or may not be okay to the caller).
 * ----------------------------------------------------------------------------- */

int
Wrapper_add_local(Wrapper *w, const String_or_char *name, const String_or_char *decl) {
  /* See if the local has already been declared */
  if (Getattr(w->localh,name)) {
    return -1;
  }
  Setattr(w->localh,name,decl);
  Printf(w->locals,"%s;\n", decl);
  return 0;
}

/* -----------------------------------------------------------------------------
 * Wrapper_add_localv()
 *
 * Same as add_local(), but allows a NULL terminated list of strings to be
 * used as a replacement for decl.   This saves the caller the trouble of having
 * to manually construct the 'decl' string before calling.
 * ----------------------------------------------------------------------------- */

int
Wrapper_add_localv(Wrapper *w, const String_or_char *name, ...) {
  va_list ap;
  int     ret;
  String *decl;
  DOH       *obj;
  decl = NewString("");
  va_start(ap,name);

  obj = va_arg(ap,void *);
  while (obj) {
    Printv(decl,obj,NIL);
    Putc(' ', decl);
    obj = va_arg(ap, void *);
  }
  va_end(ap);

  ret = Wrapper_add_local(w,name,decl);
  Delete(decl);
  return ret;
}

/* -----------------------------------------------------------------------------
 * Wrapper_check_local()
 *
 * Check to see if a local name has already been declared
 * ----------------------------------------------------------------------------- */

int
Wrapper_check_local(Wrapper *w, const String_or_char *name) {
  if (Getattr(w->localh,name)) {
    return 1;
  }
  return 0;
}

/* ----------------------------------------------------------------------------- 
 * Wrapper_new_local()
 *
 * Adds a new local variable with a guarantee that a unique local name will be
 * used.  Returns the name that was actually selected.
 * ----------------------------------------------------------------------------- */

char *
Wrapper_new_local(Wrapper *w, const String_or_char *name, const String_or_char *decl) {
  int i;
  String *nname = NewString(name);
  String *ndecl = NewString(decl);
  char      *ret;

  i = 0;

  while (Wrapper_check_local(w,nname)) {
    Clear(nname);
    Printf(nname,"%s%d",name,i);
    i++;
  }
  Replace(ndecl, name, nname, DOH_REPLACE_ID);
  Setattr(w->localh,nname,ndecl);
  Printf(w->locals,"%s;\n", ndecl);
  ret = Char(nname);
  Delete(nname);
  Delete(ndecl);
  return ret;      /* Note: nname should still exists in the w->localh hash */
}


/* -----------------------------------------------------------------------------
 * Wrapper_add_localv()
 *
 * Same as add_local(), but allows a NULL terminated list of strings to be
 * used as a replacement for decl.   This saves the caller the trouble of having
 * to manually construct the 'decl' string before calling.
 * ----------------------------------------------------------------------------- */

char *
Wrapper_new_localv(Wrapper *w, const String_or_char *name, ...) {
  va_list ap;
  char *ret;
  String *decl;
  DOH       *obj;
  decl = NewString("");
  va_start(ap,name);

  obj = va_arg(ap,void *);
  while (obj) {
    Printv(decl,obj,NIL);
    Putc(' ',decl);
    obj = va_arg(ap, void *);
  }
  va_end(ap);

  ret = Wrapper_new_local(w,name,decl);
  Delete(decl);
  return ret;
}






