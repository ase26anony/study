/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (3 + (4))))
#define ARRAY_MACRO [ (2 + (3)) ][ (4) ]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING (struct { int x; })
#else
#define EXTRA_NESTING (1)
#endif

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct S1 {
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    void (*fp2)(int (*(*nested)[5])(int, int), 
                union { 
                    char c; 
                    struct { 
                        int i; 
                        double d; 
                    } s; 
                } u);
};

/* Union with nested arrays and function pointers */
union U1 {
    char c;
    int (*arr_ptr[ (sizeof(int) + (2)) ])(int, 
                                          struct { 
                                              int x; 
                                              int y; 
                                          });
    struct {
        int (*(*complex)[ (2 * (3 + 1)) ])(int (*)(int[ (1+2) ][3]), 
                                           void*);
    } nested_struct;
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_arr[ (2 + 3) * 4 ][5];
int *ptr_array[ sizeof(struct { int x; double y; char z[ (2) ]; }) / 4 ];
int (*func_ptr_array[ ( (sizeof(int)) + (2) ) ])(int, int);

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned( (16) ), packed)) AttributedStruct {
    int data[ (4) ];
    char *ptr __attribute__((aligned( (sizeof(double)) )));
} __attribute__((packed));

/* Another struct with GCC attributes containing nested parentheses */
struct S2 {
    int x;
    int y;
} __attribute__((aligned( ( (16) + (8) ) ), 
                 packed, 
                 deprecated("Use struct S3 instead")));

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized_var = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[ (2) ][ (3) ];
};

struct Point points[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = { (2) }, 
        .z = { { (1), (2), ( (3) ) }, 
               { (4), (5), (6) } } 
    },
    [1] = { 
        .x = ( ( (10) ) ), 
        .y = { 20 }, 
        .z = NESTED_BRACES 
    }
};

/* Complex initializer with macro expansion */
int nested_init[] = NESTED_BRACES;

/* Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[ (2) ][ (3) ]), 
                              struct { 
                                  int a; 
                                  struct { 
                                      char c; 
                                  } inner; 
                              });

/* Enum with complex expressions in initializers */
enum E {
    VAL1 = ( (1) + (2) ),
    VAL2 = sizeof(struct { int x; int y[ (2) ]; }),
    VAL3 = ( (sizeof(int)) * ( (2) + (3) ) )
};

/* Requirement 1 & 4 combined: Function with complex return type */
struct S3* (*get_complex())[ (2 + (3)) ] {
    static struct S3* arr[ (2 + (3)) ][ (4) ];
    return arr;
}

/* Nested struct definition inside a function parameter */
void process_data(int (*processor)(struct { 
    int id; 
    union { 
        int i; 
        float f; 
        struct { 
            char c; 
            double d; 
        } s; 
    } data; 
}), int count[ ( (2) * (3) ) ][ (4) ]) {
    /* Function body */
}

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_FEATURE
struct ConditionalStruct {
    int (*cond_ptr)(int[ (1) ][ (2) ][ (3) ]);
    union {
        char c;
        struct {
            int i[ (2) ];
        } s;
    } u;
};
#endif

#if defined(ANOTHER_TEST) && ( (1) + (2) > (1) )
typedef union ConditionalUnion {
    int x;
    struct {
        int y[ ( (2) + (3) ) ];
    };
} ConditionalUnion;
#endif

/* Main function to avoid dead code elimination */
int main() {
    struct S1 s1 = {0};
    union U1 u1 = {0};
    struct AttributedStruct as = {0};
    
    /* Reference variables to prevent optimization */
    (void)s1.fp1;
    (void)u1.c;
    (void)as.data[0];
    (void)multi_dim_arr[0][0];
    (void)ptr_array[0];
    (void)initialized_var;
    (void)points[0].x;
    (void)nested_init[0];
    
    return 0;
}

/* Additional complex type at file scope */
struct Outer {
    struct {
        int (*inner[ (2) ])(int, 
                           struct { 
                               int a; 
                               int b[ (3) ][ (4) ]; 
                           });
    } nested;
    
    union {
        char c;
        int (*func_ptr)(int (*)(int[ (1) ][ (2) ]), 
                       void*);
    } u;
};

/* Array with nested designators */
int designator_array[ (3) ] = { 
    [0] = ( (1) + (2) ), 
    [ (1) ] = { ( (3) * (4) ) }, 
    [2] = sizeof(struct { int x[ (2) ]; }) 
};

/* Final typedef with everything nested */
typedef struct {
    int (*ultimate[ ( (2) + (sizeof(int)) ) ])(
        struct {
            int x;
            union {
                int i;
                struct {
                    char c[ ( (3) ) ];
                } s;
            } u;
        },
        int (*callback)(int[ (1) ][ (2) ][ (3) ])
    );
} UltimateType;
