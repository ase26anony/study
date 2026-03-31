/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Nested parentheses: function pointers */
    int (*func_ptr1)(void);
    
    /* Nested brackets: multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Nested braces: anonymous struct */
    struct {
        int x;
        int y;
    } point;
    
    /* Combination: array of function pointers */
    void (*callbacks[5])(int, char);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int (*)(char)))(double);

/* Function taking function pointer as parameter */
void register_callback(int (*cb)(int, char), int priority);

/* Nested function pointers in parameters */
char *(*signal_handler(int sig, void (*handler)(int)))(const char *);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4];
    char strings[5][256];
    
    /* Flexible array member */
    int data[];
};

/* Array of pointers to arrays */
int *(*(*array_of_arrays[3])[2])[4];

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int flags : 4;
    signed int value : 16;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } parts;
        unsigned char whole;
    } byte_packer;
    
    /* Nested anonymous struct */
    struct {
        long x : 8;
        long y : 8;
        long z : 16;
    } coordinates;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(N) (*)[N]
#define CALLBACK(T) void (*)(T)

/* Using macros to generate complex types */
PTR_FUNC(int) *int_func_ptr;
int (ARRAY_PTR(5)) array_ptr;
CALLBACK(struct BitFieldStruct) callback_to_struct;

/* Macro generating nested parentheses */
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)
NESTED_FUNC(double) extremely_nested_func;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated));
} __attribute__((visibility("default")));

/* Function with attributes */
int __attribute__((noinline, noclone)) 
attributed_function(int a, int b) __attribute__((warn_unused_result));

/* Variable with attribute containing parentheses */
extern int global_var __attribute__((section(".data"), aligned(32)));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*signal(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers with complex signatures */
    int (*pfa[2])(int (*)(char[10]), double);
    
    /* Union with bit-fields and anonymous struct */
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 4;
        } bits;
        unsigned char byte;
        int (*func)(int[3][2]);
    } complex_union;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Pointer to array of pointers */
    char *(*names[5])[3];
    
    /* Nested struct with all bracket types */
    struct {
        int (*compare)(const void *, const void *);
        void *data[10];
        struct {
            int x, y;
        } point;
    } nested;
} __attribute__((packed));

/* Additional complex type combinations */

/* Typedef with function pointer returning array pointer */
typedef int (*(*FuncRetArrayPtr)(void))[5];
FuncRetArrayPtr func_array_ptrs[3];

/* Union containing struct with flexible array member */
union ComplexUnion {
    struct {
        int count;
        double values[];
    } flex_struct;
    void *ptr;
    long (*operations[2])(int, ...);
};

/* Struct with pointer to self-referential type */
struct TreeNode {
    int value;
    struct TreeNode *children[4];
    void (*visit)(struct TreeNode *);
};

/* Template-like macro for generic function pointers */
#define GENERIC_CALLBACK(RET, ARGS) RET (*)ARGS
GENERIC_CALLBACK(int, (int, char *)) generic_callback;

/* Deeply nested parentheses */
int (*(*(*deep_nested)(int (*(*)(double))(char)))(float))(void);

/* Array of structs containing function pointers */
struct CallbackRecord {
    char *name;
    int (*callback)(int, char **);
    void (*cleanup)(void *);
} callback_table[] = {
    {"func1", NULL, NULL},
    {"func2", NULL, NULL}
};

/* Final test: everything combined */
struct __attribute__((aligned(64))) MasterType {
    /* All bracket types in member declarations */
    int (*(*member1)[5])(int[3][2]);
    union {
        struct {
            int x : ((8) + (2));
            int y : ((16) - (4));
        } __attribute__((packed)) bits;
        long value;
    } member2;
    void (*(*member3)(int (*)(char), double))(float);
    int member4[((2) * (3))][4];
} __attribute__((visibility("hidden")));

#endif /* TEST_GENGTYPE_COVERAGE_H */
