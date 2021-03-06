/* SF Bug #445219, submitted by Krzysztof Kozminski
   <kozminski@users.sf.net>. 

   Swig 1.3.6 gets confused by pure virtual destructors,
   as in this file:
*/

%module(ruby_minherit="1") pure_virtual

%warnfilter(813,833) E; /* C#, Java multiple inheritance */

%nodefault C;
%nodefault E;

%inline %{

class A {
 public:
  A() { };
  virtual ~A() = 0;
  virtual void something() = 0;
  virtual void method() = 0;
};

class B : public A {
public:
  B() {};
  virtual ~B() { };
  virtual void something() { };
  virtual void method() { };
};

/* class C is abstract because it doesn't define all methods in A */
class C : public A {
 public:
  virtual ~C() { };
  virtual void method() { };
}
;

/* class D is not abstract, it defines everything */
class D : public C {
 public:
  virtual ~D() { };
  virtual void something() { };
}
;

/* Another abstract class */
class AA {
  public:
     virtual void method2() = 0;
};

/* Multiple inheritance between two abstract classes */
class E : public C, public AA {
public:
   virtual void something() { };
};

/* Fill in method from AA.  This class should be constructable */
#ifdef SWIGCSHARP
%ignore F::method2(); // Work around for lack of multiple inheritance support - base AA is ignored.
#endif
class F : public E {
   public:
     virtual void method2() { }
};

%}

%{
A::~A() {}
%}

