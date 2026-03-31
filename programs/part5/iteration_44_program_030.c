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
%typedef char* string_ptr;

/* Various punctuation that should trigger default case */
%struct default_test {
    /* Semicolons, commas, asterisks in normal contexts */
    int field1;     /* ; triggers default */
    char *field2, field3;  /* , and * trigger default */
    float field4;   /* Another ; */
    
    /* Function pointer with complex signature */
    void (*complex_func)(int (*nested)(char**, int[10]), struct default_test*);
    
    /* Multiple pointer levels */
    struct default_test ****deep_ptr;
    
    /* Reference in comment: & should be ignored in comment */
    int& ref_field; /* Actually & triggers default if not in comment */
};

/* Union with bitfields */
%union bitfield_union {
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 3;
        unsigned int : 4;  /* Unnamed bitfield */
        unsigned int value : 8;
    } bits;
    unsigned short all;
};

%var struct default_test global_default;
