/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with deeply nested parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*fp2)(int (*(*nested)(int (*)(int[][5])))[10],
                union { char c; int i; } u);
};

/* Struct with nested structs/unions */
struct Outer {
    struct Inner1 {
        int x;
        struct Deeper {
            char c;
        } d;
    } i1;
    
    union Inner2 {
        int a;
        struct {
            float f;
            double d;
        } s;
    } i2;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[3];})/4)];
int arr2[sizeof(struct { int a; double b; })][10];

/* Using macros in array dimensions */
int arr3[] = NESTED_BRACES;
char arr4[] = NESTED_BRACKETS;

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned(16), packed)) AttrStruct {
    int data;
    char padding;
} __attribute__((deprecated("use NewStruct instead")));

/* GCC-style attributes with parentheses */
int var1 __attribute__((aligned(32))) = 0;
typedef int __attribute__((vector_size(16))) v4si;

/* C++11 style attributes (valid in C23/C2x with appropriate flags) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("old type")]]
#endif
struct OldStruct {
    int value;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = {2}, .z = {1, (2), 3} },
    [1] = { .x = { (3 + (4)) }, .y = 5, .z = NESTED_BRACKETS },
    [2] = COMPLEX_MACRO
};

/* Complex initializer with all delimiters */
struct {
    int a;
    int b[2][2];
    struct { int c; } inner;
} complex_var = {
    .a = (1 + (2 * (3))),
    .b = { {1, 2}, {3, 4} },
    .inner = { .c = {5} }
};

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* This section will only be parsed if TEST_COMPLEX is defined */
union U {
    char c;
    struct {
        int i;
        float f;
    } s;
    long (*func_ptr)(int, char **);
};

/* Nested conditional compilation */
#if defined(NESTED_TEST) && (NESTED_TEST > 0)
enum E {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (1 << 2)
};
#endif /* NESTED_TEST */

#elif defined(ALTERNATE)
/* Alternative complex type definitions */
typedef int (*ComplexFunc)(int (*)(int[][(2+3)]), 
                          struct { 
                              int a; 
                              union { 
                                  char c; 
                                  int i; 
                              } u; 
                          });
#else
/* Default: simpler but still complex types */
struct DefaultStruct {
    int matrix[3][(4+1)];
    void (*methods[2])(int, ...);
};
#endif /* TEST_COMPLEX */

/* More deeply nested type definitions */
typedef struct Node {
    int value;
    struct Node *children[((sizeof(int*) * 2) / sizeof(void*))];
    void (*visit)(struct Node *, int depth);
} TreeNode;

/* Function pointer type with extreme nesting */
typedef void (*(*(*ExtremeFuncPtr)(int (*(*)(int[][3]))[2]))
              (char, ...))(double, ...);

/* Union with anonymous struct containing arrays */
union Data {
    struct {
        int ids[(10 * sizeof(int))];
        char name[50];
    };
    struct {
        float values[20];
        double matrix[2][2];
    };
};

/* Requirement 1 (more): Enum with complex initializers */
enum Flags {
    FLAG_NONE = 0,
    FLAG_ONE = (1 << 0),
    FLAG_TWO = (1 << 1),
    FLAG_THREE = (1 << 2),
    FLAG_COMPLEX = ((1 << 3) | (1 << 4) | (sizeof(struct { char a; int b; })))
};

/* Main function that references defined types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct Outer o = {0};
    struct Point p = pts[0];
    TreeNode node = {0};
    
    /* Use variables to prevent optimization */
    (void)s1;
    (void)o;
    (void)p;
    (void)node;
    (void)arr1;
    (void)ptr1;
    (void)arr2;
    (void)arr3;
    (void)arr4;
    (void)var1;
    (void)complex_var;
    (void)x;
    
    return 0;
}

/* Final complex type definition outside main */
struct {
    int (*array_of_funcs[3])(int, ...);
    struct {
        union {
            int i;
            long l;
        } u;
        int arr[((2+3)*4)];
    } nested;
} global_var = {
    .array_of_funcs = {0, 0, 0},
    .nested = {
        .u = { .i = 42 },
        .arr = { [0] = 1, [((2+3)*4)-1] = 99 }
    }
};
