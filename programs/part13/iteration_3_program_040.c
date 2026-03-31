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
        double y;
    } anonymous;
    
    /* Combined: array of function pointers */
    void (*callbacks[10])(int, char);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func1)(double))(char);

/* Function pointer with nested parameter containing array */
void (*signal_handler)(int sig, void (*handler)(int, char[10]));

/* Triple nested function pointers */
char (*(*(*nested_func_ptr)(int[5]))(double))(float);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    char strings[5][100];
    
    /* Flexible array member */
    int flex_array[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int flags : 3;
    signed int value : 10;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } bits;
        unsigned char byte;
    } byte_union;
    
    /* Nested anonymous struct */
    struct {
        long field1 : 8;
        long field2 : 16;
        long field3 : 32;
    };
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Using macros to generate complex types */
PTR_FUNC(int) simple_func_ptr;
ARRAY_TYPE(PTR_FUNC(char), 5) array_of_func_ptrs;
NESTED_PTR(double) complex_nested_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
} __attribute__((deprecated));

int variable __attribute__((used, aligned(8)));

void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Function returning function pointer with array parameter */
    void (*(*get_handler(int priority))(int))[5];
    
    /* Complex nested type combining everything */
    union {
        struct {
            int (*(*func_array[3])(char[10]))(double);
            struct {
                unsigned int bits : 4;
                int flex[];
            } nested;
        } s;
        long long data[2][3];
    } u __attribute__((aligned(32)));
    
    /* Multi-dimensional array of function pointers */
    int (*(*matrix_handlers[2][3])(void))[4];
    
    /* Anonymous union with bit-fields */
    union {
        int x;
        struct {
            unsigned a : 1;
            unsigned b : 2;
            unsigned c : 5;
        } flags;
    };
} __attribute__((packed));

/* Additional complex combinations */

/* Typedef with nested parentheses */
typedef int (*(*CallbackFactory)(int))(void);

/* Struct containing array of structs containing function pointers */
struct RecursiveComplex {
    struct {
        void (*start)(void);
        int (*process)(char *data[], int count);
        void (*end)(int status);
    } operations[5];
    
    /* Pointer to self-referential structure */
    struct RecursiveComplex *next;
};

/* Union with nested array of function pointers returning pointers to arrays */
union FunctionUnion {
    int (*(*get_array(int size))[10])(float);
    char (*(*get_strings(void))[5])(int);
};

/* Test case with all bracket types in parameter declaration */
void test_function(
    int param1,
    void (*callback)(int, char[10]),
    struct UltimateTest (*factory)(int matrix[3][4]),
    int (*(*nested)(void))[5]
);

/* Macro generating complex nested type */
#define CREATE_COMPLEX_TYPE(Name, T) \
    struct Name { \
        T (*(*get_func)(int n))[10]; \
        union { \
            T data[5]; \
            struct { \
                T x; \
                T y[2]; \
            } point; \
        } value; \
    }

/* Instantiate the macro-generated type */
CREATE_COMPLEX_TYPE(ComplexInt, int);
CREATE_COMPLEX_TYPE(ComplexDouble, double);

/* Final extreme test case */
struct ExtremeNesting {
    /* 4 levels of parentheses */
    int (*(*(*(*level4)(void))(int))(double))(float);
    
    /* 3D array with all dimensions specified */
    unsigned char cube[10][20][30];
    
    /* Deeply nested anonymous structs/unions */
    struct {
        union {
            struct {
                int a : 1;
                int b : 2;
                int c : 3;
                int d : 4;
            } bits;
            long value;
        } inner;
        struct {
            void (*methods[3])(int, ...);
        } ops;
    } nested __attribute__((aligned(64)));
    
    /* Flexible array member of function pointers */
    void (*dynamic_array[])(int, ...);
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
