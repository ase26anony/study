/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING [[deprecated("test")]]
#else
  #define EXTRA_NESTING
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer with nested parentheses in parameters */
    int (*callback1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*callback2)(int (*(*)(int (*)[(2+3)*4]))(void), 
                      union { 
                          char c; 
                          struct { 
                              int i; 
                              double d; 
                          } s; 
                      });
};

/* Requirement 4: Array with complex dimensions */
int multi_array[(sizeof(struct DeepNested) + 7)/8][(2+3)*4];
int *ptr_array[sizeof(struct {int x; char y[(5+2)*3];})/4];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(sizeof(int))));
    char buffer[64] __attribute__((aligned(32)));
};

/* Requirement 1 & 3: Union with attributes and nested types */
union __attribute__((packed)) ComplexUnion {
    struct {
        int (*func_ptr)(int, ...);
        float matrix[3][3];
    } s;
    struct __attribute__((aligned(8))) {
        char *name;
        int id;
    } metadata;
};

/* Requirement 5: Initializers with nested braces/parentheses */
int complex_init = { { ( 1 + (2) ) } };
int array_init[3] = { [0] = { (5) }, [1] = { {6} }, [2] = 7 };

struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2}, .z = 3 },
    [1] = { .x = {4}, .y = (5), .z = {6} },
    [2] = { (7), {8}, 9 }
};

/* Requirement 2 & 5: Using macros in initializers */
int macro_init[] = NESTED_BRACES;
int paren_init = NESTED_PARENS;

/* Requirement 4 & 6: Conditional array declaration */
#ifdef TEST_COMPLEX
int conditional_array[sizeof(union ComplexUnion)][3] = { 
    {1, 2, 3}, 
    {4, 5, 6} 
};
#endif

/* Requirement 1: Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(int (*)(int[][3]), 
                              struct { 
                                  int a; 
                                  struct { 
                                      char c; 
                                  } inner; 
                              });

/* Requirement 3: C++11 style attribute (valid in C23/GCC) */
#if defined(__cplusplus) || __STDC_VERSION__ >= 202311L
[[deprecated("Use NewType instead")]]
#endif
typedef struct OldType {
    int value;
} OldType;

/* Requirement 1: Enum with complex expressions */
enum E {
    A = (1 << 0),
    B = (1 << 1),
    C = (1 << 2) | (1 << 3),
    D = sizeof(struct { int x; char y[((2+2)*2)]; })
};

/* Requirement 4: Variable with nested array dimensions */
char buffer[sizeof(struct AttributedStruct)][(A + B) * C];

/* Requirement 5: Struct with nested initializer */
struct NestedInit {
    struct {
        int a;
        int b;
    } inner;
    int arr[2];
} nested_var = { 
    .inner = { .a = (1+2), .b = {3} }, 
    .arr = { [0] = 4, [1] = 5 } 
};

/* Requirement 2: Macro used in type definition */
struct MacroStruct {
    int data COMPLEX_MACRO;
};

/* Requirement 6: More conditional compilation */
#if 1
struct AlwaysPresent {
    int (*func)(int (*callback)(int, int), 
                void *data);
};
#elif 0
struct NeverPresent {
    /* This won't be compiled but tests parser */
    int impossible[({ int x = 5; x; })];
};
#endif

/* Main function to avoid dead code elimination */
int main(void) {
    struct DeepNested dn = {0};
    struct AttributedStruct as = {0};
    union ComplexUnion cu = {0};
    
    /* Reference variables to prevent optimization */
    (void)dn;
    (void)as;
    (void)cu;
    (void)multi_array;
    (void)ptr_array;
    (void)complex_init;
    (void)array_init;
    (void)points;
    (void)macro_init;
    (void)paren_init;
    (void)nested_var;
    
#ifdef TEST_COMPLEX
    (void)conditional_array;
#endif
    
    return 0;
}
