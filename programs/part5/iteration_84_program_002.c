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
    void (*fp2)(int (*(*)(int (**)(int))[5])(double));
};

/* Struct with deeply nested type */
struct S2 {
    /* Array of function pointers */
    int (*(*arr[3])(int))[4];
    
    /* Nested anonymous struct */
    struct {
        union {
            struct {
                int x;
            } s;
            char c;
        } u;
    } inner;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y[3];})/sizeof(int))];

/* Using macros in array dimensions */
int arr2 NESTED_BRACKETS;
int arr3[sizeof(NESTED_PARENS)];

/* Requirement 3: Attribute specifications */
struct S3 {
    int a;
    char b;
} __attribute__((aligned(16), packed, deprecated("use S4 instead")));

/* Struct with attribute containing parentheses */
struct __attribute__((aligned((8)*(2)))) S4 {
    double d;
    int i __attribute__((deprecated));
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };
int y[] = NESTED_BRACES;

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = {
    [0] = { .x = (1), .y = {2}, .z = { {1}, {2}, {3} } },
    [1] = { .x = ( (3) ), .y = { {4} }, .z = COMPLEX_MACRO }
};

/* Union with complex initializer */
union U1 {
    int i;
    float f;
    struct {
        char c;
        short s;
    } nested;
} u1 = { .nested = { .c = ('a'), .s = {256} } };

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX
/* This struct will only be parsed if TEST_COMPLEX is defined */
struct ConditionalStruct {
    int (*complex_fp)(int (*[][(2+3)])(struct {int a;}), ...);
    int arr[sizeof(struct { char c; int i; })];
};
#endif

#if defined(USE_NESTED) || 1  /* Always true, but tests #elif */
/* Function pointer type with extreme nesting */
typedef int (*(*(*ComplexFunc)(int (*(*)(int[][3]))[2]))(void))(char);
#elif 0
/* Alternative definition */
typedef void (*SimpleFunc)(int);
#endif

#ifndef SKIP_THIS
/* Another conditional type */
enum E {
    A = (1 << 0),
    B = (1 << (1)),
    C = ( ( (2) ) * 3 )
};
#endif

/* More complex examples */
struct S5 {
    /* Multi-dimensional array with computed size */
    int matrix[(sizeof(struct {int x; char y;}) + 7)/8][4][2];
    
    /* Pointer to array of function pointers */
    int (*(*(*func_table)[(2+3)])[4])(int, int);
};

/* Nested typedefs */
typedef struct {
    int (*callback)(int (*)(int[][(2+3)]), ...);
} CallbackHolder;

/* Final complex type combining everything */
typedef union {
    struct {
        CallbackHolder ch;
        int data[((sizeof(CallbackHolder) + 15) / 16) * 16];
    } s;
    unsigned char raw[sizeof(struct {
        CallbackHolder ch;
        int data[((sizeof(CallbackHolder) + 15) / 16) * 16];
    })];
} __attribute__((aligned(32))) UltimateType;

/* Requirement 1 (more): Function with complex return type */
struct S6* (*(*create_factory(int size))[(size > 0) ? size : 1])(void) {
    return 0;
}

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some variables to prevent optimization */
    struct S1 s1 = {0};
    struct S4 s4 = {0};
    UltimateType ut = {0};
    
    (void)s1;
    (void)s4;
    (void)ut;
    (void)arr1;
    (void)ptr1;
    (void)x;
    (void)y;
    (void)pts;
    (void)u1;
    
    return 0;
}

/* Additional edge cases */
#if 1
/* Always included section with nested tokens */
int (*global_func_ptr)(int (*)(int[][(2+3)][4]), 
                       struct { 
                           int a; 
                           int b[((3+2)*2)]; 
                       }) = 0;
#endif

/* Macro that expands to something with all delimiters */
#define ULTIMATE_MACRO { ( [ { ( [ ] ) } ] ) }
int dummy_var[] = ULTIMATE_MACRO;

/* Final check: ensure we have all three delimiter types */
struct FinalCheck {
    int a;      /* Will trigger '(' in attributes if used */
    char b[5];
} __attribute__((aligned((8))));

/* Array with nested bracket initializer */
int final_array[][2] = {
    {1, 2},
    {3, (4)},
    {{5}, {6}}
};
