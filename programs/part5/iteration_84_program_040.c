/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (sizeof(struct { char c; }) * 2))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct Level1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*callback)(int[2][3]), 
               struct { 
                   int a; 
                   union { 
                       char c; 
                       double d; 
                   } u; 
               });
    
    /* Another function pointer with multiple nesting levels */
    void (*fp2)(int (*(*nested)[5])(int, 
                                    struct { 
                                        int x; 
                                        int y; 
                                    }));
};

/* Union with nested braces */
union DeepNest {
    struct {
        struct {
            struct {
                int deepest;
            } level3;
        } level2;
    } level1;
    long value;
};

#endif /* TEST_COMPLEX */

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), 
                      packed, 
                      deprecated("Use NewStruct instead"))) 
    AttributedStruct {
    int data __attribute__((aligned((8 * 2))));
    char buffer[64] __attribute__((aligned(32)));
};

/* C++11 style attribute (works in C23/C2x too) */
#if __STDC_VERSION__ >= 202311L || defined(__cplusplus)
[[deprecated("Old type"), 
  maybe_unused]] 
struct [[gnu::packed]] CppStyleStruct {
    int x [[gnu::aligned(16)]];
};
#endif

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim[(2 + 3) * 4][5];
int *pointer_array[(sizeof(struct { int x; double y; }) / sizeof(int))];

/* Array with nested type in dimension */
struct Container {
    int data[sizeof(union {
        long long ll;
        struct {
            int a, b;
        } pair;
    })];
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int complex_init = { { ( 1 + (2 * (3 + 4) ) ) } };

struct Point {
    int x;
    struct {
        int y;
        int z;
    } coord;
};

struct Point points[] = {
    [0] = { .x = (1 + 2), .coord = {2, {3}} },
    [1] = { .x = {4}, .coord = {{5}, 6} },
    [2] = { (7), {8, 9} }
};

/* Use the macro from Requirement 2 */
ARRAY_MACRO;

/* Requirement 1 (more examples): Additional nested type definitions */
typedef int (*ComplexFuncPtr)(int (*)(int[][3], 
                                      struct { 
                                          int tag; 
                                          union { 
                                              int i; 
                                              float f; 
                                          } data; 
                                      }), 
                              void *);

/* Enum with complex expression */
enum {
    VALUE = (sizeof(struct { 
                char a; 
                int b; 
                long c[(2 + 3)]; 
            }) + 8)
};

/* Requirement 6: More conditional compilation */
#if defined(USE_NESTED) && (__GNUC__ > 4)
struct ConditionalStruct {
    int field[({ 
        union { 
            int i; 
            char c[4]; 
        } u = { .i = 1 }; 
        u.i; 
    })];
};
#elif defined(ALTERNATIVE)
/* Alternative with different nesting */
union Alternative {
    struct {
        int (*func)(int, 
                    int (*)(struct { 
                        int x[2]; 
                    }));
    } s;
    long l;
};
#else
/* Default empty */
#endif

/* Function with complex parameter */
static void dummy_func(int (*param)(int, 
                                    int[][(2 * 3)], 
                                    struct { 
                                        int a; 
                                    })) 
{
    /* Local variable with nested initializer */
    int local = { { ( (1) ) } };
    
    /* Statement expression with braces (GCC extension) */
    int result = ({
        int temp = 5;
        temp * 2;
    });
}

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct AttributedStruct as = {0};
    points[0].x = complex_init;
    
    /* Use macro-expanded array */
    arr_macro[0] = 42;
    
    return 0;
}

/* Final complex type definition outside any conditional */
struct UltimateNest {
    /* Mix all delimiters */
    int (*fp)(int (*)(int[({ 
                struct { 
                    int x; 
                } s; 
                s.x = 1; 
                s.x; 
            })][3]), 
            union { 
                char c; 
                struct { 
                    int i; 
                } s; 
            }),
    struct {
        int array[(2 + (3 * 4))];
    });
};
