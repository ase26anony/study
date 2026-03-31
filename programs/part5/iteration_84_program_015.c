/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((x) + (2 * (3 + (4))))
#define ARRAY_MACRO [ (sizeof(int[2])) ][3]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING (struct { int (*fp)(int, int); })
#else
#define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct Outer {
    /* Function pointer with deeply nested parameter list */
    int (*complex_fp)(int (*callback)(int[2][3], 
                     struct { 
                         int a; 
                         struct Inner { 
                             char c; 
                             union { 
                                 int i; 
                                 float f; 
                             } u; 
                         } s; 
                     }), 
                     void *ptr);
    
    /* Nested struct with function pointer containing parentheses */
    struct {
        void (*nested_fp)(int (*(*)(int))[5], 
                         struct Tag { 
                             int x; 
                         } EXTRA_NESTING);
    } inner;
};

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim_array[(2+3)*4]
                   [sizeof(struct Outer)/sizeof(int)]
                   [5];

/* Array with nested type definition in size */
char *pointer_array[(sizeof(struct { 
    int x; 
    double y; 
    struct { 
        short s; 
    } nested; 
}) / sizeof(long))];

/* Requirement 3: Attribute specifications with multiple parentheses */
struct __attribute__((aligned(16), 
                     packed, 
                     deprecated("Use NewStruct instead"))) 
    AttributedStruct {
    int data __attribute__((aligned((8))));
    char buffer[64] __attribute__((aligned(32)));
};

/* C++11 style attributes (valid in C23 with __has_c_attribute) */
#if __has_c_attribute(deprecated)
[[deprecated("Old API")]] 
#endif
union ComplexUnion {
    int i;
    float f;
    struct {
        long l;
        double d;
    } s;
};

/* Requirement 5: Initializer lists with nested braces and parentheses */
int initialized_var = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y;
    int z[3];
};

struct Point points[] = {
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = {2}, 
        .z = { {1}, {2}, {3} } 
    },
    [1] = NESTED_BRACES,  /* Using macro with nested delimiters */
    [2] = { 
        .x = COMPLEX_PARENS(10),  /* Macro expansion with parentheses */
        .y = 20,
        .z = ARRAY_MACRO  /* This won't compile but will test parsing */
    }
};

/* Another complex type with all delimiters mixed */
typedef union {
    struct {
        int (*fp_array[2])(int, 
                          struct { 
                              int a[2][(3+4)]; 
                          });
    } s;
    long l;
} ComplexType;

/* Function with complex return type and attributes */
__attribute__((noinline, 
               returns_twice, 
               format(printf, 1, 2))) 
ComplexType* create_complex(int count) 
    __attribute__((alloc_size(1)));

/* Requirement 6: More conditional compilation */
#if defined(USE_FEATURE_A) && (FEATURE_LEVEL > 2)
enum ConditionalEnum {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    VALUE_C = (1 << (sizeof(int)*8 - 1))
};
#elif defined(USE_FEATURE_B)
enum ConditionalEnum {
    VALUE_X,
    VALUE_Y = { [0] = 1, [1] = 2 },
    VALUE_Z
};
#else
enum ConditionalEnum {
    DEFAULT_VALUE = 0
};
#endif

/* Struct with bitfields and nested anonymous struct */
struct BitfieldStruct {
    unsigned int a : 5;
    unsigned int b : 3;
    struct {
        unsigned int c : 2;
        unsigned int d : 6;
    };
    int (*fp)(void);
};

/* Array of function pointers with complex signatures */
int (*func_ptr_array[])(int, 
                       struct { 
                           int elements[2][{3}];  /* Invalid but tests parsing */
                       }) = {
    NULL,
    NULL
};

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    static struct Outer o;
    static ComplexType ct;
    
    /* Reference variables to prevent optimization */
    (void)o;
    (void)ct;
    (void)multi_dim_array;
    (void)pointer_array;
    (void)initialized_var;
    (void)points;
    
    return sizeof(struct Outer) + sizeof(ComplexType);
}

/* Final complex declaration with all delimiter types */
struct FinalTest {
    int a;
    struct {
        char b;
        union {
            int c;
            struct {
                double d;
            };
        } u;
    } s;
    int (*final_fp[3])(int (*)(int[][5], 
                             struct { 
                                 int x; 
                             }), 
                      void *restrict);
} __attribute__((aligned(64)));
