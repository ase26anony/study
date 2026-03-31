/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Level1 {
    /* Nested parentheses: function pointer array */
    int (*callbacks[5])(void);
    
    /* Nested braces: anonymous struct */
    struct {
        /* Bit-fields inside anonymous struct */
        unsigned int flags : 4;
        unsigned int state : 2;
    } status;
    
    /* Nested brackets: multi-dimensional array */
    char matrix[3][3][3];
};

/* 2. Function Pointers with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int))(char);

/* Function pointer with array parameter */
void (*signal_handler)(int sig, const char *msg[10]);

/* Triple-nested function pointer */
char *(*(*nested_fp)(int (*)(float)))(double);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4];  /* Multi-dimensional */
    long *ptr_array[10];     /* Array of pointers */
    int flex_array[];        /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions with Bit-fields */
struct BitFieldMaster {
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c : 4;
        } bits;
        unsigned short all;
    } u1;
    
    struct {
        long : 16;  /* Unnamed bit-field */
        long x : 8;
        long y : 8;
    };
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_OF(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

PTR_FUNC(int) *simple_func_ptr;
ARRAY_OF(PTR_FUNC(char), 5) func_array;
NESTED_PTR(double) complex_nested_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func)(void) __attribute__((noreturn));
} __attribute__((deprecated));

/* 7. Single Declaration Combining All Bracket Types */
struct UltimateTest {
    /* Combination 1: Function pointer with complex signature */
    void (*(*signal(int sig, void (*handler)(int)))(int))(char);
    
    /* Combination 2: Array of function pointers with nested params */
    int (*(*pfa[2][3])(int (*)(char[10])))(double);
    
    /* Combination 3: Nested union with bit-fields */
    union {
        struct {
            int x : 4;
            int y[2][2];
            void (*func)(int (*)(char));
        } inner;
        long long data;
    } u __attribute__((aligned(32)));
    
    /* Combination 4: Flexible array of pointers to functions */
    int (*(*flex_funcs[])(void))(int, char);
    
    /* Combination 5: Multi-dimensional array with attributes */
    volatile int restricted_array[4][4] __attribute__((aligned(64)));
};

/* Additional complex cases to ensure coverage */

/* Nested type definitions */
typedef struct Outer {
    struct Inner {
        union {
            int (*func_ptr)(int[5]);
            struct {
                char *(*name_generator)(void);
            } gen;
        } choice;
    } *inner_ptr;
    
    /* Anonymous union */
    union {
        int a;
        struct {
            short b;
            short c;
        };
    };
} OuterType;

/* Function returning pointer to array */
int (*func_returning_array_ptr(int size))[10];

/* Pointer to array of function pointers */
int (*(*ptr_to_func_array)[5])(int, int);

/* Complex const/volatile qualified types */
const volatile struct {
    int (*(* volatile cv_func)(const int *))[10];
    void (* volatile restart)(void);
} global_state;

/* Nested macro expansions with brackets */
#define WRAP(T) struct { T value; }
#define DOUBLE_WRAP(T) WRAP(WRAP(T))

DOUBLE_WRAP(int) double_wrapped;
WRAP(int (*)(int)) func_wrapper;

/* Template-like macro for generic function pointers */
#define GENERIC_FUNC(RET, ARG) RET (*)(ARG)
GENERIC_FUNC(GENERIC_FUNC(int, char), double) meta_func;

/* Test case with all delimiters in sequence */
struct FinalChallenge {
    int a;                                  /* default case */
    int (*b)(int);                          /* parentheses */
    int c[10];                              /* brackets */
    struct { int d; };                      /* braces */
    int (*e[5])(int (*)(char[3]));          /* all three */
} __attribute__((packed));                  /* attribute parentheses */

#endif /* TEST_GENGTYPE_COVERAGE_H */
