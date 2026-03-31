/* Test file for gengtype parser - default case triggers */
%{
#include <stdio.h>
#include <stdlib.h>
/* Preprocessor directives should trigger default case */
#ifdef TEST_DEFAULT
#define MAX_SIZE 100
#endif
%}

%typedef int my_int_type;

/* Various punctuation that should hit default case */
%struct default_test {
    /* Semicolons, commas, asterisks in normal context */
    int *pointer;           /* asterisk */
    int array[10];          /* brackets handled by consume_balanced */
    char comma_list[3];     /* comma */
    
    /* Function pointer with complex signature */
    void (*complex_func)(int (*)(char **), double);
    
    /* Nested type with multiple punctuation */
    struct inner {
        int **double_ptr;   /* double asterisk */
        char *name;         /* asterisk */
    } inner_struct;
    
    /* Union with bitfields */
    union {
        unsigned int flags : 4;  /* colon */
        signed int value : 28;   /* colon */
    } bitfield_union;
};

/* Global variable declaration */
%var struct default_test global_instance;
