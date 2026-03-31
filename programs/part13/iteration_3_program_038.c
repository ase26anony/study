/* test_gengtype_coverage.h - Complex type definitions for gengtype parser coverage */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void);
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[5];
    
    /* Nested struct with union */
    struct {
        union {
            int x;
            char y[10];
        } nested_union;
        long z;
    } anonymous_struct;
    
    /* Multi-dimensional array */
    double matrix[4][4][4];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function taking function pointer as parameter */
typedef void (*SignalHandler)(int);
typedef SignalHandler (*GetHandlerFunc)(int signum);

/* Triple-nested function pointer */
typedef int (*(*(*triple_func_ptr)(void))(int))(char);

/* Function returning pointer to function with array parameter */
char (*(*func_returning_func_ptr)(void))[10];

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int fixed_array[3][2][4];
    int flexible_array[];
};

struct NestedArrays {
    char *string_array[5][10];
    struct ArrayContainer *container_array[2];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int flags : 3;
    signed int value : 16;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 2;
            unsigned char c : 4;
        } bits;
        unsigned char byte;
    };
    
    /* Nested anonymous struct */
    struct {
        long : 32;  /* unnamed bit-field */
        long field1 : 8;
        long field2 : 24;
    };
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(N) (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Using the macros to generate complex declarations */
PTR_FUNC(int) simple_func_ptr;
int (ARRAY_PTR(5)) array_ptr_func;

typedef NESTED_FUNC(double) ComplexFuncType;

/* Macro generating struct with all bracket types */
#define COMPLEX_STRUCT(T) \
    struct T##_struct { \
        T (*methods[2])(T (*)(T[5])); \
        union { \
            T arr[3][3]; \
            struct { \
                T x; \
                T y; \
            } point; \
        } data; \
    }

COMPLEX_STRUCT(float);
COMPLEX_STRUCT(int);

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated));
} __attribute__((visibility("default")));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* Variable with attribute containing parentheses */
extern int global_var __attribute__((section(".data"), aligned(32)));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateStruct {
    /* Complex function pointer declaration */
    void (*signal_handler(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*func_ptr_array[2])(int (*callback)(char buffer[256]));
    
    /* Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 4;
            unsigned int : 24;  /* padding */
        } bits;
        unsigned int full;
    } flags;
    
    /* Multi-dimensional pointer array */
    int *(*(*nested_ptr_array[3])[2])[4];
    
    /* Flexible array member at the end */
    struct UltimateStruct *self_referential[];
} __attribute__((packed));

/* Even more complex: struct containing all bracket types recursively */
struct RecursiveNesting {
    /* Function returning pointer to array of function pointers */
    int (*(*(*get_operations(void))[5])(int, ...))[3];
    
    /* Nested anonymous union-struct combination */
    union {
        struct {
            int (*compare)(const void *, const void *);
            void (*free)(void *);
        } ops;
        void *vtable[2];
    } methods;
    
    /* Array of structs containing arrays */
    struct {
        char name[50];
        int values[10][10];
    } entries[5];
    
    /* Pointer to self with attributes */
    struct RecursiveNesting *next __attribute__((may_alias));
};

/* Additional edge cases */

/* Empty struct/union */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct OnlyBitFields {
    unsigned : 1;
    unsigned field : 7;
    unsigned : 0;  /* force alignment */
    unsigned next : 8;
};

/* Function with complex return type */
struct ReturnStruct {
    int (*func_ptrs[3])(void);
    union {
        int i;
        float f;
    } value;
};

struct ReturnStruct (*get_complex_struct(void))[2];

/* Typedef chains with brackets */
typedef int IntArray5[5];
typedef IntArray5 IntMatrix[3];
typedef IntMatrix (*MatrixFuncPtr)(void);

/* Final test: everything combined */
typedef struct {
    MatrixFuncPtr get_matrix;
    struct UltimateStruct *ultimate;
    ComplexFuncType complex_func;
    struct BitFieldStruct bitfield;
    __attribute__((aligned(64))) char cache_line[64];
} MasterStruct __attribute__((aligned(128)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
