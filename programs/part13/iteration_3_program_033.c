/* gengtype_coverage_test.h
 * Designed to exercise consume_balanced() in gengtype-parse.cc
 * Targeting lines 341-352: default case and balanced bracket handling
 */

#ifndef GENGTYPE_COVERAGE_TEST_H
#define GENGTYPE_COVERAGE_TEST_H

/* 1. Complex Nested Type Definitions with All Bracket Types */
struct Level1 {
    /* Nested parentheses: function pointer returning function pointer */
    void (*(*complex_func)(int (*(*)(char[10]))(double)))(float);
    
    /* Nested brackets: multi-dimensional arrays */
    int multi_dim[3][4][5];
    
    /* Nested braces: anonymous struct inside union */
    union {
        struct {
            unsigned int flags:3;
            unsigned int mode:2;
        } bits;
        long long raw;
    } nested_union;
};

/* 2. Function Pointer Declarations with Varied Signatures */
typedef int (*FuncPtr1)(char);
typedef void (*FuncPtr2)(int (*)(char), double);
typedef char (*(*FuncPtr3)(int (*)[5]))(void);

/* Complex function pointer with deeply nested parentheses */
void (*(*signal_handler)(int sig, 
                         void (*(*get_handler)(void))(int))
     )(int) __attribute__((deprecated));

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayStruct {
    int fixed[10];
    int multi[2][3][4];
    int flexible[];  /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldContainer {
    /* Outer anonymous union */
    union {
        /* Inner anonymous struct with bit-fields */
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3;
        };
        /* Another inner anonymous union */
        union {
            unsigned int x:4;
            unsigned int y:4;
        };
        unsigned char bytes[2];
    } data;
    
    /* Direct bit-field in struct */
    unsigned int direct_bit:5;
    
    /* Nested struct with bit-fields */
    struct {
        long field1:8;
        long field2:8;
        long field3:16;
    } nested_bits;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*(*)(T))(T)
#define ARRAY_TYPE(T, N) T (*)[N]
#define NESTED_STRUCT(T) struct { T data; union { T alt; }; }

/* Using macros to generate complex bracket patterns */
PTR_FUNC(int) macro_func_ptr;
ARRAY_TYPE(double, 5) array_ptr;
NESTED_STRUCT(long) macro_struct;

/* More complex macro usage */
#define COMPLEX_MACRO(T) \
    union { \
        T (*func_ptr)(T (*)(T[10])); \
        struct { T arr[2][3]; }; \
    }

COMPLEX_MACRO(float) complex_instance;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated("use z instead")));
    double z;
} __attribute__((visibility("default")));

/* Function with multiple attributes */
int __attribute__((noinline, noclone, hot)) 
attributed_function(int __attribute__((unused)) param1,
                    char * __attribute__((nonnull)) param2)
    __attribute__((warn_unused_result));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Combination 1: Function pointer with array and nested function params */
    void (*(*combo1[2])(int (*(*)(char[10]))(double)))(float);
    
    /* Combination 2: Array of function pointers returning struct pointers */
    struct BitFieldContainer (*(*combo2[3])(void))[2];
    
    /* Combination 3: Nested anonymous types with all brackets */
    union {
        struct {
            int (*(*func_arr[2])(int))[3];
            union {
                char (*str_ptr)[10];
                void (*void_ptr)(void);
            } u;
        } s;
        long long raw[4];
    } mega_nested;
    
    /* Combination 4: Flexible array of function pointers */
    int (*(*flex_funcs[])(int, char))(void);
    
    /* Combination 5: Bit-fields with array and function pointer */
    struct {
        unsigned int flags:4;
        int (*action)(int (*callback)(void));
        short matrix[2][2];
    } final_combo;
} __attribute__((packed));

/* Additional edge cases */

/* Empty balanced constructs */
struct EmptyConstructs {
    int (*empty_func_ptr)(void);  /* Empty param list */
    int empty_array[0];           /* Zero-length array */
    struct {} empty_struct;       /* Empty struct */
    union {} empty_union;         /* Empty union */
};

/* Nested attributes */
typedef int __attribute__((__vector_size__(16))) vector_t 
    __attribute__((aligned(16)));

/* Function with complex return type */
struct ReturnStruct {
    int a;
    double b;
};

struct ReturnStruct (*(*complex_return)(int (*)(char)))(float) 
    __attribute__((const));

/* Typedef with all bracket types */
typedef union {
    int (*(*typedef_func)(int[5]))(void);
    struct {
        char (*typedef_arr[3])[10];
        void (*typedef_ptr)(int (*)(void));
    } nested;
} ComplexTypedef;

/* Global variables using complex types */
extern struct UltimateTest global_instance;
extern ComplexTypedef *global_typedef_ptr;
extern int (*(*global_func_array[5])(int))(char);

/* Inline function to generate more parentheses */
static inline int __attribute__((always_inline)) 
test_inline(int (*(*param)(void))(int)) {
    return (param != 0) ? 1 : 0;
}

/* Multiple levels of indirection */
char ***********************extreme_indirection;

/* Array of pointers to arrays */
int (*(*array_of_ptrs_to_arrays[3])[5])[10];

#endif /* GENGTYPE_COVERAGE_TEST_H */
