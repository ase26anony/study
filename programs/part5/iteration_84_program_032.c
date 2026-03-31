/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(char));
};

/* Struct with deeply nested combinations */
struct S2 {
    /* Array of function pointers */
    int (*(*arr[5])(int))(void);
    
    /* Nested anonymous struct with function pointer */
    struct {
        int (*callback)(struct { int x; int y; }*);
    } inner;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[sizeof(struct {int x; char y;})/4];
int arr2[1 + (2 * (3 + 4))][NESTED_PARENS ? 5 : 10];

/* Requirement 3: Attribute specifications */
struct S3 {
    int a;
    char b;
} __attribute__((aligned(16), packed));

/* GCC attributes with parentheses */
struct S4 {
    int data;
} __attribute__((aligned((sizeof(int)*4)))) __attribute__((packed));

/* C++11 style attributes (valid in C23/C2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Use S5 instead")]]
#endif
struct S4_alt {
    int value;
};

/* Union with nested attributes */
union U1 {
    int i;
    char c;
    struct {
        short s;
    } nested;
} __attribute__((transparent_union));

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = {2}, .z = { (3), {4} } },
    [1] = { .x = {5}, .y = (6), .z = COMPLEX_MACRO },
    [2] = NESTED_BRACES
};

/* More complex initializer */
int matrix[2][2] = { { {1}, { (2) } }, { { (3+4) }, NESTED_BRACES } };

/* Requirement 6: Conditional compilation blocks with balanced tokens */
#ifdef TEST
union U2 {
    char c;
    struct {
        int i;
        long l;
    } s;
    void (*func)(int, char);
};
#endif

#if defined(COMPLEX_TYPES)
struct ConditionalStruct {
    int (*method)(int arr[][(sizeof(int)*2)], 
                  struct { 
                      int a; 
                      struct { 
                          char b; 
                      } inner; 
                  });
};
#elif defined(OTHER_TYPES)
enum E {
    A = (1 << 0),
    B = (1 << 1),
    C = (1 << (2))
};
#endif

/* Nested conditional compilation */
#ifndef SKIP_TYPES
    #ifdef NESTED_TEST
    typedef int (*ComplexFuncPtr)(int (*)(int[][(2+3)*4]), 
                                  union { 
                                      int i; 
                                      float f; 
                                  });
    #endif
#endif

/* Function with complex parameter */
void process(struct S1 *s, 
             int (*callback)(int, 
                            struct { 
                                int a; 
                                int b[(2+3)]; 
                            })) {
    /* Reference parameters to avoid unused warnings */
    (void)s;
    (void)callback;
}

/* Typedef with nested delimiters */
typedef struct {
    int (*compare)(const void *, const void *);
    void (*free)(void *);
} Interface;

/* Array type with nested parentheses in size */
typedef int Matrix[(2 * (3 + 4))][(5)];

/* Main function that references our types */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    struct S4 s4 = {0};
    union U1 u1 = {0};
    
    /* Use variables to avoid dead code elimination */
    (void)s1;
    (void)s2;
    (void)s3;
    (void)s4;
    (void)u1;
    (void)arr1;
    (void)ptr1;
    (void)arr2;
    (void)x;
    (void)pts;
    (void)matrix;
    
    /* Conditional code */
#ifdef TEST
    union U2 u2 = {0};
    (void)u2;
#endif
    
    return 0;
}

/* Final complex type definition outside main */
struct UltimateType {
    /* Mix of all delimiter types */
    int (*funcs[3])(int, 
                   struct { 
                       int a[2][(3+4)]; 
                       union { 
                           char c; 
                           int i; 
                       } u; 
                   });
    
    /* Nested array with computed size */
    char buffer[sizeof(struct { 
                         int x; 
                         double y; 
                         char z[(2*(3+4))]; 
                     })];
} __attribute__((aligned(32)));
