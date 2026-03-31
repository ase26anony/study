/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(void), 
                struct { 
                    union { 
                        char c; 
                        int i; 
                    } u; 
                });
};

/* Struct with deeply nested type combinations */
union U1 {
    char c;
    struct {
        int i;
        float (*compute)(double matrix[][(sizeof(int)*2)], 
                        struct { 
                            int x; 
                            struct { 
                                short s; 
                            } inner; 
                        });
    } s;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y;})/sizeof(int))];
int matrix[][3] = { {1,2,3}, {4,5,6} };

/* Array with nested type in dimension */
struct Outer {
    int data;
    struct Inner {
        char buf[10];
    } inner;
} outer_array[sizeof(struct Outer) + (2*3)];

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    char b;
} __attribute__((aligned(16), packed));

/* Struct with multiple attributes */
struct S3 {
    double d;
    int i;
} __attribute__((aligned(32))) __attribute__((packed));

/* Variable with attribute containing parentheses */
int global_var __attribute__((aligned((8*2))));

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    struct {
        int y;
        int z;
    } coord;
};

struct Point pts[] = { 
    [0] = { .x = (1), .coord = {2, {3}} },
    [1] = { .x = 4, .coord = {5, 6} }
};

/* Complex initializer with macro expansion */
int init1[] = NESTED_BRACES;
int init2[] = COMPLEX_MACRO;

/* Nested struct initialization */
struct Container {
    struct Item {
        int id;
        struct {
            float values[3];
        } data;
    } items[2];
};

struct Container cont = {
    .items = {
        [0] = { .id = 1, .data = { .values = {1.0, (2.0), 3.0} } },
        [1] = { .id = 2, .data = { .values = {4.0, 5.0, 6.0} } }
    }
};

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX
/* This section contains deeply nested type definitions */
struct ConditionalStruct {
    int (*callback)(void (*)(int), 
                   struct {
                       int depth;
                       union {
                           long l;
                           double d;
                       } value;
                   });
    int array[((sizeof(int) > 2) ? 10 : 20)][3];
};
#endif

#ifndef SKIP_NESTED
/* Another conditional type definition */
typedef union {
    char c;
    struct {
        int flags;
        struct Nested {
            void (*func)(int, char);
        } nested;
    } s;
} ConditionalUnion;
#endif

#if defined(USE_ATTRIBUTES) && (__GNUC__ > 4)
struct AttributedStruct {
    int field;
} __attribute__((aligned((16))));
#endif

/* Function pointer type with extreme nesting */
typedef int (*(*complex_fp_t)(int (*(*)(int[][(2+3)]))(void)))(char, short);

/* Usage of the complex type */
complex_fp_t get_complex_function(void);

/* Main function that references defined types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    union U1 u1 = {0};
    
    /* Reference variables to ensure they're used */
    (void)s1;
    (void)u1;
    (void)arr1;
    (void)ptr1;
    (void)matrix;
    (void)outer_array;
    (void)global_var;
    (void)x;
    (void)pts;
    (void)cont;
    (void)init1;
    (void)init2;
    
#ifdef TEST_COMPLEX
    struct ConditionalStruct cs = {0};
    (void)cs;
#endif
    
#ifndef SKIP_NESTED
    ConditionalUnion cu = {0};
    (void)cu;
#endif
    
    return 0;
}

/* Additional complex type outside main */
enum E {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = ((1 << 2) | (1 << 3))
};

/* Struct with bitfields and nested anonymous struct */
struct BitfieldStruct {
    unsigned int a : 3;
    unsigned int b : 5;
    struct {
        unsigned int c : 2;
        unsigned int d : 6;
    };
    int (*handler)(struct { int x; int y; });
};

/* Array of function pointers */
int (*func_array[])(int, char) = {
    NULL,
    NULL
};

/* Final complex type with all delimiters mixed */
struct UltimateTest {
    int a;
    int b[(2+(3*4))];
    struct {
        union {
            char *p;
            int (*arr[][(sizeof(int)+2)])(void);
        } u;
    } s;
    void (*final_callback)(
        struct {
            int x;
            int y[3][(1+2)];
        },
        union {
            long l;
            double d;
        }
    );
};
