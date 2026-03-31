/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(void);
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[5];
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags:4;
        unsigned int mode:2;
        long double data;
    } inner;
    
    /* Multi-dimensional array (brackets) */
    char matrix[4][8][16];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Pointer to function taking function pointer parameter */
typedef void (*SignalHandler)(int sig, void (*cleanup)(void));

/* Triple-nested function pointer */
typedef int (*(*(*TripleFuncPtr)(char *))(float))(double);

/* Function returning function pointer */
typedef int (*(*FuncReturningFunc)(int x))(char y);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int fixed[10][20];
    double ***dynamic_ptr_array;
    long flex_array[];  /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    union {
        struct {
            unsigned int a:1;
            unsigned int b:3;
            unsigned int c:4;
        } bits;
        unsigned short all;
    } flags;
    
    struct {
        long x:8;
        long y:8;
        long z:16;
    } __attribute__((packed)) coordinates;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_PTR(T, N) T (*)[N]
#define NESTED_FUNC(T) T (*(*)(T (*)(T)))(T)

/* Using the macros */
PTR_FUNC(int) simple_func_ptr;
ARRAY_PTR(char, 10) char_array_ptr;
NESTED_FUNC(double) complex_nested_ptr;

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(32), packed)) AttributedStruct {
    int data __attribute__((aligned(16)));
    void (*func_ptr)(void) __attribute__((nonnull(1)));
} __attribute__((deprecated("Use NewStruct instead")));

/* Function with attributes */
int __attribute__((format(printf, 1, 2))) 
log_message(const char *fmt, ...) __attribute__((warn_unused_result));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*signal_handler(int sig, 
                         void (*callback)(int, const char *)))(void);
    
    /* Array of function pointers returning function pointers */
    int (*(*func_ptr_array[2][3])(float))(double);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:2[3];  /* Bit-field array (GCC extension) */
        } bits;
        unsigned char bytes[2];
    } data;
    
    /* Multi-dimensional pointer array */
    int *(*(*ptr_matrix)[5])[10];
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        void (*action)(struct UltimateTest *);
    } actions[];
};

/* Additional edge cases */

/* Parentheses in sizeof expressions */
#define SIZEOF_ARRAY(arr) (sizeof(arr) / sizeof((arr)[0]))

/* Compound literals in type definitions */
typedef struct { int x; float y; } Point;
Point *points = (Point[]){{1, 2.0}, {3, 4.0}, {5, 6.0}};

/* Nested attributes */
typedef int __attribute__((vector_size(16))) v4si __attribute__((aligned(16)));

/* K&R style function pointer (for completeness) */
typedef int (*kr_style_func_ptr)();

/* Empty struct/union (edge case) */
struct EmptyStruct {};

/* Forward declarations that might be parsed */
struct ForwardDecl;
typedef struct ForwardDecl *ForwardPtr;

/* Enum with complex initializers */
enum ComplexEnum {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1) | (1 << 2),
    VALUE3 = sizeof(struct UltimateTest)
};

/* Type qualifiers in complex positions */
const volatile struct OuterStruct * restrict volatile cv_qualified_ptr;

/* __builtin_choose_expr with parentheses */
#define CHOOSE_TYPE(cond) __builtin_choose_expr(cond, int, long)

/* Statement expressions (GCC extension) */
#define MAX(a,b) ({ typeof(a) _a = (a); typeof(b) _b = (b); _a > _b ? _a : _b; })

/* Transparent union attribute */
typedef union __attribute__((transparent_union)) TransUnion {
    int i;
    long l;
    void *p;
} TransUnion;

/* Alias with attribute */
typedef int __attribute__((may_alias)) aliasing_int;

/* Final test: The most complex single declaration we can create */
struct __attribute__((designated_init)) FinalChallenge {
    int (*(*(*ultimate[2])(int (*(*)(char [][10]))(float)))(double))
        (long (*(*)(short))[3]);
    union {
        struct {
            unsigned int:4;
            unsigned int field:12 __attribute__((packed));
        };
        struct __attribute__((aligned(8))) {
            void (*nested_func)(struct FinalChallenge *);
        } __attribute__((deprecated)) helper;
    } __attribute__((packed)) data;
    volatile const char * restrict strings[][5];
} __attribute__((aligned(64)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
