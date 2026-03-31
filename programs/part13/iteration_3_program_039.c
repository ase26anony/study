/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Level1 {
    /* Function pointer array with nested parameters */
    int (*func_array[3])(char (*callback)(int, float));
    
    /* Nested struct with union */
    struct {
        union {
            /* Bit-fields in nested anonymous struct */
            struct {
                unsigned int flag1:1;
                unsigned int flag2:3;
                unsigned int :4;  /* Unnamed bit-field */
            } bits;
            unsigned char byte;
        } data;
        
        /* Multi-dimensional array */
        double matrix[2][3][4];
    } nested;
    
    /* Pointer to array of function pointers */
    void (*(*complex_ptr)[5])(int, ...);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*signal_handler)(int sig))(void);

/* Function taking function pointer as parameter */
void register_callback(void (*cb)(int (*)(char*), double));

/* Triple-nested function pointer */
char (*(*(*triple_func)(int))(float))(double);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4][5];  /* 4D array */
    
    /* Flexible array member at end */
    long flex_array[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:2;
            unsigned int b:4;
            unsigned int c:6;
        };
        unsigned short all;
    } flags;
    
    /* Nested anonymous struct */
    struct {
        /* Bit-field with array */
        unsigned char bits[2];
        struct {
            int x:8;
            int y:8;
            int z:8;
            int w:8;
        } packed;
    } data;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Using the macros */
PTR_FUNC(int) int_func_ptr;
ARRAY_PTR(double, 3) array_ptr;
NESTED_FUNC(char) nested_func_ptr;

/* Macro generating complex type */
#define COMPLEX_TYPE(T) \
    struct { \
        T (*funcs[2])(T (*)(T[2])); \
        union { T x; T y; } data; \
    }

COMPLEX_TYPE(float) complex_float;
COMPLEX_TYPE(int*) complex_int_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func_ptr)(void) __attribute__((nonnull(1, 2)));
} __attribute__((deprecated));

/* Function with attributes */
int __attribute__((format(printf, 1, 2)))
    log_message(const char *fmt, ...) __attribute__((warn_unused_result));

/* Variable with attribute containing parentheses */
static int __attribute__((section(".data"))) 
    global_var __attribute__((used)) = 42;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Complex function pointer declaration */
    void (*signal(int sig, 
                  void (*handler)(int, 
                                  struct UltimateType*)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[3])(int (*callback)(char matrix[2][3]));
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3[2];  /* Bit-field array (GCC extension) */
        };
        unsigned long long value;
    } flags;
    
    /* Multi-dimensional flexible array member */
    int dynamic_array[][2][3];
    
    /* Nested struct with all bracket types */
    struct {
        int (*nested_func)(void (*)(int[2]), 
                           struct { int x; int y; } point);
        char data[4][3];
    } container;
    
    /* Pointer to array of structs containing function pointers */
    struct {
        float (*compute[2])(double (*)(int));
    } (*compute_array)[5];
} __attribute__((aligned(32)));

/* Additional edge cases */

/* Empty brackets */
struct Empty {
    int empty_array[0];
    void (*empty_func)(void);
    struct {} anonymous;
};

/* Nested brackets in typedef */
typedef int (*(*nested_typedef)(int (*)(int)))(int);

/* Using the typedef */
nested_typedef complex_typedef_var;

/* Function returning array pointer (using GNU extension) */
int (*returns_array_ptr(int x))[10];

/* Struct with pointer to VLA (C99 feature) */
struct WithVLA {
    int rows;
    int cols;
    double (*matrix)[*];  /* Pointer to VLA */
};

/* Multiple attributes with nested parentheses */
int __attribute__((__noinline__, 
                   __hot__, 
                   __attribute__((__optimize__("O3")))))
    optimized_func(int x) __attribute__((const));

/* Final complex declaration combining everything */
static struct UltimateType* (*(*global_complex[2])(
    int param1, 
    struct { 
        int (*method)(int (*)(char[10])); 
        union { 
            long a; 
            double b[2]; 
        } data;
    } param2
))(void) = {0} __attribute__((used));

#endif /* TEST_GENGTYPE_COVERAGE_H */
