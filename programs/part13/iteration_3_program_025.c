/* test_gengtype_coverage.h
 * 
 * This header file is designed to exercise the `consume_balanced` function
 * in gengtype-parse.cc by providing complex nested type definitions that
 * use all three bracket types: parentheses '()', brackets '[]', and braces '{}'.
 * The goal is to trigger the uncovered lines (default case and each bracket case)
 * during gengtype parsing.
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* ==================== MACRO EXPANSIONS GENERATING BRACKETS ==================== */

/* Macro that expands to a function pointer type with parentheses */
#define PTR_FUNC(T) T (*)(T)

/* Macro that expands to a multi-dimensional array type */
#define MD_ARRAY(T, d1, d2) T[d1][d2]

/* Macro that expands to a struct with an anonymous union */
#define ANON_UNION_STRUCT(T1, T2) struct { union { T1 a; T2 b; }; }

/* ==================== FUNCTION POINTER DECLARATIONS ==================== */

/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function (nested parentheses) */
typedef int (*(*complex_func_ptr)(int (*)(char[10])))(double);

/* Function pointer with parameters containing nested parentheses */
typedef void (*signal_handler)(int sig, void (*cleanup)(void*));

/* Even more nested: pointer to function that takes a function pointer
   that itself takes a function pointer */
typedef int (*(**nested_func_ptr_ptr)(int (*(*)(int))(float)))(char);

/* ==================== COMPLEX NESTED TYPE DEFINITIONS ==================== */

/* Struct containing an array of pointers to functions */
struct FunctionTable {
    /* Array of function pointers */
    int (*callbacks[5])(int, char**);
    
    /* Pointer to array of function pointers */
    void (*(*signal_handlers)[3])(int);
    
    /* Nested: array of pointers to functions returning function pointers */
    int (*(*func_returns[2])(void))(int, int);
};

/* Union with nested struct containing bit-fields */
union DataContainer {
    struct {
        unsigned int flags : 4;
        unsigned int mode : 2;
        unsigned int : 2;  /* unnamed bit-field */
        unsigned int status : 8;
    } bits;
    
    int raw_value;
    
    /* Anonymous struct inside union */
    struct {
        long id;
        char tag;
    };
};

/* Struct with all bracket types combined in single declaration */
struct UltimateTest {
    /* Function pointer with complex signature */
    void (*signal(int sig, void (*func)(int)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[2])(int (*)(char[10]));
    
    /* Anonymous union with bit-fields */
    union {
        int x;
        long y : 8;
        struct {
            unsigned short a : 3;
            unsigned short b : 5;
        } bits;
    } u;
    
    /* Multi-dimensional arrays */
    int matrix[3][2];
    double tensor[2][3][4];
    
    /* Flexible array member */
    int flex[];
};

/* ==================== ATTRIBUTE SYNTAX WITH PARENTHESES ==================== */

/* Struct with GCC attributes */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data;
    char padding;
} __attribute__((deprecated));

/* Function pointer type with attribute */
typedef int (__attribute__((const)) *const_func_ptr)(int, int);

/* Variable with attribute containing parentheses */
extern int global_array[10] __attribute__((aligned(32)));

/* ==================== MORE COMPLEX NESTING ==================== */

/* Struct containing nested anonymous structs/unions */
struct DeepNest {
    /* Level 1: anonymous struct */
    struct {
        int id;
        
        /* Level 2: anonymous union */
        union {
            float f;
            
            /* Level 3: struct with bit-fields */
            struct {
                unsigned int a : 1;
                unsigned int b : 1;
                unsigned int c : 6;
            } flags;
            
            /* Level 3: array */
            char str[20];
        } data;
        
        /* Level 2: function pointer array */
        void (*handlers[3])(struct DeepNest*);
    } header;
    
    /* Using macro expansions */
    PTR_FUNC(int)* macro_func_ptr;
    MD_ARRAY(float, 5, 5) md_array;
    ANON_UNION_STRUCT(int, float) anonymous_union_struct;
};

/* Union with extremely complex type */
union ComplexUnion {
    /* Pointer to array of function pointers returning pointers to arrays */
    int (*(*(*func_array_ptr)[5])(int))[10];
    
    /* Nested struct with all bracket types */
    struct {
        int (*(*nested_fp)(int[3]))(void);
        struct {
            unsigned int x : 1;
            unsigned int y : 1;
        } bits[2];
        char data[];
    } nested;
};

/* ==================== EDGE CASES ==================== */

/* Empty braces (should still trigger '{' case) */
struct EmptyStruct {};

/* Single element array */
struct SingleElement {
    int arr[1];
};

/* Zero-length array (GCC extension) */
struct ZeroLength {
    int count;
    int items[0];
};

/* Nested parentheses in function-like macro */
#define APPLY(func, arg) func(arg)
#define SQUARE(x) ((x)*(x))

/* Type definition with __attribute__ containing nested parentheses */
typedef struct __attribute__((designated_init)) {
    int field1;
    int field2;
} DesignatedStruct;

/* ==================== FINAL COMPREHENSIVE TEST ==================== */

/* One declaration that combines everything */
struct ComprehensiveTest {
    /* Multi-dimensional array of function pointers */
    int (*(*func_matrix[2][3])(int))[4];
    
    /* Anonymous struct with bit-fields and array */
    struct {
        unsigned int a : 3, b : 5;
        char name[50];
        
        /* Nested anonymous union */
        union {
            int x;
            struct {
                short s;
                char c;
            } inner;
        } data;
    } header;
    
    /* Flexible array member of structs */
    struct {
        int id;
        double value;
    } records[];
} __attribute__((packed, aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
