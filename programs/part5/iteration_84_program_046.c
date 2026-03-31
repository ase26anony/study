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
struct DeeplyNested {
    /* Function pointer with nested parentheses and struct */
    int (*callback)(int (*helper)(int[2][3]), 
                    struct { 
                        int a; 
                        union { 
                            char c; 
                            struct { 
                                double d; 
                            } inner; 
                        } u; 
                    });
    
    /* Array of function pointers */
    void (*funcs[3])(int, ...);
};

/* Requirement 4: Array with complex dimensions */
int multi_array[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; char y[( { 1 + 2 } )];})/4)];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(sizeof(int))));
    char buffer[32] __attribute__((aligned(8)));
};

/* Union with nested anonymous struct */
union ComplexUnion {
    struct {
        int (*compute)(int matrix[][( { 2 * 2 } )], 
                       struct { int x; int y; } point);
    } ops;
    long value;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int complex_init = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[ { 3 } ];
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2}, .z = { { (3) }, {4}, {5} } },
    [1] = { .x = { { (6) } }, .y = 7, .z = { 8, 9, 10 } }
};

/* Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[][ { 2 } ]), 
                              union { 
                                  int i; 
                                  struct { 
                                      char c; 
                                  } s; 
                              });

/* Requirement 2: Using the macro in a declaration */
ARRAY_MACRO;

/* Enum with complex initializers */
enum States {
    IDLE = ( { 0 } ),
    RUNNING = ( { (1 << 0) | (1 << 1) } ),
    ERROR = ( { [0] = 2, [1] = 3 } [0] )
};

/* Function-like macro usage in type definition */
#define DECLARE_CALLBACK(name, type) type (*name)(type (*)(type[2]), struct { type t; })

DECLARE_CALLBACK(my_callback, int);

/* Nested struct with all delimiter types mixed */
struct UltimateNest {
    int (*level1)(int (*level2[ { 2 } ])(int (*level3)(int level4[][( { 5 } )]), 
                                         struct { 
                                             int a[ { { 3 } } ]; 
                                         }), 
                  union { 
                      char c; 
                      struct { 
                          double d; 
                      } s; 
                  });
    
    /* Array with nested designators */
    int arr[2][3] = { [0][0] = { { (1) } }, [1] = { 2, 3, { 4 } } };
};

/* Requirement 6: Conditional type definitions */
#if defined(USE_EXTRA) && (defined(TEST) || 1)
struct ConditionalStruct {
    int field __attribute__((aligned(( { 16 } ))));
    char data[( { sizeof(struct { int x; }) } )];
};
#elif defined(ALTERNATE)
union ConditionalUnion {
    struct { int i; } s;
    char c[( { 2 + 3 } )];
};
#else
/* Default empty */
#endif

/* C++11 style attributes (if compiled as C++) */
#ifdef __cplusplus
[[deprecated("Use NewType instead")]]
#endif
struct OldType {
    int value [[gnu::aligned(16)]];
};

/* Variable with complex declarator */
int (*(*complex_var)[( { 3 } )])(int, ...) = { NULL, NULL, NULL };

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    static struct DeeplyNested dn = {0};
    static struct AttributedStruct as = {0};
    
    /* Use macro expansion */
    int x = COMPLEX_PARENS(42);
    
    /* Reference conditional type */
#ifdef USE_EXTRA
    static struct ConditionalStruct cs;
    (void)cs;
#endif
    
    /* Reference array */
    (void)multi_array[0][0];
    (void)ptr_array[0];
    
    /* Reference initialized data */
    (void)complex_init;
    (void)points[0].x;
    
    /* Reference enum */
    enum States s = IDLE;
    
    return s;
}

/* Final macro expansion with all delimiters */
#define ULTIMATE_NEST { { ( [ { ( { [ ] } ) } ] ) } }
int final_array[] = ULTIMATE_NEST;
