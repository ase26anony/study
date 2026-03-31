/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(char));
};

/* Struct with deeply nested type */
struct S2 {
    /* Array of function pointers */
    int (*(*arr[5])(int))(void);
    
    /* Nested anonymous struct */
    struct {
        union {
            struct {
                int x;
            };
            long y;
        } u;
    } nested;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[sizeof(struct {int x; char y[(sizeof(int)*2)];})/4];

/* Using macros in array dimensions */
char arr2 NESTED_BRACKETS;
int arr3[] = NESTED_BRACES;

/* Requirement 3: Attribute specifications */
struct S3 {
    int a;
    char b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use S4 instead")));

/* GCC attributes with parentheses */
int var1 __attribute__((aligned((16)), 
                       section((".data"))));

/* C++11 style attributes (valid in C23/C++11) */
#if defined(__cplusplus) || __STDC_VERSION__ >= 202311L
struct [[deprecated("Old struct"), 
         maybe_unused]] S4 {
    int value;
};
#endif

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
int y[] = { 1, {2}, {{3}}, {{{4}}} };

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = {2}, .z = {1, (2), 3} },
    [1] = { .x = { { (3) } }, .y = 4, .z = {5,6,7} },
    [2] = COMPLEX_MACRO  /* Using macro expansion */
};

/* Union with complex initializer */
union U1 {
    int i;
    float f;
    struct {
        char c;
        short s;
    } st;
} u1 = { .st = { .c = 'a', .s = (256) } };

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Struct inside conditional compilation */
struct ConditionalStruct {
    int (*callback)(int (*)(int[][3]), 
                    struct { 
                        int a; 
                        char b[({ int x = 5; x; })];  /* Statement expression */
                    });
};
#elif defined(ALTERNATE)
/* Alternative with different nesting */
union ConditionalUnion {
    char c;
    struct { 
        int i; 
        long l[((sizeof(int)+3)/4)];
    } s;
};
#else
/* Default when neither is defined */
typedef struct {
    int x;
    int y[({ 2 + 3; })];  /* GCC statement expression */
} DefaultType;
#endif

/* More complex examples combining multiple requirements */
typedef int (*ComplexFuncPtr)(
    int,
    struct {
        int a;
        int b[((2*3)+4)];
    } *,
    void (*)(int, int)
);

/* Struct with attribute and complex member */
struct S5 {
    ComplexFuncPtr func;
    int data[__builtin_choose_expr(1, 10, 20)];
} __attribute__((aligned(
    (sizeof(long) > 4) ? 8 : 4
)));

/* Function-like macro with balanced tokens */
#define CALLBACK(fn, arg) (fn)((arg), {0})

/* Nested type in typedef */
typedef struct Outer {
    struct Inner {
        int (*method)(struct Inner *, 
                     int (*)(int[][2][3]));
    } *inner;
    
    union {
        int x;
        struct {
            char a;
            char b;
        } chars;
    } u;
} OuterType;

/* Variable using all the complex types */
OuterType global_var = {
    .inner = NULL,
    .u = { .chars = { 'x', 'y' } }
};

/* Array with designators and nested braces */
int complex_array[][2] = {
    [0] = {1, 2},
    [1] = {{3}, 4},
    [2] = {5, {6}},
    [3] = NESTED_BRACES
};

/* Main function that references types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    
    /* Reference variables to prevent optimization */
    (void)s1.fp1;
    (void)s2.arr;
    (void)s3.a;
    
    /* Use array to prevent elimination */
    int sum = arr1[0][0] + x + y[0] + pts[0].x + u1.i;
    
    /* Use conditional type */
#ifdef TEST_COMPLEX
    struct ConditionalStruct cs;
    (void)cs.callback;
#elif defined(ALTERNATE)
    union ConditionalUnion cu;
    (void)cu.c;
#else
    DefaultType dt;
    (void)dt.x;
#endif
    
    OuterType *ptr = &global_var;
    (void)ptr;
    
    return sum - sum;  /* Always return 0 */
}

/* Final complex type definition outside main */
enum E {
    A = (1 << 0),
    B = (1 << 1),
    C = (1 << ( { int z = 2; z; } ))  /* Statement expression */
};

/* Typedef with nested parentheses */
typedef int (*(*NestedFuncPtr)(int (*)(int)))(char);

/* One more for good measure - struct with bitfields and attributes */
struct BitfieldStruct {
    unsigned int a:4;
    unsigned int b:(( { int width = 4; width; } ));
    unsigned int c:12;
} __attribute__((packed, aligned(( { int align = 2; align; } ))));
