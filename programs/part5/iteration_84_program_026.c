/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Requirement 1: Balanced token sequences in type definitions */
struct FunctionPointerStruct {
    /* Nested function pointer with complex parameter list */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; double b; });
    
    /* Even more nesting */
    void (*fp2)(char *(*(*nested_fp)(int))[5], 
                union { 
                    long l; 
                    struct { 
                        short s; 
                        int (*fp)(int, ...); 
                    } inner; 
                });
};

/* Union with deeply nested types */
union NestedUnion {
    struct {
        int (*arr_ptrs[3])(int (*)(int[][4]));
        char *(*complex_ptr)(struct { int x; int y; } (*)(void));
    } s;
    long long ll;
};
#endif /* TEST_COMPLEX */

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2+3)*4][5];
int *pointer_array[sizeof(struct {int x; char y; double z;})/sizeof(int)];

/* Array with nested parentheses in dimension */
int complex_dim_array[((sizeof(int) > 2) ? 10 : 20)][3];

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data[4];
    char padding;
} __attribute__((deprecated("Use NewStruct instead")));

/* GCC attributes with nested parentheses */
int __attribute__((format(printf, 1, 2))) 
    (*printf_func)(const char *__restrict __format, ...);

/* Requirement 1 (more examples): Struct with nested anonymous struct */
struct Container {
    struct {
        int (*method)(struct { int x; int y; } point);
        union {
            int i;
            float f;
        } value;
    } anonymous;
    
    /* Array of function pointers */
    void (*callbacks[5])(int, ...);
};

/* Enum with complex expressions */
enum ComplexEnum {
    VALUE1 = (1 << 3),
    VALUE2 = sizeof(struct { char a; int b; }),
    VALUE3 = ((2 + 3) * 4)
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int nested_init = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z;
};

/* Complex initializer with designated initializers */
struct Point points[] = { 
    [0] = { .x = (1), .y = {2}, .z = 3 },
    [1] = { .x = ( (2+3) ), .y = { {4} }, .z = 5 },
    [2] = COMPLEX_MACRO  /* Using macro expansion */
};

/* Initializer using nested macros */
int macro_array[] = NESTED_BRACES;
int paren_array[] = NESTED_PARENS;

/* Typedef with complex type */
typedef int (*ComplexFuncPtr)(int (*)(int[][3]), 
                              struct { 
                                  int a; 
                                  struct { 
                                      char c; 
                                  } inner; 
                              });

/* Requirement 6: More conditional compilation */
#if defined(USE_ANONYMOUS_STRUCTS)
/* Anonymous struct in union */
union Data {
    struct {
        int type;
        void *data;
    };
    long long raw;
};
#endif

/* Struct with bitfields and attributes */
struct BitfieldStruct {
    unsigned int flag1:1 __attribute__((deprecated));
    unsigned int flag2:3;
    unsigned int :4;  /* unnamed bitfield */
    unsigned int flag3:8 __attribute__((packed));
} __attribute__((aligned(8)));

/* Function pointer array with nested attributes */
void (*__attribute__((const)) const_funcs[])() = { NULL, NULL };

/* Nested array of structs with function pointers */
struct Node {
    int value;
    struct Node *(*get_child)(int index);
    void (*traverse)(struct Node *(*visitor)(struct Node *));
} tree[10][10];

/* Requirement 4: Multi-dimensional array with computed size */
int computed_array[sizeof(struct { 
    int a[(2+3)*4]; 
    char b[sizeof(double)]; 
}) / sizeof(int)];

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    /* Reference variables to prevent optimization */
    (void)multi_dim_array[0][0];
    (void)points[0].x;
    (void)nested_init;
    
#ifdef TEST_COMPLEX
    struct FunctionPointerStruct fps = {0};
    (void)fps.fp1;
#endif
    
#if defined(USE_ANONYMOUS_STRUCTS)
    union Data d = {0};
    (void)d.raw;
#endif
    
    return 0;
}

/* Final conditional block with complex type */
#ifndef SKIP_FINAL
/* Ultra-nested type definition */
typedef struct {
    int (*(*nested_fp_array[2][2])(int (*)(int)))(char *(*)(void));
    union {
        struct {
            int x[((2+3)*4)];
        };
        long y;
    } u;
} UltimateType __attribute__((aligned(32), may_alias));
#endif
