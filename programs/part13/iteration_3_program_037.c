/* test_gengtype_coverage.h
 * 
 * This header is designed to exercise the `consume_balanced` function in
 * gengtype-parse.cc by providing complex nested type definitions that use
 * all three bracket types: (), [], and {}.
 * The goal is to trigger the uncovered lines 341-352, ensuring each case
 * (default, '(', '[', '{') is executed.
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* ==================== 1. Complex Nested Type Definitions ==================== */

/* A struct containing an array of pointers to functions, which in turn
 * contain nested unions with bit-fields. */
struct Outer {
    /* Function pointer array with nested parameter */
    int (*func_array[3])(int, char);
    
    /* Nested union with bit-fields inside a struct */
    struct {
        union {
            unsigned int flags : 4;
            unsigned int mode  : 2;
        } bits;
        int data;
    } inner;
    
    /* Pointer to a struct containing a flexible array member */
    struct FlexContainer {
        int count;
        int flex[];
    } *flex_ptr;
};

/* ==================== 2. Function Pointer Declarations with Varied Signatures ==================== */

/* Pointer to function returning pointer to function */
int (*(*complex_fp)(int (*)(char[10])))(double);

/* Even more nested: pointer to function that takes a function pointer
 * returning a function pointer */
void (*(*nested_fp)(int (*(*)(char))(float)))(long);

/* ==================== 3. Multi-dimensional Arrays and Flexible Array Members ==================== */

/* Multi-dimensional arrays */
int multi_dim[2][3][4];
char strings[5][10];

/* Struct with flexible array member */
struct FlexStruct {
    int len;
    double items[];
};

/* ==================== 4. Nested Anonymous Structs/Unions and Bit-fields ==================== */

struct BitFieldContainer {
    /* Anonymous struct */
    struct {
        unsigned int a : 1;
        unsigned int b : 3;
        unsigned int c : 4;
    };
    
    /* Anonymous union */
    union {
        int x;
        long y : 8;
    };
    
    /* Named union with bit-fields */
    union {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2;
        unsigned int flag3 : 5;
    } flags;
};

/* ==================== 5. Macro Expansions Generating Brackets ==================== */

#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))[]

/* Use macros to generate bracket-heavy declarations */
PTR_FUNC(int) *func_ptr;
ARRAY_TYPE(PTR_FUNC(char), 5) array_of_func_ptrs;
NESTED_PTR(double) complex_nested;

/* ==================== 6. Attribute Syntax with Parentheses ==================== */

/* Struct with attribute */
struct __attribute__((aligned(16))) AlignedStruct {
    int value;
    char data[32];
} __attribute__((packed));

/* Function with attribute */
void __attribute__((noreturn)) fatal_error(void) __attribute__((cold));

/* Variable with attribute */
int important_var __attribute__((used, section(".data")));

/* ==================== 7. Include All Bracket Types in Single Declaration ==================== */

/* The ultimate test: a single declaration combining all bracket types */
struct UltimateTest {
    /* Function pointer with complex signature */
    void (*signal(int sig, void (*func)(int)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[2])(int (*)(char[10]));
    
    /* Anonymous union with bit-field */
    union {
        int x;
        long y : 8;
    } u;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Nested struct with flexible array */
    struct {
        int count;
        struct {
            char *name;
            int values[];
        } items[2];
    } container;
} __attribute__((aligned(32)));

/* ==================== Additional Edge Cases ==================== */

/* Empty brackets */
struct Empty {
    int x;
}; /* This should hit default case for ';' */

/* Nested brackets to maximum depth */
struct DeepNest {
    int (*(*(*deep)[5])(int (*)(char))[3])(void);
};

/* Typedef with complex type */
typedef int (*(*Callback)(int (*)(char)))(void);

/* Function returning pointer to array */
int (*returns_array_ptr(void))[10];

/* Pointer to array of function pointers */
int (*(*array_of_funcs[5])(void))[3];

#endif /* TEST_GENGTYPE_COVERAGE_H */
