/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*)(int), char);
    
    /* Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int flag3:4;
        } bits;
        long long value;
    } data_union;
    
    /* Multi-dimensional array */
    double matrix[3][4][2];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[10];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function taking function pointer parameter */
typedef void (*SignalHandler)(int, void (*)(int));

/* Pointer to function returning pointer to function */
typedef int (*(*FactoryFunc)(char *))(double);

/* Nested function pointers in struct */
struct CallbackContainer {
    int (*comparator)(const void *, const void *);
    void (*cleanup)(void (*)(void *));
    char *(*formatter)(int (*)(int), ...);
};

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayStruct {
    int fixed[5][10][2];
    char *string_table[3][20];
    int flexible[];
};

struct NestedArrays {
    struct {
        float data[2][3];
    } inner[4];
    long (*ptr_matrix[2][2])(void);
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:8;
    } part1;
    
    union {
        struct {
            unsigned int x:1;
            unsigned int y:2;
            unsigned int z:5;
        } flags;
        unsigned char byte;
    } part2;
    
    struct {
        long :16;  /* Unnamed bit-field */
        long field:32;
        long :0;   /* Force alignment */
    } part3;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define CALLBACK(T) void (*)(T (*)(int), int)

/* Using macros to create complex types */
PTR_FUNC(int) *int_func_ptr;
ARRAY_TYPE(PTR_FUNC(char), 5) complex_array;
CALLBACK(double) callback_handler;

struct MacroStruct {
    PTR_FUNC(void) void_func;
    ARRAY_TYPE(struct BitFieldStruct, 3) bitfield_array;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
    char * __attribute__((aligned(8))) aligned_ptr;
} __attribute__((deprecated));

typedef int __attribute__((vector_size(16))) v4si;

int __attribute__((noinline, noclone)) 
attributed_function(int (*)(int) __attribute__((nonnull))) 
__attribute__((warn_unused_result));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Function pointer with complex signature */
    void (*signal(int sig, void (*handler)(int, void (*)(int))))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[2])(int (*)(char[10]), void *);
    
    /* Anonymous union with bit-fields */
    union {
        int x;
        struct {
            long y:8;
            long z:24;
        } bits;
    } u;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Nested struct with function pointer */
    struct {
        char *(*get_name)(void);
        int (*process)(int (*)(int[5]), double);
    } operations;
    
    /* Flexible array member at end */
    struct UltimateType *self_referential[];
};

/* Additional complex combinations */
typedef union {
    struct {
        int (*compare)(const void *, const void *);
        void (*free)(void *);
    } funcs;
    void *ptr;
} GenericCallback;

/* Pointer to array of pointers to functions */
int (*(*nested_array_func[3][2])(float))[5];

/* Function returning pointer to array of function pointers */
int (*(*(*meta_func)(void))[10])(int, int);

/* Struct with all bracket types in members */
struct AllBrackets {
    int a;                          /* default case */
    int b[5];                       /* '[' case */
    struct { int c; } d;            /* '{' case */
    int (*e)(int);                  /* '(' case */
    int (*f[2])(int (*)(int[3]));   /* all brackets */
};

/* Test case for default branch - characters between brackets */
/* This should trigger default case for: ; , * = identifiers */
struct DefaultCaseTest {
    int x, *y, **z;                 /* commas, asterisks */
    struct Inner {                  /* struct keyword, identifier */
        char a;                     /* identifier, semicolon */
    } inner;                        /* identifier */
    void (*func)(void);             /* void, parentheses, semicolon */
    int array[10];                  /* brackets, semicolon */
} instance = {                      /* equal sign, braces */
    0, NULL, NULL,                  /* numbers, identifiers */
    {'a'},                          /* braces, character literal */
    NULL,                           /* identifier */
    {0}                             /* braces, number */
};                                  /* semicolon */

/* Additional edge cases */
/* Empty brackets */
int empty_array[];
struct {} empty_struct;
void (*empty_func_ptr)(void);

/* Nested brackets to maximum depth */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int (*func)(int (*)(int (*)(int)));
        } l3;
    } l2;
} l1;

/* Function-like macro with brackets */
#define APPLY(func, arg) func(arg)
#define CREATE_PTR(T) T*

/* Using the macros */
CREATE_PTR(int) int_ptr;
CREATE_PTR(struct OuterStruct) struct_ptr;

#endif /* TEST_GENGTYPE_COVERAGE_H */
