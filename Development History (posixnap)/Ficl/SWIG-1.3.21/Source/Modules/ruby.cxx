/********************************************************************
 * Ruby module for SWIG
 *
 * $Header: /home/jabba/convert/cvs/Ficl/SWIG-1.3.21/Source/Modules/ruby.cxx,v 1.1 2004-02-01 04:05:29 alex Exp $
 *
 * Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
 * Copyright (C) 2000  Information-technology Promotion Agency, Japan
 *
 * Masaki Fukushima
 *
 ********************************************************************/

char cvsroot_ruby_cxx[] = "$Header: /home/jabba/convert/cvs/Ficl/SWIG-1.3.21/Source/Modules/ruby.cxx,v 1.1 2004-02-01 04:05:29 alex Exp $";

#include "swigmod.h"
#define SWIG_PROTECTED_TARGET_METHODS 1

#ifndef MACSWIG
#include "swigconfig.h"
#endif

#include <ctype.h>
#include <string.h>
#include <limits.h> /* for INT_MAX */

 class RClass {
 private:
  String *temp;
 public:
  String *name;    /* class name (renamed) */
  String *cname;   /* original C class/struct name */
  String *mname;   /* Mangled name */
  
  /**
   * The C variable name used in the SWIG-generated wrapper code to refer to
   * this class; usually it is of the form "cClassName.klass", where cClassName
   * is a swig_class struct instance and klass is a member of that struct.
   */
  String *vname;

  /**
   * The C variable name used in the SWIG-generated wrapper code to refer to
   * the module that implements this class's methods (when we're trying to
   * support C++ multiple inheritance). Usually it is of the form
   * "cClassName.mImpl", where cClassName is a swig_class struct instance
   * and mImpl is a member of that struct.
   */
  String *mImpl;

  String *type;
  String *prefix;
  String *header;
  String *init;

  int constructor_defined;
  int destructor_defined;

  RClass() {
    temp = NewString("");
    name = NewString("");
    cname = NewString("");
    mname = NewString("");
    vname = NewString("");
    mImpl = NewString("");
    type = NewString("");
    prefix = NewString("");
    header = NewString("");
    init = NewString("");
    constructor_defined = 0;
    destructor_defined = 0;
  }
  
  ~RClass() {
    Delete(name);
    Delete(cname);
    Delete(vname);
    Delete(mImpl);
    Delete(mname);
    Delete(type);
    Delete(prefix);
    Delete(header);
    Delete(init);
    Delete(temp);
  }

  void set_name(const String_or_char *cn, const String_or_char *rn, const String_or_char *valn) {
    /* Original C/C++ class (or struct) name */
    Clear(cname);
    Append(cname,cn);

    /* Mangled name */
    Delete(mname);
    mname = Swig_name_mangle(cname);

    /* Renamed class name */
    Clear(name);
    Append(name,valn);

    /* Variable name for the VALUE that refers to the Ruby Class object */
    Clear(vname);
    Printf(vname, "c%s.klass", name);
    
    /* Variable name for the VALUE that refers to the Ruby Class object */
    Clear(mImpl);
    Printf(mImpl, "c%s.mImpl", name);

    /* Prefix */
    Clear(prefix);
    Printv(prefix,(rn ? rn : cn), "_", NIL);
  }

  char *strip(const String_or_char *s) {
    Clear(temp);
    Append(temp, s);
    if (Strncmp(s, prefix, Len(prefix)) == 0) {
      Replaceall(temp,prefix,"");
    }
    return Char(temp);
  }
};


static const char *
usage = "\
Ruby Options (available with -ruby)\n\
     -ldflags        - Print runtime libraries to link with\n\
     -globalmodule   - Wrap everything into the global module\n\
     -minherit       - Attempt to support multiple inheritance\n\
     -feature <name> - Set feature name to <name> (used by `require')\n";


#define RCLASS(hash, name) (RClass*)(Getattr(hash, name) ? Data(Getattr(hash, name)) : 0)
#define SET_RCLASS(hash, name, klass) Setattr(hash, name, NewVoid(klass, 0))


class RUBY : public Language {
private:

  String *module;
  String *modvar;
  String *feature;
  int current;
  Hash *classes;		/* key=cname val=RClass */
  RClass *klass;		/* Currently processing class */
  Hash *special_methods;	/* Python style special method name table */

  File *f_directors;
  File *f_directors_h;
  File *f_runtime;
  File *f_runtime_h;
  File *f_header;
  File *f_wrappers;
  File *f_init;

  bool use_kw;
  bool useGlobalModule;
  bool multipleInheritance;

  // Wrap modes
  enum {
    NO_CPP,
    MEMBER_FUNC,
    CONSTRUCTOR_ALLOCATE,
    CONSTRUCTOR_INITIALIZE,
    DESTRUCTOR,
    MEMBER_VAR,
    CLASS_CONST,
    STATIC_FUNC,
    STATIC_VAR
  };

public:

  /* ---------------------------------------------------------------------
   * RUBY()
   *
   * Initialize member data
   * --------------------------------------------------------------------- */

  RUBY() {
    module = 0;
    modvar = 0;
    feature = 0;
    current = NO_CPP;
    classes = 0;
    klass = 0;
    special_methods = 0;
    f_runtime = 0;
    f_header = 0;
    f_wrappers = 0;
    f_init = 0;
    use_kw = false;
    useGlobalModule = false;
    multipleInheritance = false;
  }
  
  /* ---------------------------------------------------------------------
   * main()
   *
   * Parse command line options and initializes variables.
   * --------------------------------------------------------------------- */

