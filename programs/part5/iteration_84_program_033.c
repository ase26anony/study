/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING [[[[ ]]]]
#else
#define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct S1 {
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; double b; });
    void (*fp2)(char *(*arr[])[(2+3)*4], int);
};

/* Union with nested anonymous struct */
union U1 {
    long l;
    struct {
        int (*fp)(struct { int x; int y; } point);
        char c;
    } s;
};

/* Enum with complex array in declaration */
enum E1 {
    VAL1 = sizeof(int[(2+3)*4]),
    VAL2 = sizeof(struct { int x; char y; })
};

/* Typedef with multiple levels of nesting */
typedef int (*complex_fp_t)(int (*)(int[][(sizeof(int)+3)], 
                                   union { 
                                       int i; 
                                       struct { char c; } s; 
                                   } u), 
                           struct { 
                               int a; 
                               int b[2][(1+2)*3]; 
                           });

/* Requirement 3: Attribute specifications with multiple parentheses */

/* GCC attributes with nested parentheses */
struct S2 {
    int a;
    char b;
} __attribute__((aligned(16), 
                 packed, 
                 deprecated("Use struct S3 instead")));

/* Struct with multiple attributes */
struct S3 {
    int x __attribute__((aligned((2+2)*4)));
    double y;
} __attribute__((packed));

/* C++11 style attributes (valid in C23/C2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Old struct")]] 
[[maybe_unused]]
#endif
struct S4 {
    int data[10];
};

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with parenthesized size expressions */
int arr1[(2+3)*4][5];
int arr2[sizeof(struct { int x; char y; }) / sizeof(int)];

/* Array of pointers with function pointer elements */
int (*arr3[(sizeof(int*) + 3)/2])(int, char);

/* Complex array dimension using nested type */
int arr4[sizeof(union {
    struct { int a; int b; } s;
    long l;
})];

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Variable with nested initializer */
int x = { { ( 1 + (2) ) } };

/* Struct initializer with designators */
struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = {3, 4} },
    [1] = { .x = 5, .y = 6, .z = {7, (8)} }
};

/* Complex nested initializer using macros */
int init1[] = NESTED_BRACES;
int init2[] = COMPLEX_MACRO;

/* Requirement 6: Conditional compilation blocks */

#ifdef TEST_NESTED
/* Type definition inside conditional block */
struct ConditionalStruct {
    int (*fp)(int (*)(int[][(2+3)], struct { int a; }), 
              union { char c; int i; });
    char data[NESTED_BRACKETS];
};
#endif

#if defined(USE_ATTRIBUTES) && (__GNUC__ > 4)
struct AttributedStruct {
    int field __attribute__((aligned((16+0)*1)));
} __attribute__((packed));
#endif

/* Most complex example combining multiple requirements */
struct UltimateType {
    /* Nested function pointer with attributes */
    int (__attribute__((const)) *ultimate_fp)(
        int (*)(int[2][(3+4)], 
                struct { 
                    int a; 
                    int b[sizeof(struct { int x; })]; 
                }),
        union {
            long l;
            struct { char c; } s;
        }
    );
    
    /* Array with complex dimension */
    char buffer[sizeof(int[(1+2)*(3+4)])];
    
    /* Nested anonymous struct */
    struct {
        int flags;
        int (*methods[2])(void);
    } inner;
} __attribute__((aligned(64)));

/* Requirement 5 continued: More complex initializers */
struct NestedInit {
    struct {
        int a;
        int b;
    } inner;
    int arr[2][2];
};

struct NestedInit ni = {
    .inner = { .a = (1+2), .b = {3} },
    .arr = { {1, 2}, {3, 4} }
};

/* Array with nested initializer containing parentheses */
int complex_init[][2] = {
    { (1), {2} },
    { 3, 4 },
    { 5, (6+7) }
};

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some types to prevent optimization */
    struct S1 s1 = {0};
    struct UltimateType ut = {0};
    
    (void)s1;
    (void)ut;
    (void)x;
    (void)arr1;
    (void)pts;
    (void)ni;
    (void)complex_init;
    
    return 0;
}

/* Final conditional block with balanced tokens */
#if 0
/* This code is never compiled but parsed */
struct NeverCompiled {
    int (*fp)(int (*)(int[][({ int x = 1; x; })], 
                      struct { 
                          int a[({ 2; })]; 
                      }));
};
#endif
