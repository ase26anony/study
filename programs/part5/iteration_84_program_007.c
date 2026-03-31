/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING ([[{({})}]])
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Outer {
    /* Function pointer with nested parentheses in parameter */
    int (*callback1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*callback2)(int (*(*)(int (*)(int)))(int), 
                      struct { 
                          union { 
                              char c; 
                              int (*fp)(int (*arr)[(2+3)*4]); 
                          } u; 
                      });
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int multi_array1[(sizeof(struct Outer) + 7) * 2][5];
int *ptr_array[(sizeof(struct {int x; double y;})/sizeof(int))];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    char data[16];
    int (*func_ptr)(int, int);
} __attribute__((deprecated("Use NewStruct instead")));

/* C++11 style attribute (valid in C23/C2x with appropriate flags) */
#if __STDC_VERSION__ > 201710L
struct [[deprecated("Old struct"), gnu::packed]] NewStruct {
    int value;
};
#endif

/* Requirement 1: Union with nested type definitions */
union ComplexUnion {
    struct {
        int (*comparator)(const void *, const void *);
        int matrix[2][(3+4)*2];
    } s;
    struct {
        void (*cleanup)(struct { int status; } *);
    } u;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int initialized = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

/* Complex initializer with designated initializers */
struct Point points[] = {
    [0] = { .x = (1), .y = {2}, .z = { [0] = 1, [1] = (2+3), [2] = 4 } },
    [1] = { .x = { (3) }, .y = 5, .z = NESTED_BRACES },
    [2] = COMPLEX_MACRO
};

/* Array initialized with macro containing nested delimiters */
int arr_init[] = NESTED_BRACES;

/* Requirement 4: Multi-dimensional array with expression in dimension */
int complex_array[(2+3)*4][sizeof(struct Point)/sizeof(int)];

/* Requirement 6: Conditional type definitions */
#ifdef SPECIAL_FEATURE
enum __attribute__((packed)) BitFieldEnum {
    FLAG_A = 1 << 0,
    FLAG_B = 1 << 1,
    FLAG_C = 1 << 2
};

typedef int (*ComplexFuncPtr)(int (*)(int[][(2*3)]), 
                              union { 
                                  long l; 
                                  struct { short s; } s; 
                              });
#endif

/* Typedef with nested parentheses */
typedef void (*SignalHandler)(int, 
                              struct siginfo_t *, 
                              void (*)(struct { 
                                  int sig; 
                                  char ctx[16]; 
                              }));

/* Requirement 1: Struct containing all delimiter types nested */
struct UltimateTest {
    /* Parentheses in function pointer */
    int (*func1)(int (*)(int (*)(int)));
    
    /* Brackets in array of function pointers */
    int (*func_array[((2)*(3))])(int, int);
    
    /* Braces in anonymous struct */
    struct {
        int data[({ int x = 5; x; })];
    } anon;
    
    /* All mixed together */
    void (*mixed)(int[][{ .x = 5 }.x], 
                  struct { 
                      union { 
                          int i; 
                          char c[(((2)+3)*4)]; 
                      } u; 
                  });
};

/* Requirement 5: More complex initializers */
struct NestedInit {
    int a;
    struct {
        int b;
        int c[2];
    } inner;
} nested_var = { 
    .a = (1 + (2 * (3 + 4))), 
    .inner = { 
        .b = {5}, 
        .c = { [0] = 6, [1] = (7) } 
    } 
};

/* Function with attribute containing parentheses */
void __attribute__((constructor(101))) 
__attribute__((noinline)) init_func(void) {
    /* Reference variables to avoid dead code elimination */
    static int dummy = sizeof(struct UltimateTest) + 
                      sizeof(multi_array1) + 
                      sizeof(points);
}

/* Main function that references our types */
int main(void) {
    /* Use the types to avoid dead code elimination */
    struct Outer o;
    union ComplexUnion cu;
    struct UltimateTest ut;
    
    /* Reference them */
    (void)o;
    (void)cu;
    (void)ut;
    
    /* Reference initialized data */
    if (initialized > 0) {
        return points[0].x + arr_init[0];
    }
    
    return 0;
}

/* Final conditional block with deeply nested tokens */
#if defined(EXTRA) && (EXTRA == 1)
struct ExtraType {
    int (*fp)(int (*)(int (*[({ .x = 1 }.x)])), 
              struct { 
                  int a[[[{ .y = 2 }.y]]]; 
              });
};
#endif
