/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACES { { { { { } } } } }
#define NESTED_BRACKETS [ [ [ [ [ ] ] ] ] ]
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer with nested parentheses */
    int (*fp1)(int (*)(int (**)(int (*)[3]), struct { int x; }), 
               double (*)[(2+3)*4]);
    
    /* Array of function pointers with complex signatures */
    void (*(*fp_array)[5])(int, 
                           struct { 
                               int a; 
                               union { 
                                   char c; 
                                   int i; 
                               } u; 
                           });
    
    /* Nested struct with function pointer member */
    struct {
        int (*nested_fp)(int (*callback)(int[2][3], 
                        struct { 
                            char *p; 
                            int arr[sizeof(struct {int x;})]; 
                        }));
    } inner;
};

/* Union with complex type definitions */
union ComplexUnion {
    /* Function pointer with attributes and nested params */
    int (__attribute__((aligned(16))) *ufp)(
        int, 
        struct { 
            int a; 
            int b[(2+3)*4]; 
        }
    );
    
    /* Array with computed size containing nested struct */
    char data[sizeof(struct {
        int x;
        double y;
        struct {
            short s;
            long l;
        } nested;
    })];
};

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with parenthesized size expressions */
int multi_dim_array[(sizeof(struct {int x; char c;}) + 7) / 4]
                   [((2+3)*4)]
                   [5];

/* Array of pointers with complex size expression */
void *pointer_array[(
    sizeof(
        union {
            struct { int a; double b; } s;
            long long ll;
        }
    ) / sizeof(void*)
)];

/* Requirement 3: Attribute specifications with multiple parentheses */

/* Struct with GCC attributes containing parentheses */
struct __attribute__((aligned(16), 
                      packed, 
                      deprecated("Use NewStruct instead"))) 
AttributedStruct {
    int field1 __attribute__((aligned(8)));
    char field2 __attribute__((deprecated));
    
    /* Nested struct with attribute */
    struct __attribute__((packed)) {
        short a;
        int b;
    } nested;
};

/* C++11 style attributes (if compiled as C++) */
#ifdef __cplusplus
[[deprecated("Old type")]] 
[[nodiscard]]
struct CppStruct {
    int value [[maybe_unused]];
};
#endif

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Complex initializer */
int complex_init = { { ( 1 + (2 * (3 + (4)))) } };

/* Array with designated initializers containing nested braces */
struct Point {
    int x;
    struct { int y; int z; } coord;
} points[] = {
    [0] = { .x = (1 + 2), .coord = { .y = {3}, .z = 4 } },
    [1] = { .x = 5, .coord = { .y = 6, .z = 7 } },
    [2] = { .x = 8, .coord = { NESTED_BRACES } }
};

/* Initializer using macro with nested delimiters */
int macro_init[] = COMPLEX_MACRO;

/* Requirement 6: Conditional compilation blocks with balanced tokens */

#ifdef TEST_COMPLEX_TYPES
/* This section only included if TEST_COMPLEX_TYPES is defined */

/* Typedef with nested parentheses */
typedef int (*ComplexFuncPtr)(
    int, 
    struct { 
        int a; 
        int b[((2+3)*4)]; 
    }, 
    void (*)(int, int)
);

/* Enum with complex expressions in initializers */
enum ComplexEnum {
    VALUE1 = (sizeof(struct { int x; })),
    VALUE2 = ((2+3)*4),
    VALUE3 = VALUE1 + VALUE2
};

/* Union nested in conditional block */
union ConditionalUnion {
    char c;
    struct {
        int i;
        int j[(2+3)*4];
    } s;
};

#endif /* TEST_COMPLEX_TYPES */

/* More conditional blocks */
#if defined(USE_FEATURE_A) || defined(USE_FEATURE_B)

/* Struct with bitfields and nested type */
struct FeatureStruct {
    unsigned int flags : 4;
    signed int value : 12;
    
    /* Anonymous union with nested struct */
    union {
        struct {
            char a;
            int b[3];
        } s1;
        struct {
            double d;
            short s;
        } s2;
    };
};

#elif defined(USE_FEATURE_C)

/* Alternative type definition */
struct AltStruct {
    int (*methods[3])(
        struct AltStruct *self,
        int param,
        void (*callback)(int, int)
    );
};

#else
/* Default type when no features are defined */
struct DefaultStruct {
    int value;
};
#endif

/* Function with complex parameter list (more nested parentheses) */
void complex_function(
    int (*param1)(int, 
                  struct { 
                      int x; 
                      int y[(2+3)*4]; 
                  }), 
    void (*param2)(int (**)(int (*)[3]), 
                   union { 
                       long a; 
                       double b; 
                   })
) {
    /* Local variable with nested initializer */
    int local = { { ( 1 ) } };
    
    /* Array with nested brace initializer */
    int arr[2][2] = { { {1}, {2} }, { {3}, {4} } };
}

/* Main function that references some types to avoid dead code elimination */
int main(void) {
    struct DeepNested dn = {0};
    struct AttributedStruct as = {0};
    
    /* Use macro expansions */
    int use_macro = NESTED_PARENS;
    
    /* Reference conditional types */
#ifdef TEST_COMPLEX_TYPES
    ComplexFuncPtr fp = NULL;
#endif
    
    /* Reference initialized arrays */
    int val = points[0].x + complex_init + macro_init[0];
    
    return 0;
}

/* Additional edge cases */

/* Typedef with function type containing nested parentheses */
typedef int (*(*ComplexTypedef)(int, int))(int, int);

/* Struct with array of structs containing function pointers */
struct FinalStruct {
    struct {
        int (*(*member)(int, int))[3];
    } nested_array[2];
    
    /* Zero-length array with attribute */
    int flexible[] __attribute__((aligned(8)));
};

/* Global variable with complex type and initializer */
struct DeepNested global_var = {
    .fp1 = NULL,
    .fp_array = NULL,
    .inner = {
        .nested_fp = NULL
    }
};
