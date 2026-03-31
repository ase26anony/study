/* test_gengtype_coverage.h - Complex type definitions to exercise gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Outer {
    /* Function pointer array with nested parentheses */
    int (*callbacks[5])(void);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags : 4;
        unsigned int status : 2;
    } bitfield_container;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][3][3];
    
    /* Union with anonymous struct */
    union {
        struct {
            int x;
            int y;
        } point;
        long coordinates;
    } position;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int))(char);

/* Function pointer with array parameter */
void (*signal_handler)(int sig, const char *msg[10]);

/* Nested function pointers in parameters */
typedef void (*callback_t)(int (*comparator)(const void *, const void *));

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4];
    char *string_table[5][10];
    
    /* Flexible array member */
    int flexible[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldMaster {
    /* Anonymous union */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            unsigned int c : 3;
        } bits;
        unsigned char byte;
    };
    
    /* Nested anonymous struct */
    struct {
        long : 16;  /* Unnamed bit-field */
        long value : 32;
        long : 16;
    } data;
    
    /* Another level of nesting */
    struct {
        union {
            int x;
            struct {
                short a : 4;
                short b : 4;
                short c : 8;
            } parts;
        } inner;
    } nested;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(N) (*)[N]
#define NESTED_FUNC_PTR(R, A) R (*)(A (*)(R))

/* Using the macros */
PTR_FUNC(int) int_func_ptr;
int (ARRAY_PTR(10)) array_ptr;
NESTED_FUNC_PTR(double, int) nested_func_ptr;

/* Macro generating complex type */
#define COMPLEX_TYPE(T) \
    struct { \
        T (*funcs[2])(T (*)(T[5])); \
        union { T x; T y; } data; \
    }

COMPLEX_TYPE(float) complex_float;
COMPLEX_TYPE(char *) complex_string;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16))) AlignedStruct {
    int data[4];
} __attribute__((packed));

int variable __attribute__((used, section(".data")));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2)))
log_error(const char *fmt, ...) __attribute__((cold));

/* Type attribute with nested parentheses */
typedef int __attribute__((mode(SI), aligned(8))) aligned_int;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Combination 1: Function pointer returning array pointer */
    int (*(*func_ret_array)(int))[10];
    
    /* Combination 2: Array of function pointers with complex params */
    void (*(*signal_table[5])(int sig, void (*handler)(int)))(int);
    
    /* Combination 3: Nested everything */
    struct {
        union {
            int (*calc)(int matrix[3][3]);
            struct {
                unsigned int : 4;
                unsigned int mode : 4;
            } config;
        } processor;
        char *(*get_name)(void);
    } modules[2];
    
    /* Combination 4: Function with function pointer parameter */
    int (*sort)(void *base, size_t nmemb, size_t size,
                int (*compar)(const void *, const void *));
    
    /* Flexible array of pointers to functions */
    int (*flex_funcs[])(int, int);
};

/* Additional complex declarations to ensure coverage */
/* Pointer to array of function pointers */
int (*(*parray[3])[5])(void);

/* Function returning pointer to array */
int (*returns_array_ptr(float f))[10];

/* Nested attributes */
struct __attribute__((aligned(32)))
__attribute__((packed)) DoubleAttribute {
    long long data[4];
};

/* Multiple bracket combinations in typedef */
typedef int (*(*ComplexTypedef)[10])(char (*(*))(void));

/* Test case for default case coverage */
/* This declaration contains various characters that should hit the default case */
struct DefaultCaseTest {
    int *ptr;           /* asterisk */
    char name[50];      /* identifier, brackets, semicolon */
    float price;        /* identifier, semicolon */
    void (*method)(void); /* mix of tokens */
};

/* Final stress test: deeply nested everything */
struct StressTest {
    struct {
        union {
            int (*(*level1)(int (*)(char[10])))[5];
            struct {
                unsigned int : 1;
                unsigned int flag : 1;
            } bits;
        } u;
        int matrix[2][2];
    } nested[3];
    
    void (*(*func_ptr_array[2])(int, 
          struct { int x; int y; }))(
          int (*)(char (*)[5]));
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
