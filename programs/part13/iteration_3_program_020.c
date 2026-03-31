/* test_gengtype_coverage.h - Complex type definitions to exercise gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags : 4;
        unsigned int status : 2;
    } bitfield_container;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][4][2];
    
    /* Union containing anonymous struct */
    union {
        struct {
            int x;
            int y;
        } point;
        long coordinates;
    } position;
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function returning pointer to function */
int (*(*complex_func_ptr)(int))(char);

/* Function pointer with array parameter */
void (*signal_handler)(int sig, const char *msg[10]);

/* Nested function pointers in parameters */
typedef void (*callback_type)(int (*)(char), double (*)[3]);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int md_array[2][3][4];
    char strings[5][256];
    
    /* Flexible array member */
    int flexible[];
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct AnonymousNested {
    /* Anonymous union */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 3;
            unsigned int c : 4;
        } bits;
        unsigned short packed;
    };
    
    /* Anonymous struct */
    struct {
        float x, y, z;
    };
    
    /* Another level of nesting */
    struct {
        union {
            int i;
            float f;
        } value;
        int tag : 2;
    } tagged_union;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))(void)

/* Using the macros to generate bracket sequences */
PTR_FUNC(int) *int_func_ptr;
ARRAY_TYPE(ARRAY_TYPE(char, 10), 5) char_array_2d;
NESTED_PTR(double) complex_double_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[4];
} __attribute__((deprecated));

int variable __attribute__((used, aligned(8)));

void __attribute__((noreturn)) 
__attribute__((format(printf, 1, 2)))
attributed_function(const char *fmt, ...);

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*(*signal(int sig, void (*handler)(int)))(int))(void);
    
    /* Array of function pointers with complex signatures */
    int (*(*func_ptr_array[3])(int (*)(char[10]), double))[2];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            unsigned int c : 3;
        } bits;
        unsigned char byte;
    } flags;
    
    /* Multi-dimensional array */
    long double tensor[2][3][4][1];
    
    /* Pointer to array */
    int (*ptr_to_array)[10];
    
    /* Flexible array member of structs */
    struct {
        int id;
        char name[20];
    } items[];
} __attribute__((packed));

/* Additional complex type combinations */
typedef union {
    struct {
        int (*(*nested_func)(int[5]))(void);
        char (*string_ptrs[10])(int, char (*)[3]);
    } s;
    void *generic_ptr;
} MegaUnion;

/* Function with complex return type */
struct OuterStruct (*(*get_factory(void))(int))[2];

/* Template-like macro with all brackets */
#define CREATE_COMPLEX_TYPE(N) \
    struct Complex##N { \
        int (*funcs[N])(int (*)(char), void *); \
        union { \
            int i; \
            float f; \
        } data[N]; \
    }

/* Instantiate the macro */
CREATE_COMPLEX_TYPE(5) complex5;
CREATE_COMPLEX_TYPE(10) complex10;

/* Pointer to const volatile qualified function pointer */
int (*(* const volatile cv_func_ptr)(void))(int);

/* Nested arrays of function pointers */
typedef int (*(*nested_fp_array[2][3])(float))[4];

/* Struct with all possible bracket combinations */
struct AllBrackets {
    int a;                          /* default case */
    int b[10];                      /* '[' case */
    int (*c)(int);                  /* '(' case */
    struct {                        /* '{' case */
        int x;
        int y;
    } point;
    int (*(*d)(int[5]))(void);      /* Mixed '(' and '[' */
    union {                         /* Another '{' */
        int i;
        float f;
        struct {                    /* Nested '{' */
            char c;
        } s;
    } u;
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
