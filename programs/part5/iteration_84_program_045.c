/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (3 + (4))))
#define ARRAY_MACRO int arr_macro[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define ENABLE_COMPLEX 1
#else
#define ENABLE_COMPLEX 0
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Outer {
    /* Function pointer with nested parentheses in parameter list */
    int (*complex_fp)(int (*callback)(int[2][3]), 
                      struct { 
                          int a; 
                          struct { 
                              char c; 
                          } inner; 
                      });
    
    /* Nested array within struct */
    int matrix[(2+3)*4][5];
};

/* Requirement 4: Array with complex dimension containing sizeof */
int *pointer_array[(sizeof(struct {int x; double y;}) / sizeof(int))];

/* Requirement 3: GCC attributes with multiple parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    char data[16];
    int value __attribute__((aligned((8))));
};

/* Union with nested braces */
union ComplexUnion {
    struct {
        int x;
        int y;
    } point;
    struct {
        struct {
            int depth;
        } nested;
    } container;
};

/* Requirement 5: Initializer with deeply nested braces/parentheses */
int complex_init = { { ( 1 + (2 + (3 * (4)))) } };

/* Array initializer with designators */
struct Point {
    int x;
    int y;
    int z[2];
};

struct Point points[] = {
    [0] = { .x = (1), .y = {2}, .z = { {3}, {4} } },
    [1] = { .x = 5, .y = 6, .z = {7, 8} }
};

/* Function pointer typedef with extreme nesting */
typedef void (*ExtremeFuncPtr)(
    int (*first)(char (*second)[5], 
                 struct { 
                     int a; 
                     int b[(2+3)*4]; 
                 }),
    union {
        int x;
        double y;
    } param2
);

/* Requirement 6: Conditional compilation block */
#if ENABLE_COMPLEX
struct ConditionalStruct {
    int (*cond_fp)(int, 
                   struct { 
                       char c; 
                       int i[({ 
                           int x = 5; 
                           x; 
                       })]; 
                   });
};
#endif

/* Another macro usage */
ARRAY_MACRO;

/* Enum with complex initializer expressions */
enum ComplexEnum {
    VALUE1 = (1 << (2 + (3))),
    VALUE2 = sizeof(struct { char a; int b; }),
    VALUE3 = ({ int x = 5; x * 2; }) /* GCC statement expression */
};

/* Nested anonymous struct/union */
struct AnonymousContainer {
    union {
        struct {
            int a;
            int b;
        };
        struct {
            long c;
            long d;
        } named;
    };
    int (*func_array[2])(int, 
                        struct { 
                            int x; 
                            int y[3]; 
                        });
};

/* Variable with attribute containing parentheses */
int aligned_var __attribute__((aligned((16)))) = 0;

/* Complex typedef with everything combined */
typedef struct {
    int (*method)(int a[(2+3)],
                  struct {
                      union {
                          int x;
                          long y;
                      } u;
                  } param);
    int array[({ 
        /* GCC statement expression in array size */
        int size = 10; 
        while (size > 5) size--; 
        size; 
    })];
} UltimateType;

/* Main function to avoid dead code elimination */
int main(void) {
    struct Outer o;
    struct PackedStruct ps;
    union ComplexUnion cu;
    UltimateType ut;
    
    /* Reference variables to prevent optimization */
    (void)o;
    (void)ps;
    (void)cu;
    (void)ut;
    (void)complex_init;
    (void)points;
    (void)pointer_array;
    (void)aligned_var;
    (void)arr_macro;
    
    return 0;
}

/* Final conditional block with nested tokens */
#ifdef EXTRA_TEST
int extra_array[][3] = {
    {1, 2, 3},
    {4, 5, 6},
    {7, 8, 9}
};

typedef int (*ExtraFuncPtr)(
    int,
    int[][ (2 + (3 * (4))) ],
    struct {
        int a;
        struct {
            int b;
            int c;
        } inner[2];
    }
);
#endif
