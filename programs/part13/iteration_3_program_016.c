/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Nested parentheses: function pointers */
    int (*func_ptr)(int, char);
    
    /* Nested brackets: multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Nested braces: anonymous struct */
    struct {
        unsigned int flags:3;
        unsigned int mode:2;
    } status;
    
    /* Combination: array of function pointers */
    void (*handlers[5])(void);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_fp)(int (*)(char)))(double);

/* Function taking function pointer as parameter */
void register_callback(int (*cb)(int, void*), void* data);

/* Nested parentheses in function parameters */
char* (*string_processor)(char (*)(int), int (*)[10]);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    long* ptr_array[10];
    
    /* Flexible array member */
    int flexible[];
};

/* Array of pointers to arrays */
int* (*array_of_ptrs[5])[10];

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3;
        } bits;
        unsigned int full;
    } u1;
    
    struct {
        long x:8;
        long y:8;
        long z:16;
    } __attribute__((packed)) packed_bits;
    
    /* Anonymous union with bit-fields */
    union {
        int i;
        struct {
            unsigned char r:4;
            unsigned char g:4;
            unsigned char b:4;
            unsigned char a:4;
        } color;
    };
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define CALLBACK(T) void (*)(T)

/* Use macros to generate complex types */
PTR_FUNC(int)* int_func_ptr;
ARRAY_PTR(char, 10) char_array_ptr;
CALLBACK(struct OuterStruct) callback_to_struct;

/* Macro generating nested parentheses */
#define NESTED_FP(T) T (*(*)(T (*)(T)))(T)
NESTED_FP(double) extremely_nested_fp;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data[4];
    char padding;
} __attribute__((deprecated("Use NewStruct instead")));

/* Function with attributes */
int __attribute__((warn_unused_result, noinline)) 
calculate(int a, int b) __attribute__((const));

/* Variable with attribute containing parentheses */
extern int global_var __attribute__((visibility("hidden"), 
                                     section(".data")));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Function returning function pointer with array parameter */
    void (*(*signal_handler)(int sig, 
                            void (*callback)(int, void*)))(int);
    
    /* Array of function pointers taking function pointers */
    int (*(*func_array[3])(int (*)(char[10]), 
                          void*))[5];
    
    /* Nested union with bit-fields and anonymous struct */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:1;
            int data[2];
        } s;
        long long combined;
    } u __attribute__((aligned(8)));
    
    /* Multi-dimensional flexible array member */
    int flex_matrix[][3][2];
    
    /* Pointer to array of pointers to functions */
    char (*(*(*meta_ptr)[5])(int))[10];
} __attribute__((packed));

/* Additional complex combinations */

/* Typedef with all bracket types */
typedef struct {
    int (*compare)(const void*, const void*);
    void* data_array[10];
    struct {
        int count;
        int max;
    } limits;
} Comparator;

/* Union containing struct with all bracket types */
union UniversalContainer {
    struct {
        int (*init)(void);
        char buffer[100][50];
        struct {
            short x;
            short y;
        } point;
    } s;
    void* ptr;
    long long as_int;
};

/* Function prototype with deeply nested parameters */
void process_data(
    int (*(*get_processor)(void))(int),
    char (*(*data)[10])[20],
    struct {
        int (*filter)(int);
        int thresholds[5];
    } config
);

/* Enum with last element (triggers default case after braces) */
enum TestEnum {
    VALUE_A,
    VALUE_B,
    VALUE_C  /* No comma here - triggers default case after last element */
};

/* Final complex declaration combining everything */
static volatile struct UltimateType* const complex_global 
    __attribute__((used, section(".special"))) = 
    (struct UltimateType*)0x1000;

#endif /* TEST_GENGTYPE_COVERAGE_H */
