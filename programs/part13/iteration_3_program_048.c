/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Nested parentheses: function pointers */
    int (*func_ptr)(int, char);
    
    /* Nested brackets: multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Nested braces: anonymous struct */
    struct {
        int x;
        int y;
    } point;
    
    /* Combination: array of function pointers */
    void (*callbacks[5])(void);
};

/* 2. Function pointer declarations with varied signatures */
typedef int (*ComplexFuncPtr)(int (*)(char[10]), void *);
typedef void (*SignalHandler)(int sig, void (*)(int));
typedef char *(*StringProcessor)(const char *(*)(int), int);

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayContainer {
    int md_array[2][3][4];
    double grid[10][10];
    char *strings[];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct BitFieldStruct {
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 3;
            unsigned int flag3 : 4;
        } bits;
        unsigned short all_flags;
    } flags;
    
    struct {
        signed int value : 16;
        unsigned int : 8; /* unnamed bit-field */
        signed int scale : 8;
    } measurements;
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))()

/* Use the macros in declarations */
PTR_FUNC(int) *int_func_ptr;
ARRAY_TYPE(char *, 10) string_array;
NESTED_PTR(double) complex_nested_ptr;

/* 6. Attribute syntax with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    char buffer[64] __attribute__((aligned(32)));
} __attribute__((deprecated));

int __attribute__((const, pure)) pure_function(int x) __attribute__((warn_unused_result));

/* 7. Single declaration combining all bracket types */
struct UltimateTest {
    /* Function returning pointer to function with array parameter */
    void (*signal(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers taking function pointer parameters */
    int (*pfa[2])(int (*)(char[10]), void *);
    
    /* Anonymous union with bit-fields */
    union {
        int x;
        long y : 8;
        struct {
            unsigned a : 4;
            unsigned b : 4;
        } bits;
    } data;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Nested struct with all bracket types */
    struct {
        void (**func_table)(int, int (*)(void));
        char *(*get_name)(void)[5];
        struct {
            int matrix[2][2];
        } inner;
    } nested;
};

/* Additional complex cases */

/* Function pointer returning pointer to array */
int (*(*func_returning_array_ptr)(void))[10];

/* Array of pointers to functions returning pointers to functions */
void (*(*func_array[5])(int))(void);

/* Struct with pointer to array of function pointers */
struct RecursiveStruct {
    int (*(*(*complex)[10])(char *))[5];
    struct RecursiveStruct *next;
};

/* Union containing all bracket types */
union AllBracketsUnion {
    int (*func)(int[5]);
    char *array[10];
    struct {
        int x;
    } str;
};

/* Typedef with complex declarator */
typedef int (*(*ComplexTypedef)[5])(void);

/* Variable with __attribute__ containing parentheses */
extern int global_var __attribute__((visibility("default"), section(".data")));

/* Function declaration with complex parameter */
void process_matrix(int (*matrix)[10][10], 
                    void (*callback)(int, int (*)(void)),
                    char *strings[]);

/* Nested macro expansion with brackets */
#define WRAPPER(T) T __attribute__((aligned(sizeof(T))))
#define DOUBLE_WRAPPER(T) WRAPPER(T) *

WRAPPER(int) wrapped_int;
DOUBLE_WRAPPER(float) double_wrapped_float;

#endif /* TEST_GENGTYPE_COVERAGE_H */
