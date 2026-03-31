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
#else
#define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Struct with deeply nested parentheses in function pointer */
struct S1 {
    /* Function pointer with nested parameter containing array and struct */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; char b; });
    
    /* Another with multiple nesting levels */
    void (*fp2)(int (*(*nested)[5])(double), 
                struct T { 
                    union U { 
                        int x; 
                        float y; 
                    } u; 
                });
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int arr1[(2+3)*4][5];
int arr2[sizeof(struct { int x; double y; }) / sizeof(int)];
int *ptr_array[(sizeof(struct S1) + 7) & ~7];

/* Requirement 3: GCC attributes with multiple parentheses */
struct S2 {
    int data;
    char buffer[64];
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* More attribute usage */
struct S3 {
    long counter;
    short flags;
} __attribute__((aligned(32))) 
  __attribute__((packed));

/* Requirement 1 & 3 combined: Struct with attributes and nested types */
struct S4 {
    /* Nested anonymous struct with attribute */
    struct {
        int id __attribute__((aligned(8)));
        float value;
    } item;
    
    /* Pointer to function with attribute */
    void (*handler)(void) __attribute__((noreturn));
};

/* Requirement 5: Initializers with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };
int y = { NESTED_PARENS };
int z[] = COMPLEX_MACRO;

/* Complex initializer with designated initializers */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = { .x = (1 + 2), .y = {3}, .z = 4 },
    [1] = { .x = {5}, .y = (6 * 7), .z = {8} },
    [2] = { .x = NESTED_PARENS, .y = 9, .z = 10 }
};

/* Union with nested initializer */
union Data {
    int i;
    float f;
    struct {
        char a;
        char b;
    } chars;
};

union Data data = { .chars = { .a = 'x', .b = 'y' } };

/* Requirement 4 & 5: Array with complex initializer */
int matrix[][3] = {
    { 1, (2), {3} },
    { (4), 5, 6 },
    { 7, 8, (9) }
};

/* Requirement 1: Enum with complex expressions */
enum E {
    A = (1 << 0),
    B = (1 << 1),
    C = (A | B),
    D = (C + 1) * 2
};

/* Requirement 1: Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[][3]), 
                              struct { 
                                  int a; 
                                  struct { 
                                      char c; 
                                  } s; 
                              });

/* Requirement 2 & 4: Using macros in array dimensions */
int macro_arr[] = NESTED_BRACES;
char nested_arr[][2] = { NESTED_BRACES };

/* Requirement 6: Conditional type definitions */
#ifdef TEST_NESTED
/* This struct only exists if TEST_NESTED is defined */
struct ConditionalStruct {
    int (*very_nested)(int (*(*)[(2+3)])[4], 
                       union { 
                           long l; 
                           struct { 
                               short s; 
                           } inner; 
                       });
};
#endif

/* Requirement 1: One more deeply nested case */
struct Final {
    /* Array of function pointers */
    void (*(*func_table)[(sizeof(int*) + 3) / 4])(int, ...);
    
    /* Nested struct with bitfield */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : (2 + 1);
        unsigned int flag3 : 4;
    } bits;
    
    /* Anonymous union */
    union {
        int i;
        double d;
        struct {
            char c[((2*3)+1)];
        } str;
    } value;
};

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    struct S4 s4 = {0};
    struct Final f = {0};
    
    /* Use variables to prevent optimization */
    (void)s1;
    (void)s2;
    (void)s3;
    (void)s4;
    (void)f;
    (void)arr1;
    (void)arr2;
    (void)ptr_array;
    (void)x;
    (void)y;
    (void)z;
    (void)points;
    (void)data;
    (void)matrix;
    
    return 0;
}

/* End of file with trailing content that might contain delimiters */
int trailing_array[] = { [0] = 1, [1] = {2}, [2] = (3) };
