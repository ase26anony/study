/* test_gengtype_coverage.h - Complex type definitions for gengtype parser coverage */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*callback)(int, char));
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flags:4;
            unsigned int mode:2;
            unsigned int :2; /* Unnamed bit-field */
        } bits;
        unsigned char bytes[2];
    } nested_union;
    
    /* Multi-dimensional array */
    double matrix[3][3][3];
    
    /* Pointer to array of function pointers */
    void (*(*complex_ptr)[10])(int, ...);
};

/* 2. Function pointer declarations with varied signatures */
typedef int (*FuncPtr1)(char *(*str_processor)(const char*));
typedef void (*FuncPtr2)(int (*comparator)(const void*, const void*));
typedef char (*(*FuncPtr3)(int))[10];

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    long (*ptr_array)[5];
    char flexible[];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct BitFieldStruct {
    struct {
        unsigned int a:1;
        unsigned int b:3;
        unsigned int c:4;
    };
    union {
        struct {
            unsigned int x:8;
            unsigned int y:8;
        };
        unsigned short word;
    } anon_union;
    unsigned int :16; /* Padding */
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*(*))(T)
#define ARRAY_TYPE(N, T) T (*)[N]
#define COMPLEX_PTR(T) T (*(*(*)(int))[5])(void)

/* Use the macros in declarations */
PTR_FUNC(int) complex_func_ptr;
ARRAY_TYPE(10, double) array_ptr;
COMPLEX_PTR(char) ultra_complex;

/* 6. Attribute syntax with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func_ptr)(void) __attribute__((deprecated));
} __attribute__((visibility("default")));

/* Function with attributes */
int __attribute__((noinline, noclone)) 
attributed_function(int x) __attribute__((warn_unused_result));

/* 7. Single declaration combining all bracket types */
struct UltimateType {
    /* Function returning pointer to function with array parameter */
    void (*(*signal_handler)(int sig, 
                             void (*(*callback_factory)(int))[5])
          )(int event, char data[10]);
    
    /* Array of function pointers taking function pointers */
    int (*(*func_ptr_array[3])(int (*)(char[10]), 
                               void (*)(void)))[2];
    
    /* Nested anonymous struct with bit-fields */
    struct {
        union {
            struct {
                unsigned int a:1;
                unsigned int b:1;
            };
            unsigned char byte;
        } inner;
        int value;
    } anonymous;
    
    /* Multi-dimensional flexible array member */
    int flex_multi[][2][3];
    
    /* Pointer to array of pointers to functions */
    char (*(*(*triple_ptr)[5])(int))[10];
} __attribute__((packed));

/* Additional complex combinations */

/* Typedef with all bracket types */
typedef struct {
    int (*(*member1))(int (*)[5]);
    union {
        void (*func)(int (*)(void));
        struct {
            unsigned int:4;
            unsigned int field:4;
        };
    } member2;
    char array[2][(sizeof(int) > 2) ? 3 : 1];
} ComplexTypedef;

/* Variable declaration with complex type */
ComplexTypedef __attribute__((aligned(32))) global_var;

/* Function prototype with complex parameters */
void process_complex(
    int (*(*param1))(int[2][3]),
    struct UltimateType (*param2)[10],
    void (*param3)(struct { int x; int y; })
) __attribute__((nonnull(1, 2)));

/* Nested typedef with function pointer chain */
typedef int (*(*(*ChainPtr)(int)))(char);
typedef ChainPtr (*(*ChainArray[5]))(void);

/* Union containing all bracket types */
union AllBracketsUnion {
    int (*func_ptr)(int (*)(char[5]));
    struct {
        int matrix[2][2];
        void (*actions[3])(void);
    } struct_part;
    long (*array_ptr)[4];
};

/* Final test: deeply nested parentheses */
int (*(*(*(*deeply_nested)(int (*(*))(char)))(float))[5])(double);

#endif /* TEST_GENGTYPE_COVERAGE_H */
