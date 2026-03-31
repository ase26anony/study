/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*callback)(int, char));
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags:4;
        unsigned int status:2;
        struct {
            int x:8;
            int y:8;
        } nested_bits;
    } bitfield_struct;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][4][5];
    
    /* Union with anonymous struct */
    union {
        struct {
            int a;
            char b;
        } s;
        long long ll;
    } data_union;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int (*)(char[10])))(double);

/* Function taking function pointer that returns array pointer */
void (*signal_handler(int sig, void (*handler)(int)))(const char *);

/* Triple nested function pointer */
char *(*(**triple_ptr)(int (*)(float)))(void);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int multi_dim[2][3][4];
    char strings[5][100];
    
    /* Flexible array member */
    int flexible_array[];
};

struct NestedArrays {
    /* Array of pointers to arrays */
    int *(*array_of_ptrs[10])[5];
    
    /* 4D array */
    float hypercube[2][3][4][5];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousContainer {
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:3;
            unsigned int c:4;
        } bits;
        unsigned short all;
    };
    
    /* Anonymous struct */
    struct {
        int x;
        struct {
            char a;
            char b;
        } inner;
    };
    
    /* Bit-field with multiple members */
    struct {
        unsigned int flag1:1;
        unsigned int flag2:2;
        unsigned int :5;  /* Unnamed bit-field */
        unsigned int value:8;
    } flags;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Use macros to generate complex types */
PTR_FUNC(int) simple_func_ptr;
ARRAY_PTR(char, 10) string_array_ptr;
NESTED_FUNC(double) nested_func_ptr;

/* Macro generating struct with all brackets */
#define COMPLEX_TYPE(Name, T) \
    struct Name { \
        T (*func)(T (*)[5]); \
        union { \
            T array[10]; \
            struct { \
                T x; \
                T y; \
            } point; \
        } data; \
        T matrix[3][3]; \
    }

COMPLEX_TYPE(ComplexInt, int);
COMPLEX_TYPE(ComplexDouble, double);

/* 6. Attribute Syntax with Parentheses */
/* Struct with alignment attribute */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int a;
    char b;
    double c __attribute__((aligned(8)));
} __attribute__((deprecated));

/* Function with attributes */
void __attribute__((noreturn, format(printf, 1, 2))) 
attributed_func(const char *fmt, ...);

/* Variable with section attribute */
int __attribute__((section(".data"), used)) global_var;

/* Type attribute */
typedef int __attribute__((may_alias)) aliasing_int;

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateType {
    /* Complex function pointer declaration */
    void (*(*signal(int sig, void (*func)(int)))(int, 
          void (*callback)(char (*)[10])))(double);
    
    /* Array of function pointers with nested parameters */
    int (*(*pfa[2][3])(int (*)(char[10]), 
          struct { int x; int y; }))[5];
    
    /* Union with anonymous struct and bit-fields */
    union {
        struct {
            unsigned int a:4;
            unsigned int b:4;
            struct {
                int x:8;
                int y:8;
            } nested;
        } bits;
        long long full;
    } data;
    
    /* Multi-dimensional array with pointer elements */
    int *(*ptr_matrix[2][2])[3];
    
    /* Flexible array member of structs */
    struct {
        int id;
        char name[20];
    } items[];
} __attribute__((aligned(32)));

/* Additional complex combinations */

/* Typedef with function pointer returning array pointer */
typedef int (*(*CallbackType)(void))[10];

/* Struct containing all bracket types in members */
struct AllBrackets {
    int (*func_ptr)(int);                     /* () */
    int array[5][10];                         /* [] */
    struct { int a; char b; } nested;         /* {} */
    union { 
        int x; 
        struct { short s; char c; } y; 
    } u;                                      /* {} with nested {} */
    int (*(*complex)[5])(char (*)[10]);       /* [] and () combined */
};

/* Nested typedefs with attributes */
typedef struct __attribute__((packed)) {
    int a;
    struct {
        char b __attribute__((aligned(2)));
        short c;
    } inner __attribute__((packed));
} PackedWithAttributes;

/* Function with complex return type and attributes */
struct AllBrackets __attribute__((warn_unused_result))
(*get_complex_func(void))(
    int param1,
    void (*callback)(struct AllBrackets *)
);

/* Template-like macro for generating parameterized types */
#define PARAMETRIC_TYPE(T, N) \
    struct Parametric_##T##_##N { \
        T (*process)(T input[N]); \
        T data[N][N]; \
        union { \
            T array[N]; \
            struct { \
                T min; \
                T max; \
            } range; \
        } storage; \
    }

/* Instantiate parametric types */
PARAMETRIC_TYPE(int, 5);
PARAMETRIC_TYPE(double, 3);
PARAMETRIC_TYPE(char, 10);

/* Final mega-struct combining everything */
struct MegaStruct {
    /* From UltimateType */
    struct UltimateType ultimate;
    
    /* From AllBrackets */
    struct AllBrackets brackets;
    
    /* Parametric types */
    struct Parametric_int_5 int_param;
    struct Parametric_double_3 double_param;
    
    /* Function pointer with all brackets */
    int (*(*(*mega_func)(struct AllBrackets (*)[5]))(
        int (*)(char[10][20])
    ))[3];
    
    /* Anonymous union with bit-fields and arrays */
    union {
        struct {
            unsigned int flags[2];
            struct {
                int a:4;
                int b:4;
                int c:8;
            } bits;
        } s;
        unsigned char bytes[16];
    } __attribute__((packed)) data_pack;
    
    /* Flexible array of function pointers */
    void (*(*flex_funcs[]))(int, ...);
} __attribute__((aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
