/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(void), 
                union { 
                    char c; 
                    struct { 
                        long l; 
                    } s; 
                });
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[(1+2)*3];})/4)];

/* Nested struct with arrays */
struct Matrix {
    int data[(sizeof(int[(1+2)*(3+4)])/sizeof(int))];
    struct {
        double values[((1<<3) + 5)];
    } inner;
};

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    char b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use struct S3 instead")));

/* GCC attributes with parentheses */
struct S3 {
    int x __attribute__((aligned((8)*(2))));
    int y;
} __attribute__((packed));

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
struct Point {
    int x;
    int y;
} pts[] = { 
    [0] = { .x = (1), .y = {2} },
    [1] = { .x = {3}, .y = (4) },
    [2] = { .x = ( (5) ), .y = { {6} } }
};

/* Complex initializer using macros */
int nested_init[] = NESTED_BRACES;

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX
/* Struct with deeply nested members */
union U1 {
    char c;
    struct {
        int i;
        struct {
            long l;
            char str[({ int x = 5; x; })];
        } inner;
    } s;
};
#endif

#if defined(USE_ATTRIBUTES)
/* Attribute in conditional block */
struct S4 {
    int field;
} __attribute__((aligned(32), 
                 deprecated("Old version"))) var1;
#elif defined(USE_NESTED)
/* Alternative nested definition */
struct S5 {
    int (*callback)(void (*)(int[][(2+3)]), 
                    struct { 
                        union { 
                            int a; 
                            char b; 
                        } u; 
                    });
};
#else
/* Default case */
struct S6 {
    int simple;
};
#endif

/* More complex examples combining requirements */
typedef struct {
    /* Function pointer array with attributes */
    int (*(*funcs[((2)*(3))])(int, 
                              struct { 
                                  int x; 
                              })) 
        __attribute__((nonnull(1, 2)));
    
    /* Nested anonymous struct */
    struct {
        int matrix[2][(3+4)];
        struct {
            char *ptr;
        } inner;
    } data;
} ComplexType;

/* Initializer with all delimiter types */
ComplexType ct = {
    .funcs = { NULL, NULL },
    .data = {
        .matrix = { {1,2,3,4,5,6,7}, {8,9,10,11,12,13,14} },
        .inner = {
            .ptr = (char*)(&x)
        }
    }
};

/* Array with computed size containing nested type */
int computed_arr[sizeof(struct {
    int a[(1+2)*3];
    struct {
        char b[4];
    } s;
})];

/* Macro used in type definition */
#define DECLARE_FUNC_PTR(name) \
    int (*name)(int, struct { int x; })

DECLARE_FUNC_PTR(my_func_ptr);

/* Nested switch-like structure in comments (won't affect parsing) */
/*
 * This comment contains balanced delimiters: { ( [ ] ) }
 * But they're in comments, so they shouldn't trigger consume_balanced
 */

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference variables to prevent optimization */
    (void)arr1;
    (void)ptr1;
    (void)x;
    (void)pts;
    (void)nested_init;
    (void)ct;
    (void)computed_arr;
    
    return 0;
}

/* Final conditional block with complex content */
#ifdef EXTRA_TEST
/* Extreme nesting */
struct Ultimate {
    int (*(*(*ultimate_callback)[5])(int (*(*)(int[][{ int x = 2; x; }]))(void), 
                                     union { 
                                         struct { 
                                             int a; 
                                         } s; 
                                         char c; 
                                     }))(char *str[(sizeof(struct {int y;})/4)]);
};
#endif
