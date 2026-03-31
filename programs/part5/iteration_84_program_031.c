/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACES { { { { } } } }
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{({})}]])
#endif

/* Requirement 1: Type definitions with deeply nested delimiters */
struct S1 {
    /* Function pointer with nested parentheses in parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*fp2)(int (*(*nested)(int[((2+3)*4)][5]))(void), 
                union { 
                    char c; 
                    struct { 
                        int i; 
                        double d; 
                    } s; 
                } u);
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y[3];})/sizeof(int))];

/* Struct with array member using macro-expanded initializer */
struct ArrayHolder {
    int data[3];
};

/* Requirement 3: GCC attributes with multiple parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    char a;
    int b __attribute__((aligned(8)));
    double c;
} __attribute__((deprecated("Use NewStruct instead")));

/* Another struct with complex nested attributes */
struct [[deprecated("Complex nested")]] ComplexStruct {
    int x;
} __attribute__((aligned((16 > 8) ? 16 : 8)));

/* Requirement 1 & 5: Union with nested types and initializer */
union U {
    char c;
    struct {
        int i;
        float f;
    } s;
    long l;
} u1 = { .s = { .i = (1 + (2 * (3))), .f = {0.5f} } };

/* Requirement 5: Initializer lists with nested braces and parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2, {3}} },
    [1] = { .x = {4}, .y = {5, 6} },
    [2] = NESTED_BRACES,  /* Using macro */
};

/* Requirement 4: Multi-dimensional array with parenthesized size */
int matrix[ (sizeof(int*) > 4) ? 10 : 20 ][ ((1+2)*3) ];

/* Requirement 1: Typedef with extreme nesting */
typedef int (*ComplexFuncPtr)(
    int (*)(int[][(2+3)], struct { int a; int b; }), 
    union { 
        char c; 
        struct { 
            int i; 
        } s; 
    }
);

/* Requirement 2 & 5: Using macros in initializers */
int nested_init[] = NESTED_BRACES;
int complex_init = { NESTED_PARENS };

/* Requirement 6: Conditional compilation blocks */
#ifdef TEST_NESTING
struct ConditionalStruct {
    int (*cond_fp)(int (*(*)[{2}])());
    char cond_arr[({ int x = 5; x; })];
};
#elif defined(ALTERNATE)
union ConditionalUnion { 
    char c; 
    struct { 
        int i; 
    } s; 
};
#else
/* Default when neither is defined */
enum ConditionalEnum {
    VALUE1 = (1 << (sizeof(int)*8 - 1)),
    VALUE2 = [{2}],  /* Invalid but tests bracket consumption */
};
#endif

/* Requirement 1: Even more complex nested structure */
struct Outer {
    struct {
        int (*inner_fp[2])(int (*)(int[][3]), 
                          struct { 
                              int a; 
                              struct { 
                                  char c; 
                              } nested; 
                          });
    } inner;
    
    /* Array of function pointers */
    void (*func_array[((2+2)*2)])(int, ...);
};

/* Requirement 3 & 4: Variable with attribute and complex array */
int __attribute__((aligned((16)), 
                   vector_size(16))) vector[4] 
                   = { [0] = 1, [3] = {4} };

/* Requirement 5: Nested designated initializers */
struct NestedInit {
    struct {
        int a;
        int b[2];
    } inner;
    int c;
} ni = { 
    .inner = { 
        .a = (1 + (2 * (3 + (4)))), 
        .b = { {5}, {6} } 
    }, 
    .c = {7} 
};

/* Requirement 2: Macro used in type definition */
#define FUNCTION_PTR_TYPE int(*)(int, float)
typedef FUNCTION_PTR_TYPE func_ptr_t;

/* Final complex type combining everything */
typedef struct UltimateType {
    int (*ultimate_fp)(
        int (*(*nested_array[({2})])(
            struct { 
                int x; 
                int y[((1+2)*3)]; 
            }
        ))[],  /* Flexible array member pointer */
        ...
    );
    
    /* Anonymous union with bitfield */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : ({ sizeof(int)*8 - 1 });
        };
        unsigned int flags;
    };
} __attribute__((packed, aligned(({8})))) UltimateType;

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1;
    struct PackedStruct ps;
    struct Point *ppt = pts;
    ComplexFuncPtr cfp = NULL;
    struct Outer o;
    UltimateType ut;
    
    /* Reference variables to prevent optimization */
    (void)s1;
    (void)ps;
    (void)ppt;
    (void)cfp;
    (void)o;
    (void)ut;
    (void)x;
    (void)arr1;
    (void)ptr1;
    (void)u1;
    (void)matrix;
    (void)nested_init;
    (void)complex_init;
    (void)vector;
    (void)ni;
    
    return 0;
}

/* Requirement 6: More conditional compilation at end */
#if 0
/* This block won't be compiled but will be parsed */
struct DisabledStruct {
    int (*disabled_fp)(int (*(*)[{{{}}}])());
    char disabled_arr[((({1}))])];
};
#endif

/* Final check: Ensure all braces are balanced */
/* {{([{()}])}} - This comment tests token scanning in comments too */
