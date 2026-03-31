/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void);
    
    /* Pointer to function returning pointer to array */
    char (*(*complex_func)(int))[10];
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int flag3:4;
        } bits;
        unsigned char bytes[2];
    } nested_union;
    
    /* Multi-dimensional array */
    double matrix[4][4][4];
};

/* 2. Function Pointer Declarations with Varied Signatures */
typedef int (*FuncPtr1)(int, char);
typedef void (*FuncPtr2)(int (*)(char), double);
typedef char *(*FuncPtr3)(int (*[5])(void), float);

/* Pointer to function returning pointer to function */
int (*(*nested_func_ptr)(int (*(*)(double))(float)))(char);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4][5];
    long flexible_array[];
};

struct HasFlexible {
    int count;
    /* Flexible array member at end */
    struct ArrayContainer items[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousContainer {
    /* Anonymous struct */
    struct {
        int x;
        int y;
        unsigned int z:8;
        unsigned int w:24;
    };
    
    /* Anonymous union */
    union {
        long a;
        double b;
        struct {
            short s1;
            short s2:4;
            short s3:12;
        } c;
    };
    
    /* Another level of nesting */
    struct {
        union {
            int i;
            char c[4];
        } u;
        int (*func)(int (*)(int[5]), char);
    } deeper;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Using macros to generate complex types */
PTR_FUNC(int) simple_func_ptr;
ARRAY_PTR(char, 10) char_array_ptr;
NESTED_FUNC(double) complex_nested_func;

struct MacroStruct {
    PTR_FUNC(void) void_func;
    ARRAY_PTR(struct AnonymousContainer, 5) container_array;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated));
    double z __attribute__((vector_size(32)));
} __attribute__((packed));

/* Function with attributes */
int __attribute__((noinline, noclone)) 
attributed_function(int __attribute__((unused)) param1, 
                    char * __attribute__((nonnull)) param2)
    __attribute__((warn_unused_result));

/* Variable with attribute */
extern int global_var __attribute__((weak, visibility("hidden")));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*signal_handler(int sig, 
                         void (*callback)(int, void*)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*func_ptr_array[2])(int (*)(char[10]), 
                            struct OuterStruct*);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3;
            unsigned int d:26;
        } bits;
        unsigned int full;
    } flags;
    
    /* Multi-dimensional array with pointer elements */
    void *ptr_matrix[3][4][5];
    
    /* Pointer to array of pointers to functions */
    char (*(*(*ultimate_ptr)[5])(int (*)(void)))[10];
    
    /* Flexible array member of structs */
    struct {
        int id;
        char name[20];
        double values[];
    } items[];
} __attribute__((aligned(32)));

/* Additional complex typedefs */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*destroy)(void *);
    unsigned int size:16;
    unsigned int type:8;
} Interface;

/* Union with function pointers */
union FuncUnion {
    int (*int_func)(int);
    double (*double_func)(double);
    void (*void_func)(void);
    struct {
        int (*nested)(int (*)(int));
        char tag;
    } special;
};

/* Template-like macro for generic function pointers */
#define GENERIC_FUNC(RET, ARG) RET (*)(ARG)
#define GENERIC_FUNC2(RET, ARG1, ARG2) RET (*)(ARG1, ARG2)

/* Using the generic macros */
GENERIC_FUNC(int, char *) int_from_string;
GENERIC_FUNC2(void, int, double) complex_operation;

/* Final test: extremely nested declaration */
struct FinalChallenge {
    int (*(*(*(*insane_ptr)(int (*(*)(double[2]))(float)))(char (*(*)(void))[5]))(long))[3];
    
    struct {
        union {
            int x;
            struct {
                unsigned int a:1;
                unsigned int b:1;
                unsigned int c:30;
            } y;
        } u1;
        
        union {
            double d;
            int (*func)(int (*[3])(void));
        } u2;
    } nested_unions[2];
    
    /* Zero-length array (GCC extension) */
    int zero_array[0];
    
    /* Array of pointers to flexible array members */
    struct ArrayContainer *flex_ptrs[4];
} __attribute__((packed, aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