  virtual void main(int argc, char *argv[]) {

    /* Set location of SWIG library */
    SWIG_library_directory("ruby");
    
    /* Look for certain command line options */
    for (int i = 1; i < argc; i++) {
      if (argv[i]) {
	if (strcmp(argv[i],"-feature") == 0) {
	  if (argv[i+1]) {
	    char *name = argv[i+1];
	    feature = NewString(name);
	    Swig_mark_arg(i);
	    Swig_mark_arg(i+1);
	    i++;
	  } else {
	    Swig_arg_error();
	  }
	} else if (strcmp(argv[i],"-globalmodule") == 0) {
          useGlobalModule = true;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i],"-minherit") == 0) {
          multipleInheritance = true;
	  Swig_mark_arg(i);
	} else if (strcmp(argv[i],"-help") == 0) {
	  Printf(stderr,"%s\n", usage);
	} else if (strcmp (argv[i],"-ldflags") == 0) {
	  printf("%s\n", SWIG_RUBY_RUNTIME);
	  SWIG_exit (EXIT_SUCCESS);
	}
      }
    }

    /* Add a symbol to the parser for conditional compilation */
    Preprocessor_define("SWIGRUBY 1", 0);
    
    /* Add typemap definitions */
    SWIG_typemap_lang("ruby");
    SWIG_config_file("ruby.swg");
    allow_overloading();
  }

  /**
   * Generate initialization code to define the Ruby module(s),
   * accounting for nested modules as necessary.
   */
  void defineRubyModule() {
    List *modules = Split(module,':',INT_MAX);
    if (modules != 0 && Len(modules) > 0) {
      String *mv = 0;
      Iterator m;
      m = First(modules);
      while (m.item) {
        if (Len(m.item) > 0) {
	  if (mv != 0) {
            Printv(f_init, tab4, modvar,
	      " = rb_define_module_under(", modvar, ", \"", m.item, "\");\n", NIL);
	  } else {
            Printv(f_init, tab4, modvar,
	      " = rb_define_module(\"", m.item, "\");\n", NIL);
	    mv = NewString(modvar);
	  }
	}
	m = Next(m);
      }
      Delete(mv);
      Delete(modules);
    }
  }

  void registerMagicMethods() {

    special_methods = NewHash();

    /* Python style special method name. */
    /* Basic */
    Setattr(special_methods, "__repr__", "inspect");
    Setattr(special_methods, "__str__", "to_s");
    Setattr(special_methods, "__cmp__", "<=>");
    Setattr(special_methods, "__hash__", "hash");
    Setattr(special_methods, "__nonzero__", "nonzero?");
  
    /* Callable */
    Setattr(special_methods, "__call__", "call");
  
    /* Collection */
    Setattr(special_methods, "__len__", "length");
    Setattr(special_methods, "__getitem__", "[]");
    Setattr(special_methods, "__setitem__", "[]=");
  
    /* Operators */
    Setattr(special_methods, "__add__", "+");
    Setattr(special_methods, "__pos__", "+@");
    Setattr(special_methods, "__sub__", "-");
    Setattr(special_methods, "__neg__", "-@");
    Setattr(special_methods, "__mul__", "*");
    Setattr(special_methods, "__div__", "/");
    Setattr(special_methods, "__mod__", "%");
    Setattr(special_methods, "__lshift__", "<<");
    Setattr(special_methods, "__rshift__", ">>");
    Setattr(special_methods, "__and__", "&");
    Setattr(special_methods, "__or__", "|");
    Setattr(special_methods, "__xor__", "^");
    Setattr(special_methods, "__invert__", "~");
    Setattr(special_methods, "__lt__", "<");
    Setattr(special_methods, "__le__", "<=");
    Setattr(special_methods, "__gt__", ">");
    Setattr(special_methods, "__ge__", ">=");
    Setattr(special_methods, "__eq__", "==");

    /* Other numeric */
    Setattr(special_methods, "__divmod__", "divmod");
    Setattr(special_methods, "__pow__", "**");
    Setattr(special_methods, "__abs__", "abs");
    Setattr(special_methods, "__int__", "to_i");
    Setattr(special_methods, "__float__", "to_f");
    Setattr(special_methods, "__coerce__", "coerce");
  }

  /* ---------------------------------------------------------------------
   * top()
   * --------------------------------------------------------------------- */

  virtual int top(Node *n) {

    /**
     * See if any Ruby module options have been specified as options
     * to the %module directive.
     */
    Node *swigModule = Getattr(n, "module");
    if (swigModule) {
      Node *options = Getattr(swigModule, "options");
      if (options) {
        if (Getattr(options, "directors")) {
          allow_directors();
        }
	if (Getattr(options, "dirprot")) {
          allow_dirprot();
	}
        if (Getattr(options, "ruby_globalmodule")) {
          useGlobalModule = true;
        }
        if (Getattr(options, "ruby_minherit")) {
          multipleInheritance = true;
        }
      }
    }

    /* Set comparison with none for ConstructorToFunction */
    setSubclassInstanceCheck(NewString("CLASS_OF(self) != Qnil")); // FIXME
    // setSubclassInstanceCheck(NewString("CLASS_OF(self) != cFoo.klass"));

    /* Initialize all of the output files */
    String *outfile = Getattr(n,"outfile");
    String *outfile_h = Getattr(n, "outfile_h");

    f_runtime = NewFile(outfile,"w");
    if (!f_runtime) {
      Printf(stderr,"*** Can't open '%s'\n", outfile);
      SWIG_exit(EXIT_FAILURE);
    }

    if (directorsEnabled()) {
      f_runtime_h = NewFile(outfile_h,"w");
      if (!f_runtime_h) {
        Printf(stderr,"*** Can't open '%s'\n", outfile_h);
        SWIG_exit(EXIT_FAILURE);
      }
    }

    f_init = NewString("");
    f_header = NewString("");
    f_wrappers = NewString("");
    f_directors_h = NewString("");
    f_directors = NewString("");

    /* Register file targets with the SWIG file handler */
    Swig_register_filebyname("header",f_header);
    Swig_register_filebyname("wrapper",f_wrappers);
    Swig_register_filebyname("runtime",f_runtime);
    Swig_register_filebyname("init",f_init);
    Swig_register_filebyname("director",f_directors);
    Swig_register_filebyname("director_h",f_directors_h);

    modvar = 0;
    current = NO_CPP;
    klass = 0;
    classes = NewHash();
    
    registerMagicMethods();

    Swig_banner(f_runtime);

    if (NoInclude) {
      Printf(f_runtime, "#define SWIG_NOINCLUDE\n");
    }

    if (directorsEnabled()) {
      Printf(f_runtime,"#define SWIG_DIRECTORS\n");
    }

    /* typedef void *VALUE */
    SwigType *value = NewSwigType(T_VOID);
    SwigType_add_pointer(value);
    SwigType_typedef(value,(char*)"VALUE");
    Delete(value);

    /* Set module name */
    set_module(Char(Getattr(n,"name")));

    if (directorsEnabled()) {
      Swig_banner(f_directors_h);
      Printf(f_directors_h, "#ifndef SWIG_%s_WRAP_H_\n", module);
      Printf(f_directors_h, "#define SWIG_%s_WRAP_H_\n\n", module);
      Printf(f_directors_h, "class Swig::Director;\n\n");
      Swig_insert_file("director.swg", f_directors);
      Printf(f_directors, "\n\n");
      Printf(f_directors, "/* ---------------------------------------------------\n");
      Printf(f_directors, " * C++ director class methods\n");
      Printf(f_directors, " * --------------------------------------------------- */\n\n");
      Printf(f_directors, "#include \"%s\"\n\n", Swig_file_filename(outfile_h));
    }

    Printf(f_header,"#define SWIG_init    Init_%s\n", feature);
    Printf(f_header,"#define SWIG_name    \"%s\"\n\n", module);
    Printf(f_header,"static VALUE %s;\n", modvar);

    /* Start generating the initialization function */
    Printv(f_init,
	   "\n",
	   "#ifdef __cplusplus\n",
	   "extern \"C\"\n",
	   "#endif\n",
	   "SWIGEXPORT(void) Init_", feature, "(void) {\n",
	   "int i;\n",
	   "\n",
	   NIL);

    Printv(f_init, tab4, "SWIG_InitRuntime();\n", NIL);

    if (!useGlobalModule)
      defineRubyModule();

    Printv(f_init,
	   "\n",
	   "for (i = 0; swig_types_initial[i]; i++) {\n",
	   "swig_types[i] = SWIG_TypeRegister(swig_types_initial[i]);\n",
	   "SWIG_define_class(swig_types[i]);\n",
	   "}\n",
	   NIL);
    Printf(f_init,"\n");

    Language::top(n);

    /* Finish off our init function */
    Printf(f_init,"}\n");
    SwigType_emit_type_table(f_runtime,f_wrappers);

    /* Close all of the files */
    Dump(f_header,f_runtime);

    if (directorsEnabled()) {
      Dump(f_directors, f_runtime);
      Dump(f_directors_h, f_runtime_h);
      Printf(f_runtime_h, "\n");
      Printf(f_runtime_h, "#endif\n");
      Close(f_runtime_h);
    }

    Dump(f_wrappers,f_runtime);
    Wrapper_pretty_print(f_init,f_runtime);

    Delete(f_header);
    Delete(f_wrappers);
    Delete(f_init);
    Close(f_runtime);
    Delete(f_runtime);

    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------------
   * importDirective()
   * ----------------------------------------------------------------------------- */
  
  virtual int importDirective(Node *n) {
    String *modname = Getattr(n,"module");
    if (modname) {
      Printf(f_init,"rb_require(\"%s\");\n", modname);
    }
    return Language::importDirective(n);
  }

  /* ---------------------------------------------------------------------
   * set_module(const char *mod_name)
   *
   * Sets the module name.  Does nothing if it's already set (so it can
   * be overridden as a command line option).
   *---------------------------------------------------------------------- */

  void set_module(const char *s) {
    String *mod_name = NewString(s);
    if (module == 0) {
      /* Start with the empty string */
      module = NewString("");
      
      /* Account for nested modules */
      List *modules = Split(mod_name,':',INT_MAX);
      if (modules != 0 && Len(modules) > 0) {
        String *last = 0;
	Iterator m = First(modules);
	while (m.item) {
	  if (Len(m.item) > 0) {
	    String *cap = NewString(m.item);
	    (Char(cap))[0] = toupper((Char(cap))[0]);
	    if (last != 0) {
	      Append(module, "::");
	    }
	    Append(module, cap);
	    last = m.item;
	  }
	  m = Next(m);
	}
	if (feature == 0) {
	  feature = Copy(last);
	}
	(Char(last))[0] = toupper((Char(last))[0]);
	modvar = NewStringf("m%s", last);
	Delete(modules);
      }
    }
    Delete(mod_name);
  }
  
  /* --------------------------------------------------------------------------
   * nativeWrapper()
   * -------------------------------------------------------------------------- */
  virtual int nativeWrapper(Node *n) {
    String *funcname = Getattr(n,"wrap:name");
    Swig_warning(WARN_LANG_NATIVE_UNIMPL, input_file, line_number, 
		 "Adding native function %s not supported (ignored).\n", funcname);
    return SWIG_NOWRAP;
  }

  /**
   * Process the comma-separated list of aliases (if any).
   */
  void defineAliases(Node *n, const String_or_char *iname) {
    String *aliasv = Getattr(n,"feature:alias");
    if (aliasv) {
      List *aliases = Split(aliasv,',',INT_MAX);
      if (aliases && Len(aliases) > 0) {
	Iterator alias = First(aliases);
	while (alias.item) {
          if (Len(alias.item) > 0) {
            Printv(klass->init, tab4, "rb_define_alias(", klass->vname, ", \"", alias.item, "\", \"", iname, "\");\n", NIL);
	  }
	  alias = Next(alias);
	}
      }
      Delete(aliases);
    }
  }
  
  /* ---------------------------------------------------------------------
   * create_command(Node *n, char *iname)
   *
   * Creates a new command from a C function.
   *              iname = Name of function in scripting language
   *
   * A note about what "protected" and "private" mean in Ruby:
   *
   * A private method is accessible only within the class or its subclasses,
   * and it is callable only in "function form", with 'self' (implicit or
   * explicit) as a receiver.
   *
   * A protected method is callable only from within its class, but unlike
   * a private method, it can be called with a receiver other than self, such
   * as another instance of the same class.
   * --------------------------------------------------------------------- */

  void create_command(Node *n, const String_or_char *iname) {

    String *alloc_func = Swig_name_wrapper(iname);
    String *wname = Swig_name_wrapper(iname);
    if (CPlusPlus) {
      Insert(wname,0,"VALUEFUNC(");
      Append(wname,")");
    }
    if (current != NO_CPP)
      iname = klass->strip(iname);
    if (Getattr(special_methods, iname)) {
      iname = GetChar(special_methods, iname);
    }
    
    String *s = NewString("");
    String *temp = NewString("");
    
    switch (current) {
    case MEMBER_FUNC: 
      {
#ifdef SWIG_PROTECTED_TARGET_METHODS
	const char* rb_define_method = is_protected(n) ?
	  "rb_define_protected_method" : "rb_define_method";
#else        
	const char* rb_define_method = "rb_define_method";
#endif	
	if (multipleInheritance) {
	  Printv(klass->init, tab4, rb_define_method,"(", klass->mImpl, ", \"",
		 iname, "\", ", wname, ", -1);\n", NIL);
	} else {
	  Printv(klass->init, tab4, rb_define_method, "(", klass->vname, ", \"",
		 iname, "\", ", wname, ", -1);\n", NIL);
	}
      }      
      break;
    case CONSTRUCTOR_ALLOCATE:
      Printv(s, tab4, "rb_define_alloc_func(", klass->vname, ", ", alloc_func, ");\n", NIL);
      Replaceall(klass->init,"$allocator", s);
      break;
    case CONSTRUCTOR_INITIALIZE:
      Printv(s, tab4, "rb_define_method(", klass->vname,
	     ", \"initialize\", ", wname, ", -1);\n", NIL);
      Replaceall(klass->init,"$initializer", s);
      break;
    case MEMBER_VAR:
      Append(temp,iname);
      Replaceall(temp,"_set", "=");
      Replaceall(temp,"_get", "");
      if (multipleInheritance) {
        Printv(klass->init, tab4, "rb_define_method(", klass->mImpl, ", \"",
               temp, "\", ", wname, ", -1);\n", NIL);
      } else {
        Printv(klass->init, tab4, "rb_define_method(", klass->vname, ", \"",
               temp, "\", ", wname, ", -1);\n", NIL);
      }
      break;
    case STATIC_FUNC:
      Printv(klass->init, tab4, "rb_define_singleton_method(", klass->vname,
	     ", \"", iname, "\", ", wname, ", -1);\n", NIL);
      break;
    case NO_CPP:
      if (!useGlobalModule) {
        Printv(s, tab4, "rb_define_module_function(", modvar, ", \"",
               iname, "\", ", wname, ", -1);\n",NIL);
        Printv(f_init,s,NIL);
      } else {
        Printv(s, tab4, "rb_define_global_function(\"",
               iname, "\", ", wname, ", -1);\n",NIL);
        Printv(f_init,s,NIL);
      }
      break;
    case DESTRUCTOR:
    case CLASS_CONST:
    case STATIC_VAR:
      assert(false); // Should not have gotten here for these types
    default:
      assert(false);
    }
    
    defineAliases(n, iname);

    Delete(temp);
    Delete(s);
    Delete(wname);
    Delete(alloc_func);
  }
  
  /* ---------------------------------------------------------------------
   * applyInputTypemap()
   *
   * Look up the appropriate "in" typemap for this parameter (p),
   * substitute the correct strings for the $target and $input typemap
   * parameters, and dump the resulting code to the wrapper file.
   * --------------------------------------------------------------------- */

  Parm *applyInputTypemap(Parm *p, String *ln, String *source, Wrapper *f) {
    String *tm;
    SwigType *pt = Getattr(p,"type");
    if ((tm = Getattr(p,"tmap:in"))) {
      Replaceall(tm,"$target",ln);
      Replaceall(tm,"$source",source);
      Replaceall(tm,"$input",source);
      Setattr(p,"emit:input",Copy(source));
      Printf(f->code,"%s\n", tm);
      p = Getattr(p,"tmap:in:next");
    } else {
      Swig_warning(WARN_TYPEMAP_IN_UNDEF, input_file, line_number,
        "Unable to use type %s as a function argument.\n", SwigType_str(pt,0));
      p = nextSibling(p);
    }
    return p;
  }

  Parm *skipIgnoredArgs(Parm *p) {
    while (checkAttribute(p,"tmap:in:numinputs","0")) {
      p = Getattr(p,"tmap:in:next");
    }
    return p;
  }

  /* ---------------------------------------------------------------------
   * marshalInputArgs()
   *
   * Process all of the arguments passed into the scripting language
   * method and convert them into C/C++ function arguments using the
   * supplied typemaps.
   * --------------------------------------------------------------------- */
  
  void marshalInputArgs(Node *n, ParmList *l, int numarg, int numreq, String *kwargs, bool allow_kwargs, Wrapper *f) {
    int i;
    Parm *p;
    String *tm;
    String *source;
    String *target;

    source = NewString("");
    target = NewString("");

    bool use_director = (current == CONSTRUCTOR_INITIALIZE && Swig_directorclass(n));

    /**
     * The 'start' value indicates which of the C/C++ function arguments
     * produced here corresponds to the first value in Ruby's argv[] array.
     * The value of start is either zero or one. If start is zero, then
     * the first argument (with name arg1) is based on the value of argv[0].
     * If start is one, then arg1 is based on the value of argv[1].
     */
    int start = (current == MEMBER_FUNC || current == MEMBER_VAR || use_director) ? 1 : 0;

    int varargs = emit_isvarargs(l);

    Printf(kwargs,"{ ");
    for (i = 0, p = l; i < numarg; i++) {

      p = skipIgnoredArgs(p);

      String *pn = Getattr(p,"name");
      String *ln = Getattr(p,"lname");

      /* Produce string representation of source argument */
      Clear(source);

      /* First argument is a special case */
      if (i == 0) {
        Printv(source, (start == 0) ? "argv[0]" : "self", NIL);
      } else {
        Printf(source,"argv[%d]",i-start);
      }

      /* Produce string representation of target argument */
      Clear(target);
      Printf(target,"%s",Char(ln));

      if (i >= (numreq)) { /* Check if parsing an optional argument */
	Printf(f->code,"    if (argc > %d) {\n", i -  start);
      }
      
      /* Record argument name for keyword argument handling */
      if (Len(pn)) {
        Printf(kwargs,"\"%s\",", pn);
      } else {
	Printf(kwargs,"\"arg%d\",", i+1);
      }

      /* Look for an input typemap */
      p = applyInputTypemap(p, ln, source, f);
      if (i >= numreq) {
	Printf(f->code,"}\n");
      }
    }
    
    /* Finish argument marshalling */
    Printf(kwargs," NULL }");
    if (allow_kwargs) {
      Printv(f->locals, tab4, "char *kwnames[] = ", kwargs, ";\n", NIL);
    }
    
    /* Trailing varargs */
    if (varargs) {
      if (p && (tm = Getattr(p,"tmap:in"))) {
        Clear(source);
	Printf(source,"argv[%d]",i-start);
	Replaceall(tm,"$input",source);
	Setattr(p,"emit:input",Copy(source));
	Printf(f->code,"if (argc > %d) {\n", i-start);
	Printv(f->code,tm,"\n",NIL);
	Printf(f->code,"}\n");
      }
    }
    
    Delete(source);
    Delete(target);
  }

  /* ---------------------------------------------------------------------
   * insertConstraintCheckingCode(ParmList *l, Wrapper *f)
   *
   * Checks each of the parameters in the parameter list for a "check"
   * typemap and (if it finds one) inserts the typemapping code into
   * the function wrapper.
   * --------------------------------------------------------------------- */
  
  void insertConstraintCheckingCode(ParmList *l, Wrapper *f) {
    Parm *p;
    String *tm;
    for (p = l; p;) {
      if ((tm = Getattr(p,"tmap:check"))) {
	Replaceall(tm,"$target",Getattr(p,"lname"));
	Printv(f->code,tm,"\n",NIL);
	p = Getattr(p,"tmap:check:next");
      } else {
	p = nextSibling(p);
      }
    }
  }

  /* ---------------------------------------------------------------------
   * insertCleanupCode(ParmList *l, String *cleanup)
   *
   * Checks each of the parameters in the parameter list for a "freearg"
   * typemap and (if it finds one) inserts the typemapping code into
   * the function wrapper.
   * --------------------------------------------------------------------- */
  
  void insertCleanupCode(ParmList *l, String *cleanup) {
    String *tm;
    for (Parm *p = l; p; ) {
      if ((tm = Getattr(p,"tmap:freearg"))) {
	Replaceall(tm,"$source",Getattr(p,"lname"));
	Printv(cleanup,tm,"\n",NIL);
	p = Getattr(p,"tmap:freearg:next");
      } else {
	p = nextSibling(p);
      }
    }
  }

  /* ---------------------------------------------------------------------
   * insertArgOutputCode(ParmList *l, String *outarg, int& need_result)
   *
   * Checks each of the parameters in the parameter list for a "argout"
   * typemap and (if it finds one) inserts the typemapping code into
   * the function wrapper.
   * --------------------------------------------------------------------- */
  
  void insertArgOutputCode(ParmList *l, String *outarg, int& need_result) {
    String *tm;
    for (Parm *p = l; p; ) {
      if ((tm = Getattr(p,"tmap:argout"))) {
	Replaceall(tm,"$source",Getattr(p,"lname"));
	Replaceall(tm,"$target","vresult");
	Replaceall(tm,"$result","vresult");
	Replaceall(tm,"$arg",Getattr(p,"emit:input"));
	Replaceall(tm,"$input",Getattr(p,"emit:input"));
	Printv(outarg,tm,"\n",NIL);
	need_result = 1;
	p = Getattr(p,"tmap:argout:next");
      } else {
	p = nextSibling(p);
      }
    }
  }
  
  /* ---------------------------------------------------------------------
   * validIdentifier()
   *
   * Is this a valid identifier in the scripting language?
   * Ruby method names can include any combination of letters, numbers
   * and underscores. A Ruby method name may optionally end with
   * a question mark ("?"), exclamation point ("!") or equals sign ("=").
   *
   * Methods whose names end with question marks are, by convention,
   * predicate methods that return true or false (e.g. Array#empty?).
   *
   * Methods whose names end with exclamation points are, by convention,
   * "mutators" that modify the instance in place (e.g. Array#sort!).
   *
   * Methods whose names end with an equals sign are attribute setters
   * (e.g. Thread#critical=).
   * --------------------------------------------------------------------- */

  virtual int validIdentifier(String *s) {
    char *c = Char(s);
    while (*c) {
      if ( !( isalnum(*c) || (*c == '_') || (*c == '?') || (*c == '!') || (*c == '=') ) ) return 0;
      c++;
    }
    return 1;
  }
  
  /* ---------------------------------------------------------------------
   * functionWrapper()
   *
   * Create a function declaration and register it with the interpreter.
   * --------------------------------------------------------------------- */

  virtual int functionWrapper(Node *n) {
    String *nodeType;
    bool constructor;
    bool destructor;
    String *storage;
    bool isVirtual;

    String *symname = Copy(Getattr(n,"sym:name"));
    SwigType *t = Getattr(n,"type");
    ParmList *l = Getattr(n,"parms");
    Node *parent   = Getattr(n,"parentNode");
    String *tm;
    
    int need_result = 0;
    
    /* Ruby needs no destructor wrapper */
    if (current == DESTRUCTOR)
      return SWIG_NOWRAP;

    nodeType = Getattr(n, "nodeType");
    constructor = (!Cmp(nodeType, "constructor")); 
    destructor = (!Cmp(nodeType, "destructor")); 
    storage   = Getattr(n, "storage");
    isVirtual = (Cmp(storage, "virtual") == 0);

    /* If the C++ class constructor is overloaded, we only want to
     * write out the "new" singleton method once since it is always
     * the same. (It's the "initialize" method that will handle the
     * overloading). */

    if (current == CONSTRUCTOR_ALLOCATE &&
        Swig_symbol_isoverloaded(n) &&
	Getattr(n, "sym:nextSibling") != 0) return SWIG_OK;
    
    String *overname = 0;
    if (Getattr(n, "sym:overloaded")) {
      overname = Getattr(n, "sym:overname");
    } else {
      if (!addSymbol(symname, n))
        return SWIG_ERROR;
    }

    String *cleanup = NewString("");
    String *outarg = NewString("");
    String *kwargs = NewString("");
    Wrapper *f = NewWrapper();

    /* Rename predicate methods */
    if (Getattr(n, "feature:predicate")) {
      Append(symname, "?");
    }

    /* Determine the name of the SWIG wrapper function */
    String *wname = Swig_name_wrapper(symname);
    if (overname && current != CONSTRUCTOR_ALLOCATE) {
      Append(wname,overname);
    }
  
    /* Emit arguments */
    if (current != CONSTRUCTOR_ALLOCATE) {
      emit_args(t,l,f);
    }

    /* Attach standard typemaps */
    if (current != CONSTRUCTOR_ALLOCATE) {
      emit_attach_parmmaps(l, f);
    }
    Setattr(n, "wrap:parms", l);

    /* Get number of arguments */
    int  numarg = emit_num_arguments(l);
    int  numreq = emit_num_required(l);
    int  varargs = emit_isvarargs(l);
    bool allow_kwargs = use_kw || Getattr(n,"feature:kwargs");
    
    bool use_director = (current == CONSTRUCTOR_INITIALIZE && Swig_directorclass(n));
    int start = (current == MEMBER_FUNC || current == MEMBER_VAR || use_director) ? 1 : 0;

    /* Now write the wrapper function itself */
    if        (current == CONSTRUCTOR_ALLOCATE) {
      Printf(f->def, "#ifdef HAVE_RB_DEFINE_ALLOC_FUNC\n");
      Printv(f->def, "static VALUE\n", wname, "(VALUE self) {", NIL);
      Printf(f->def, "#else\n");
      Printv(f->def, "static VALUE\n", wname, "(int argc, VALUE *argv, VALUE self) {", NIL);
      Printf(f->def, "#endif\n");
    } else if (current == CONSTRUCTOR_INITIALIZE) {
      Printv(f->def, "static VALUE\n", wname, "(int argc, VALUE *argv, VALUE self) {", NIL);
      if (!varargs) {
	Printf(f->code,"if ((argc < %d) || (argc > %d))\n", numreq-start, numarg-start);
      } else {
	Printf(f->code,"if (argc < %d)\n", numreq-start);
      }
      Printf(f->code,"rb_raise(rb_eArgError, \"wrong # of arguments(%%d for %d)\",argc);\n",numreq-start);
    } else {
      Printv(f->def, "static VALUE\n", wname, "(int argc, VALUE *argv, VALUE self) {", NIL);
      if (!varargs) {
	Printf(f->code,"if ((argc < %d) || (argc > %d))\n", numreq-start, numarg-start);
      } else {
	Printf(f->code,"if (argc < %d)\n", numreq-start);
      }
      Printf(f->code,"rb_raise(rb_eArgError, \"wrong # of arguments(%%d for %d)\",argc);\n",numreq-start);
    }

    /* Now walk the function parameter list and generate code */
    /* to get arguments */
    if (current != CONSTRUCTOR_ALLOCATE) {
      marshalInputArgs(n, l, numarg, numreq, kwargs, allow_kwargs, f);
    }

    // FIXME?
    if (use_director) {
      numarg--;
      numreq--;
    }

    /* Insert constraint checking code */
    insertConstraintCheckingCode(l, f);
  
    /* Insert cleanup code */
    insertCleanupCode(l, cleanup);

    /* Insert argument output code */
    insertArgOutputCode(l, outarg, need_result);

    /* if the object is a director, and the method call originated from its
     * underlying python object, resolve the call by going up the c++ 
     * inheritance chain.  otherwise try to resolve the method in python.  
     * without this check an infinite loop is set up between the director and 
     * shadow class method calls.
     */

    // NOTE: this code should only be inserted if this class is the
    // base class of a director class.  however, in general we haven't
    // yet analyzed all classes derived from this one to see if they are
    // directors.  furthermore, this class may be used as the base of
    // a director class defined in a completely different module at a
    // later time, so this test must be included whether or not directorbase
    // is true.  we do skip this code if directors have not been enabled
    // at the command line to preserve source-level compatibility with
    // non-polymorphic swig.  also, if this wrapper is for a smart-pointer
    // method, there is no need to perform the test since the calling object
    // (the smart-pointer) and the director object (the "pointee") are
    // distinct.

    if (directorsEnabled()) {
      if (!is_smart_pointer()) {
        if (/*directorbase &&*/ !constructor && !destructor && isVirtual) {
          Wrapper_add_local(f, "director", "Swig::Director *director = 0");
          Printf(f->code, "director = dynamic_cast<Swig::Director *>(arg1);\n");
          Printf(f->code, "if (director && (director->swig_get_self() == self)) director->swig_set_up();\n");
	}
      }
    }

    /* Now write code to make the function call */
    if (current != CONSTRUCTOR_ALLOCATE) {
      if (current == CONSTRUCTOR_INITIALIZE) {
	String *action = Getattr(n,"wrap:action");
	if (action) {
	  Append(action,"DATA_PTR(self) = result;");
	}
      }
      emit_action(n,f);
    }

    /* Return value if necessary */
    if (SwigType_type(t) != T_VOID && current != CONSTRUCTOR_ALLOCATE && current != CONSTRUCTOR_INITIALIZE) {
      need_result = 1;
      if (Getattr(n, "feature:predicate")) {
	Printv(f->code, tab4, "vresult = (result ? Qtrue : Qfalse);\n", NIL);
      } else {
	tm = Swig_typemap_lookup_new("out",n,"result",0);
	if (tm) {
	  Replaceall(tm,"$result","vresult");
	  Replaceall(tm,"$source","result");
	  Replaceall(tm,"$target","vresult");
          if (Getattr(n, "feature:new"))
            Replaceall(tm,"$owner", "1");
          else
            Replaceall(tm,"$owner", "0");

          // FIXME: this will not try to unwrap directors returned as non-director
          //        base class pointers!
    
          /* New addition to unwrap director return values so that the original
           * python object is returned instead. 
           */
          bool unwrap = false;
          String *decl = Getattr(n, "decl");
          int is_pointer = SwigType_ispointer_return(decl);
          int is_reference = SwigType_isreference_return(decl);
          if (is_pointer || is_reference) {
            String *type = Getattr(n, "type");
            //Node *classNode = Swig_methodclass(n);
            //Node *module = Getattr(classNode, "module");
            Node *module = Getattr(parent, "module");
            Node *target = Swig_directormap(module, type);
            if (target) unwrap = true;
          }
          if (unwrap) {
            Wrapper_add_local(f, "resultdirector", "Swig::Director *resultdirector = 0");
            Printf(f->code, "resultdirector = dynamic_cast<Swig::Director *>(result);\n");
            Printf(f->code, "if (resultdirector) {\n");
            Printf(f->code, "  vresult = resultdirector->swig_get_self();\n");
            Printf(f->code, "} else {\n"); 
            Printf(f->code,"%s\n", tm);
            Printf(f->code, "}\n");
          } else {
            Printf(f->code,"%s\n", tm);
          }
	} else {
	  Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number,
		       "Unable to use return type %s.\n", SwigType_str(t,0));
	}
      }
    }

    /* Extra code needed for new and initialize methods */  
    if (current == CONSTRUCTOR_ALLOCATE) {
      need_result = 1;
      Printf(f->code, "VALUE vresult = SWIG_NewClassInstance(self, SWIGTYPE%s);\n", Char(SwigType_manglestr(t)));
      Printf(f->code, "#ifndef HAVE_RB_DEFINE_ALLOC_FUNC\n");
      Printf(f->code, "rb_obj_call_init(vresult, argc, argv);\n");
      Printf(f->code, "#endif\n");
    } else if (current == CONSTRUCTOR_INITIALIZE) {
      need_result = 1;
      // Printf(f->code, "DATA_PTR(self) = result;\n");
    }

    /* Dump argument output code; */
    Printv(f->code,outarg,NIL);

    /* Dump the argument cleanup code */
    if (current != CONSTRUCTOR_ALLOCATE)
      Printv(f->code,cleanup,NIL);

    /* Look for any remaining cleanup.  This processes the %new directive */
    if (Getattr(n, "feature:new")) {
      tm = Swig_typemap_lookup_new("newfree",n,"result",0);
      if (tm) {
	Replaceall(tm,"$source","result");
	Printv(f->code,tm, "\n",NIL);
      }
    }

    /* Special processing on return value. */
    tm = Swig_typemap_lookup_new("ret",n,"result",0);
    if (tm) {
      Replaceall(tm,"$source","result");
      Printv(f->code,tm, NIL);
    }

    /* Wrap things up (in a manner of speaking) */
    if (need_result) {
      if (current == CONSTRUCTOR_ALLOCATE) {
	Printv(f->code, tab4, "return vresult;\n", NIL);
      } else if (current == CONSTRUCTOR_INITIALIZE) {
	Printv(f->code, tab4, "return self;\n", NIL);
      } else {
	Wrapper_add_local(f,"vresult","VALUE vresult = Qnil");
	Printv(f->code, tab4, "return vresult;\n", NIL);
      }
    } else {
      Printv(f->code, tab4, "return Qnil;\n", NIL);
    }

    /* Error handling code */
    /*
    Printf(f->code,"fail:\n");
    Printv(f->code,cleanup,NIL);
    Printv(f->code,"return Qnil;\n",NIL);
    */
    Printf(f->code,"}\n");

    /* Substitute the cleanup code */
    Replaceall(f->code,"$cleanup",cleanup);

    /* Emit the function */
    Wrapper_print(f, f_wrappers);

    /* Now register the function with the interpreter */
    if (!Swig_symbol_isoverloaded(n)) {
      create_command(n, symname);
    } else {
      if (current == CONSTRUCTOR_ALLOCATE) {
        create_command(n, symname);
      } else {
	Setattr(n, "wrap:name", wname);
	if (!Getattr(n, "sym:nextSibling"))
          dispatchFunction(n);
      }
    }
    
    Delete(kwargs);
    Delete(cleanup);
    Delete(outarg);
    DelWrapper(f);
    Delete(symname);
  
    return SWIG_OK;
  }

  /* ------------------------------------------------------------
   * dispatchFunction()
   * ------------------------------------------------------------ */
   
  void dispatchFunction(Node *n) {
    /* Last node in overloaded chain */

    int maxargs;
    String *tmp = NewString("");
    String *dispatch = Swig_overload_dispatch(n, "return %s(nargs, args, self);", &maxargs);
	
    /* Generate a dispatch wrapper for all overloaded functions */

    Wrapper *f       = NewWrapper();
    String  *symname = Getattr(n, "sym:name");
    String  *wname   = Swig_name_wrapper(symname);

    Printv(f->def,	
	   "static VALUE ", wname,
	   "(int nargs, VALUE *args, VALUE self) {",
	   NIL);
    
    Wrapper_add_local(f, "argc", "int argc");
    if (current == MEMBER_FUNC || current == MEMBER_VAR) {
      Printf(tmp, "VALUE argv[%d]", maxargs+1);
    } else {
      Printf(tmp, "VALUE argv[%d]", maxargs);
    }
    Wrapper_add_local(f, "argv", tmp);
    Wrapper_add_local(f, "ii", "int ii");
    if (current == MEMBER_FUNC || current == MEMBER_VAR) {
      Printf(f->code, "argc = nargs + 1;\n");
      Printf(f->code, "argv[0] = self;\n");
      Printf(f->code, "for (ii = 1; (ii < argc) && (ii < %d); ii++) {\n", maxargs);
      Printf(f->code, "argv[ii] = args[ii-1];\n");
      Printf(f->code, "}\n");
    } else {
      Printf(f->code, "argc = nargs;\n");
      Printf(f->code, "for (ii = 0; (ii < argc) && (ii < %d); ii++) {\n", maxargs);
      Printf(f->code, "argv[ii] = args[ii];\n");
      Printf(f->code, "}\n");
    }
    
    Replaceall(dispatch, "$args", "nargs, args, self");
    Printv(f->code, dispatch, "\n", NIL);
    Printf(f->code, "rb_raise(rb_eArgError, \"No matching function for overloaded '%s'\");\n", symname);
    Printf(f->code,"return Qnil;\n");
    Printv(f->code, "}\n", NIL);
    Wrapper_print(f, f_wrappers);
    create_command(n, Char(symname));

    DelWrapper(f);
    Delete(dispatch);
    Delete(tmp);
    Delete(wname);
  }
  
  /* ---------------------------------------------------------------------
   * variableWrapper()
   * --------------------------------------------------------------------- */

  virtual int variableWrapper(Node *n) {

    char *name  = GetChar(n,"name");
    char *iname = GetChar(n,"sym:name");
    SwigType *t = Getattr(n,"type");
    String *tm;
    String *getfname, *setfname;
    Wrapper *getf, *setf;

    getf = NewWrapper();
    setf = NewWrapper();

    /* create getter */
    getfname = NewString(Swig_name_get(iname));
    Printv(getf->def, "static VALUE\n", getfname, "(", NIL);
    Printf(getf->def, "VALUE self");
    Printf(getf->def, ") {");
    Wrapper_add_local(getf,"_val","VALUE _val");

    tm = Swig_typemap_lookup_new("varout",n, name, 0);
    if (tm) {
      Replaceall(tm,"$result","_val");
      Replaceall(tm,"$target","_val");
      Replaceall(tm,"$source",name);
      Printv(getf->code,tm, NIL);
    } else {
      Swig_warning(WARN_TYPEMAP_VAROUT_UNDEF, input_file, line_number,
		   "Unable to read variable of type %s\n", SwigType_str(t,0));
    }
    Printv(getf->code, tab4, "return _val;\n}\n", NIL);
    Wrapper_print(getf,f_wrappers);

    if (Getattr(n,"feature:immutable")) {
      setfname = NewString("NULL");
    } else {
      /* create setter */
      setfname = NewString(Swig_name_set(iname));
      Printv(setf->def, "static VALUE\n", setfname, "(VALUE self, ", NIL);
      Printf(setf->def, "VALUE _val) {");
    
      tm = Swig_typemap_lookup_new("varin",n,name,0);
      if (tm) {
	Replaceall(tm,"$input","_val");
	Replaceall(tm,"$source","_val");
	Replaceall(tm,"$target",name);
	Printv(setf->code,tm,"\n",NIL);
      } else {
	Swig_warning(WARN_TYPEMAP_VARIN_UNDEF, input_file, line_number,
		     "Unable to set variable of type %s\n", SwigType_str(t,0));
      }
      Printv(setf->code, tab4, "return _val;\n",NIL);
      Printf(setf->code,"}\n");
      Wrapper_print(setf,f_wrappers);
    }

    /* define accessor method */
    if (CPlusPlus) {
      Insert(getfname,0,"VALUEFUNC(");
      Append(getfname,")");
      Insert(setfname,0,"VALUEFUNC(");
      Append(setfname,")");
    }

    String *s = NewString("");
    switch (current) {
    case STATIC_VAR:
      /* C++ class variable */
      Printv(s,
	     tab4, "rb_define_singleton_method(", klass->vname, ", \"",
	     klass->strip(iname), "\", ", getfname, ", 0);\n",
	     NIL);
      if (!Getattr(n,"feature:immutable")) {
	Printv(s,
	       tab4, "rb_define_singleton_method(", klass->vname, ", \"",
	       klass->strip(iname), "=\", ", setfname, ", 1);\n",
	       NIL);
      }
      Printv(klass->init,s,NIL);
      break;
    default:
      /* C global variable */
      /* wrapped in Ruby module attribute */
      assert(current == NO_CPP);
      if (!useGlobalModule) {
        Printv(s,
               tab4, "rb_define_singleton_method(", modvar, ", \"",
               iname, "\", ", getfname, ", 0);\n",
               NIL);
        if (!Getattr(n,"feature:immutable")) {
          Printv(s,
                 tab4, "rb_define_singleton_method(", modvar, ", \"",
                 iname, "=\", ", setfname, ", 1);\n",
                 NIL);
        }
      } else {
        Printv(s,
               tab4, "rb_define_global_method(\"",
               iname, "\", ", getfname, ", 0);\n",
               NIL);
        if (!Getattr(n,"feature:immutable")) {
          Printv(s,
                 tab4, "rb_define_global_method(\"",
                 iname, "=\", ", setfname, ", 1);\n",
                 NIL);
        }
      }
      Printv(f_init,s,NIL);
      Delete(s);
      break;
    }
    Delete(getfname);
    Delete(setfname);
    DelWrapper(setf);
    DelWrapper(getf);
    return SWIG_OK;
  }


  /* ---------------------------------------------------------------------
   * validate_const_name(char *name)
   *
   * Validate constant name.
   * --------------------------------------------------------------------- */

  char *
  validate_const_name(char *name, const char *reason) {
    if (!name || name[0] == '\0')
      return name;
    
    if (isupper(name[0]))
      return name;
    
    if (islower(name[0])) {
      name[0] = toupper(name[0]);
      Swig_warning(WARN_RUBY_WRONG_NAME, input_file, line_number,
		   "Wrong %s name (corrected to `%s')\n", reason, name);
      return name;
    }
    
    Swig_warning(WARN_RUBY_WRONG_NAME, input_file, line_number,
		 "Wrong %s name\n", reason);
    
    return name;
  }
  
  /* ---------------------------------------------------------------------
   * constantWrapper()
   * --------------------------------------------------------------------- */

  virtual int constantWrapper(Node *n) {
    Swig_require("constantWrapper",n, "*sym:name", "type", "value", NIL);
    
    char *iname     = GetChar(n,"sym:name");
    SwigType *type  = Getattr(n,"type");
    char *value     = GetChar(n,"value");
    
    if (current == CLASS_CONST) {
      iname = klass->strip(iname);
    }
    validate_const_name(iname, "constant");
    SetChar(n, "sym:name", iname);
    
    /* Special hook for member pointer */
    if (SwigType_type(type) == T_MPOINTER) {
      String *wname = Swig_name_wrapper(iname);
      Printf(f_header, "static %s = %s;\n", SwigType_str(type, wname), value);
      value = Char(wname);
    }
    String *tm = Swig_typemap_lookup_new("constant", n, value, 0);
    if (tm) {
      Replaceall(tm, "$source", value);
      Replaceall(tm, "$target", iname);
      Replaceall(tm, "$symname", iname);
      Replaceall(tm, "$value", value);
      if (current == CLASS_CONST) {
        if (multipleInheritance) {
          Replaceall(tm, "$module", klass->mImpl);
          Printv(klass->init, tm, "\n", NIL);
        } else {
          Replaceall(tm, "$module", klass->vname);
          Printv(klass->init, tm, "\n", NIL);
        }
      } else {
        if (!useGlobalModule) {
          Replaceall(tm, "$module", modvar);
        } else {
          Replaceall(tm, "$module", "rb_cObject");
        }
	Printf(f_init, "%s\n", tm);
      }
    } else {
      Swig_warning(WARN_TYPEMAP_CONST_UNDEF, input_file, line_number,
		   "Unsupported constant value %s = %s\n", SwigType_str(type, 0), value);
    }
    Swig_restore(n);
    return SWIG_OK;
  }
  
  /* -----------------------------------------------------------------------------
   * classDeclaration() 
   *
   * Records information about classes---even classes that might be defined in
   * other modules referenced by %import.
   * ----------------------------------------------------------------------------- */

  virtual int classDeclaration(Node *n) {
    String *name = Getattr(n,"name");
    String *symname = Getattr(n,"sym:name");
    String *tdname = Getattr(n,"tdname");
  
    name = tdname ? tdname : name;
    String *namestr = SwigType_namestr(name);
    klass = RCLASS(classes, Char(namestr));
    if (!klass) {
      klass = new RClass();
      String *valid_name = NewString(symname ? symname : namestr);
      validate_const_name(Char(valid_name), "class");
      klass->set_name(namestr, symname, valid_name);
      SET_RCLASS(classes, Char(namestr), klass);
      Delete(valid_name);
    }
    Delete(namestr);
    return Language::classDeclaration(n);
  }

  /**
   * Process the comma-separated list of mixed-in module names (if any).
   */
  void includeRubyModules(Node *n) {
    String *mixin = Getattr(n,"feature:mixin");
    if (mixin) {
      List *modules = Split(mixin,',',INT_MAX);
      if (modules && Len(modules) > 0) {
	Iterator mod = First(modules);
	while (mod.item) {
          if (Len(mod.item) > 0) {
            Printf(klass->init, "rb_include_module(%s, rb_eval_string(\"%s\"));\n", klass->vname, mod.item);
	  }
	  mod = Next(mod);
	}
      }
      Delete(modules);
    }
  }

  void handleBaseClasses(Node *n) {
    List *baselist = Getattr(n,"bases");
    if (baselist && Len(baselist)) {
      Iterator base = First(baselist);
      while (base.item) {
        String *basename = Getattr(base.item,"name");
        String *basenamestr = SwigType_namestr(basename);
        RClass *super = RCLASS(classes, Char(basenamestr));
        Delete(basenamestr);
        if (super) {
          SwigType *btype = NewString(basename);
          SwigType_add_pointer(btype);
          SwigType_remember(btype);
          if (multipleInheritance) {
            String *bmangle = SwigType_manglestr(btype);
            Insert(bmangle,0,"((swig_class *) SWIGTYPE");
            Append(bmangle,"->clientdata)->mImpl");
            Printv(klass->init, "rb_include_module(", klass->mImpl, ", ", bmangle, ");\n", NIL);
            Delete(bmangle);
          } else {
            String *bmangle = SwigType_manglestr(btype);
            Insert(bmangle,0,"((swig_class *) SWIGTYPE");
            Append(bmangle,"->clientdata)->klass");
            Replaceall(klass->init,"$super",bmangle);
            Delete(bmangle);
          }
          Delete(btype);
        }
        base = Next(base);
        if (!multipleInheritance) {
          /* Warn about multiple inheritance for additional base class(es) listed */
          while (base.item) {
            basename = Getattr(n,"name");
            Swig_warning(WARN_RUBY_MULTIPLE_INHERITANCE, input_file, line_number, 
                         "Warning for %s: Base %s ignored. Multiple inheritance is not supported in Ruby.\n", basename, basename);
            base = Next(base);
          }
        }
      }
    }
  }

  /**
   * Check to see if a %markfunc was specified.
   */
  void handleMarkFuncDirective(Node *n) {
    String *markfunc = Getattr(n, "feature:markfunc");
    if (markfunc) {
      Printf(klass->init, "c%s.mark = (void (*)(void *)) %s;\n", klass->name, markfunc);
    } else {
      Printf(klass->init, "c%s.mark = 0;\n", klass->name);
    }
  }

  /**
   * Check to see if a %freefunc was specified.
   */
  void handleFreeFuncDirective(Node *n) {
    String *freefunc = Getattr(n, "feature:freefunc");
    if (freefunc) {
      Printf(klass->init, "c%s.destroy = (void (*)(void *)) %s;\n", klass->name, freefunc);
    } else {
      if (klass->destructor_defined) {
	Printf(klass->init, "c%s.destroy = (void (*)(void *)) free_%s;\n", klass->name, klass->mname);
      }
    }
    Replaceall(klass->header,"$freeproto", "");
  }
  
  /* ----------------------------------------------------------------------
   * classHandler()
   * ---------------------------------------------------------------------- */

  virtual int classHandler(Node *n) {

    String *name = Getattr(n,"name");
    String *symname = Getattr(n,"sym:name");
    String *namestr = SwigType_namestr(name); // does template expansion

    klass = RCLASS(classes, Char(namestr));
    assert(klass != 0);
    Delete(namestr);
    String *valid_name = NewString(symname);
    validate_const_name(Char(valid_name), "class");

    Clear(klass->type);
    Printv(klass->type, Getattr(n,"classtype"), NIL);
    Printv(klass->header, "\nswig_class c", valid_name, ";\n", NIL);
    Printv(klass->init, "\n", tab4, NIL);
    if (multipleInheritance) {
      if (!useGlobalModule) {
        Printv(klass->init, klass->vname, " = rb_define_class_under(", modvar,
               ", \"", klass->name, "\", rb_cObject);\n", NIL);
      } else {
        Printv(klass->init, klass->vname, " = rb_define_class(\"",
               klass->name, "\", rb_cObject);\n", NIL);
      }
      Printv(klass->init, klass->mImpl, " = rb_define_module_under(", klass->vname, ", \"Impl\");\n", NIL);
    } else {
      if (!useGlobalModule) {
        Printv(klass->init, klass->vname, " = rb_define_class_under(", modvar,
               ", \"", klass->name, "\", $super);\n", NIL);
      } else {
        Printv(klass->init, klass->vname, " = rb_define_class(\"",
               klass->name, "\", $super);\n", NIL);
      }
    }

    SwigType *tt = NewString(name);
    SwigType_add_pointer(tt);
    SwigType_remember(tt);
    String *tm = SwigType_manglestr(tt);
    Printf(klass->init, "SWIG_TypeClientData(SWIGTYPE%s, (void *) &c%s);\n", tm, valid_name);
    Delete(tm);
    Delete(tt);
    Delete(valid_name);
    
    includeRubyModules(n);

    Printv(klass->init, "$allocator",NIL);
    Printv(klass->init, "$initializer",NIL);

    Printv(klass->header,
	   "$freeproto",
	   NIL);

    Language::classHandler(n);

    handleBaseClasses(n);
    handleMarkFuncDirective(n);
    handleFreeFuncDirective(n);
    
    if (multipleInheritance) {
      Printv(klass->init, "rb_include_module(", klass->vname, ", ", klass->mImpl, ");\n", NIL);
    }

    Printv(f_header, klass->header,NIL);

    String *s = NewString("");
    Printv(s, tab4, "rb_undef_alloc_func(", klass->vname, ");\n", NIL);
    Replaceall(klass->init,"$allocator", s);
    Replaceall(klass->init,"$initializer", "");
    Replaceall(klass->init,"$super", "rb_cObject");
    Delete(s);

    Printv(f_init,klass->init,NIL);
    klass = 0;
    return SWIG_OK;
  }

  /* ----------------------------------------------------------------------
   * memberfunctionHandler()
   *
   * Method for adding C++ member function
   *
   * By default, we're going to create a function of the form :
   *
   *         Foo_bar(this,args)
   *
   * Where Foo is the classname, bar is the member name and the this pointer
   * is explicitly attached to the beginning.
   *
   * The renaming only applies to the member function part, not the full
   * classname.
   *
   * --------------------------------------------------------------------- */
  
  virtual int memberfunctionHandler(Node *n) {
    current = MEMBER_FUNC;
    Language::memberfunctionHandler(n);
    current = NO_CPP;
    return SWIG_OK;
  }
  
  /* ---------------------------------------------------------------------
   * constructorHandler()
   *
   * Method for adding C++ member constructor
   * -------------------------------------------------------------------- */

  virtual int constructorHandler(Node *n) {
    int   use_director = Swig_directorclass(n);

    /* First wrap the allocate method */
    current = CONSTRUCTOR_ALLOCATE;
    Swig_name_register((String_or_char *) "construct", (String_or_char *) "%c_allocate");
    Language::constructorHandler(n);

    /* 
     * If we're wrapping the constructor of a C++ director class, prepend a new parameter
     * to receive the scripting language object (e.g. 'self')
     *
     */
    Swig_save("ruby:constructorHandler",n,"parms",NIL);
    if (use_director) {
      Parm *parms = Getattr(n, "parms");
      Parm *self;
      String *name = NewString("self");
      String *type = NewString("VALUE");
      self = NewParm(type, name);
      Delete(type);
      Delete(name);
      Setattr(self, "lname", "Qnil");
      if (parms) set_nextSibling(self, parms);
      Setattr(n, "parms", self);
      Setattr(n, "wrap:self", "1");
      Delete(self);
    }

    /* Now do the instance initialize method */
    current = CONSTRUCTOR_INITIALIZE;
    Swig_name_register((String_or_char *) "construct", (String_or_char *) "new_%c");
    Language::constructorHandler(n);

    /* Restore original parameter list */
    Delattr(n, "wrap:self");
    Swig_restore(n);

    /* Done */
    Swig_name_unregister((String_or_char *) "construct");
    current = NO_CPP;
    klass->constructor_defined = 1;
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------
   * destructorHandler()
   * -------------------------------------------------------------------- */

  virtual int destructorHandler(Node *n) {
    current = DESTRUCTOR;
    Language::destructorHandler(n);

    String *freefunc = NewString("");
    String *freeproto = NewString("");
    String *freebody = NewString("");
  
    Printv(freefunc, "free_", klass->mname, NIL);
    Printv(freeproto, "static void ", freefunc, "(", klass->type, " *);\n", NIL);
    Printv(freebody, "static void\n",
	   freefunc, "(", klass->type, " *", Swig_cparm_name(0,0), ") {\n",
	   tab4, NIL);
    if (Extend) {
      String *wrap = Getattr(n, "wrap:code");
      if (wrap) {
	File *f_code = Swig_filebyname("header");
	if (f_code) {
	  Printv(f_code, wrap, NIL);
	}
      }
      /*    Printv(freebody, Swig_name_destroy(name), "(", Swig_cparm_name(0,0), ")", NIL); */
      Printv(freebody,Getattr(n,"wrap:action"), NIL);
    } else {
      /* When no extend mode, swig emits no destroy function. */
      if (CPlusPlus)
	Printf(freebody, "delete %s", Swig_cparm_name(0,0));
      else
	Printf(freebody, "free((char*) %s)", Swig_cparm_name(0,0));
    }
    Printv(freebody, ";\n}\n", NIL);
  
    Replaceall(klass->header,"$freeproto", freeproto);
    Printv(f_wrappers, freebody, NIL);
  
    klass->destructor_defined = 1;
    current = NO_CPP;
    Delete(freefunc);
    Delete(freeproto);
    Delete(freebody);
    return SWIG_OK;
  }

  /* ---------------------------------------------------------------------
   * membervariableHandler()
   *
   * This creates a pair of functions to set/get the variable of a member.
   * -------------------------------------------------------------------- */

  virtual int membervariableHandler(Node *n) {
    current = MEMBER_VAR;
    Language::membervariableHandler(n);
    current = NO_CPP;
    return SWIG_OK;
  }

  /* -----------------------------------------------------------------------
   * staticmemberfunctionHandler()
   *
   * Wrap a static C++ function
   * ---------------------------------------------------------------------- */

  virtual int staticmemberfunctionHandler(Node *n) {
    current = STATIC_FUNC;
    Language::staticmemberfunctionHandler(n);
    current = NO_CPP;
    return SWIG_OK;
  }
  
  /* ----------------------------------------------------------------------
   * memberconstantHandler()
   *
   * Create a C++ constant
   * --------------------------------------------------------------------- */

  virtual int memberconstantHandler(Node *n) {
    current = CLASS_CONST;
    Language::memberconstantHandler(n);
    current = NO_CPP;
    return SWIG_OK;
  }
  
  /* ---------------------------------------------------------------------
   * staticmembervariableHandler()
   * --------------------------------------------------------------------- */

  virtual int staticmembervariableHandler(Node *n) {
    current = STATIC_VAR;
    Language::staticmembervariableHandler(n);
    current = NO_CPP;
    return SWIG_OK;
  }

  /* C++ director class generation */
  virtual int classDirector(Node *n) {
    return Language::classDirector(n);
  }
  
  virtual int classDirectorInit(Node *n) {
    String *declaration;
    declaration = Swig_director_declaration(n);
    Printf(f_directors_h, "\n");
    Printf(f_directors_h, "%s\n", declaration);
    Printf(f_directors_h, "public:\n");
    Delete(declaration);
    return Language::classDirectorInit(n);
  }
  
  virtual int classDirectorEnd(Node *n) {
    Printf(f_directors_h, "};\n\n");
    return Language::classDirectorEnd(n);
  }
  
  virtual int unrollVirtualMethods(Node *n, Node *parent, Hash *vm, int default_director, int &virtual_destructor) {
    return Language::unrollVirtualMethods(n, parent, vm, default_director, virtual_destructor);
  }
  
  /* ------------------------------------------------------------
   * classDirectorConstructor()
   * ------------------------------------------------------------ */

  virtual int classDirectorConstructor(Node *n) {
    Node *parent = Getattr(n, "parentNode");
    String *sub = NewString("");
    String *decl = Getattr(n, "decl");
    String *supername = Swig_class_name(parent);
    String *classname = NewString("");
    Printf(classname, "SwigDirector_%s", supername);

    /* insert self and disown parameters */
    Parm *p, *ip;
    ParmList *superparms = Getattr(n, "parms");
    ParmList *parms = CopyParmList(superparms);
    String *type = NewString("VALUE");
    p = NewParm(type, NewString("self"));
    set_nextSibling(p, parms);
    parms = p;
    for (ip = parms; nextSibling(ip); ) ip = nextSibling(ip);
    p = NewParm(NewString("bool"), NewString("disown"));
    Setattr(p, "arg:byname", "1");
    Setattr(n, "director:postfix_args", p);
    Setattr(p, "value", "0");
    set_nextSibling(ip, p);
    
    /* constructor */
    {
      Wrapper *w = NewWrapper();
      String *call;
      String *basetype = Getattr(parent, "classtype");
      String *target = Swig_method_decl(decl, classname, parms, 0, 0);
      call = Swig_csuperclass_call(0, basetype, superparms);
      Printf(w->def, "%s::%s: %s, Swig::Director(self, disown) { }", classname, target, call);
      Delete(target);
      Wrapper_print(w, f_directors);
      Delete(call);
      DelWrapper(w);
    }
    
    /* constructor header */
    {
      String *target = Swig_method_decl(decl, classname, parms, 0, 1);
      Printf(f_directors_h, "    %s;\n", target);
      Delete(target);
    }

    Delete(sub);
    Delete(classname);
    Delete(supername);
    Delete(parms);
    return Language::classDirectorConstructor(n);
  }
  
  virtual int classDirectorDefaultConstructor(Node *n) {
    String *classname;
    Wrapper *w;
    classname = Swig_class_name(n);
    w = NewWrapper();
    Printf(w->def, "SwigDirector_%s::SwigDirector_%s(VALUE self, bool disown) : Swig::Director(self, disown) { }", classname, classname);
    Wrapper_print(w, f_directors);
    DelWrapper(w);
    Printf(f_directors_h, "    SwigDirector_%s(VALUE self, bool disown = true);\n", classname);
    Delete(classname);
    return Language::classDirectorDefaultConstructor(n);
  }
  
  /* ---------------------------------------------------------------
   * classDirectorMethod()
   *
   * Emit a virtual director method to pass a method call on to the 
   * underlying Ruby instance.
   *
   * --------------------------------------------------------------- */

  void exceptionSafeMethodCall(String *className, Node *n, Wrapper *w, int argc, String *args) {

    Wrapper *body = NewWrapper();
    Wrapper *rescue = NewWrapper();

    String *methodName = Getattr(n, "sym:name");
    String *bodyName = NewStringf("%s_%s_body", className, methodName);
    String *rescueName = NewStringf("%s_%s_rescue", className, methodName);
    String *depthCountName = NewStringf("%s_%s_call_depth", className, methodName);

    // Check for an exception typemap of some kind
    String *tm = Swig_typemap_lookup_new("director:except", n, "result", 0);
    if (!tm) {
      tm = Getattr(n, "feature:director:except");
    }

    if ((tm != 0) && (Len(tm) > 0) && (Strcmp(tm, "1") != 0))
    {
      // Declare a global to hold the depth count
      Printf(f_directors, "static int %s = 0;\n", depthCountName);

      // Function body
      Printf(body->def, "VALUE %s(VALUE data) {\n", bodyName);
      Wrapper_add_localv(body, "args", "Swig::body_args *", "args", "= reinterpret_cast<Swig::body_args *>(data)", NIL);
      Wrapper_add_localv(body, "result", "VALUE", "result", "= Qnil", NIL);
      Printf(body->code, "%s++;\n", depthCountName, NIL);
      Printv(body->code, "result = rb_funcall2(args->recv, args->id, args->argc, args->argv);\n", NIL);
      Printf(body->code, "%s--;\n", depthCountName, NIL);
      Printv(body->code, "return result;\n", NIL);
      Printv(body->code, "}", NIL);
      
      // Exception handler
      Printf(rescue->def, "VALUE %s(VALUE args, VALUE error) {\n", rescueName); 
      Replaceall(tm, "$error", "error");
      Printf(rescue->code, "if (%s == 0) ", depthCountName);
      Printv(rescue->code, Str(tm), "\n", NIL);
      Printf(rescue->code, "%s--;\n", depthCountName);
      Printv(rescue->code, "rb_exc_raise(error);\n", NIL);
      Printv(rescue->code, "}", NIL);
      
      // Main code
      Wrapper_add_localv(w, "args", "Swig::body_args", "args", NIL);
      Printv(w->code, "args.recv = swig_get_self();\n", NIL);
      Printf(w->code, "args.id = rb_intern(\"%s\");\n", methodName);
      Printf(w->code, "args.argc = %d;\n", argc);
      if (argc > 0) {
        Wrapper_add_localv(w, "i", "int", "i", NIL);
        Printf(w->code, "args.argv = new VALUE[%d];\n", argc);
        Printf(w->code, "for (i = 0; i < %d; i++) {\n", argc);
        Printv(w->code, "args.argv[i] = Qnil;\n", NIL);
        Printv(w->code, "}\n", NIL);
      } else {
        Printv(w->code, "args.argv = 0;\n", NIL);
      }
      Printf(w->code,
        "result = rb_rescue2((VALUE(*)(ANYARGS)) %s, reinterpret_cast<VALUE>(&args), (VALUE(*)(ANYARGS)) %s, reinterpret_cast<VALUE>(&args), rb_eStandardError, 0);\n",
        bodyName, rescueName);
      if (argc > 0) {
        Printv(w->code, "delete [] args.argv;\n", NIL);
      }

      // Dump wrapper code
      Wrapper_print(body, f_directors);
      Wrapper_print(rescue, f_directors);
    }
    else
    {
      if (argc > 0) {
        Printf(w->code, "result = rb_funcall(swig_get_self(), rb_intern(\"%s\"), %d%s);\n", methodName, argc, args);
      } else {
        Printf(w->code, "result = rb_funcall(swig_get_self(), rb_intern(\"%s\"), 0, NULL);\n", methodName);
      }
    }

    // Clean up
    Delete(bodyName);
    Delete(rescueName);
    Delete(depthCountName);
    DelWrapper(body);
    DelWrapper(rescue);
  }
  
  virtual int classDirectorMethod(Node *n, Node *parent, String *super) {
    int is_void = 0;
    int is_pointer = 0;
    String *decl;
    String *type;
    String *name;
    String *classname;
    String *declaration;
    ParmList *l;
    Wrapper *w;
    String *tm;
    String *wrap_args;
    String *return_type;
    Parm* p;
    String *value = Getattr(n, "value");
    String *storage = Getattr(n,"storage");
    bool pure_virtual = false;
    int status = SWIG_OK;
    int idx;

    if (Cmp(storage,"virtual") == 0) {
      if (Cmp(value,"0") == 0) {
        pure_virtual = true;
      }
    }

    classname = Getattr(parent, "sym:name");
    type = Getattr(n, "type");
    name = Getattr(n, "name");

    w = NewWrapper();
    declaration = NewString("");
	
    /* determine if the method returns a pointer */
    decl = Getattr(n, "decl");
    is_pointer = SwigType_ispointer_return(decl);
    is_void = (!Cmp(type, "void") && !is_pointer);

    /* form complete return type */
    return_type = Copy(type);
    {
    	SwigType *t = Copy(decl);
	SwigType *f = 0;
	f = SwigType_pop_function(t);
	SwigType_push(return_type, t);
	Delete(f);
	Delete(t);
    }

    /* virtual method definition */
    l = Getattr(n, "parms");
    String *target;
    String *pclassname = NewStringf("SwigDirector_%s", classname);
    String *qualified_name = NewStringf("%s::%s", pclassname, name);
    target = Swig_method_decl(decl, qualified_name, l, 0, 0);
    String *rtype = SwigType_str(type, 0);
    Printf(w->def, "%s %s {", rtype, target);
    Delete(qualified_name);
    Delete(target);
    /* header declaration */
    target = Swig_method_decl(decl, name, l, 0, 1);
    Printf(declaration, "    virtual %s %s;\n", rtype, target);
    Delete(target);
    
    /* attach typemaps to arguments (C/C++ -> Ruby) */
    String *arglist = NewString("");


    /**
     * For each parameter to the C++ member function, copy the parameter name
     * to its "lname"; this ensures that Swig_typemap_attach_parms() will do
     * the right thing when it sees strings like "$1" in your "directorin" typemaps.
     * Not sure if it's OK to leave it like this, but seems OK so far.
     */
    typemap_copy_pname_to_lname(l);
    
    Swig_typemap_attach_parms("in", l, w);
    Swig_typemap_attach_parms("directorin", l, w);
    Swig_typemap_attach_parms("directorout", l, w);
    Swig_typemap_attach_parms("directorargout", l, w);

    int num_arguments = emit_num_arguments(l);
    int i;
    char source[256];

    wrap_args = NewString("");
    int outputs = 0;
    if (!is_void) outputs++;
	
    /* build argument list and type conversion string */
    for (i=0, idx=0, p = l; i < num_arguments; i++) {

      while (Getattr(p, "tmap:ignore")) {
	p = Getattr(p, "tmap:ignore:next");
      }

      if (Getattr(p, "tmap:directorargout") != 0) outputs++;
      
      String* parameterName = Getattr(p, "name");
      String* parameterType = Getattr(p, "type");
      
      Putc(',',arglist);
      if ((tm = Getattr(p, "tmap:directorin")) != 0) {
        sprintf(source, "obj%d", idx++);
        Replaceall(tm, "$input", source);
        Replaceall(tm, "$owner", "0");
        Printv(wrap_args, tm, "\n", NIL);
        Wrapper_add_localv(w, source, "VALUE", source, "= Qnil", NIL);
        Printv(arglist, source, NIL);
	p = Getattr(p, "tmap:directorin:next");
	continue;
      } else if (Cmp(parameterType, "void")) {
	/**
         * Special handling for pointers to other C++ director classes.
	 * Ideally this would be left to a typemap, but there is currently no
	 * way to selectively apply the dynamic_cast<> to classes that have
	 * directors.  In other words, the type "SwigDirector_$1_lname" only exists
	 * for classes with directors.  We avoid the problem here by checking
	 * module.wrap::directormap, but it's not clear how to get a typemap to
	 * do something similar.  Perhaps a new default typemap (in addition
	 * to SWIGTYPE) called DIRECTORTYPE?
	 */
	if (SwigType_ispointer(parameterType) || SwigType_isreference(parameterType)) {
	  Node *module = Getattr(parent, "module");
	  Node *target = Swig_directormap(module, parameterType);
	  sprintf(source, "obj%d", idx++);
	  String *nonconst = 0;
	  /* strip pointer/reference --- should move to Swig/stype.c */
	  String *nptype = NewString(Char(parameterType)+2);
	  /* name as pointer */
	  String *ppname = Copy(parameterName);
	  if (SwigType_isreference(parameterType)) {
      	     Insert(ppname,0,"&");
	  }
	  /* if necessary, cast away const since Ruby doesn't support it! */
	  if (SwigType_isconst(nptype)) {
	    nonconst = NewStringf("nc_tmp_%s", parameterName);
	    String *nonconst_i = NewStringf("= const_cast<%s>(%s)", SwigType_lstr(parameterType, 0), ppname);
	    Wrapper_add_localv(w, nonconst, SwigType_lstr(parameterType, 0), nonconst, nonconst_i, NIL);
	    Delete(nonconst_i);
	    Swig_warning(WARN_LANG_DISCARD_CONST, input_file, line_number,
		         "Target language argument '%s' discards const in director method %s::%s.\n", SwigType_str(parameterType, parameterName), classname, name);
	  } else {
	    nonconst = Copy(ppname);
	  }
	  Delete(nptype);
	  Delete(ppname);
	  String *mangle = SwigType_manglestr(parameterType);
	  if (target) {
	    String *director = NewStringf("director_%s", mangle);
	    Wrapper_add_localv(w, director, "Swig::Director *", director, "= 0", NIL);
	    Wrapper_add_localv(w, source, "VALUE", source, "= Qnil", NIL);
	    Printf(wrap_args, "%s = dynamic_cast<Swig::Director *>(%s);\n", director, nonconst);
	    Printf(wrap_args, "if (!%s) {\n", director);
	    Printf(wrap_args,   "%s = SWIG_NewPointerObj(%s, SWIGTYPE%s, 0);\n", source, nonconst, mangle);
	    Printf(wrap_args, "} else {\n");
	    Printf(wrap_args,   "%s = %s->swig_get_self();\n", source, director);
	    Printf(wrap_args, "}\n");
	    Delete(director);
	    Printv(arglist, source, NIL);
	  } else {
	    Wrapper_add_localv(w, source, "VALUE", source, "= Qnil", NIL);
	    Printf(wrap_args, "%s = SWIG_NewPointerObj(%s, SWIGTYPE%s, 0);\n", 
	           source, nonconst, mangle); 
	    //Printf(wrap_args, "%s = SWIG_NewPointerObj(%s, SWIGTYPE_p_%s, 0);\n", 
	    //       source, nonconst, base);
	    Printv(arglist, source, NIL);
	  }
	  Delete(mangle);
	  Delete(nonconst);
	} else {
	  Swig_warning(WARN_TYPEMAP_DIRECTORIN_UNDEF, input_file, line_number,
		       "Unable to use type %s as a function argument in director method %s::%s (skipping method).\n", SwigType_str(parameterType, 0), classname, name);
          status = SWIG_NOWRAP;
	  break;
	}
      }
      p = nextSibling(p);
    }

    /* declare method return value 
     * if the return value is a reference or const reference, a specialized typemap must
     * handle it, including declaration of c_result ($result).
     */
    if (!is_void) {
      Wrapper_add_localv(w, "c_result", SwigType_lstr(return_type, "c_result"), NIL);
    }
    /* declare Ruby return value */
    Wrapper_add_local(w, "result", "VALUE result");

    /* direct call to superclass if _up is set */
    Printf(w->code, "if (swig_get_up()) {\n");
    if (pure_virtual) {
    	Printf(w->code, "throw Swig::DirectorPureVirtualException();\n");
    } else {
    	if (is_void) {
    	  Printf(w->code, "%s;\n", Swig_method_call(super,l));
    	  Printf(w->code, "return;\n");
	} else {
    	  Printf(w->code, "return %s;\n", Swig_method_call(super,l));
	}
    }
    Printf(w->code, "}\n");
    
    /* wrap complex arguments to VALUEs */
    Printv(w->code, wrap_args, NIL);

    /* pass the method call on to the Ruby object */
    exceptionSafeMethodCall(classname, n, w, idx, arglist);

    /*
    * Ruby method may return a simple object, or an Array of objects.
    * For in/out arguments, we have to extract the appropriate VALUEs from the Array,
    * then marshal everything back to C/C++ (return value and output arguments).
    */

    /* Marshal return value and other outputs (if any) from VALUE to C/C++ type */

    String* cleanup = NewString("");
    String* outarg = NewString("");

    if (outputs > 1) {
      Wrapper_add_local(w, "output", "VALUE output");
      Printf(w->code, "if (TYPE(result) != T_ARRAY) {\n");
      Printf(w->code, "throw Swig::DirectorTypeMismatchException(\"Ruby method failed to return an array.\");\n");
      Printf(w->code, "}\n");
    }

    idx = 0;

    /* Marshal return value */
    if (!is_void) {
      /* This seems really silly.  The node's type excludes qualifier/pointer/reference markers,
       * which have to be retrieved from the decl field to construct return_type.  But the typemap
       * lookup routine uses the node's type, so we have to swap in and out the correct type.
       * It's not just me, similar silliness also occurs in Language::cDeclaration().
       */
      Setattr(n, "type", return_type);
      tm = Swig_typemap_lookup_new("directorout", n, "result", w);
      Setattr(n, "type", type);
      if (tm == 0) {
        String *name = NewString("result");
        tm = Swig_typemap_search("directorout", return_type, name, NULL);
	Delete(name);
      }
      if (tm != 0) {
	if (outputs > 1) {
	  Printf(w->code, "output = rb_ary_entry(result, %d);\n", idx++);
	  Replaceall(tm, "$input", "output");
	} else {
	  Replaceall(tm, "$input", "result");
	}
	/* TODO check this */
	if (Getattr(n,"wrap:disown")) {
	  Replaceall(tm,"$disown","SWIG_POINTER_DISOWN");
	} else {
	  Replaceall(tm,"$disown","0");
	}
	Replaceall(tm, "$result", "c_result");
	Printv(w->code, tm, "\n", NIL);
      } else {
	Swig_warning(WARN_TYPEMAP_OUT_UNDEF, input_file, line_number,
		     "Unable to return type %s in director method %s::%s (skipping method).\n", SwigType_str(return_type, 0), classname, name);
        status = SWIG_ERROR;
      }
    }
	  
    /* Marshal outputs */
    for (p = l; p; ) {
      if ((tm = Getattr(p, "tmap:directorargout")) != 0) {
	if (outputs > 1) {
	  Printf(w->code, "output = rb_ary_entry(result, %d);\n", idx++);
	  Replaceall(tm, "$input", "output");
	} else {
	  Replaceall(tm, "$input", "result");
	}
	Replaceall(tm, "$result", Getattr(p, "name"));
	Printv(w->code, tm, "\n", NIL);
	p = Getattr(p, "tmap:directorargout:next");
      } else {
	p = nextSibling(p);
      }
    }

    /* any existing helper functions to handle this? */
    if (!is_void) {
      if (!SwigType_isreference(return_type)) {
        Printf(w->code, "return c_result;\n");
      } else {
        Printf(w->code, "return *c_result;\n");
      }
    }

    Printf(w->code, "}\n");

    /* emit the director method */
    if (status == SWIG_OK) {
      Wrapper_print(w, f_directors);
      Printv(f_directors_h, declaration, NIL);
    }

    /* clean up */
    Delete(wrap_args);
    Delete(arglist);
    Delete(rtype);
    Delete(return_type);
    Delete(pclassname);
    Delete(cleanup);
    Delete(outarg);
    DelWrapper(w);
    return status;
  }
  
  virtual int classDirectorConstructors(Node *n) {
    return Language::classDirectorConstructors(n);
  }
  
  virtual int classDirectorMethods(Node *n) {
    return Language::classDirectorMethods(n);
  }
  
  virtual int classDirectorDisown(Node *n) {
    return Language::classDirectorDisown(n);
  }

  void typemap_copy_pname_to_lname(ParmList *parms)
  {
    Parm *p;
    String *pname;
    String *lname;

    p = parms;
    while (p) {
      pname = Getattr(p,"name");
      lname = Copy(pname);
      Setattr(p,"lname",lname);
      p = nextSibling(p);
    }
  }
};  /* class RUBY */
  
/* -----------------------------------------------------------------------------
 * swig_ruby()    - Instantiate module
 * ----------------------------------------------------------------------------- */

extern "C" Language *
swig_ruby(void) {
  return new RUBY();
}


/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */

