/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((int (*[(x)])(void)){0})
#define BRACKET_MACRO [[maybe_unused]]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING 3
#else
#define EXTRA_NESTING 1
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses and parameter struct */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; char b; });
    
    /* Another with more nesting */
    void (*fp2)(int (*(*nested)[5])(char (*(*)[2])[3]));
};

/* Union with nested braces and parentheses */
union U1 {
    char c;
    struct {
        int i;
        struct {
            short s;
            int arr[(2+3)*4];  /* Requirement 4 */
        } inner;
    } s;
};

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned(16), packed)) AttrStruct {
    int x __attribute__((aligned((8))));
    char y;
} __attribute__((deprecated));

/* C++11 style attribute (valid in C23/C++11) */
#if defined(__cplusplus) || __STDC_VERSION__ >= 202311L
[[deprecated("Use NewStruct instead")]]
#endif
struct OldStruct {
    int value;
};

/* Requirement 1 & 4: Complex array declarations */
typedef int Matrix[(sizeof(struct {int x; double y;})/sizeof(int))][5];

/* Array with parenthesized size expression */
extern int *ptr_array[(2+3)*4][sizeof(struct S1)];

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = { {2}, {3} } },
    [1] = { .x = 4, .y = NESTED_BRACES }  /* Macro expansion */
};

/* More complex initializer */
int nested_init[2][2] = { { { (1) }, { 2 } }, { { 3 }, { 4 } } };

/* Requirement 1: Even more nesting */
typedef struct {
    enum { RED, GREEN, BLUE } color;
    union {
        struct {
            int (*compare)(const void *, const void *);
        } funcs;
        char data[16];
    } u;
} ComplexType;

/* Function pointer returning pointer to array */
int (*(*complex_func)(void))[10];

/* Nested struct with all delimiter types */
struct UltimateNest {
    struct {
        int (*fp)(int (*arr[][(2+3)])[5], 
                 struct { 
                     union { 
                         char c; 
                         int i; 
                     } u; 
                 } s);
    } level1;
    
    int arr[((2*3)+4)][5][2];
    
    struct {
        char *p;
    } level2 __attribute__((aligned((32))));
};

/* Requirement 6: Conditional blocks with balanced tokens */
#if EXTRA_NESTING > 1
struct ConditionalStruct {
    int (*method)(struct { int x; int y; } point,
                  int matrix[][(3+2)*2]);
};
#endif

#ifdef SPECIAL_CASE
union ConditionalUnion {
    char str[({ int x = 5; x + 1; })];  /* GCC statement expression */
    int num;
};
#endif

/* Using the macro with brackets */
#if defined(__cplusplus)
[[BRACKET_MACRO]]
#endif
int global_var = 42;

/* Main function that references types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    union U1 u1 = {0};
    Matrix m = {0};
    
    /* Reference variables */
    (void)s1;
    (void)u1;
    (void)m;
    (void)complex_func;
    (void)global_var;
    (void)pts;
    (void)nested_init;
    (void)x;
    
    return 0;
}

/* Final complex typedef with everything */
typedef int (*(*UltimateType)(struct {
    int a[({ int y = 2; y * 3; })];  /* GCC extension */
    struct __attribute__((packed)) {
        char c;
        int i __attribute__((aligned(8)));
    } s;
}))[10];
