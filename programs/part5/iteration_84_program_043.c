/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{({})}]])
#else
#define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer with nested parameter containing array and struct */
    int (*callback1)(int (*helper)(int[2][3]), struct { int a; char b; });
    
    /* Even more nesting */
    void (*callback2)(int (*(*nested)[5])(int, int), 
                      struct { 
                          union { 
                              int x; 
                              char y; 
                          } u; 
                      });
};

/* Requirement 4: Array with complex dimensions */
int multi_array[(sizeof(struct DeepNested) + 3) * 2][5];
int *ptr_array[(sizeof(struct { int x; double y; }) / sizeof(int))];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    char buffer[64] __attribute__((aligned(32)));
};

/* Another struct with nested attributes */
struct __attribute__((aligned((16)), 
                     packed, 
                     deprecated("Use NewStruct instead"))) OldStruct {
    int value;
};

/* Requirement 1: Union with nested type definitions */
union ComplexUnion {
    struct {
        int (*compare)(const void *, const void *);
        void (*cleanup)(void *);
    } ops;
    
    struct {
        int matrix[3][(2+3)*4];
        struct AttributedStruct *ptr;
    } data;
};

/* Requirement 5: Initializers with nested braces/parentheses */
int initialized_var = { { ( 1 + (2 + (3)) ) } };

struct Point {
    int x;
    int y;
    int z;
};

/* Array with designated initializers containing nested expressions */
struct Point points[] = {
    [0] = { .x = (1 + 2), .y = {3}, .z = 4 },
    [1] = { .x = {5}, .y = (6 * (7 + 8)), .z = 9 },
    [2] = COMPLEX_MACRO,  /* Using macro expansion */
};

/* Requirement 4 & 5: Complex array with nested initializer */
int complex_init[][2] = {
    { (1), {2} },
    { {3}, (4) },
    NESTED_BRACES,  /* Expands to nested braces */
};

/* Requirement 1: Typedef with extreme nesting */
typedef int (*(*ComplexFuncPtr)[5])(int (*)(int[][3]), 
                                    struct { 
                                        int a; 
                                        struct { 
                                            char b; 
                                        } inner; 
                                    });

/* Requirement 6: Conditionally compiled section */
#ifdef TEST_FEATURE
/* This struct only exists if TEST_FEATURE is defined */
struct ConditionalStruct {
    int (*method)(int param[(2+3)*4]);
    union {
        int a;
        long b[(sizeof(struct Point)/4)];
    } u;
};
#endif

/* Requirement 3: Variable with attribute containing parentheses */
int global_var __attribute__((aligned(16), 
                              section(".data"),
                              used)) = 42;

/* Function declaration with nested parameter types */
extern void register_callback(void (*cb)(int, 
                                         struct { 
                                             int id; 
                                             char name[32]; 
                                         }));

/* Requirement 2: Using macros in type definitions */
struct MacroStruct {
    int array[] NESTED_BRACKETS;  /* Will expand to [ [ [ [ ] ] ] ] */
    int value EXTRA_NESTING;      /* Conditional macro expansion */
};

/* Requirement 4: Multi-dimensional array with parenthesized size */
char buffer[ (2 < 3 ? 10 : 20) ][ ((sizeof(int) * 8) / 2) ];

/* Requirement 5: Nested initializer in union */
union InitUnion {
    struct {
        int a;
        int b;
    } s;
    int arr[2];
} u = { .s = { .a = (1 + (2 * (3 + 4))), .b = {5} } };

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    struct DeepNested dn = {0};
    struct AttributedStruct as = {0};
    union ComplexUnion cu;
    
    /* Reference variables to prevent optimization */
    (void)dn;
    (void)as;
    (void)cu;
    (void)multi_array;
    (void)ptr_array;
    (void)initialized_var;
    (void)points;
    (void)complex_init;
    (void)global_var;
    (void)buffer;
    (void)u;
    
#ifdef TEST_FEATURE
    struct ConditionalStruct cs = {0};
    (void)cs;
#endif
    
    return 0;
}

/* Final complex type definition to ensure all delimiters are tested */
enum __attribute__((deprecated)) OldEnum {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (1 << (1 + 1))
};

/* One more for good measure - typedef with everything */
typedef volatile const struct {
    _Atomic int (*fp[2])(int (*)(int[][(2+3)]), 
                         union { 
                             long l; 
                             double d; 
                         });
    char data[] __attribute__((aligned((8))));
} UltimateType __attribute__((packed));
