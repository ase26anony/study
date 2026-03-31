/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((int (*[(x)])(void)){0})
#define BRACKET_MACRO [[maybe_unused]]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
  #define ENABLE_NESTED 1
#else
  #define ENABLE_NESTED 0
#endif

/* Requirement 1: Balanced token sequences in type definitions */
struct Outer {
    /* Function pointer with deeply nested parentheses */
    int (*complex_fp)(int (*callback)(int[2][(3+2)*sizeof(int)]), 
                      struct { 
                          int a; 
                          union { 
                              char c; 
                              long l; 
                          } u; 
                      } s);
    
    /* Nested struct with all delimiter types */
    struct Inner {
        int (*arr_ptr)[(sizeof(struct {int x; char y;}) + 7)/8];
        void (*vfunc)(int, ...);
    } inner;
    
    /* Array of function pointers */
    void (*(*signal_handler[3])(int))(int);
};

/* Union with nested attributes */
union Data {
    char bytes[16];
    struct {
        int id;
        float value;
    } __attribute__((aligned(16), packed)) tagged;
} __attribute__((aligned(32)));

/* Requirement 4: Array declarations with complex dimensions */
int multi_dim[(2+3)*4][5];
int *ptr_array[(sizeof(struct {int x; double y;})/sizeof(int))];

/* Requirement 3: Attribute specifications */
struct __attribute__((aligned((16)), packed)) PackedStruct {
    int a;
    char b;
    double c __attribute__((aligned(8)));
};

/* C++11 style attribute (valid in C23/C2x) */
#if __STDC_VERSION__ >= 202311L
[[deprecated("Use NewType instead")]]
#endif
typedef struct OldType {
    int field;
} OldType;

/* Requirement 5: Initializer lists with nested braces/parentheses */
int complex_init = { { ( 1 + (2) ) } };

struct Point {
    int x;
    int y[2];
};

struct Point points[] = { 
    [0] = { .x = (1), .y = {2, {3}} },
    [1] = { .x = {4}, .y = NESTED_BRACES }
};

/* Function with complex return type */
int (*(*get_handler(void))[2])(int, float) {
    static int (*arr[2])(int, float) = { NULL, NULL };
    return &arr;
}

/* Enum with complex underlying type */
enum __attribute__((packed)) ByteEnum : unsigned char {
    ZERO = 0,
    MAX = 255
};

/* Requirement 6: Conditional blocks containing target sequences */
#if ENABLE_NESTED
union ConditionalUnion {
    char c; 
    struct { 
        int i; 
        int j[(2*(3+1))]; 
    } s;
};
#endif

#ifdef __cplusplus
/* C++ specific constructs */
template<typename T>
class Container {
    T data[((sizeof(T) + 15) / 16) * 16];
public:
    Container() {}
};
#endif

/* Main function referencing defined types to avoid dead code elimination */
int main(void) {
    struct Outer o = {0};
    struct Point p = points[0];
    
    /* Use macro with nested delimiters */
    int (*func_array[])(void) = COMPLEX_PARENS(2);
    
    /* Reference conditionally compiled type */
#if ENABLE_NESTED
    union ConditionalUnion cu;
    (void)cu;
#endif
    
    return (int)(size_t)&o + p.x;
}

/* Additional complex type definitions */

/* Function type with nested attributes */
typedef int (__attribute__((cdecl)) *CallbackType)(
    int param1,
    struct {
        int a;
        int b[3];
    } param2
);

/* Bitfield struct with nested constructs */
struct BitfieldStruct {
    unsigned int a : (1 + (2));
    unsigned int b : 3;
    int c : (sizeof(int)*8 - 5);
};

/* Anonymous struct/union */
struct Anonymous {
    union {
        struct {
            int x;
            int y;
        };
        long long ll;
    };
};

/* Array with designators and nested braces */
int designator_array[10] = {
    [0] = 1,
    [1 ... 3] = {2, {3}, 4},
    [9] = (5 + (6 * 7))
};

/* Nested switch-like macro usage */
#define APPLY_ATTR(x) x __attribute__((aligned(x)))
APPLY_ATTR(struct AlignedStruct {
    int data;
});

/* Final type with all delimiter types mixed */
struct UltimateType {
    int (*((*array_of_func_ptrs)[(2+3)]))(int, 
        struct {
            int a[2][{3}];
            union {
                char c;
                int i;
            } u;
        });
    void (*initialize)(int, ...) __attribute__((format(printf, 1, 2)));
};
