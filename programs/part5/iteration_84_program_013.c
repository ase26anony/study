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
    
    /* Even more nesting */
    void (*fp2)(int (*(*)(int (*)[(2+3)])[5])(char));
};

/* Union with nested structures */
union U1 {
    char c;
    struct {
        int i;
        struct {
            float f;
            double d;
        } inner;
    } s;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y[3];})/sizeof(int))];

/* Using macros in array dimensions */
int arr2[] = NESTED_BRACES;
char arr3[][2] = {NESTED_BRACES};

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    double b;
} __attribute__((aligned(16), packed));

/* Struct with GCC attributes containing parentheses */
struct S3 __attribute__((aligned((sizeof(double)*2)))) {
    long l;
    short s;
};

/* Requirement 1 (more): Typedef with complex nested types */
typedef int (*complex_func_t)(
    int (*callback)(int[][(sizeof(int)*2)], 
                    struct { 
                        int x; 
                        struct { 
                            char c; 
                        } inner; 
                    }),
    void *ptr[(1+2)*3]
);

/* Enum with complex expressions */
enum E1 {
    VAL1 = (1 << (sizeof(int)*8 - 1)),
    VAL2 = ((2 + 3) * 4),
    VAL3 = {0}  /* GCC extension */
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x1 = { { ( 1 + (2) ) } };
int x2 = { { { { ( (5) ) } } } };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = {2}, .z = { (3), (4) } },
    [1] = { .x = {5}, .y = { {6} }, .z = NESTED_BRACES }
};

/* Array with nested initializer */
int matrix[2][2] = { { {1}, {2} }, { {3}, { {4} } } };

/* Requirement 6: More conditional compilation */
#if defined(USE_NESTED)
struct ConditionalStruct {
    int a[(1+2)*3];
    union {
        char c;
        int i[(sizeof(struct {short s;})/2)];
    } u;
};
#elif defined(USE_OTHER)
struct AlternativeStruct {
    float f[ {1, 2, 3}.x ];  /* Invalid but tests token consumption */
};
#else
/* Default: empty struct */
struct DefaultStruct {
    int dummy;
};
#endif

/* Function with complex parameter and nested attributes */
#ifdef __GNUC__
void __attribute__((noinline, 
                    format(printf, 1, 
                           (2)))) 
complex_func(char *fmt, ...) {
    /* Function body */
}
#endif

/* C++11 attributes if compiled as C++ */
#ifdef __cplusplus
[[deprecated("Use new_struct instead")]]
struct OldStruct {
    int data[[gnu::aligned(16)]];
};
#endif

/* Requirement 4: More complex array declarations */
/* Array of function pointers */
int (*func_array[3])(int, 
                     struct { 
                         int a; 
                         int b[(2+3)]; 
                     }) = {NULL, NULL, NULL};

/* Multi-dimensional array with computed sizes */
int arr4[ sizeof(int[2][3]) / sizeof(int) ][ ( { int x = 5; x; } ) ];

/* Nested anonymous structs/unions */
struct Outer {
    struct {
        int a;
        union {
            char c;
            int i;
        } u;
    } inner;
    
    struct {
        struct {
            int deep;
        } deeper;
    } nested;
};

/* Typedef with all three delimiters deeply nested */
typedef struct {
    int (*fp)(int (*)(int[][3], 
                      struct { 
                          int x; 
                          union { 
                              char c; 
                              int i; 
                          } u; 
                      }),
              void *args[{1, 2, 3}.x]);
    char data[((2+3)*4)];
} ultra_complex_t;

/* Requirement 5: Complex designated initializers */
struct ComplexInit {
    int a;
    struct {
        int b;
        int c[2];
    } s;
} ci = { 
    .a = (1 + (2 * (3))), 
    .s = { 
        .b = {4}, 
        .c = { {5}, {6} } 
    } 
};

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some variables to prevent optimization */
    struct S1 s1 = {0};
    ultra_complex_t uc = {0};
    
    (void)s1;
    (void)uc;
    (void)x1;
    (void)x2;
    (void)arr1;
    (void)ptr1;
    (void)ci;
    
    return 0;
}

/* Final macro expansion with all delimiters */
#define ULTIMATE_NESTING \
    { ( [ { ( [ { } ] ) } ] ) }

/* Use it somewhere */
int final_array[] = ULTIMATE_NESTING;
