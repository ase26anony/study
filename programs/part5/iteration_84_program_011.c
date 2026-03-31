/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */
struct S1 {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    
    /* Even more nested */
    void (*fp2)(int (*(*)(int (*)(int)))(int), 
                struct { 
                    union { 
                        char c; 
                        int i; 
                    } u; 
                });
};

/* Nested struct/union definitions */
union U1 {
    struct {
        int (*arr_ptr)[(2+3)*4];
        char *(*func_tab[5])(int, float);
    } s;
    long long ll;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int arr2[sizeof(struct {int x; char y[10];})/4];
int *ptr_arr[ (sizeof(int*) > 4) ? 8 : 16 ];

/* Struct with array member using macro */
struct ArrayStruct {
    int data NESTED_BRACKETS;
};

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned(16), packed)) AttrStruct {
    int x __attribute__((aligned(8)));
    char y;
} __attribute__((deprecated));

/* GCC-style attributes with parentheses */
int var_attr __attribute__((aligned(32), 
                            section(".data"))) = 0;

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
int matrix[2][2] = { { {1}, {2} }, { {3}, {4} } };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = { {3}, {4} } },
    [1] = { .x = 5, .y = 6, .z = COMPLEX_MACRO }
};

/* Complex initializer with all delimiters */
struct {
    int a;
    int b[3];
    struct {
        int c;
    } inner;
} complex_var = { 
    .a = (1 + (2 * (3))),
    .b = { [0] = {1}, [1] = {2}, [2] = {3} },
    .inner = { .c = ({ int temp = 5; temp; }) }
};

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
    /* Type with deeply nested parentheses */
    typedef int (*complex_func_t)(int (*)(int (*[2])(int)), 
                                  struct { 
                                      union { 
                                          int i; 
                                          void *p; 
                                      } u; 
                                  });
    
    complex_func_t func_array[] = { NULL, NULL };
#endif

#if defined(USE_NESTED) || 1
    /* Struct with all three delimiter types mixed */
    struct MixedDelimiters {
        int (*fp)(int a[2][3], struct { int b; } s);
        int arr[ (2 + (3 * 4)) ];
        union {
            struct {
                int x;
            } s;
            long l;
        } u;
    };
#endif

#ifndef SKIP_THIS
    /* Enum with complex initializers */
    enum E {
        VAL1 = (1 << 0),
        VAL2 = (1 << 1),
        VAL3 = sizeof(struct { char a; int b; })
    };
#endif

/* More complex cases */
typedef struct Node {
    struct Node *(*get_next)(struct Node *n);
    void (*process)(int (*callback)(int, int), 
                    void *data);
    int value[ (sizeof(void*) + 3) & ~3 ];
} Node_t;

/* Function pointer returning array pointer */
int (*(*complex_return)(void))[10];

/* Nested type in parameter */
void dummy_func(int (*param)(struct { int x; int y; })) {
    /* Empty body - just for declaration */
}

/* Main function to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    union U1 u1 = {0};
    struct MixedDelimiters md = {0};
    Node_t node = {0};
    
    /* Reference variables to prevent optimization */
    (void)s1;
    (void)u1;
    (void)md;
    (void)node;
    (void)arr1;
    (void)arr2;
    (void)ptr_arr;
    (void)x;
    (void)pts;
    (void)complex_var;
    
    return 0;
}

/* Final complex type definition */
struct UltimateType {
    /* All three delimiters deeply nested */
    int (*(*ultimate_fp[2])(
        int a, 
        struct { 
            int (*nested[3])(int b[2][2]); 
        } s
    ))(int c[ (1 + (2 * (3))) ]);
    
    /* Array with nested everything */
    struct {
        union {
            int i;
            void *p;
        } u NESTED_BRACKETS;
    } data[ ({ int x = 5; x; }) ];
} __attribute__((packed, aligned(64)));
