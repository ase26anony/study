/* test_gengtype_coverage.h - Complex type definitions to exercise gengtype's consume_balanced parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct Level1 {
    /* Function pointer array with nested parameters */
    int (*callbacks[5])(int, char);
    
    /* Nested struct with union */
    struct {
        union {
            int ival;
            float fval;
        } data;
        unsigned int flags:4;
    } nested;
    
    /* Pointer to array */
    int (*matrix_ptr)[10];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int (*)(char)))(double);

/* Function taking function pointer parameter */
void register_callback(void (*cb)(int, int (*)(char)), int priority);

/* Nested function pointer in struct */
struct FuncContainer {
    char* (*name_generator)(int id, char (*transform)(char));
    int (*comparator)(const void*, const void*);
};

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct MultiDim {
    int matrix[3][4][5];          /* Triple dimension */
    char* strings[10][20];         /* 2D string array */
    
    /* Flexible array member at end */
    int flexible[];
};

struct WithFlexible {
    int count;
    /* Flexible array of structs */
    struct {
        int x;
        double y;
    } items[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousNested {
    /* Anonymous union */
    union {
        struct {
            unsigned int a:3;
            unsigned int b:5;
            unsigned int c:8;
        } bits;
        unsigned short all;
    } flags;
    
    /* Anonymous struct */
    struct {
        long x:16;
        long y:16;
        long z:16;
    } coordinates;
    
    /* Nested anonymous struct in union */
    union {
        struct {
            unsigned char r:2;
            unsigned char g:2;
            unsigned char b:2;
            unsigned char a:2;
        } color_bits;
        unsigned char color_byte;
    } pixel;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_OF(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))[]

/* Use macros to generate complex types */
PTR_FUNC(int)* global_func_ptr;
ARRAY_OF(PTR_FUNC(char), 5) func_array;
struct MacroStruct {
    NESTED_PTR(double) complex_ptr;
};

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    char* name __attribute__((nonnull(1, 2)));
} __attribute__((deprecated));

int __attribute__((format(printf, 1, 2))) 
    log_message(const char* fmt, ...);

/* Function with multiple attributes */
void __attribute__((noinline, hot, constructor(101)))
    initialization_hook(void);

/* 7. Include All Bracket Types in Single Declaration - The ultimate test */
struct UltimateTest {
    /* Complex function pointer with nested params */
    void (*(*signal_handler)(int sig, 
                             void (*(*get_handler)(int))(int)))
         (int);
    
    /* Array of function pointers taking function pointers */
    int (*(*func_array[3])(int (*)(char[10]), 
                          void*))[5];
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2;
            unsigned int c:3[2];  /* Bitfield array (GCC extension) */
        } bits;
        unsigned long value;
    } flags;
    
    /* Multi-dimensional array */
    double matrix[2][3][4];
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        char* (*get_name)(void);
    } items[];
} __attribute__((aligned(32)));

/* Additional complex nested type */
typedef union {
    struct {
        int (*(*level1)(int (*)(char)))[10];
        struct {
            void (*action)(int, int (*)(void));
            union {
                int x;
                long y[5];
            } data;
        } nested;
    } s;
    long long raw[8];
} SuperNestedType;

/* Even more nesting */
struct Outer {
    struct Middle {
        struct Inner {
            int (*calc)(struct Inner*, int (*(*)(void))[]);
            union {
                int a;
                struct {
                    short x:4;
                    short y:4;
                    short z:4[2];  /* Another bitfield array */
                } parts;
            } u;
        } *inner_ptr;
        
        /* 2D array of pointers to Inner */
        struct Inner* grid[5][5];
    } mid;
    
    /* Function returning pointer to array */
    int (*(*get_matrix)(int size))[10];
};

/* Test case with all bracket types in typedef */
typedef int (*(*(*UltimateFuncPtr)(int (*(*)(char[10]))(double)))
             [5])(void);

/* Final stress test: everything combined */
struct __attribute__((packed)) FinalStressTest {
    UltimateFuncPtr funcs[2];
    struct Outer outer;
    struct UltimateTest ultimate;
    SuperNestedType nested;
    int (*(*complex_array[2][3])(int (*)(char), void*))();
    union {
        struct {
            unsigned int:4;
            unsigned int field1:4;
            unsigned int field2:8[3];
        } bits;
        unsigned char bytes[16];
    } __attribute__((aligned(8))) data;
} __attribute__((aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
