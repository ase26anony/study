/* Test file for gengtype parser - focusing on default case triggers */
%{
#include <stdio.h>
#include <stdlib.h>
/* Preprocessor directives should trigger default case */
#ifdef TEST_DEFAULT
#define MAX_SIZE 100
#endif
%}

%typedef int my_int_type;
%typedef char* string_ptr;

/* Various punctuation that should trigger advance() */
%struct default_test {
    /* Semicolons in field definitions */
    int field1; /* comment with ; inside */
    
    /* Commas in parameter lists */
    void (*func1)(int, char**, double);
    
    /* Asterisks for pointers */
    struct default_test *next;
    struct default_test **prev_ptr;
    
    /* Ampersand in function pointer (though not standard C) */
    int (&ref_func)(void);
    
    /* Equals sign in bitfield */
    unsigned int flags : 4 = 0;
    
    /* Colon in bitfield */
    unsigned int status : 2;
    
    /* Question mark (triggers error but tests advance) */
    int? questionable_field;
};
