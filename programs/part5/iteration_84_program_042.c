/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (3 + (4))))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING [[deprecated("test")]]
#else
  #define EXTRA_NESTING
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct Level1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*fp2)(int (*(*nested)[5])(char (*(*)[10])[20]), 
                union { 
                    long l; 
                    struct { 
                        short s; 
                        int (*fp)(int (*(*)[3])[4]); 
                    } inner; 
                });
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; double y;})/sizeof(int))];

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    char c;
    int i __attribute__((aligned(8)));
    long l;
} EXTRA_NESTING;

/* Another struct with nested attributes */
union __attribute__((aligned((16+8)/2))) ComplexUnion {
    struct {
        int a __attribute__((deprecated));
        int b;
    } s;
    char data[16];
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int complex_init = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2, {3}} }, 
    [1] = { .x = COMPLEX_PARENS(5), .y = {6, 7} }
};

/* Use the macro from Requirement 2 */
ARRAY_MACRO;

/* Requirement 1 (more): Enum with complex expressions */
enum E {
    VAL1 = (1 << (2+3)),
    VAL2 = sizeof(struct { char a[(2*3)+4]; }),
    VAL3 = (int){ (2 + (3 * (4))) }
};

/* Requirement 4 (more): Array with nested type in size */
typedef struct {
    int matrix[3][4];
} Matrix3x4;

Matrix3x4 matrices[sizeof(Matrix3x4[2])/sizeof(Matrix3x4)];

/* Requirement 3 (more): C++11 style attributes (works in C23/C++11) */
#ifdef __cplusplus
[[deprecated("Complex type")]]
#endif
typedef int (*ComplexFuncPtr)(int (*)(int[][3], struct {int a;}), 
                              void (*)(char (*(*)[5])[10]));

/* Requirement 6: More conditional compilation */
#if defined(TEST_NESTING) && (__STDC_VERSION__ >= 201112L)
_Static_assert(sizeof(struct Level1) > 0, "Level1 must have size");
#endif

#ifdef TEST_COMPLEX
/* Additional complex type inside conditional block */
struct ConditionalStruct {
    int (*fp)(struct { int a[(2+(3*4))]; } inner);
};
#endif

/* Requirement 5 (more): Nested initializer with designators */
struct NestedInit {
    struct {
        int a;
        int b[2];
    } inner;
    int c;
} nested_var = { 
    .inner = { 
        .a = (1+2), 
        .b = { (3), {4} } 
    }, 
    .c = {5} 
};

/* Function pointer array with complex signatures */
int (*func_array[])(int, ...) = {
    NULL,
    (int (*)(int, ...))0x1234
};

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference variables to prevent optimization */
    (void)multi_dim[0][0];
    (void)ptr_array;
    (void)complex_init;
    (void)points;
    (void)arr_macro;
    (void)matrices;
    (void)nested_var;
    (void)func_array;
    
    return 0;
}

/* Final complex type definition with all delimiters mixed */
struct UltimateTest {
    /* Parentheses in function pointer */
    int (*(*ultimate_fp)[(2+3)])(char (*(*)[{5}])[10]);
    
    /* Braces in nested struct */
    struct {
        int a[({2})];
        union {
            long l;
            struct { short s; } inner;
        } u;
    } nested;
    
    /* Brackets in array */
    int ultimate_array[sizeof(struct { char c; })][3];
} __attribute__((aligned((16))));
