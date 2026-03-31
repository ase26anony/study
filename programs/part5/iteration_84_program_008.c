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
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))(double));
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
            } s;
            char c;
        } u;
    } nested;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *arr2[sizeof(struct {int x; char y[(sizeof(double)/2)];})/4];

/* Using macros in array dimensions */
int arr3 NESTED_BRACKETS;
int arr4[] = NESTED_BRACES;

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
#if __STDC_VERSION__ >= 202311L || defined(__cplusplus)
struct [[deprecated("Old struct"), 
         maybe_unused]] S4 {
    int x[10];
};
#endif

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
int y[] = { 1, {2}, {{3}}, {{{4}}} };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = {3,4} },
    [1] = { .x = {{5}}, .y = { {6} }, .z = { {7}, {8} } }
};

/* Complex initializer using macros */
int init1[] = COMPLEX_MACRO;

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_COMPLEX
    /* This struct will only be parsed if TEST_COMPLEX is defined */
    union U1 { 
        char c; 
        struct { 
            int i; 
            long (*func)(int (*)(int[][3]), ...);
        } s; 
    };
#elif defined(TEST_SIMPLE)
    struct Simple {
        int x;
    };
#else
    /* Default case with nested tokens */
    struct Default {
        int matrix[2][(3+4)*2];
        void (*callback)(struct { int a; int b; } param);
    };
#endif

/* More conditional blocks */
#if defined(__GNUC__) && !defined(__clang__)
    /* GCC-specific attributes with parentheses */
    __attribute__((constructor((300))))
    void init_func(void) {
        /* Empty but attribute has parentheses */
    }
#endif

/* Nested conditional compilation */
#ifdef OUTER
    #ifdef INNER
        struct DoublyNested {
            int (*triple_nested)(int (*(*[2])())[3]);
        };
    #endif
#endif

/* Function with complex parameter type */
void complex_func(
    int (*param1)(int (*)(int[][(2*3)]), ...),
    struct { 
        int a; 
        struct { 
            char b[sizeof(int[2])]; 
        } inner; 
    } param2
) {
    /* Local variable with complex type */
    int (*(*local_var)(int (*(*)(double))[3]))(void);
    
    /* Avoid unused parameter warnings */
    (void)param1;
    (void)param2;
    (void)local_var;
}

/* Typedef with extreme nesting */
typedef int (*(*(*ExtremeType)[5])(int (*(*)(void))[2]))(char);

/* Enum with complex expressions (C23 allows this) */
enum E {
    A = (1 << 2),
    B = sizeof(struct { char arr[(2+3)*4]; }),
    C = (int){3}  /* compound literal in enum (C23) */
};

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    
    /* Use the variables to prevent optimization */
    (void)arr1;
    (void)arr2;
    (void)x;
    (void)y;
    (void)pts;
    
    complex_func(NULL, (struct { int a; struct { char b[8]; } inner; }){0});
    
    return 0;
}

/* Final complex type definition wrapping everything together */
struct Ultimate {
    /* Mix of all token types */
    int (*funcs[3])(int (*)(int[2][3]), 
                    struct { 
                        int a; 
                        union { 
                            char c; 
                            int i; 
                        } u; 
                    });
    
    /* Attributes */
    int attr __attribute__((aligned((16))));
    
    /* Nested array with parenthesized size */
    char data[(sizeof(int) + sizeof(long)) * 2];
    
    /* Anonymous union */
    union {
        struct {
            int x;
        } s;
        double d;
    };
} __attribute__((packed));
