/* GCC garbage collection type description - Test default case triggers */
%{
#include <stdio.h>
#include <stdlib.h>
/* Preprocessor directives should trigger default case */
#ifdef TEST_DEFAULT
#define MAX_SIZE 100
#endif
%}

%typedef int my_int_type;

/* Various punctuation that should trigger advance() in default case */
%struct default_test {
    /* Semicolon in field definition */
    int field1; /* This semicolon triggers default */
    
    /* Comma in function parameters */
    int (*func_ptr)(int, char**, double); /* Commas trigger default */
    
    /* Asterisk for pointers */
    struct default_test *next; /* Asterisk triggers default */
    
    /* Ampersand (in comments shouldn't matter, but in type might) */
    /* int& ref; */ /* C++ reference, might be ignored */
    
    /* Equals sign in initializers (if supported) */
    int counter = 0; /* Equals sign triggers default */
    
    /* Colon in bitfields */
    unsigned int flags : 4; /* Colon triggers default */
    
    /* Question mark (not valid C, but tests default) */
    /* int? optional; */ /* Would trigger default if parsed */
    
    /* Tilde for destructors */
    /* ~default_test(); */ /* C++, triggers default */
};

%var struct default_test global_default;
