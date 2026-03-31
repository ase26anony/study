/* gengtype_coverage_test.h
 * Complex type definitions to test gengtype's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

#ifndef GENGTYPE_COVERAGE_TEST_H
#define GENGTYPE_COVERAGE_TEST_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parameters */
    int (*callbacks[5])(void (*)(int), char);
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flags:3;
            unsigned int mode:2;
        } bits;
        long long value;
    } data;
    
    /* Multi-dimensional array */
    double matrix[3][4][2];
};

/* 2. Function pointers with varied signatures and nested parentheses */
typedef void (*(*complex_func_ptr)(int (*)(char[10])))(double);

/* Function returning pointer to function with array parameter */
int (*(*get_processor(void))(int))[5];

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayContainer {
    int fixed[3][2];
    int flexible[];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct BitFieldStruct {
    struct {
        unsigned char a:1;
        unsigned char b:2;
        unsigned char :5; /* Unnamed bit-field */
    };
    
    union {
        struct {
            unsigned int x:8;
            unsigned int y:8;
            unsigned int z:16;
        };
        unsigned int full;
    } parts;
    
    /* Array of function pointers */
    void (*actions[3])(int);
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(N) int (*)[N]
#define COMPLEX_PTR(T) T (*(*)(T (*)[5]))(T)

/* Using the macros */
PTR_FUNC(int) simple_func_ptr;
ARRAY_TYPE(10) array_ptr;
COMPLEX_PTR(double) very_complex_ptr;

/* Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(void) handler;
    ARRAY_TYPE(3) matrix_ptr;
};

/* 6. Attribute syntax with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
    char __attribute__((aligned(8))) aligned_char;
} __attribute__((deprecated));

/* Function with attributes */
int __attribute__((noinline, noclone)) 
attributed_function(int __attribute__((unused)) param) 
    __attribute__((warn_unused_result));

/* Variable with attribute */
extern int global_var __attribute__((visibility("hidden"), weak));

/* 7. All bracket types in single declaration (the ultimate test) */
struct UltimateTest {
    /* Function pointer with complex signature */
    void (*(*signal_handler)(int sig, 
                             void (*(*callback_registrar)(int (*)(char[10])))
                                 (double)))(int);
    
    /* Array of function pointers returning function pointers */
    int (*(*func_array[2][3])(float (*)[5]))(char);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:8;
            unsigned int d:16;
        };
        unsigned int full_word;
        struct {
            unsigned char bytes[4];
        };
    } flags;
    
    /* Multi-dimensional flexible array member */
    int dynamic_matrix[][3][2];
    
    /* Pointer to array of pointers to functions */
    void (*(*(*meta_func_ptr))(int))[5];
    
    /* Nested struct with all bracket types */
    struct {
        int (*nested_callback)(int (*)(int[2][2]));
        union {
            char str[10];
            int nums[3];
        } data;
        unsigned int bitfield:1;
    } inner;
} __attribute__((aligned(32)));

/* Additional complex typedefs */
typedef struct {
    int (*(*getter)(void))[5];
    void (*setter)(int (*values)[5]);
} Container;

/* Union with function pointer array */
union FuncUnion {
    int (*int_funcs[3])(int);
    void (*void_funcs[2])(void);
    char (*(*string_func)(void))[10];
};

/* Struct with nested attribute */
struct NestedAttribute {
    int value __attribute__((aligned(sizeof(long))));
    struct {
        char data[8];
    } __attribute__((packed)) packed_member;
};

/* Complex function declaration using all bracket types */
extern int (*(*register_callback(
    void (*(*factory)(int (*)(char[10])))(double),
    int priorities[3][2]
))[5])(void (*)(int));

/* Template for generating more complex types (via macro) */
#define CREATE_COMPLEX_TYPE(name, T) \
    struct name##_struct { \
        T (*(*processor)(T (*)[5]))(T); \
        union { \
            T array[10]; \
            struct { \
                T a:4; \
                T b:4; \
            } bits; \
        } data; \
        T (*methods[3])(T (*)(T)); \
    }

/* Instantiate the template */
CREATE_COMPLEX_TYPE(IntComplex, int);
CREATE_COMPLEX_TYPE(CharComplex, char);

/* Final test: declaration combining everything */
struct FinalCombination {
    /* From macro */
    IntComplex_struct macro_generated;
    
    /* Direct complex type */
    void (*(*direct_ptr)(int (*)(char[10][5])))(double[2][3]);
    
    /* Attributed nested struct */
    struct __attribute__((packed)) {
        unsigned char:2;
        unsigned char field1:3;
        unsigned char field2:3;
    } packed_bits;
    
    /* Array of unions with function pointers */
    union {
        int (*int_func)(int);
        void (*void_func)(void);
    } func_choices[4];
    
    /* Flexible array of pointers to arrays */
    int (*flex_array[])[3];
} __attribute__((aligned(64)));

#endif /* GENGTYPE_COVERAGE_TEST_H */
