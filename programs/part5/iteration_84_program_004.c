/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_PARENS ( ( ( ( ( ) ) ) ) )
#define NESTED_BRACKETS [ [ [ [ ] ] ] ]
#define NESTED_BRACES { { { { } } } }
#define COMPLEX_MACRO { ( [ { } ] ) }

/* Requirement 1: Balanced token sequences in type definitions */

/* Struct with deeply nested function pointer */
struct DeepNested {
    /* Function pointer with nested parentheses */
    int (*level1)(int (*level2)(int (*level3)(int (*level4)(int)), int[2]), 
                  struct { 
                      int a; 
                      struct { 
                          char b; 
                      } inner; 
                  });
    
    /* Array of function pointers with complex signatures */
    void (*(*fp_array[((2+3)*4)])())(int, ...);
};

/* Union with nested type definitions */
union ComplexUnion {
    struct {
        /* Multi-dimensional array with parenthesized size */
        int matrix[(sizeof(int*) == 8 ? 64 : 32)][(2+3)*4];
        
        /* Pointer to array with nested brackets */
        char (*ptr_to_arr)[5][(sizeof(struct {int x; char y;}) + 7)/8];
    } data;
    
    /* Function pointer with attributes */
    long double (*compute)(int, ...) __attribute__((format(printf, 1, 2)));
};

/* Requirement 3: Attribute specifications with multiple parentheses */

/* Struct with GCC attributes containing parentheses */
struct __attribute__((aligned((16)), 
                      packed, 
                      deprecated("Use NewStruct instead"))) 
    AttributedStruct {
    int field1 __attribute__((aligned((8))));
    char field2 __attribute__((deprecated));
};

/* Variable with C11 attributes (if compiled as C++) */
#ifdef __cplusplus
[[deprecated("Old variable")]] 
[[gnu::always_inline]]
#endif
static volatile int attributed_var = 0;

/* Requirement 4: Array declarations with complex dimensions */

/* Multi-dimensional array with computed sizes */
extern int complex_array[
    (sizeof(struct DeepNested) + sizeof(union ComplexUnion)) / 
    sizeof(int) + 1
][
    (offsetof(struct AttributedStruct, field2) * 2)
];

/* Array where size uses nested parentheses in expression */
static char* ptr_array[(2 * (3 + (4 * (5 + 6))))];

/* Requirement 5: Initializer lists with nested braces and parentheses */

/* Complex initializer */
struct Point {
    int x;
    int y;
    int z[3];
};

/* Initializer with deeply nested braces and parentheses */
struct Point points[] = {
    [0] = { 
        .x = { ( 1 + (2 * (3 + 4))) }, 
        .y = { { { 5 } } }, 
        .z = { (6), {7}, (8 + (9)) } 
    },
    [1] = COMPLEX_MACRO,  /* Using macro expansion */
    [2] = NESTED_BRACES
};

/* Variable with nested initializer */
int initialized = { { ( 1 + (2 + (3 + (4)))) } };

/* Requirement 6: Conditional compilation blocks with balanced tokens */

#ifdef TEST_COMPLEX_TYPES
/* Type definition inside conditional compilation */
typedef struct {
    /* Nested anonymous struct */
    struct {
        enum { 
            VALUE1 = (1 << 0), 
            VALUE2 = (1 << (1 + 1)) 
        } flags;
        
        /* Array with nested brackets in size */
        void* buffer[sizeof(struct { int a[(2+3)]; })];
    } inner;
    
    /* Function pointer with complex return type */
    struct { int a; char b; } (*get_data)(int, ...);
} ConditionalType;

#elif defined(ALTERNATE_DEF)
/* Alternative type definition */
union Alternate {
    long double ld;
    struct {
        int i;
        char c;
    } s;
};

#else
/* Default type when neither macro is defined */
typedef int DefaultType[(2 + (3 * 4))];
#endif

/* Additional complex type mixing all requirements */
typedef struct UltimateTest {
    /* Attribute with parentheses */
    __attribute__((aligned(( (16) ))))
    
    /* Member with all nested delimiters */
    int (*(*ultimate_member)(
        int (*callback)(int[2][(3+4)], 
                       struct { 
                           int x; 
                           char y[(sizeof(double) + 3)/4]; 
                       }),
        ...))[ (sizeof(int) * (2 + 3)) ];
    
    /* Initializer in declaration (GCC extension) */
    int with_init = { ( { ( 42 ) } ) };
} UltimateTest_t;

/* Function prototype with complex parameter */
extern void complex_function(
    int param1[(2 * (3 + 4))],
    struct { 
        int (*method)(int, ...); 
    } param2,
    ...  /* Variadic parameter */
);

/* Main function that references types to avoid dead code elimination */
int main(void) {
    struct DeepNested dn = {0};
    union ComplexUnion cu = {0};
    struct AttributedStruct as = {0};
    
    /* Reference variables to prevent optimization */
    (void)dn;
    (void)cu;
    (void)as;
    (void)points;
    (void)initialized;
    (void)attributed_var;
    (void)complex_array;
    (void)ptr_array;
    
#ifdef TEST_COMPLEX_TYPES
    ConditionalType ct = {0};
    (void)ct;
#endif
    
    UltimateTest_t ut = {0};
    (void)ut;
    
    return 0;
}

/* Final test: Type definition with everything nested */
struct FinalChallenge {
    /* Nested everything */
    struct {
        union {
            int (*fp[({ int x = 5; x; })])(
                int (*)(int[({ int y = 3; y; })][2], 
                       struct { 
                           int a; 
                       }),
                ...
            );
            char c;
        } u;
    } s;
};
