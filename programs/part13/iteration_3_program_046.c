/* test_gengtype_coverage.h - Header file designed to exercise gengtype's 
 * consume_balanced function for all bracket types and default case.
 * This file contains complex nested type definitions that will cause
 * gengtype's parser to recursively process parentheses, brackets, and braces.
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Nested struct with function pointer array */
    struct Inner1 {
        /* Function pointer returning pointer to function */
        int (*(*func_table[5])(int))(char);
        
        /* Union with bit-fields (braces) */
        union {
            unsigned int flags : 4;
            unsigned int mode : 2;
            unsigned int : 26;  /* Unnamed bit-field */
        } status;
        
        /* Multi-dimensional array (brackets) */
        double matrix[3][3][2];
    } inner1;
    
    /* Anonymous struct with flexible array member */
    struct {
        int count;
        /* Flexible array member (special bracket case) */
        int data[];
    } flex_container;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
typedef int (*(*ComplexFuncPtr)(int (*)(char[10])))(void);

/* Function pointer with nested parentheses in parameters */
void (*signal_handler)(int sig, void (*handler)(int, siginfo_t*, void*));

/* 3. Multi-dimensional Arrays and Complex Declarations */
/* Three-dimensional array with pointer elements */
int (*volatile multi_array[2][3][4])(float);

/* Array of pointers to arrays */
char *(*(string_table[10]))[20];

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldContainer {
    /* Outer anonymous union */
    union {
        /* Inner anonymous struct with bit-fields */
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c : 4;
            unsigned int : 24;  /* Padding */
        } bits;
        
        unsigned int full_word;
    } data;
    
    /* Another level of nesting */
    struct {
        union {
            long x : 8;
            long y : 16;
            long z : 8;
        } nested_union;
    } another_level;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*(*)(T))(T)
#define ARRAY_TYPE(N, T) T (*)[N]
#define COMPLEX_PTR(T) T (*(*(*)(T (*)[5]))(int))[10]

/* Use the macros to create complex declarations */
PTR_FUNC(int) macro_func_ptr;

ARRAY_TYPE(7, double) array_ptr;

struct MacroStruct {
    COMPLEX_PTR(char) complex_member;
};

/* 6. Attribute Syntax with Parentheses */
/* Struct with alignment attribute (double parentheses) */
struct __attribute__((aligned(32), packed)) AttributedStruct {
    int data[8];
    char __attribute__((aligned(8))) aligned_char;
} __attribute__((deprecated));

/* Function with attributes */
int __attribute__((noinline, noclone)) 
attributed_function(int x) __attribute__((warn_unused_result));

/* Variable with section attribute */
int global_var __attribute__((section(".data.unusual"), used));

/* 7. Include All Bracket Types in Single Declaration */
/* Ultimate complex declaration combining all bracket types */
struct UltimateType {
    /* Member 1: Function pointer with complex signature */
    void (*(*signal_member)(int sig, 
                           void (*func)(int, 
                                       struct UltimateType*)))(int);
    
    /* Member 2: Array of function pointers */
    int (*(*func_array[2][3])(int (*)(char[10])))(float);
    
    /* Member 3: Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 3;
            unsigned int : 4;
            unsigned int flags3 : 8;
        } bits;
        
        unsigned int raw;
        
        struct {
            char a;
            short b;
        } __attribute__((packed)) packed_data;
    } data_union;
    
    /* Member 4: Multi-dimensional flexible array-like member */
    struct {
        int rows;
        int cols;
        int (*matrix)[];
    } dynamic_array;
    
    /* Member 5: Pointer to array of pointers to functions */
    float (*(*(*complex_pointer)[5])(double (*)[3]))(int);
} __attribute__((aligned(64)));

/* Additional edge cases */
/* Empty struct (still has braces) */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct OnlyBitfields {
    unsigned int a : 1;
    unsigned int b : 1;
    unsigned int : 30;
};

/* Declaration with all brackets in parameter */
typedef int (*(*AllBracketsInParam)(int (*array_param[2])[3],
                                   struct { int x; int y; } point,
                                   void (*func)(void)))(char);

/* Nested attribute specifications */
struct __attribute__((aligned(__alignof__(long double)))) 
       DoubleAttributed {
    int value;
} __attribute__((may_alias));

/* Function with parameter containing array of function pointers */
void process_callbacks(
    int (*callbacks[10])(int, void*),
    void (*completion)(int result, char error_msg[])
);

/* Typedef combining everything */
typedef struct {
    union {
        int (*(*func_ptr)(int (*)(char)))[5];
        struct {
            unsigned int : 16;
            unsigned int value : 16;
        };
    } choice;
    
    int (*matrix[][10])(float, double);
} UltimateTypedef;

#endif /* TEST_GENGTYPE_COVERAGE_H */
