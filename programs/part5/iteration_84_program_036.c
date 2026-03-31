/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING [[deprecated("test")]]
#else
  #define EXTRA_NESTING
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Outer {
    /* Function pointer with nested parameter list */
    int (*callback1)(int (*inner_cb)(int[2][3]), struct { int a; });
    
    /* Another with more nesting */
    void (*callback2)(int (*(*nested)[5])(char (*(*)[10])[20]));
    
    /* Array of function pointers */
    double (*funcs[3])(int, ...);
};

/* Requirement 4: Array with complex dimensions */
int multi_array[(2+3)*4][5];
int *ptr_array[sizeof(struct {int x; char y[(10)];})/4];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) PackedStruct {
    char data[32];
    int __attribute__((deprecated)) old_field;
} EXTRA_NESTING;

/* Union with nested anonymous struct */
union Data {
    long value;
    struct {
        int tag;
        char payload[256];
    } EXTRA_NESTING;
};

/* Requirement 1: Enum with complex initializers */
enum States {
    IDLE = 0,
    ACTIVE = (1 << 0),
    PAUSED = (1 << 1),
    ERROR = (IDLE | ACTIVE)  /* Parenthesized expression */
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int initialized = { { ( 1 + (2) ) } };

struct Point {
    int x, y;
};

struct Point points[] = {
    [0] = { .x = (1), .y = {2} },
    [1] = { .x = {3}, .y = (4) },
    [2] = COMPLEX_MACRO  /* Macro expansion */
};

/* Nested struct initialization */
struct Container {
    struct {
        int a;
        struct {
            char b;
        } inner;
    } nested;
} container = { .nested = { .a = 5, .inner = { .b = 'x' } } };

/* Function pointer typedef with extreme nesting */
typedef int (*(*ComplexFunc)(int (*(*arg1)[10])(float), 
                             struct { 
                                 union { 
                                     int i; 
                                     char c[4]; 
                                 } u; 
                             }))[20];

/* Requirement 4: Variable Length Array in struct (C99) */
struct VLAHolder {
    int size;
    int data[];  /* Flexible array member */
};

/* Requirement 6: Conditional type definitions */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(int) == 4, "int must be 4 bytes");
#endif

#ifdef USE_ANONYMOUS
/* Anonymous struct/union */
struct {
    int id;
    union {
        float f;
        int i;
    };
} anonymous_var = { .id = 1, .i = 42 };
#endif

/* Requirement 3: Multiple attribute styles */
#ifdef __cplusplus
[[gnu::always_inline, gnu::const]] inline int compute() { return 0; }
#else
int compute(void) __attribute__((always_inline, const));
#endif

/* Main function that references our types to avoid dead code elimination */
int main(void) {
    static struct Outer outer_instance = {
        .callback1 = NULL,
        .callback2 = NULL,
        .funcs = { NULL, NULL, NULL }
    };
    
    /* Use array to prevent optimization */
    multi_array[0][0] = sizeof(struct Outer);
    ptr_array[0] = &multi_array[0][0];
    
    /* Reference initialized data */
    points[0].x = initialized;
    container.nested.inner.b = 'y';
    
    return compute();
}

/* Trailing content with more nesting */
int trailing_array[] = NESTED_BRACES;
int (*trailing_func)(void) = (int (*)(void))NESTED_PARENS;
