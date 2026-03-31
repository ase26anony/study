/* gengtype-test.c - Complex type definitions to exercise consume_balanced() */

#include <stddef.h>

/* Requirement 2: Macro expansions with nested delimiters */
#define NESTED_BRACES { { ( [ { } ] ) } }
#define COMPLEX_PARENS(x) ((int (*[(x)])())(NULL))
#define BRACKET_MACRO [sizeof(struct { char c; })]

/* Requirement 6: Conditional compilation with balanced tokens */
#ifdef TEST_COMPLEX
#define EXTRA_NESTING ( { [ ( { } ) ] } )
#else
#define EXTRA_NESTING /* nothing */
#endif

/* Requirement 1: Struct with deeply nested function pointer */
struct Outer {
    /* Function pointer with nested parameter containing array and struct */
    int (*callback)(
        int (*helper)(int matrix[2][3], 
                     struct { 
                         int a; 
                         union { 
                             char c; 
                             double d; 
                         } u; 
                     }),
        void *ptr
    );
    
    /* Nested struct inside union */
    union {
        struct {
            int x;
            /* Array of function pointers */
            void (*actions[3])(int, char);
        } data;
        long value;
    } u;
};

/* Requirement 4: Array with complex dimensions */
int multi_array[(2 + 3) * sizeof(struct { char a; int b; })][5];
int *pointer_array[BRACKET_MACRO];

/* Requirement 3: GCC attributes with parentheses */
struct __attribute__((aligned(16), packed, deprecated("Use NewStruct"))) Attributed {
    int field1;
    char field2;
} __attribute__((visibility("hidden")));

/* Another struct with nested attributes */
typedef struct __attribute__((aligned((4)*sizeof(int)))) {
    int data;
    /* Function pointer attribute */
    void (*func __attribute__((noreturn)))(void);
} ComplexTypedef;

/* Requirement 5: Initializers with nested braces/parentheses */
int initialized = { { ( 1 + (2 * (3 + 4) ) ) } };

struct Point {
    int x;
    int y[2];
    struct {
        int z;
    } nested;
};

struct Point points[] = {
    [0] = { 
        .x = (1 + (2 - 3)), 
        .y = {2, {3}}, 
        .nested = { .z = ({ 5; }) }
    },
    [1] = NESTED_BRACES
};

/* Union with anonymous struct */
union Container {
    struct {
        int tag;
        /* Nested array in anonymous struct */
        char data[(sizeof(int) + 3) & ~3];
    };
    long long as_int;
};

/* Enum with complex initializer */
enum States {
    IDLE = 0,
    ACTIVE = ({ 1; }),
    /* Parenthesized expression in enum */
    ERROR = (1 << (sizeof(char)*8 - 1))
};

/* Requirement 1: More nested type definitions */
typedef int (*ComplexFuncPtr)(
    int,
    struct {
        int a;
        int b[({ 2; })];
    } *arg
);

/* Function-like macro used in type definition */
#define DECLARE_FUNC(name, type) type (*name)(type, type)

DECLARE_FUNC(fp, int) = NULL;

/* Nested conditional compilation */
#if defined(__GNUC__) && __GNUC__ >= 4
struct GccSpecific {
    int feature __attribute__((vector_size(16)));
} __attribute__((aligned(32)));
#elif defined(_MSC_VER)
struct MsvcSpecific {
    __declspec(align(16)) int data;
};
#else
struct Fallback {
    int data;
};
#endif

/* Requirement 4: Variable Length Array in struct (C99) */
struct WithVLA {
    int rows;
    int cols;
    /* Note: VLA in struct is a GCC extension */
    int matrix[][(2 * (3 + 1))];
};

/* Requirement 5: Designated initializer with nesting */
struct NestedInit {
    struct {
        struct {
            int deepest;
        } level2;
    } level1;
} ni = { .level1 = { .level2 = { .deepest = ({ 42; }) } } };

/* Main function that references types to avoid dead code elimination */
int main(void) {
    static struct Outer o;
    static ComplexTypedef ct;
    
    /* Use variables to prevent optimization */
    (void)o;
    (void)ct;
    (void)multi_array;
    (void)pointer_array;
    (void)initialized;
    (void)points;
    (void)ni;
    
    return 0;
}

/* Final conditional block with balanced tokens */
#ifdef ULTRA_NESTED
struct Ultimate {
    int (*ultra[({ 2; })])(
        struct {
            int a[({ 3; })][({ 4; })];
            union {
                char c;
                int i[({ ({ 5; }); })];
            } u;
        } *
    );
} __attribute__((aligned((({ 64; })))));
#endif
