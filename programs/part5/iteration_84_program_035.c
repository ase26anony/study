/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (x + 1)))
#define ARRAY_DECL int arr[] = NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING (struct { int (*f)(int, int); } *)
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nesting */
    void (*fp2)(int (*(*)(int[][4]))[5], 
                union { 
                    char c; 
                    struct { 
                        int (*callback)(int, int (*)(void)); 
                    } s; 
                });
};

/* Nested struct/union definitions */
union U1 {
    struct {
        int (*compare)(const void *, const void *);
        int matrix[3][(2+1)*2];
    } data;
    struct {
        int (*allocator)(size_t, size_t (*)(size_t));
    } mem;
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_array[(sizeof(struct {int x; double y;})/8)][(2+3)*4];
int *ptr_array[(sizeof(union {char a[5]; int b;})/sizeof(int))];

/* Requirement 3: Attribute specifications */
struct S2 {
    int value;
    char data[16];
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* GCC-style attributes with parentheses */
int global_var __attribute__((aligned(32), 
                              section(".data"),
                              used)) = 42;

/* Requirement 5: Initializer lists with nested braces/parentheses */
struct Point {
    int x;
    int y;
    int z;
};

struct Point points[] = {
    [0] = { .x = (1 + (2 * 3)), .y = {2}, .z = 0 },
    [1] = { .x = { { ( 1 + (2) ) } }, .y = 4, .z = 5 },
    [2] = { .x = COMPLEX_PARENS(3), .y = 6, .z = 7 }
};

/* Complex initializer using macro */
ARRAY_DECL;

/* More complex type definitions */
typedef int (*ComplexFuncPtr)(
    int, 
    struct { 
        int a; 
        int b[2][(sizeof(int)*2)]; 
    }, 
    void (*)(int, int)
);

/* Nested anonymous structs/unions */
struct Outer {
    struct {
        struct {
            int depth;
        } inner;
    } middle;
    
    union {
        int i;
        struct {
            char c;
            int arr[3][4];
        } s;
    } u;
};

/* Function pointer returning array pointer */
int (*(*signal(int sig, int (*func)(int)))(int))[5];

/* Requirement 6: Conditional blocks */
#ifdef TEST_COMPLEX
struct ConditionalStruct {
    EXTRA_NESTING func_ptr;
    int data[((2*3)+4)];
};
#endif

#if defined(USE_ALT_DEF)
enum E {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (1 << 2)
};
#elif defined(USE_OTHER)
union AltUnion {
    struct { int x; } s;
    char data[8];
};
#else
/* Default: empty */
#endif

/* Requirement 1: More nesting examples */
typedef struct Node {
    struct Node *children[((sizeof(void*)/4)+1)];
    void (*operations[3])(
        struct Node *, 
        int (*)(const char *, ...),
        struct { int count; } *
    );
} Node;

/* Initializer with deeply nested braces */
int deep_init[2][3] = {
    { {1}, {2}, { {3}, {4}, {5} } },
    { {6}, {7}, {8} }
};

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1;
    union U1 u1;
    struct Point *p = points;
    Node n;
    
    /* Reference variables to prevent optimization */
    (void)s1;
    (void)u1;
    (void)p;
    (void)n;
    (void)multi_array;
    (void)ptr_array;
    (void)global_var;
    (void)arr;
    
    return 0;
}
