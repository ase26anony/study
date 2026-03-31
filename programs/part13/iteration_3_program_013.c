/* test_gengtype_coverage.h - Complex type definitions for gengtype parser coverage */
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
        unsigned int flags:3;
        unsigned int mode:2;
    } __attribute__((packed));
    
    /* Combination: array of function pointers */
    void (*callbacks[10])(int, char);
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func1)(double))(char);

/* Function pointer with nested parameter containing array */
void (*signal_handler)(int sig, void (*handler)(int, char[256]));

/* Pointer to function taking function pointer as parameter */
char *(*string_processor)(int (*filter)(const char*), char **);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4][5];  /* Deep nesting of brackets */
    long *ptr_array[8][9];
    char flexible_array[];  /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
union ComplexUnion {
    struct {
        unsigned int a:1;
        unsigned int b:2;
        unsigned int c:3;
        unsigned int d:26;
    } bits;
    
    struct {
        float x;
        double y;
        struct {
            short s;
            char c;
        } nested;
    } values;
    
    void *(*func_array[5])(struct ArrayContainer*);
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Using macros to generate complex types */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(int, 5), 3) macro_array;
NESTED_PTR(char) nested_ptr_func;

/* Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(void) callback;
    ARRAY_TYPE(char, 100) buffer;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(32), packed)) AttributedStruct {
    int data __attribute__((aligned(16)));
    char *ptr __attribute__((nonnull(1, 2)));
} __attribute__((deprecated("Use NewStruct instead")));

/* Function with attributes */
int __attribute__((format(printf, 1, 2)))
    __attribute__((noinline))
    log_message(const char *fmt, ...);

/* Variable with attribute */
extern int global_var __attribute__((weak, visibility("hidden")));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Function returning function pointer with array parameter */
    void (*(*get_handler(int priority))(int))[10];
    
    /* Array of function pointers taking function pointers */
    int (*(*dispatcher[5])(int (*)(char), void*))(double);
    
    /* Nested union with bit-fields and array */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:2;
            unsigned int:0;  /* Force alignment */
            unsigned int flags[4];
        } bits;
        
        struct {
            float (*transform[3][3])(float, float);
            void (*cleanup)(void);
        } ops;
    } u __attribute__((aligned(64)));
    
    /* Multi-dimensional flexible array-like structure */
    struct {
        int count;
        int data[];  /* Flexible array member */
    } flex;
    
    /* Pointer to array of pointers to functions */
    char (*(*(*lang_parser)[26])(int (*)(char*)))[100];
} __attribute__((packed));

/* Additional complex combinations */
typedef union {
    /* Anonymous struct with bit-fields */
    struct {
        unsigned int a:4;
        unsigned int b:4;
        unsigned int c:8;
        unsigned int d:16;
    };
    
    /* Function pointer array */
    void (*handlers[8])(union UltimateType*, int);
    
    /* Nested array with pointers */
    struct UltimateType *(*factory[2])(int, ...);
} ComplexUnion2 __attribute__((transparent_union));

/* Even more nesting */
struct RecursiveContainer {
    struct RecursiveContainer *next;
    struct RecursiveContainer *prev;
    
    /* Mutual recursion with another type */
    union {
        struct RecursiveContainer *container;
        struct UltimateType *ultimate;
        void *generic;
    } link;
    
    /* Array of function pointers with complex signature */
    int (*(*processors[10])(struct RecursiveContainer*, 
                           int (*(*)(int))(char),
                           char data[][256]))[20];
};

/* Template-like macro for generating complex types */
#define GENERATE_COMPLEX_TYPE(name, T) \
    struct name##_struct { \
        T (*processor)(T (*)(T[10]), T**); \
        T data[5][5]; \
        union { \
            T single; \
            T array[10]; \
        } variant; \
    }

/* Instantiate the macro-generated types */
GENERATE_COMPLEX_TYPE(IntComplex, int);
GENERATE_COMPLEX_TYPE(PtrComplex, void*);

/* Final test: The most complex single declaration possible */
struct __attribute__((aligned(128))) FinalTest {
    /* Everything combined */
    int (*(*(*ultimate_array[2][3])(int (*(*)(char[10]))(double), 
                                   struct UltimateType*))
         [5][6])(void (*(*)(int))(char), 
                 union ComplexUnion2 (*)(struct RecursiveContainer***));
    
    /* Anonymous struct with everything */
    struct {
        unsigned int:16;
        unsigned int field1:4 __attribute__((packed));
        unsigned int field2:4;
        unsigned int:8;
        
        /* Nested array of function pointers */
        void (*(*nested_funcs[3])[5])(int, ...);
        
        /* Flexible array of pointers to arrays */
        int (*flex_ptrs[])[10];
    };
    
    /* Zero-length array at end */
    char terminator[0];
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
