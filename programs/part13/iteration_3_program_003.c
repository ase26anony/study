/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*)(int), char);
    
    /* Nested union with bit-fields (braces) */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int:4;  /* Unnamed bit-field */
            unsigned int value:24;
        } bits;
        unsigned int raw;
    } bit_union;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][4][2];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[10];
};

/* 2. Function Pointer Declarations with Varied Signatures */
typedef void (*SimpleFuncPtr)(int);
typedef int (*(*NestedFuncPtr)(char (*)[10]))(double);
typedef void (*(*(*DeeplyNestedFuncPtr)(int (*)(char)))(float))[5];

/* Function pointer with parameters containing nested parentheses */
void (*signal_handler(int sig, void (*handler)(int)))(int);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    char* string_array[10];
    
    /* Flexible array member */
    int flexible_array[];
};

/* Array of function pointers */
int (*func_ptr_array[3])(int, char);

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct ComplexBitfieldStruct {
    /* Anonymous union */
    union {
        struct {
            unsigned char a:2;
            unsigned char b:3;
            unsigned char c:3;
        };
        unsigned char byte;
    };
    
    /* Anonymous struct */
    struct {
        long x:16;
        long y:16;
        long z:16;
        long w:16;
    };
    
    /* Nested anonymous struct in union */
    union {
        struct {
            short s1:4;
            short s2:4;
            short s3:4;
            short s4:4;
        };
        unsigned short word;
    } nested_anon;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define NESTED_FUNC_PTR(R, A) R (*)(A (*)(R))

/* Using macros to generate complex types */
PTR_FUNC(int)* int_func_ptr;
ARRAY_PTR(char, 10) char_array_ptr;
NESTED_FUNC_PTR(double, int) complex_nested_ptr;

/* Macro that expands to include all bracket types */
#define ULTRA_COMPLEX(T) \
    struct { \
        T (*(*func)(T (*)[5]))(T); \
        union { \
            T array[10]; \
            struct { T a:8; T b:8; }; \
        } u; \
    }

ULTRA_COMPLEX(int) ultra_var;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated));
} __attribute__((visibility("default")));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* Variable with attribute containing parentheses */
int global_var __attribute__((section(".data"), aligned(32)));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*(*signal(int sig, void (*func)(int)))(int))(int);
    
    /* Array of function pointers with nested parameters */
    int (*(*pfa[2])(int (*)(char[10])))(double);
    
    /* Anonymous union with bit-fields */
    union {
        int x;
        long y:8;
        struct {
            short a:4;
            short b:4;
            short c:8;
        };
    } u;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Pointer to array of function pointers */
    void (*(*func_ptr_array_ptr)[5])(int, ...);
    
    /* Nested struct with flexible array member */
    struct {
        int count;
        char data[];
    } flexible;
} __attribute__((packed));

/* Additional edge cases */

/* Empty struct/union */
struct EmptyStruct {};

/* Single element arrays */
struct SingleArray {
    int single[1];
};

/* Zero-length array (GCC extension) */
struct ZeroLength {
    int len;
    char data[0];
};

/* Nested type definitions with typedef */
typedef struct Node {
    struct Node* next;
    struct Node* prev;
    void* data;
} Node;

/* Self-referential structure */
typedef struct TreeNode {
    int value;
    struct TreeNode* left;
    struct TreeNode* right;
    struct TreeNode* parent;
} TreeNode;

/* Function returning pointer to function */
int (*(*make_callback(int type))(void))(int);

/* Array of pointers to functions returning pointers to arrays */
int (*(*callback_table[10])(void))[5];

/* Mix of all bracket types in typedef */
typedef int (*(*(*ExtremeType)(int (*)(char[10])))[5])(void);

/* Final test: declaration using all features */
ExtremeType __attribute__((used)) test_var;

#endif /* TEST_GENGTYPE_COVERAGE_H */
