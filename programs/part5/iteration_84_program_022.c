/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses in parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*nested)(int[][(2+3)]))(double), 
                union { char c; float f; } u);
};

/* Struct with all three delimiter types deeply nested */
struct S2 {
    int (*arr_ptr)[(sizeof(struct {int x;})/4)];
    void (*func_array[2])(struct { int (*nested)(int i[(2+3)*4]); } s);
    char data[({ int x = 5; x; })];
};

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    int a;
    char b;
    double c __attribute__((aligned(32)));
} __attribute__((deprecated));

/* C++11 style attributes (valid in C23/C2x with appropriate flags) */
#if __STDC_VERSION__ >= 202311L || defined(__cplusplus)
[[deprecated("Use NewStruct instead")]]
#endif
struct OldStruct {
    int value;
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; char y[10];})/sizeof(int))];

/* Array with nested type in dimension */
struct Container {
    int data[ (int)(sizeof(struct { char a; double b; }) / sizeof(char)) ];
    float matrix[3][ (2 + (3 * 4)) ];
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = { (3+4), {5} } },
    [1] = { .x = { { (6) } }, .y = 7, .z = NESTED_BRACES }
};

/* Union with nested initializer */
union U {
    char c;
    struct { 
        int i; 
        float f[2]; 
    } s;
};

union U u1 = { .s = { .i = (10), .f = { {1.0}, {2.0} } } };

/* Requirement 6: Conditional compilation blocks with balanced tokens */
#ifdef TEST_COMPLEX
    struct ConditionalStruct {
        int (*complex_func)(int (*)(int[][(1+2)*3]), 
                           struct { 
                               union { 
                                   char c; 
                                   int i[(2+{3})];  /* Note: {3} is invalid here, 
                                                      but shows token balance */
                               } u; 
                           });
    };
#elif defined(TEST_SIMPLE)
    typedef int (*FuncPtr)(int, int);
    FuncPtr funcs[] = { NULL, NULL, NULL };
#else
    /* Default case with nested tokens */
    enum E {
        A = (1 << (sizeof(struct { char a; }) - 1)),
        B = [(2)]  /* Invalid but balanced tokens for parser */
    };
#endif

/* More complex examples combining multiple requirements */
typedef struct {
    __attribute__((aligned(8))) int value;
    void (*methods[2])(
        struct {
            int id;
            char name[({ int len = 10; len; })];
        } *arg
    );
} ObjectType;

/* Function-like macro with balanced delimiters */
#define CALL_FUNC(f, args) f args
#define CREATE_STRUCT(type) (type){ .x = 1, .y = 2 }

/* Nested type in typedef */
typedef int (*(*ComplexFuncPtr)[3])(double, 
    struct {
        int a[ (2 + 3) * 4 ];
        char b;
    });

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    
    /* Use macro expansions */
    int arr1[] = NESTED_PARENS;
    int arr2[] = NESTED_BRACKETS;
    int arr3[] = NESTED_BRACES;
    int arr4[] = COMPLEX_MACRO;
    
    /* Reference initialized variables */
    (void)x;
    (void)pts;
    (void)u1;
    (void)multi_dim;
    (void)ptr_array;
    
    return 0;
}

/* Additional edge cases */
#if 0
    /* This block is never compiled but contains balanced tokens */
    struct NeverCompiled {
        int (*impossible)[({ 
            struct { 
                int x[ (1 + (2 + (3))) ]; 
            } s; 
            0; 
        })];
    };
#endif

/* Final test with all delimiters mixed */
struct UltimateTest {
    int a;
    struct {
        int b[ (int[]){1, 2, 3}[0] ];
        void (*c)(struct { int d; } (*)[ ({ 2 + 3; }) ]);
    } inner;
} __attribute__((packed));
