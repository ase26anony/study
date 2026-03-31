/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(int, char);
    
    /* Nested struct with union */
    struct InnerStruct {
        union {
            int x;
            double y;
            /* Pointer to function returning pointer to array */
            int (*(*complex_func)(void))[5];
        } data;
        
        /* Multi-dimensional array */
        int matrix[2][3][4];
        
        /* Anonymous struct with bit-fields */
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 3;
            unsigned int : 4;  /* Unnamed bit-field */
            signed int value : 8;
        } bits;
    } inner;
    
    /* Flexible array member */
    int flex_array[];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*func_ptr1)(int (*)(char)))(double);

/* Complex function pointer with array parameter */
void (*signal_handler)(int sig, void (*callback)(int, const char*));

/* Function returning pointer to array of function pointers */
int (*(*get_handler_table(void))[10])(int, int);

/* 3. Multi-dimensional Arrays and Nested Types */
typedef int Matrix4D[2][3][4][5];

struct ArrayContainer {
    Matrix4D md_array;
    char* string_array[5][10];
    /* Pointer to flexible array member */
    struct {
        int length;
        int data[];
    } *flex_struct_ptr;
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
union MegaUnion {
    struct {
        /* Anonymous union inside anonymous struct */
        union {
            long long ll;
            struct {
                unsigned int a : 16;
                unsigned int b : 16;
                unsigned int c : 16;
                unsigned int d : 16;
            } parts;
        } data;
        
        /* Function pointer with bit-field parameters */
        void (*bit_callback)(unsigned int : 4, unsigned int : 4);
    } s;
    
    /* Array of pointers to functions with complex signatures */
    int (*(*func_table[5])(int (*)[3]))[2];
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*(*)(T (*)[2]))(T)
#define ARRAY_DECL(T, N) T (*name)[N]
#define NESTED_PTR(T) T (*(*(*)(void))[5])(void)

/* Use the macros to generate complex declarations */
PTR_FUNC(int) macro_func_ptr;

struct MacroStruct {
    ARRAY_DECL(char, 10) string_ptr;
    NESTED_PTR(double) nested_ptr_func;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func_ptr)(void) __attribute__((nonnull(1, 2)));
} __attribute__((deprecated));

/* Attribute on variable declaration */
int global_var __attribute__((used, section(".data"))) = 42;

/* Attribute on function declaration */
void __attribute__((format(printf, 1, 2))) 
log_message(const char *fmt, ...) __attribute__((nothrow));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Complex function pointer declaration */
    void (*(*signal(int sig, void (*handler)(int)))(int, const char*));
    
    /* Array of function pointers with nested parameters */
    int (*(*func_array[2][3])(int (*)(char[10]), void*))[5];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : 4;
            unsigned int : 8;
            unsigned int c : 16;
        } bits;
        unsigned int full;
    } flags;
    
    /* Multi-dimensional array with pointer elements */
    struct Node* (*graph[10][10])(int, struct Node*);
    
    /* Flexible array member of function pointers */
    int (*flex_funcs[])(int, int);
    
    /* Nested struct with all bracket types */
    struct {
        int (*(*nested_func)(int[2][2]))(void);
        char data[3][4];
        union {
            short s;
            long l;
        } value;
    } nested;
} __attribute__((aligned(32)));

/* Additional complex typedefs */
typedef int (*(*ComplexCallback)(int (*array)[5], void (*func)(void)))(char);

/* Union with nested struct containing array of function pointers */
union FinalUnion {
    struct {
        int (*compare)(const void*, const void*);
        void (*sort)(int[], int, int (*)(int, int));
        char* (*format)(int, char[20]);
    } funcs;
    
    struct {
        int data[10];
        struct {
            unsigned int valid : 1;
            unsigned int : 7;
            unsigned int count : 8;
        } status;
    } info;
};

/* Template-like macro for generic function pointers */
#define GENERIC_FUNC_PTR(RET, PARAMS) RET (*)(PARAMS)
#define ARRAY_FUNC_PTR(T, N) T (*(*[N])(void))(void)

/* Use the template macros */
GENERIC_FUNC_PTR(int, (int, char*)) generic_func;
ARRAY_FUNC_PTR(double, 5) array_of_func_ptrs;

/* Struct with pointer to incomplete type */
struct ForwardDecl;
struct HasForwardPtr {
    struct ForwardDecl *fwd;
    void (*method)(struct ForwardDecl*);
};

/* Enum with last value for array size */
enum Constants {
    MAX_SIZE = 100,
    BUFFER_LEN = 256
};

/* Variable length array in struct (C99 feature) */
struct VLAHolder {
    int rows;
    int cols;
    double matrix[][MAX_SIZE];  /* Flexible array member with constant */
};

/* Multiple levels of indirection */
char ****quadruple_ptr;
int (*(*(*triple_func_ptr)(int))[3])(char);

/* __builtin_choose_expr with brackets */
#define SELECT_TYPE(T, cond) __builtin_choose_expr(cond, T[10], T[5])
typedef SELECT_TYPE(int, 1) SelectedArray;

#endif /* TEST_GENGTYPE_COVERAGE_H */
