/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (sizeof(struct { int a; })))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ( { [ ( { } ) ] } )
#else
#define EXTRA_NESTING ( { } )
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct S1 {
    /* Function pointer with nested parentheses and struct */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Another with more nesting */
    void (*fp2)(int (*(*nested)[5])(char (*(*)[10])[20]), 
                union { 
                    long l; 
                    struct { 
                        short s; 
                        int (*inner_fp)(int[((2+3)*4)][5]); 
                    } inner; 
                } u);
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; char y[( (10) )];})/4)];

/* Requirement 3: GCC attributes with multiple parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data[3];
    char __attribute__((aligned((8)))) aligned_char;
} __attribute__((deprecated("Use NewStruct instead")));

/* C++11 style attribute (valid in C23/C2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Old type")]]
#endif
union ComplexUnion {
    int i;
    struct [[maybe_unused]] {
        float f;
        double d;
    } nested;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
struct Point { int x; int y; } pts[] = { 
    [0] = { .x = (1), .y = {2} },
    [1] = { .x = { (3 + (4)) }, .y = 5 }
};

/* More complex type definitions */
typedef int (*ComplexFuncPtr)(
    int (*param1)(int[][(sizeof(int)*2)], 
                  struct { 
                      int a; 
                      struct { 
                          char c; 
                      } inner; 
                  }),
    void *ptr_array[((1<<3)-1)]
);

/* Requirement 2: Using the macro in a type declaration */
ARRAY_MACRO;

/* Nested struct with all delimiter types */
struct UltimateNest {
    /* Parentheses in function pointer */
    void (*(*signal_handler))(int, ...);
    
    /* Brackets in array */
    int matrix[ (2 * (3 + 4)) ][ ((5)) ];
    
    /* Braces in anonymous struct */
    struct {
        union {
            struct {
                int depth;
            } s;
        } u;
    } anonymous;
    
    /* Mix of all three */
    int (*(*callbacks[3]))(struct { int x; int y; } points[], 
                          int limits[][ (10) ]);
};

/* Conditional compilation block with balanced tokens */
#ifdef TEST_FEATURE
struct ConditionalStruct {
    int (*cond_fp)(int (*)(int[ { return 2; } ][3]));  /* Invalid but tests token consumption */
    char data[{1,2,3}];  /* GCC extension: array designator */
};
#elif defined(OTHER_FEATURE)
enum ConditionalEnum {
    VAL1 = ( (1) + (2) ),
    VAL2 = { 3 }  /* Invalid in C but tests brace consumption */
};
#else
/* Default: valid nested constructs */
struct DefaultStruct {
    int (*valid_fp)(int (*)(int[2][3]));
    int valid_array[ sizeof(struct { int a[(2+3)]; }) ];
};
#endif

/* Function with complex return type */
__attribute__((noinline)) 
struct { 
    int (*ret_fp)(int, ...); 
    int arr[({ int x = 5; x; })];  /* Statement expression GCC extension */
}* complex_function(int param[({ int y = 10; y; })]);

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference some types to prevent optimization */
    struct S1 s1 = {0};
    struct UltimateNest un = {0};
    
    (void)s1;
    (void)un;
    (void)arr1;
    (void)ptr1;
    (void)x;
    (void)pts;
    
    return 0;
}

/* Additional edge cases */
typedef struct {
    /* Nested parentheses in bitfield */
    unsigned int flags : ( (8) );
    
    /* Array of function pointers */
    int (*(*func_table)[ (sizeof(void*)*2) ])(int, ...);
    
    /* Anonymous union with nested struct */
    union {
        struct {
            int (*(*nested_fp_arr[2]))[3];
        } s;
    };
} EdgeCaseStruct;

/* Macro that expands to contain all delimiter types */
#define ALL_DELIMS(x) { (x)[0], { (x)[1] }, [2] = (x)[2] }

/* Using it */
int all_delims_test[] = ALL_DELIMS((int[]){1,2,3});

/* Final test: deeply nested everything */
struct FinalTest {
    int (*(*(*deep_fp))(int (*(*)(int[][ { .a = 1 } ]))[5]))(
        struct {
            int a[ ( { int z = 3; z; } ) ];
            union {
                struct {
                    char c;
                } s;
            } u;
        }
    );
};
