/* test_gengtype_coverage.h
 * Complex type definitions to exercise consume_balanced() parser
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags : 4;
        unsigned int : 4;  /* Unnamed bit-field */
        signed int value : 8;
    } bitfield_struct;
    
    /* Multi-dimensional array (brackets) */
    double matrix[2][3][4];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[5];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Simple function pointer */
typedef void (*SimpleFuncPtr)(int);

/* Pointer to function returning function pointer */
typedef int (*(*FuncReturningFunc)(char))(double);

/* Complex nested parentheses */
void (*signal_handler(int sig, void (*handler)(int)))(int);

/* Function taking function pointer that takes array */
int process_array(int (*processor)(char[10]), int count);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct WithFlexArray {
    int length;
    int data[];  /* Flexible array member */
};

struct MultiDimArrays {
    int grid[3][4][5];
    char *string_table[10][20];
    float (*ptr_array[5])[7];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousContainer {
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            unsigned int c : 3;
        } bits;
        unsigned int full;
    } u1;
    
    struct {
        long x : 8;
        long y : 8;
        long z : 16;
    } __attribute__((packed));
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Use the macros to generate bracket sequences */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(char, 5), 3) macro_array;
NESTED_PTR(double) complex_nested_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func)(void) __attribute__((noreturn));
} __attribute__((deprecated));

int variable __attribute__((used, section(".data"))) = 42;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Combination 1: Function pointer with complex signature */
    void (*(*signal(int sig, void (*func)(int)))(int))(void);
    
    /* Combination 2: Array of function pointers with nested params */
    int (*(*pfa[2])(int (*)(char[10])))(double);
    
    /* Combination 3: Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3[2];  /* Bit-field with array declarator */
        } s;
        long l;
    } u __attribute__((aligned(8)));
    
    /* Combination 4: Multi-dimensional flexible-like array */
    int (*flex_matrix[])[3][4];
    
    /* Combination 5: Deeply nested parentheses */
    int (*(*(*deep_nest)(void))[5])(char (*(*)(int))[10]);
};

/* Additional complex cases to ensure coverage */

/* Nested typedef with all bracket types */
typedef struct {
    union {
        int (*funcs[2])(int[3]);
        struct {
            char *(*get_name)(void);
            void (*set_name)(char[50]);
        } ops;
    } u;
    int (*(*get_callback)(void))(int, char **);
} ComplexType;

/* Template-like macro with multiple bracket levels */
#define CALLBACK(T) T (*(*callback)(T (*)(T[2])))(T)
CALLBACK(float) cb_instance;

/* Struct with array of structs containing function pointers */
struct RecursivePattern {
    struct {
        int (*compare)(const void *, const void *);
        void (*action)(int[10]);
    } operations[5];
    
    struct RecursivePattern *next;
};

/* Union with nested anonymous struct containing bit-fields and arrays */
union MixedUnion {
    struct {
        unsigned char bytes[4];
        struct {
            unsigned int ready : 1;
            unsigned int error : 2;
            unsigned int : 5;  /* Padding */
        } status;
    } packet;
    unsigned int raw;
    void (*handlers[3])(union MixedUnion *);
};

/* Function declarations with complex return types */
struct OuterStruct *(*factory(int mode))(void);
int (*(*create_processor(void))[10])(char *);

/* Typedef combining everything */
typedef int (*(*(*UltimateFuncPtr)(int (*)[3]))(char (*(*)(void))[5]))(void);

#endif /* TEST_GENGTYPE_COVERAGE_H */
