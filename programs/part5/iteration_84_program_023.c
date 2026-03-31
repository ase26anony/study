/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{(([]))}]])
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(char), 
                union { 
                    long l; 
                    struct { 
                        short s; 
                        int (*nested_fp)(int (*(*)[5])(void)); 
                    } inner; 
                });
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[sizeof(struct {int x; char y[(sizeof(double)*2)];})/4];

/* Nested in typedef */
typedef int matrix_t[( (sizeof(int) > 2) ? 10 : 20 )]
                    [ ( (1 << 2) + 3 ) ]
                    [ sizeof(struct { char c; int i; }) ];

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    char b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* GCC attributes with parentheses */
struct __attribute__((aligned((16)), 
                     packed, 
                     mode(__SI__))) S3 {
    int data[((2)*(3))];
};

/* C++11 style attributes (valid in C23/C2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Old structure")]]
[[nodiscard("Important return value")]]
#endif
struct S4 {
    int value;
};

/* Requirement 1 & 4 combined: Complex nested type */
union U1 {
    struct {
        int (*callbacks[((sizeof(void*)*2))])
                      (int, 
                       struct { 
                           int x; 
                           int y[3][((1+2)*3)]; 
                       });
    } handler;
    long data;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x1 = { { ( 1 + (2) ) } };
int x2 = NESTED_BRACES;

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } inner;
};

struct Point pts[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = {2, {3}}, 
        .inner = { .z = (((4))) } 
    },
    [1] = COMPLEX_MACRO  /* Using macro expansion */
};

/* More complex initialization */
int arr2[][2] = { 
    { (1), {2} }, 
    { {3}, (4) }, 
    NESTED_BRACES 
};

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_NESTING
/* This section only parsed if TEST_NESTING is defined */
struct ConditionalStruct {
    int (*func_ptr)(int [][(2+3)]);
    union {
        char c;
        struct { 
            int i; 
            int j[((sizeof(int)+3)/2)]; 
        } s;
    } u;
};
#endif

#if defined(USE_ATTRIBUTES) && (__GNUC__ > 4)
struct __attribute__((aligned(( (16) )))) AlignedStruct {
    int data[ ( (2) * (3) ) ][4];
};
#elif defined(USE_ANOTHER)
enum E { A = (1), B = (2), C = ((A) + (B)) };
#endif

/* Function with complex parameter */
void complex_func(int (*param)(int, 
                               int (*)(char, 
                                       struct { 
                                           int a; 
                                           int b[3]; 
                                       }), 
                               int[][(2*3)]),
                  int arr[sizeof(struct {int x;})][5]) {
    /* Function body */
}

/* Requirement 1: Typedef with extreme nesting */
typedef int (*(*complex_fp_t)(int (*(*)(int[][3]))(void), 
                              struct { 
                                  union { 
                                      int i; 
                                      long l; 
                                  } u; 
                                  int arr[((2)+(3))][4]; 
                              }))
            (char, 
             int (*)[5], 
             void (*)(struct { int x; }));

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1;
    struct S2 s2;
    struct S3 s3;
    struct S4 s4;
    union U1 u1;
    
    /* Reference variables to prevent optimization */
    (void)s1;
    (void)s2;
    (void)s3;
    (void)s4;
    (void)u1;
    (void)x1;
    (void)x2;
    (void)pts;
    (void)arr1;
    (void)ptr1;
    (void)arr2;
    
    complex_func(NULL, NULL);
    
    return 0;
}

/* Final complex type definition wrapping everything */
struct UltimateType {
    /* All types of nesting */
    int (*fp)(int, 
              int (*)(char, 
                      int[][(2*3)], 
                      struct { 
                          int a; 
                          union { 
                              char c; 
                              int i; 
                          } u; 
                      }),
              int (*(*)[5])(void));
    
    /* Array with computed size */
    char data[sizeof(struct {
        int field1;
        int field2[((sizeof(int)*2))];
        struct {
            short s;
            long l;
        } nested;
    })];
    
    /* Nested struct initialization */
    struct {
        int values[3];
    } inner = { .values = { (1), {2}, ((3)) } };
};
