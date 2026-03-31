/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (3 + (4))))
#define ARRAY_MACRO [sizeof(struct { int x; })]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct FunctionPointerStruct {
    /* Nested function pointer with complex parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; double b; });
    
    /* Even more nesting */
    void (*fp2)(char *(*arr[5])(int, struct { int x; }), 
                union { 
                    long l; 
                    struct { 
                        short s; 
                        int (*nested_fp)(int (*)(int[][4])); 
                    } inner; 
                });
};

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data __attribute__((aligned((8))));
    char buffer[32] __attribute__((aligned(32)));
};

/* Union with nested attributes */
union __attribute__((deprecated)) DeprecatedUnion {
    int i;
    struct {
        float f;
        char c;
    } s __attribute__((packed));
};
#endif /* TEST_COMPLEX */

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2 + 3) * 4][5];
int *pointer_array[sizeof(struct { int x; char y[10]; }) / 4];

/* Array with macro-expanded dimension */
int macro_array ARRAY_MACRO;

/* Requirement 1 (more examples): Complex typedefs */
typedef int (*ComplexFuncPtr)(int (*)(int[][3]), 
                              struct { 
                                  int a; 
                                  struct { 
                                      char c; 
                                      int i; 
                                  } nested; 
                              });

/* Enum with complex expressions */
enum {
    VALUE1 = (1 << (2 + 3)),
    VALUE2 = sizeof(int[(2 * (3 + 4))]),
    VALUE3 = (int){ (5 + (6 * 7)) }
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized_var = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

/* Complex initializer */
struct Point points[] = {
    [0] = { .x = (1), .y = {2}, .z = { {1}, {2}, {3} } },
    [1] = { .x = COMPLEX_PARENS(10), .y = {20}, .z = NESTED_BRACES }
};

/* Nested struct with initializer */
struct Outer {
    struct Inner {
        int a;
        struct Deeper {
            char c;
            int i;
        } d;
    } inner;
    int arr[2][(3 + 4)];
} outer_instance = {
    .inner = {
        .a = (5 + (6 * 7)),
        .d = {
            .c = 'x',
            .i = { (8 + 9) }
        }
    },
    .arr = { {1, 2, 3, 4, 5, 6, 7}, {8, 9, 10, 11, 12, 13, 14} }
};

/* Function with complex return type and parameters */
ComplexFuncPtr get_func_ptr(int param1[(2 + 3)], 
                           struct { 
                               int a; 
                               int b[(sizeof(int) + 2)]; 
                           } param2) {
    static int (*local_fp)(int, int) = NULL;
    return NULL;
}

/* Requirement 6: More conditional compilation */
#if defined(USE_CPP_ATTR) && __cplusplus
/* C++11 attributes with nested parentheses */
struct [[deprecated("Use NewStruct instead")]] OldStruct {
    int value [[maybe_unused]];
};
#elif defined(USE_GCC_ATTR)
/* GCC attributes with deeply nested parentheses */
int variable __attribute__((aligned((16)), 
                           section((".data" ".special"))));
#endif

/* Bitfield struct with complex expressions */
struct BitfieldStruct {
    unsigned int a : (1 + 2);
    unsigned int b : sizeof(char) * 8;
    unsigned int c : (int)(3.14 * 2);
};

/* Anonymous struct/union */
struct Container {
    union {
        struct {
            int x;
            int y;
        };
        struct {
            long data;
            char tag;
        };
    };
    int (*methods[3])(struct Container *, int);
};

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    /* Use the variables to prevent optimization */
    int sum = multi_dim_array[0][0] + pointer_array[0][0] + macro_array;
    sum += initialized_var + points[0].x + outer_instance.inner.a;
    
    /* Reference function pointer type */
    ComplexFuncPtr fp = get_func_ptr(NULL, (struct { int a; int b[6]; }){0});
    
    return sum > 0 ? 0 : 1;
}

/* Final complex type definition */
typedef struct UltimateType {
    int (*(*complex_array[2][3])(int (*)(int[][(2+3)])))(char *(*)(void));
    union {
        struct {
            int a : (2 * 3);
            int b : (4 + 5);
        };
        long long data;
    } u;
    int arr[sizeof(struct { char a; int b; double c; })];
} UltimateType_t;
