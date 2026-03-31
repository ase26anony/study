/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ) ) )
#define NESTED_BRACKETS [ [ [ ] ] ]
#define NESTED_BRACES { { { } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ([[{(([]))}]])
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer returning pointer to array, taking nested params */
    int (*(*complex_fp)(int (*)(int[2][(3+4)*2], 
                               struct { 
                                   int a; 
                                   char b[(sizeof(int)*2)]; 
                               }), 
                        union {
                            long l;
                            struct { short s; } inner;
                        }))[10];
    
    /* Nested struct with function pointer member */
    struct {
        void (*nested_callback)(int, 
                               struct { 
                                   int x; 
                                   double y[3][(2+1)]; 
                               }, 
                               char **);
    } helper;
};

/* Requirement 4: Array with complex dimensions containing parentheses */
int multi_dim_array[(2+3)*(sizeof(struct {char c; int i;})/4)]
                   [5]
                   [((int)(3.14 * 2))];

/* Requirement 3: GCC attributes with multiple parentheses */
struct __attribute__((aligned((16)), 
                     packed, 
                     deprecated("Use NewStruct instead"))) 
       AttributedStruct {
    int data __attribute__((aligned((8)*2)));
    char buffer[128] __attribute__((packed));
};

/* Requirement 1 & 3: Union with attributes and nested types */
union __attribute__((transparent_union)) ComplexUnion {
    int i;
    struct {
        float f;
        double d[(2+(3*4))];
    } __attribute__((packed)) s;
    void (*func_ptr)(int, ...);
};

/* Requirement 5: Initializer with deeply nested braces/parentheses */
int initialized_var = { { ( 1 + (2 + (3 * (4))) ) } };

/* Array initializer using macro */
int macro_array[] = NESTED_BRACES;

/* Struct initializer with designated initializers */
struct Point {
    int x;
    int y[3][2];
    struct {
        float z;
    } nested;
} points[] = { 
    [0] = { 
        .x = (1 + (2 * (3))), 
        .y = { {1, 2}, {3, (4)}, {5, 6} },
        .nested = { .z = { (3.14) } }
    },
    [1] = COMPLEX_MACRO  /* Using macro expansion */
};

/* Requirement 4: Pointer array with complex size expression */
void *ptr_array[(sizeof(struct DeepNested) + 
                (offsetof(struct AttributedStruct, buffer)))/8];

/* Requirement 1: Typedef with nested parentheses */
typedef int (*(*complex_callback_t)(int (*)(int[][3]), 
                                   struct { 
                                       int counter; 
                                   }))();

/* Requirement 6: Conditional type definitions */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* C11 attributes */
struct [[deprecated, 
         maybe_unused]] 
       ModernStruct {
    int data [[gnu::aligned(16)]];
};
#elif defined(__cplusplus) && __cplusplus >= 201103L
/* C++11 attributes */
class [[deprecated("C++ version")]] TestClass {
    [[nodiscard]] int* get_data() { return nullptr; }
};
#endif

/* Requirement 1: Enum with complex initializers */
enum States {
    IDLE = 0,
    ACTIVE = (1 << (sizeof(char)*8 - 1)),
    /* Using nested parentheses in value */
    ERROR = (IDLE | (ACTIVE & ~(0xFF))),
    /* Macro in enum value */
    SPECIAL = { NESTED_PARENS, 0 }
};

/* Requirement 2 & 5: Variable using macro with nested delimiters */
struct BraceTest {
    int levels;
    char pattern[10];
} bt = { 
    .levels = 3, 
    .pattern = NESTED_BRACKETS  /* Will expand to [ [ [ ] ] ] */
};

/* Function with complex parameter type */
static void process_data(int matrix[][(2*3)], 
                        struct DeepNested *dn,
                        complex_callback_t cb) 
    __attribute__((nonnull((2)), 
                   warn_unused_result));

/* Requirement 1: Another struct with function pointer chain */
struct CallbackChain {
    /* Pointer to function returning pointer to function... */
    void (*(*(*chain)(int))())(char);
    
    /* Array of function pointers */
    int (*handlers[(sizeof(int*)*2)])(int, ...);
};

/* Main function to avoid dead code elimination */
int main(void) {
    /* Reference variables to prevent optimization */
    struct DeepNested dn = {0};
    struct AttributedStruct as = {0};
    
    (void)multi_dim_array[0][0][0];
    (void)initialized_var;
    (void)points;
    (void)ptr_array;
    (void)bt;
    
#ifdef TEST_COMPLEX
    (void)EXTRA_NESTING;
#endif
    
    return 0;
}

/* Requirement 4: Variable length array in C99 mode */
#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
void vla_example(int n) {
    int vla[n][(n+1)/2];
    struct {
        int data;
        char flex[];
    } *p = __builtin_alloca(sizeof(*p) + n * sizeof(char));
    (void)vla;
    (void)p;
}
#endif
#endif

/* Requirement 3: More attribute variations */
int global_var 
    __attribute__((section((".data" ".special")),
                   aligned((16))),
                   used) = 42;

/* Final complex type combining all elements */
typedef struct {
    union {
        struct AttributedStruct as;
        struct DeepNested dn;
    } u;
    
    int (*array_of_funcs[((int)(sizeof(struct {int x;})/4))])(
        int param1,
        struct {
            int a;
            int b[2][2];
        } param2,
        ...
    );
    
    enum States state;
} UltimateType __attribute__((packed, aligned(32)));
