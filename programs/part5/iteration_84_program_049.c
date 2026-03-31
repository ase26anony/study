/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{({})}]])
#else
#define EXTRA_NESTING ([])
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Level1 {
    /* Function pointer with nested parameter containing array and struct */
    void (*callback1)(int (*helper)(int[2][(3+2)*sizeof(int)], 
                                   struct { 
                                       int x; 
                                       struct { 
                                           char c; 
                                       } inner; 
                                   }), 
                      double);
    
    /* Another function pointer with multiple nesting levels */
    char* (*callback2)(int (*(*nested)[(sizeof(int)+3)/2])(int[][5]), 
                       union {
                           long l;
                           struct {
                               short s;
                           } us;
                       });
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int multi_dim_array[(2+3)*(4+1)][sizeof(struct Level1)/sizeof(int)]
                   [NESTED_BRACKETS [0] + 1];

/* Requirement 3: GCC attributes with multiple parentheses */
struct __attribute__((aligned((16)), 
                     packed, 
                     deprecated("Use NewStruct instead"))) 
       AttributedStruct {
    int data __attribute__((aligned((8+8))));
    
    /* Nested struct with attribute */
    struct __attribute__((packed)) {
        char a;
        int b __attribute__((aligned((1<<4))));
    } inner;
};

/* Requirement 1: Union with nested type definitions */
union ComplexUnion {
    /* Array of function pointers */
    int (*(*func_array)[(5*2)-3])(int, ...);
    
    /* Struct with nested anonymous struct */
    struct {
        struct {
            int depth;
        } level2;
        
        /* Pointer to array with computed size */
        float (*matrix)[(sizeof(struct AttributedStruct)+7)/8][8];
    } nested_struct;
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized_var = { { ( 1 + (2 + (3)) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

/* Complex initializer with designated initializers */
struct Point points[] = {
    [0] = { .x = (1 + (2*3)), 
            .y = {2}, 
            .z = { [(2+1)/3] = 5, [1] = {3} } },
    [1] = { .x = COMPLEX_MACRO,  /* Using macro expansion */
            .y = EXTRA_NESTING[0], 
            .z = NESTED_BRACES }
};

/* Requirement 4: Pointer to multi-dimensional array with complex size */
int (*complex_ptr)[(sizeof(union ComplexUnion) > 16) ? 8 : 4]
                  [((2<<3) + 1)]
                  [NESTED_PARENS ? 1 : 2];

/* Requirement 1: Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[][(4+1)], 
                                     struct { 
                                         int tag; 
                                         union { 
                                             int i; 
                                             float f; 
                                         } value; 
                                     }), 
                                void*);

/* Requirement 6: Conditional type definition */
#if defined(USE_EXTRA_FEATURES) && (__STDC_VERSION__ >= 201112L)
/* C11 anonymous struct/union */
struct C11Struct {
    union {
        struct {
            int a;
        };
        struct {
            long b;
        };
    };
    int arr[][(3*2)+1];
};
#elif defined(USE_GNU_EXTENSIONS)
/* GNU extension with nested attributes */
struct GnuStruct {
    int field __attribute__((aligned((16)), 
                            deprecated, 
                            unused));
} __attribute__((packed));
#else
/* Default fallback */
struct Fallback {
    int simple;
};
#endif

/* Requirement 5: More complex initializers */
struct {
    int a;
    struct {
        int b[2];
    } inner;
} anonymous_var = { 
    .a = ( { int x = 5; x; } ),  /* GCC statement expression */
    .inner = { .b = { [(1+2)-3] = 1, [1] = {2} } }
};

/* Function with complex parameter type */
void process_data(int (*processor)(int, 
                                   struct Level1*, 
                                   int[][(sizeof(int)*2)]),
                  ComplexFuncPtr callback) {
    /* Local array with computed size */
    char buffer[(processor != NULL) ? 128 : 64];
    
    /* Nested struct declaration */
    struct Local {
        int counter;
        struct {
            int flag;
        } status;
    } local_var = { .counter = 0, .status = { .flag = (1) } };
    
    (void)buffer;
    (void)local_var;
    (void)callback;
}

/* Main function that references defined types to avoid dead code elimination */
int main(void) {
    struct Level1 l1 = {0};
    struct AttributedStruct as = {0};
    union ComplexUnion cu;
    
    /* Use variables to prevent optimization */
    multi_dim_array[0][0][0] = 1;
    initialized_var = points[0].x;
    complex_ptr = NULL;
    
    /* Reference macros */
    int macro_test = NESTED_PARENS + NESTED_BRACKETS[0] + COMPLEX_MACRO;
    
    (void)l1;
    (void)as;
    (void)cu;
    (void)macro_test;
    
    return 0;
}

/* Requirement 1: Additional deeply nested type at file scope */
enum ErrorCodes {
    OK = 0,
    ERR_PARSE = ( ( (1 << 0) ) ),
    ERR_TYPE = { ( (2) ) }[0],  /* Compound literal in enum (GCC extension) */
    ERR_MEM = [(3)]  /* Another GCC extension */
};

/* Final complex type definition wrapping everything */
typedef struct UltimateType {
    struct Level1 base;
    struct AttributedStruct attr;
    union ComplexUnion data;
    int (*operations[(sizeof(struct Level1)+7)/8])(struct UltimateType*,
                                                   int[][(16*16)],
                                                   void (*)(int, ...));
} UltimateType;
