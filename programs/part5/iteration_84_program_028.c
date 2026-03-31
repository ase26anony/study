/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct S1 {
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(void));
};

/* Union with nested anonymous struct containing array of function pointers */
union U1 {
    char c;
    struct {
        int i;
        double (*arr[((sizeof(int)+3)/2)])(int (*)(int[][5]), ...);
    } s;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[(1+2)*3];})/4)];
int arr2[sizeof(union { struct { int a; char b; } s; double d; })];

/* Requirement 3: Attribute specifications with multiple parentheses */
struct S2 {
    int x;
    char y;
} __attribute__((aligned(16), packed, 
                 deprecated("Use struct S3 instead")));

struct S3 {
    long a;
    short b;
} __attribute__((aligned((8)*(2)), 
                 packed, 
                 may_alias));

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x1 = { { ( 1 + (2) ) } };
int x2 = NESTED_BRACES;

struct Point {
    int x;
    int y[3];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2, (3), {4}} },
    [1] = { .x = {5}, .y = NESTED_BRACKETS }
};

int matrix[][3] = { 
    {1, 2, 3}, 
    {{4}, {5}, {6}}, 
    COMPLEX_MACRO 
};

/* Requirement 6: Conditional compilation blocks with balanced tokens */
#ifdef TEST_COMPLEX
/* Struct with function pointer returning pointer to array */
struct S4 {
    int (*(*func)(int, ...))[(sizeof(int[2][3]) + 7) & ~7];
    union {
        struct {
            int a;
        } nested;
        char buf[((16+15)/16)*16];
    } u;
};
#endif

#ifndef NO_NESTED
/* Enum with complex underlying type */
enum E1 : int {
    VAL1 = (1 << (sizeof(int)*8 - 1)),
    VAL2 = {2},
    VAL3 = [3]  /* This is invalid but tests bracket consumption */
};

/* Typedef with nested parentheses */
typedef int (*FuncPtr)(int (*callback)(int[][(2*3)], ...), ...);
#endif

/* More complex nested type definitions */
struct Outer {
    struct Inner1 {
        int a;
        struct Deeper {
            char c;
            int arr[((2*3)+4)][5];
        } d;
    } i1;
    
    union Inner2 {
        long l;
        struct {
            float f;
            double d;
        } s;
    } i2;
};

/* Function pointer with deeply nested parameter list */
void (*global_fp)(int, 
                  struct { 
                      int a; 
                      char b[({ int x = 5; x; })];  /* GCC statement expression */
                  }, 
                  ...) = NULL;

/* Requirement 1 (more examples): */
/* Array of pointers to functions returning pointers to arrays */
int (*(*arr_fp[3])(void))[5];

/* Struct containing bitfield with parenthesized size */
struct BitfieldStruct {
    unsigned int a : (sizeof(int)*8 - 1);
    unsigned int b : ((1+2)*3);
};

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct Point p = {.x = x1, .y = {0}};
    
#ifdef TEST_COMPLEX
    struct S4 s4 = {0};
    (void)s4;
#endif
    
    (void)s1;
    (void)p;
    (void)arr1;
    (void)ptr1;
    (void)arr2;
    (void)x2;
    (void)pts;
    (void)matrix;
    (void)global_fp;
    (void)arr_fp;
    
    return 0;
}

/* Final conditional block with complex types */
#if defined(EXTRA_TYPES) && (__STDC_VERSION__ >= 201112L)
/* _Generic type selection with nested parentheses */
#define TYPE_NAME(X) _Generic((X), \
    int: "int", \
    struct S1: "S1", \
    default: "unknown" \
)

/* Static assertions with parentheses */
_Static_assert(sizeof(struct S2) == (sizeof(int) + sizeof(char)), 
               "S2 should be packed");
#endif
