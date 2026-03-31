/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Level1 {
    /* Function pointer with nested parentheses */
    void (*func_ptr1)(int (*)(char[10]));
    
    /* Multi-dimensional array */
    int matrix[5][10][2];
    
    /* Nested struct with bit-fields */
    struct {
        unsigned int flags:3;
        unsigned int status:2 __attribute__((packed));
    } nested;
    
    /* Union inside struct */
    union {
        long long big;
        struct {
            short a;
            short b;
        } parts;
    } data_union;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_fp1)(double))(char);

/* Function taking function pointer as parameter */
void register_callback(void (*cb)(int, void*), int priority);

/* Even more nested function pointers */
char *(*(**signal_handler)(int, void (*)(int)))(const char*);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int fixed[10];
    int multi_dim[3][4][5];
    int flexible[];
};

/* Array of function pointers */
int (*func_array[5])(int, char**);

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    /* Anonymous union */
    union {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            unsigned int c:8;
        } bits;
        unsigned int full;
    };
    
    /* Anonymous struct */
    struct {
        long x:16;
        long y:16;
    };
    
    /* Regular bit-field */
    unsigned int control:2;
    unsigned int mode:3 __attribute__((aligned(4)));
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))[]

/* Usage of macros to generate complex types */
PTR_FUNC(int)* macro_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(int, 5), 3) macro_array;
NESTED_PTR(char) complex_macro_type;

/* Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(void) callback;
    ARRAY_TYPE(struct { int x; y; }, 10) items;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    char buffer[64] __attribute__((aligned(32)));
} __attribute__((deprecated));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2)))
fatal_error(const char *fmt, ...);

/* Variable with attribute */
int global_counter __attribute__((used, section(".data")));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Combination 1: Function pointer with array and nested function */
    void (*(*signal_table[10])(int sig, void (*handler)(int)))(int);
    
    /* Combination 2: Array of pointers to functions returning pointers to arrays */
    int (*(*processor[2])(int code))[10];
    
    /* Combination 3: Nested anonymous struct/union with bit-fields and arrays */
    union {
        struct {
            int (*compare)(const char*, const char*);
            unsigned int settings:8;
        } ops;
        struct {
            void (*execute)(int (*)(void));
            int params[3];
        } actions;
    } controller __attribute__((aligned(32)));
    
    /* Combination 4: Flexible array member of function pointers */
    void (*(*dynamic_funcs[]))(int, ...);
    
    /* Combination 5: Multi-dimensional array with attribute */
    volatile double measurements[5][10] __attribute__((aligned(64)));
};

/* Additional complex type combining everything */
typedef union {
    struct {
        int (*(*get_func)(int idx))(void);
        char data[100];
    } module;
    struct {
        void (*init)(struct UltimateTest*);
        void (*cleanup)(struct UltimateTest*, int (*callback)(void));
    } operations;
} MasterUnion __attribute__((transparent_union));

/* Template-like macro for generating complex declarations */
#define CREATE_COMPLEX_TYPE(name, T) \
    struct name##_struct { \
        T (*(*operate)(T input))[sizeof(T)]; \
        union { \
            T (*direct)(T); \
            T (*indirect)(T (*)(T)); \
        } methods; \
        T buffer[10]; \
    }

/* Instantiate the template macro */
CREATE_COMPLEX_TYPE(IntProcessor, int);
CREATE_COMPLEX_TYPE(CharProcessor, char);

/* Even more nesting */
struct RecursiveNesting {
    struct {
        int (*level1)(int (*level2)(int (*level3)(int)));
    } depth;
    
    int (*(*(*ultimate)[5])(int (*)(char*)))(double);
};

/* Test case with all bracket types in parameter declaration */
void test_all_brackets(
    int matrix[][10],                          /* [] */
    void (*callback)(int, char**),             /* () */
    struct { int x; int y; } point,            /* {} */
    int (*(*func_array[]))(void)               /* All: ()[]() */
) __attribute__((warn_unused_result));

/* Final extreme case - everything combined */
struct __attribute__((packed)) FinalChallenge {
    union {
        struct {
            int (*(*(*func_table[2][3])(int))[5])(void);
            unsigned int config:16;
        };
        struct {
            void (*(*handlers[10]))(struct FinalChallenge*);
            char data[];
        };
    } __attribute__((aligned(128)));
    
    volatile int (*(*atomic_op)(volatile int*)) __attribute__((transaction_safe));
} __attribute__((may_alias));

#endif /* TEST_GENGTYPE_COVERAGE_H */
