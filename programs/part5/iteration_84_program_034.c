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
    void (**fp2)(int (*(*)(int (**)(int))[5])(void));
};

/* Struct with deeply nested combinations */
struct S2 {
    /* Array of function pointers */
    int (*(*arr1[ (2+3)*4 ])(int))[5];
    
    /* Nested anonymous struct */
    struct {
        struct {
            int x;
        } inner;
    } nested;
};

/* Requirement 4: Array declarations with complex dimensions */
int arr1[(2+3)*4][5];
int *ptr1[(sizeof(struct {int x; int y[3];})/sizeof(int))];
int arr2[1 + (2 * (3 + 4))][NESTED_BRACKETS[0] ? 5 : 10];

/* Requirement 3: Attribute specifications */
struct S3 {
    int a;
    char b;
} __attribute__((aligned(16), packed, 
                 deprecated("Use S4 instead")));

struct S4 {
    long c;
    short d;
} __attribute__((aligned((8)*(2)), 
                 packed, 
                 warning("Experimental")));

/* C++11 style attributes (valid in C23/C2x with __has_c_attribute) */
#if __has_c_attribute(deprecated)
[[deprecated("Old struct")]]
#endif
struct S5 {
    int val;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };
int y[] = NESTED_BRACES;
int z[3] = { [0] = { (1), {2}, 3 }, [1] = COMPLEX_MACRO };

struct Point {
    int x;
    int y;
    int z[2];
};

struct Point pts[] = { 
    [0] = { .x = (1), .y = {2}, .z = { (3), 4 } },
    [1] = { .x = {5}, .y = (6), .z = NESTED_BRACES }
};

/* Union with nested initializer */
union U1 {
    char c;
    struct { 
        int i; 
        int j[2]; 
    } s;
};

union U1 u1 = { .s = { .i = (10), .j = { {20}, 30 } } };

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
/* Struct inside conditional block */
struct ConditionalStruct {
    int (*callback)(int (*)(int[][(2+3)]), 
                    union { 
                        long a; 
                        struct { 
                            short b[2]; 
                        } s; 
                    });
};

/* Array with computed size in conditional block */
int cond_arr[sizeof(struct { 
    char a; 
    int b[(1+(2*3))]; 
})];
#endif

#if defined(USE_NESTED)
/* Another conditional type */
typedef struct {
    int (**funcs[({ int x = 5; x; })])(
        void *(*)(int, char **),
        struct { 
            int tag; 
            union { 
                int i; 
                float f; 
            } value; 
        }
    );
} ComplexType;
#endif

/* Function with complex return type */
int (*(*complex_func(void))[3])(int, int) {
    static int (*arr[3])(int, int) = { NULL, NULL, NULL };
    return &arr;
}

/* Main function that references types to avoid dead code elimination */
int main(void) {
    struct S1 s1 = {0};
    struct S2 s2 = {0};
    struct S3 s3 = {0};
    struct S4 s4 = {0};
    
    /* Reference variables to prevent optimization */
    (void)s1;
    (void)s2;
    (void)s3;
    (void)s4;
    (void)arr1;
    (void)ptr1;
    (void)arr2;
    (void)x;
    (void)y;
    (void)z;
    (void)pts;
    (void)u1;
    
#ifdef TEST_COMPLEX
    struct ConditionalStruct cs = {0};
    (void)cs;
    (void)cond_arr;
#endif
    
#if defined(USE_NESTED)
    ComplexType ct = {0};
    (void)ct;
#endif
    
    /* Call complex function */
    int (*(*fp)[3])(int, int) = complex_func();
    (void)fp;
    
    return 0;
}

/* Additional edge cases */

/* Typedef with nested parentheses */
typedef int (*FuncPtr)(int (*)(int), void (*[2])(void));

/* Enum with last comma (C99/C11 feature) */
enum E {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1),
    VALUE3 = (sizeof(struct { char a[(2+3)]; })),
};

/* Bitfield with parenthesized size */
struct BitfieldStruct {
    unsigned int a : (1 + 2);
    unsigned int b : (sizeof(short) * 8);
};

/* Forward declaration in parentheses */
struct Forward;
int (*global_fp)(struct Forward *, int (*(*)(void))[5]);

/* Designated initializers with nested designators */
struct NestedDesignator {
    struct {
        int a[3];
        struct {
            int b;
            int c;
        } inner;
    } outer;
};

struct NestedDesignator nd = {
    .outer = {
        .a = { [0] = 1, [1] = (2), [2] = 3 },
        .inner = {
            .b = (4),
            .c = {5}
        }
    }
};

/* Static assertion with parentheses (C11) */
_Static_assert((sizeof(int) == 4), "int must be 4 bytes");
_Static_assert(((1 + 2) * 3) == 9, "Math should work");

/* Inline assembly with braces (GCC extension) */
void asm_example(void) {
    int a = 0;
    __asm__ volatile (
        "mov %[val], %0"
        : "=r" (a)
        : [val] "i" ((1 + 2))
        : "memory"
    );
    (void)a;
}
