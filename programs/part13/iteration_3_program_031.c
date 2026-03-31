/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

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
        unsigned int flags;
        char name[32];
    } inner;
    
    /* Combination: array of function pointers */
    void (*callbacks[10])(int, char);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func1)(int (*)(float)))(char);

/* Function taking array of function pointers */
void register_handlers(int (*handlers[])(int, void*), int count);

/* Pointer to function with nested parameter */
char* (*string_processor)(int (*comparator)(const char*, const char*));

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int fixed[5][10];
    double deep[2][3][4][5];
    int flexible[];
};

struct HasFlexArray {
    size_t count;
    /* Flexible array member at end */
    struct ArrayContainer items[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    unsigned int mode:3;
    unsigned int enabled:1;
    unsigned int:4;  /* Unnamed bit-field */
    
    union {
        struct {
            unsigned short x:8;
            unsigned short y:8;
        } coord;
        unsigned int value;
    } data;
    
    struct {
        long long timestamp:48;
        unsigned int type:4;
        unsigned int reserved:12;
    } __attribute__((packed)) metadata;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_OF(T, N) T [N]
#define CALLBACK(T) void (*)(T)

/* Using macros to generate complex types */
PTR_FUNC(int)* int_func_ptr;
ARRAY_OF(PTR_FUNC(char), 5) func_array;
CALLBACK(struct BitFieldStruct) complex_callback;

/* Macro that expands to nested brackets */
#define NESTED_PTR(T) T (*(*)(void))(void)
NESTED_PTR(double) nested_ptr_func;

/* 6. Attribute Syntax with Parentheses */
struct AlignedStruct {
    int data[4];
} __attribute__((aligned(16), packed));

__attribute__((always_inline)) 
static inline int attributed_func(int x) __attribute__((const));

volatile int global_var __attribute__((section(".data")));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Function returning pointer to function with array parameter */
    void (*(*signal_handler)(int sig, void (*callback)(int)))(int);
    
    /* Array of function pointers taking function pointer parameters */
    int (*processor_array[3])(int (*)(char[10]), void*);
    
    /* Union with anonymous struct containing bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:5;
        } bits;
        unsigned char bytes[2];
    } control;
    
    /* Multi-dimensional array */
    float transform[4][4];
    
    /* Nested struct with flexible array member */
    struct {
        int count;
        struct {
            int id;
            char name[];
        } items[];
    } container;
    
    /* Function pointer with complex return type */
    struct AlignedStruct (*(*factory)(int size))(void);
} __attribute__((aligned(32)));

/* Additional complex type combinations */
typedef union {
    struct {
        int (*compare)(const void*, const void*);
        void (*free)(void*);
    } ops;
    void* ptrs[2];
} GenericHandler;

/* Array of pointers to functions returning pointers to arrays */
int (*(*func_table[5])(int))[10];

/* Struct containing all bracket types in members */
struct AllBrackets {
    int (*func)(int[5]);          /* () and [] */
    struct {                     /* {} */
        int x;
        int y;
    } point;
    int matrix[2][2];            /* [][] */
    void (*actions[3])(void);    /* [] and () */
};

/* Test case for default character handling */
/* This declaration contains various characters that should hit the default case */
struct DefaultTest {
    int a;      /* semicolon */
    char *b;    /* asterisk and semicolon */
    long c;     /* identifier */
    unsigned d:1; /* colon */
};

/* Complex typedef with attributes */
typedef __attribute__((may_alias)) struct {
    int (*vtable[10])(void);
    union {
        float f;
        int i;
    } value;
} AliasedType;

/* Final test: deeply nested combination */
struct DeepNest {
    int (*(*(*level1)(int (*)(char)))(float))[5];
    struct {
        union {
            struct {
                unsigned int a:1;
                unsigned int b:1;
            } flags;
            int value;
        } u;
    } nested;
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
