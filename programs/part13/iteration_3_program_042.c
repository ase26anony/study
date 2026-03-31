/* gengtype_coverage_test.h
 * Complex type definitions to test gengtype's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

#ifndef GENGTYPE_COVERAGE_TEST_H
#define GENGTYPE_COVERAGE_TEST_H

/* ==================== MACRO EXPANSIONS WITH BRACKETS ==================== */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_2D(T, N, M) T [N][M]
#define NESTED_PTR_FUNC(T) T (*(*)(T (*)(T)))(T)
#define ATTR_WRAP(...) __attribute__((__VA_ARGS__))

/* ==================== FUNCTION POINTERS WITH NESTED PARENTHESES ==================== */

/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*complex_func_ptr)(int (*)(char)))(double);

/* Even more nested: pointer to function taking function pointer parameter */
typedef void (*(*nested_func_ptr)(int (*)(char[10]), void (*)(int)))(float);

/* Function pointer with array parameter */
typedef int (*func_with_array)(int matrix[3][4]);

/* ==================== COMPLEX STRUCT DEFINITIONS ==================== */

/* Struct with all bracket types in one declaration */
struct MasterStruct {
    /* Function pointer member with complex signature */
    void (*signal_handler)(int sig, void (*cleanup)(void*));
    
    /* Array of function pointers */
    int (*func_array[5])(char *);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int :4;  /* Unnamed bit-field */
            unsigned int value:8;
        } bits;
        unsigned int raw;
    } flags;
    
    /* Multi-dimensional array */
    double matrix[2][3][4];
    
    /* Pointer to array */
    int (*ptr_to_array)[10];
    
    /* Flexible array member */
    int flexible_array[];
};

/* Union with deeply nested types */
union ComplexUnion {
    /* Function pointer returning struct pointer */
    struct MasterStruct* (*get_struct)(int id);
    
    /* Array of pointers to functions returning function pointers */
    void (*(*func_ptr_array[3])(int))();
    
    /* Nested struct with bit-fields */
    struct {
        long double ld;
        unsigned char:2;
        unsigned int nested_bit:5;
        struct {
            short s;
            char c;
        } inner;
    } nested;
};

/* ==================== TYPE DEFINITIONS WITH MACRO EXPANSIONS ==================== */

/* Using macro that expands to include parentheses */
typedef PTR_FUNC(int) int_func_ptr_t;

/* Using array macro */
typedef ARRAY_2D(float, 5, 5) float_matrix_t;

/* Even more complex macro expansion */
typedef NESTED_PTR_FUNC(double) nested_double_func_t;

/* ==================== ATTRIBUTED DECLARATIONS ==================== */

/* Struct with GCC attributes */
struct ATTR_WRAP(packed, aligned(16)) AttributedStruct {
    int data ATTR_WRAP(aligned(8));
    char buffer[64] ATTR_WRAP(aligned(32));
    
    /* Function pointer with attribute */
    void (*callback)(void) ATTR_WRAP(nonnull(1));
};

/* Variable with attribute containing parentheses */
extern volatile int global_counter ATTR_WRAP(used, section(".data")) = 0;

/* ==================== THE ULTIMATE CHALLENGE ==================== */
/* Single declaration combining all bracket types as specified */

struct UltimateChallenge {
    /* Complex function pointer declaration */
    void (*signal(int sig, void (*func)(int)))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[2])(int (*)(char[10]));
    
    /* Anonymous union with bit-field */
    union {
        int x;
        long y:8;
        struct {
            unsigned a:1;
            unsigned b:2;
            unsigned c:3;
        } bits;
    } u;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Pointer to flexible array member in nested struct */
    struct {
        int count;
        double values[];
    } *flex_struct_ptr;
    
    /* Function returning pointer to array */
    int (*(*get_array_ptr)(void))[5];
    
    /* Nested struct with all bracket types */
    struct {
        /* Nested function pointer */
        void (*(*nested_fp)(int[2][2]))(void);
        
        /* Union in struct in struct */
        union {
            char *str;
            void (*action)(struct UltimateChallenge *self);
        } handler;
        
        /* Bit-field array */
        unsigned int flags[2]:16;
    } inner;
};

/* ==================== ADDITIONAL EDGE CASES ==================== */

/* Empty struct (still has braces) */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct BitFieldOnly {
    unsigned a:1, b:1, c:1, d:1;
    unsigned long long big:48;
};

/* Typedef with function pointer and array */
typedef int (*(*ConfusingTypedef)[5])(float (*)(double));

/* K&R style function pointer (for historical coverage) */
typedef int (*kr_style_ptr)();

/* ==================== EXTERNAL DECLARATIONS ==================== */

/* Variable declarations using complex types */
extern struct MasterStruct global_master;
extern union ComplexUnion *union_ptr_array[10];
extern const volatile struct AttributedStruct attributed_instances[2];

/* Function prototype with complex return type */
struct UltimateChallenge *create_challenge(int (*init_func)(struct MasterStruct*));

/* Inline function with attribute */
static inline void ATTR_WRAP(always_inline)) 
update_counter(volatile int *counter) {
    (*counter)++;
}

#endif /* GENGTYPE_COVERAGE_TEST_H */
