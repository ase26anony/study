/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(void), 
                union { 
                    char c; 
                    struct { 
                        int (*nested_fp)(int, int); 
                    } s; 
                });
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[(sizeof(int)*2)];})/4)];
int arr2[sizeof(int[(NESTED_PARENS, 10)])];

/* Requirement 3: Attribute specifications */
struct S2 {
    int a;
    char b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use struct S3 instead")));

struct S3 {
    long x __attribute__((aligned((8)*(2))));
    double y;
} __attribute__((packed));

/* More complex type with attributes */
typedef struct S4 {
    int (*callback)(int, int) __attribute__((nonnull(1, 2)));
    int data[10] __attribute__((aligned(32)));
} S4_t __attribute__((may_alias));

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };
int y[] = NESTED_BRACES;
int z = { ( [ { 0 } ] ) };  /* Using macro expansion */

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } nested;
};

struct Point pts[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = {(2), {3}}, 
        .nested = { .z = {4} } 
    },
    [1] = COMPLEX_MACRO  /* Macro expansion in initializer */
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
    long array[sizeof(struct { char a; int b; })];
};

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX
    /* This struct will only be parsed if TEST_COMPLEX is defined */
    struct ConditionalStruct {
        int (*method)(struct { int x; } param);
        int data[((sizeof(int) + 3) & ~3)];
    };
#elif defined(ALTERNATE)
    /* Alternative complex type */
    union ConditionalUnion {
        struct { 
            int (*fp)(int, int); 
        } s;
        char data[({ int x = 5; x * 2; })];
    };
#else
    /* Default complex type */
    typedef struct DefaultStruct {
        enum { A = 1, B = 2 } tag;
        int values[((A > B) ? 10 : 20)];
    } DefaultStruct_t;
#endif

/* Function pointer type with extreme nesting */
typedef int (*(*complex_fp_t)(int (*(*)(int[][(2+3)]))(void)))(char, 
    struct { 
        union { 
            int a; 
            float b[(sizeof(double)/sizeof(float))]; 
        } u; 
    });

/* Array of function pointers */
void (*func_array[])(int, ...) = {
    NULL,
    (void (*)(int, ...))0x1234,
    NULL
};

/* Nested anonymous structs/unions */
struct Outer {
    struct {
        union {
            struct {
                int depth1;
                struct {
                    int depth2;
                };
            };
            float f;
        };
    } anonymous;
};

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    S4_t s4 = {0};
    struct Point p = pts[0];
    
    /* Reference variables to prevent optimization */
    volatile int use_x = x;
    volatile int use_y = y[0];
    
    return (s1.fp1 == NULL) ? 0 : 
           (s4.callback == NULL) ? 1 : 
           (p.x > 0) ? 2 : 3;
}

/* Additional complex declarations after main */
#if defined(__cplusplus) && __cplusplus >= 201103L
/* C++11 attributes */
struct [[deprecated("C++11 attribute test"), 
         maybe_unused]] CPP11Struct {
    int value [[gnu::aligned(16)]];
    char data[10] [[gnu::aligned(32)]];
};
#endif

/* Final extreme test case */
struct UltimateTest {
    /* Everything combined */
    int (*(*ultimate_fp)(
        struct { 
            int a[((2+3)*4)]; 
            union { 
                char c; 
                int i; 
            } u __attribute__((aligned(8)));
        } param,
        int (*(*nested)(int[][(sizeof(struct {int x;})/4)]))(void)
    ))(void) __attribute__((warn_unused_result));
    
    /* Initializer with everything */
    int init[3] __attribute__((aligned(16))) = { 
        { ( [ { 1 } ] ) }, 
        (2 + (3 * (4))), 
        sizeof(int[(NESTED_PARENS, 5)]) 
    };
};
