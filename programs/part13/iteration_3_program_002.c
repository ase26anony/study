/* gengtype_coverage_test.h
 * Complex type definitions to test consume_balanced() parser
 */

#ifndef GENGTYPE_COVERAGE_TEST_H
#define GENGTYPE_COVERAGE_TEST_H

/* 1. Complex Nested Type Definitions */
struct Level1 {
    /* Function pointer array with nested parameters */
    int (*callbacks[5])(void);
    
    struct Level2 {
        /* Pointer to function returning pointer to array */
        char (*(*func_ptr)(int))[10];
        
        union Level3 {
            /* Nested anonymous struct with bit-fields */
            struct {
                unsigned int flags : 4;
                unsigned int mode : 2;
                unsigned int : 26;  /* Padding */
            } bits;
            
            /* Array of function pointers with complex signatures */
            void (*(*signal_handlers[3])(int, void (*)(int)))(int);
        } u;
    } inner;
    
    /* Multi-dimensional array */
    double matrix[3][4][2];
};

/* 2. Function Pointer Declarations with Varied Signatures */
typedef int (*Comparator)(const void *, const void *);

/* Pointer to function returning pointer to function */
typedef void (*(*FactoryFunc)(int))(void);

/* Extremely nested function pointer */
typedef int (*(*(*ComplexFuncPtr)(char (*)[10]))(double))(float);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct Container {
    int fixed[5][10];
    char *strings[];
};

struct NestedArrays {
    int a[2];
    int b[2][3];
    int c[2][3][4];
    struct Container *containers[5];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    /* Anonymous union */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c : 4;
        } s1;
        
        struct {
            unsigned int x : 8;
            unsigned int y : 8;
        } s2;
    };
    
    /* Another anonymous struct */
    struct {
        long field1 : 16;
        long field2 : 16;
        long field3 : 16;
        long field4 : 16;
    };
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(N) (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Using macros to generate complex types */
typedef PTR_FUNC(int) int_func_ptr_t;
typedef int_func_ptr_t *array_of_func_ptrs_t[5];

struct MacroStruct {
    NESTED_FUNC(void) complex_member;
    int (ARRAY_PTR(10)) array_ptr_member;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
    char * __attribute__((aligned(8))) aligned_ptr;
} __attribute__((deprecated));

typedef int __attribute__((const)) pure_func_t(int, int);

/* Function with attributes */
void __attribute__((noreturn)) 
__attribute__((format(printf, 1, 2))) 
log_error(const char *fmt, ...);

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Combination 1: Function pointer with array and nested function param */
    void (*(*handler_table[5])(int sig, void (*callback)(int)))(int);
    
    /* Combination 2: Array of pointers to functions returning pointers to arrays */
    char (*(*string_processors[3])(int len))[];
    
    /* Combination 3: Nested anonymous union with bit-fields and function pointer */
    union {
        struct {
            unsigned int config : 8;
            unsigned int state : 8;
            int (*get_value)(int (*)(char[10]));
        } config_state;
        
        struct {
            long long data;
            void (*processor)(int matrix[3][4]);
        } data_processor;
    } control_block;
    
    /* Multi-dimensional flexible array member */
    int dynamic_matrix[][2][3];
    
    /* Attributed nested struct */
    struct __attribute__((packed)) {
        char id[16];
        int (*validator)(const char (*)[10]);
    } metadata;
};

/* Additional complex typedefs */
typedef struct {
    int (*(*get_func)(int id))(void);
    union {
        int (*array_funcs[5])(int);
        struct {
            void (*void_func)(void);
            char *(*str_func)(char (*)[20]);
        } func_pair;
    } func_union;
} FuncContainer;

/* Test case with all bracket types in typedef */
typedef int (*(*(*AllBracketsType)(int (*(*)[5])(char)))[10])(double);

/* Macro that expands to include all bracket types */
#define ULTIMATE_MACRO(T) T (*(*func_array[2])(T (*(*)(T[T]))(T)))(T)

/* Use the macro */
typedef ULTIMATE_MACRO(int) ultimate_int_func_t;

/* Final test: struct with everything */
struct ComprehensiveTest {
    /* From macro */
    ultimate_int_func_t macro_result;
    
    /* Direct complex declaration */
    struct {
        int (*vtable[10])(void);
        union {
            struct {
                unsigned int : 16;
                unsigned int field : 8;
            };
            long long raw;
        } data;
    } object;
    
    /* Attributed array of function pointers */
    int (__attribute__((const)) *pure_funcs[5])(int, int);
    
    /* Nested anonymous struct with flexible array */
    struct {
        int count;
        struct UltimateTest *tests[];
    } test_suite;
};

#endif /* GENGTYPE_COVERAGE_TEST_H */
