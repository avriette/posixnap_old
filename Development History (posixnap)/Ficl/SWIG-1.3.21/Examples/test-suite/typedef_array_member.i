%module typedef_array_member
%inline %{

typedef char amember[20];

struct Foo {
   amember x;
};

%}
