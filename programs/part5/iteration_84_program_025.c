/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct S1 {
    int (*fp1)(int (*)(int[2][3]), struct { int a; });
    void (*fp2)(int (*(*)(int (*)[(2+3)*4]))[5]);
};

/* Union with nested anonymous struct */
union U1 {
    char c;
    struct {
        int i;
        double (*fn)(struct { int x; int y; }[2]);
    } s;
};

/* Typedef with complex array of function pointers */
typedef int (*(*complex_type)[(sizeof(int)+3)/2])(int, ...);

/* Requirement 3: Attribute specifications with multiple parentheses */

/* GCC attributes */
struct S2 {
    int data;
    char buffer[64];
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* Struct with multiple attributes */
struct S3 {
    int x __attribute__((aligned((2+2)*4)));
    int y;
} __attribute__((packed));

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with parenthesized size expressions */
int arr1[(2+3)*4][5];
int arr2[sizeof(struct {int x; int y;})/sizeof(int)][3];

/* Array of pointers with nested type in size expression */
int *ptr_array[sizeof(struct { 
    char c; 
    double d; 
    int (*fn)(int, int); 
}) / sizeof(void*)];

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Complex initializers */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point pts[] = { 
    [0] = { .x = (1 + (2 * (3))), .y = {2}, .z = {1, {2}, 3} },
    [1] = { .x = 4, .y = 5, .z = NESTED_BRACES }
};

/* Nested initializer with macros */
int init_array[] = COMPLEX_MACRO;

/* Requirement 6: Conditional compilation blocks with balanced tokens */

#ifdef TEST_COMPLEX
/* This struct will only be parsed if TEST_COMPLEX is defined */
struct ConditionalStruct {
    int (*conditional_fp)(int (*)(int[][(2+3)*2]), 
                         union { 
                             long l; 
                             struct { short s; } inner; 
                         });
    char data[sizeof(struct { int a; double b; })];
};
#endif

#if defined(USE_NESTED) || 1
/* Always included due to || 1 */
enum E {
    VAL1 = (1 + (2 * (3 + (4)))),
    VAL2 = sizeof(int[(2+2)][3]),
    VAL3
};
#endif

#ifndef SKIP_THIS
/* Struct with all three delimiter types mixed */
struct MixedDelimiters {
    int (*func_array[2])(int, 
                        struct { 
                            int a[(1+2)*3]; 
                            union { 
                                char c; 
                                int i; 
                            } u; 
                        });
    void (*callback)(int (*)(int[][(sizeof(int)+1)]), 
                    char *ptr[({ 
                        struct { int x; } s; 
                        sizeof(s); 
                    })]);
};
#endif

/* More complex examples to ensure thorough coverage */

/* Function pointer returning pointer to array */
typedef int (*(*func_ptr_ret_array)(int))[10];

/* Struct containing anonymous union with bitfield */
struct WithAnonymous {
    int tag;
    union {
        struct {
            int x : (1+2);
            int y : 4;
        } bits;
        double d;
        void (*fn)(int (*)(int[2]), ...);
    } data;
};

/* Nested struct with attributes */
struct Outer {
    struct Inner {
        int a __attribute__((aligned((8))));
        int b;
    } inner __attribute__((packed));
    
    /* Array with computed size containing function pointer */
    void (*actions[({ 
        struct Temp { int x; char c; } t; 
        sizeof(t) / sizeof(void*); 
    })])();
};

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct Point p = pts[0];
    
    /* Use the variables to prevent optimization */
    if (x > 0) {
        return arr1[0][0] + p.x;
    }
    
    return 0;
}

/* Final complex type definition to ensure parser handles EOF correctly */
struct Final {
    int (*last_fp)(
        int, 
        struct { 
            int a[({ 
                int x = 1; 
                x + 2; 
            })]; 
        }, 
        ...
    ) __attribute__((noreturn));
};
