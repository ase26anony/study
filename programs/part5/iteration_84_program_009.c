/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct FunctionPointerStruct {
    /* Nested function pointer with complex parameter list */
    int (*level1)(int (*level2)(int (*level3)(int[2][3]), 
                                struct { 
                                    int a; 
                                    struct { 
                                        char b; 
                                    } inner; 
                                }), 
                 union { 
                     long x; 
                     double y; 
                 });
    
    /* Another deeply nested example */
    void (*another)(int, 
                   struct { 
                       int (*nested_fp)(int[((2+3)*4)][5]); 
                   }, 
                   char);
};

/* Union with nested anonymous struct */
union ComplexUnion {
    char c;
    struct {
        int i;
        float f;
        struct {
            double d;
            long l;
        } deeper;
    } s;
    long long ll;
};
#endif /* TEST_COMPLEX */

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), 
                      packed, 
                      deprecated("Use NewStruct instead"))) 
    AttributedStruct {
    int data __attribute__((aligned(8)));
    char buffer[64] __attribute__((aligned(32)));
};

/* Also test GNU attributes in different positions */
int global_var __attribute__((section(".data"), 
                             used, 
                             visibility("hidden"))) = 42;

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2+3)*4][5];
int *pointer_array[sizeof(struct {int x; char y; double z;})/sizeof(int)];

/* Array with nested type definition in size */
struct Container {
    int data[sizeof(union { 
        long a; 
        struct { 
            short b; 
            char c; 
        } s; 
    })];
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int complex_init = { { ( 1 + (2 * (3 + (4)))) } };

struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = { .x = (1), .y = {2}, .z = 3 },
    [1] = { .x = {4}, .y = (5 + (6 * 7)), .z = {8} },
    [2] = { .x = 9, .y = 10, .z = 11 }
};

/* Nested initializer using macros */
int macro_init[] = NESTED_BRACES;
int mixed_macro_init[] = COMPLEX_MACRO;

/* Enum with complex expressions */
enum ComplexEnum {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1) | (1 << 2),
    VALUE3 = sizeof(struct { int a; int b; }) / sizeof(int),
    VALUE4 = ( (2 + 3) * 4 )
};

/* Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[][(2*3)], 
                                     struct { 
                                         int field; 
                                     }), 
                             void *);

/* Another conditional block */
#if defined(USE_FEATURE) && (FEATURE_VERSION > 2)
/* Requirement 1 (more): Struct with nested arrays and function pointers */
struct NestedType {
    int (*callbacks[3])(int, 
                       struct { 
                           int param; 
                       }, 
                       char);
    union {
        int i;
        struct {
            float f;
            double d;
        } fs;
    } data[2][2];
};
#endif

/* Function with complex return type and parameters */
ComplexFuncPtr get_function(int param1, 
                           struct { 
                               int a; 
                               int b; 
                           } param2, 
                           int param3[(sizeof(int) + 3)/2]) {
    static int (*func)(int (*)(int[][(2*3)], 
                              struct { 
                                  int field; 
                              }), 
                      void *) = NULL;
    return func;
}

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    struct AttributedStruct as = {0};
    struct Point *p = points;
    
    /* Use the macro-expanded arrays */
    int size1 = sizeof(macro_init) / sizeof(macro_init[0]);
    int size2 = sizeof(mixed_macro_init) / sizeof(mixed_macro_init[0]);
    
    /* Reference the complex array */
    multi_dim_array[0][0] = 1;
    
    /* Reference the pointer array */
    pointer_array[0] = &multi_dim_array[0][0];
    
    return (as.data + p->x + size1 + size2) & 0;
}

/* Final conditional block with deeply nested tokens */
#ifdef EXTREME_NESTING
/* Extreme nesting test case */
struct Extreme {
    int (*(*level1[2]))(int (*)(int (*[][(2+{3?4:5})])(struct { 
                                   int a; 
                                   struct { 
                                       union { 
                                           char c; 
                                           int i; 
                                       } u; 
                                   } s; 
                               }), 
                           int), 
                      void *);
};
#endif
