/* Java various.i library tests */
%module java_lib_various

%include "various.i"

%apply char **STRING_ARRAY { char **received };
%apply char **STRING_ARRAY { char **get_names };
%apply char **STRING_ARRAY { char **languages };
%apply char *BYTE { char *chars };
%apply char **STRING_OUT { char **string_ptr };

%{
char *langs[] = { "Hungarian", "Afrikaans", "Norwegian", 0 };
%}

%immutable languages; /* TODO: fix, because there is a problem with setters */
%inline %{
char **languages = &langs[0];
%}

%inline %{

int check_animals(char **received) {
    const char *expected[] = {"Cat","Dog","Cow","Goat", 0};
    int strings_match = 1;
    int i=0;
    while (expected[i]) {
        if (strcmp(received[i], expected[i]) != 0)
            strings_match = 0;
        i++;
    }
    return strings_match;
}

char **get_names() {
    static char *values[] = { "Dave", "Mike", "Susan", "John", "Michelle", 0};
    return &values[0];
}

void charout(char *chars) {
    if(chars != NULL)
        sprintf(chars, "by jove");
}

void char_ptr_ptr_out(char **string_ptr) {
    static char ret[] = "returned string";
    *string_ptr = ret;
}

%}

