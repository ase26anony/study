/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (sizeof(struct { char c; }) * 2))
#define ARRAY_DECL int arr NESTED_BRACES

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define EXTRA_NESTING [[deprecated("test")]]
#else
  #define EXTRA_NESTING
#endif

/* Requirement 1: Struct with deeply nested parentheses in function pointer */
struct S1 {
    /* Function pointer with nested parameter containing array and struct */
    int (*fp1)(int (*callback)(int[2][3]), struct { int a; });
    
    /* Another with multiple nesting levels */
    void (*fp2)(int (*(*nested)[5])(char (*(*)[3])[2]), 
                struct { 
                    union { 
                        int x; 
                        double y; 
                    } u; 
                });
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int multi_array[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; char pad[7];})/4)];

/* Requirement 3: GCC attributes with multiple parentheses */
struct S2 {
    int data;
    char buffer[64];
} __attribute__((aligned(16), packed, 
                 deprecated("Use S3 instead")));

/* C++11 style attribute (valid in C23/C++11) */
#ifdef __cplusplus
[[deprecated("Complex type")]]
#endif
struct S3 {
    /* Nested anonymous struct with attribute */
    struct {
        int a __attribute__((aligned(8)));
        int b;
    } inner;
    
    /* Array with attribute */
    int values[10] __attribute__((aligned(32)));
};

/* Requirement 1 & 4: Union with array of function pointers */
union U1 {
    int (*funcs[3])(int (*(*)[2])[3], 
                    struct { 
                        short s; 
                        long l; 
                    });
    struct {
        int tag;
        union {
            float f;
            double d;
        } value;
    } variant;
};

/* Requirement 5: Initializer lists with nested braces/parentheses */
int x = { { ( 1 + (2) ) } };

struct Point {
    int x;
    struct {
        int y;
        int z;
    } coord;
};

struct Point pts[] = { 
    [0] = { 
        .x = (1 + (2 * 3)), 
        .coord = { .y = {2}, .z = 3 } 
    },
    [1] = { 
        .x = 4, 
        .coord = { .y = 5, .z = 6 } 
    }
};

/* Requirement 2 & 5: Using macro in initializer */
int init_with_macro[] = NESTED_BRACES;

/* Requirement 4: Typedef with complex array type */
typedef int (*(*complex_arr_t)[(sizeof(int*) + 7) & ~7])
             [2][3];

/* Requirement 1: Enum with computed values in parentheses */
enum E {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (sizeof(struct { char a; int b; })),
    VAL4 = ((int)((long)(1.5 * 100.0)))
};

/* Requirement 6: Conditional type definitions */
#if defined(TEST_NESTING) && (__STDC_VERSION__ >= 201112L)
    _Static_assert(sizeof(int) == 4, "int must be 4 bytes");
    
    struct ConditionalStruct {
        int (*method)(int a[_Generic(a, default: 2)]);
        int data[(_Alignof(long double) > 8 ? 16 : 8)];
    };
#elif defined(ANOTHER_TEST)
    union ConditionalUnion {
        char c; 
        struct { 
            int i; 
            int j[(2+3)*4]; 
        } s;
    };
#endif

/* Requirement 3 & 1: Variable with multiple attributes */
int global_var 
    __attribute__((aligned(16), 
                   deprecated("old var"), 
                   warning("use carefully")))
    = { { ( (5) + (3) ) } };

/* Function with complex return type and attributes */
#ifdef __GNUC__
__attribute__((noinline, 
               hot, 
               constructor(101)))
#endif
static struct S1* create_s1(void) {
    static struct S1 instance = {
        .fp1 = NULL,
        .fp2 = NULL
    };
    return &instance;
}

/* Main function that references types to avoid dead code elimination */
int main(void) {
    struct S1 *s1 = create_s1();
    (void)s1;
    
    struct S2 s2 = {0};
    (void)s2;
    
    struct S3 s3 = {{0}};
    (void)s3;
    
    union U1 u1 = {0};
    (void)u1;
    
    /* Use array to prevent optimization */
    int sum = x + pts[0].x + init_with_macro[0];
    
    /* Reference conditional types */
#if defined(TEST_NESTING)
    struct ConditionalStruct cs = {0};
    (void)cs;
#elif defined(ANOTHER_TEST)
    union ConditionalUnion cu = {0};
    (void)cu;
#endif
    
    return sum > 0 ? 0 : 1;
}

/* Requirement 1: Additional deeply nested type at file scope */
struct Outer {
    struct Middle {
        struct Inner {
            int (*deep_func)(int (*(*arr[3])[4])[5],
                            union {
                                struct A { int x; } a;
                                struct B { long y[(2+3)]; } b;
                            });
        } inner;
        
        /* Nested array with parenthesized size */
        char buffer[(sizeof(struct Inner) + 15) & ~15];
    } mid;
    
    /* Attribute on bitfield */
    unsigned int flags:4 __attribute__((packed));
};

/* Requirement 4: Complex array declaration with nested type in size */
extern int extern_array[sizeof(struct {
    int a;
    double b;
    struct {
        short s[3];
        char c;
    } nested;
}) / sizeof(int)];

/* Requirement 5: Designated initializer with nesting */
struct Outer outer_instance = {
    .mid = {
        .inner = { .deep_func = NULL },
        .buffer = { [0] = 1, [1] = 2 }
    },
    .flags = 3
};
