/* test_gengtype_coverage.h
 * Complex type definitions to exercise gengtype's consume_balanced function
 * Targeting lines 341-352 in gengtype-parse.cc
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* ==================== MACRO EXPANSIONS WITH BRACKETS ==================== */

/* Macro that expands to include parentheses */
#define FUNCTION_POINTER(T) T (*)(T)

/* Macro that expands to include brackets */
#define ARRAY_TYPE(T, N) T [N]

/* Macro that expands to include braces (via struct initialization) */
#define INIT_VALUE {0, 0, 0}

/* Complex macro combining all bracket types */
#define COMPLEX_DECL(T) T (*array[2])(T (*)(T[10]))

/* ==================== FUNCTION POINTERS WITH NESTED PARENTHESES ==================== */

/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*func_returning_func_ptr)(void))(double);

/* Nested parentheses in parameters */
typedef void (*complex_func_ptr)(int (*callback)(char[10]), void (*)(int));

/* Even more nested */
typedef int (*(*(*deeply_nested_fp)(int (*)(char)))(float))[5];

/* Function pointer with attributes (extra parentheses) */
typedef void (__attribute__((noreturn)) *noreturn_func_ptr)(void);

/* ==================== STRUCTS WITH ALL BRACKET TYPES ==================== */

/* Struct with function pointer member containing nested parentheses */
struct SignalHandler {
    /* Function returning function pointer */
    void (*signal(int sig, void (*handler)(int)))(int);
    
    /* Array of function pointers */
    int (*handlers[5])(int, char);
    
    /* Nested function pointer in array */
    void (*(*callback_array[3])(int))(void);
};

/* Struct with anonymous union and bit-fields (braces) */
struct DeviceRegister {
    unsigned int control;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int ready:1;
            unsigned int error:2;
            unsigned int mode:3;
        } bits;
        unsigned int raw;
    } status;
    
    /* Multi-dimensional array */
    unsigned char data[4][256];
};

/* Extremely complex struct combining all bracket types */
struct UltimateTest {
    /* Combination 1: Array of pointers to functions with nested params */
    int (*(*func_array[2][3])(int (*)(char[10])))(double);
    
    /* Combination 2: Flexible array member of structs */
    struct {
        int x;
        double y;
        void (*callback)(int);
    } flexible[];
    
    /* Combination 3: Nested anonymous struct with bit-fields */
    struct {
        union {
            struct {
                unsigned int a:4;
                unsigned int b:4;
                unsigned int c:8;
            };
            unsigned short packed;
        } flags[2];
        
        /* Pointer to array */
        int (*matrix_ptr)[10][20];
    } nested;
    
    /* Function pointer with complex return type */
    struct DeviceRegister (*(*get_register)(int id))(void);
};

/* ==================== UNIONS WITH COMPLEX MEMBERS ==================== */

union ComplexUnion {
    /* Function pointer */
    int (*func_ptr)(int);
    
    /* Array */
    char data[100];
    
    /* Nested struct with bit-fields */
    struct {
        unsigned int type:4;
        unsigned int size:12;
        unsigned int flags:16;
    } header;
    
    /* Pointer to array of function pointers */
    void (*(*func_ptr_array)[5])(int);
};

/* ==================== TYPEDEFS WITH MACRO EXPANSIONS ==================== */

/* Use the macros to generate complex types */
typedef FUNCTION_POINTER(int) int_func_ptr_t;
typedef ARRAY_TYPE(int_func_ptr_t, 5) func_ptr_array_t;

/* Complex type using macro */
typedef COMPLEX_DECL(double) complex_double_func_t;

/* ==================== VARIABLE DECLARATIONS WITH ATTRIBUTES ==================== */

/* Variable with attribute (double parentheses) */
struct SignalHandler global_handler __attribute__((aligned(16), packed));

/* Array with attribute */
unsigned char aligned_buffer[1024] __attribute__((aligned(64)));

/* Function pointer variable with attribute */
extern void (__attribute__((const)) * const math_func)(double) __attribute__((deprecated));

/* ==================== MORE COMPLEX NESTING ==================== */

/* Struct containing union containing struct containing array of function pointers */
struct DeepNest {
    union {
        struct {
            int count;
            /* Array of pointers to functions returning pointers to arrays */
            int (*(*callbacks[3])(int))[10];
        } data;
        struct {
            /* Function with nested parameter */
            void (*init)(int (*config)(char[5][10]));
            /* Multi-dimensional flexible array member */
            int matrix[][3][4];
        } config;
    } container;
    
    /* Pointer to function returning pointer to struct with bit-fields */
    struct {
        unsigned int a:1, b:1, c:1, d:29;
    } (*(*get_flags)(void));
};

/* ==================== EDGE CASES ==================== */

/* Empty braces (should still trigger consume_balanced) */
struct EmptyBraces {
    struct {} empty;
    union {} another_empty;
};

/* Single element arrays */
struct SingleElement {
    int one[1];
    void (*single_func[1])(int);
};

/* Zero-length array (GCC extension) */
struct ZeroLength {
    int header;
    char data[0];  /* Flexible array member, old style */
};

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Function with complex parameter */
extern void register_callback(int (*callback)(int, void (*)(char)), 
                              void *context __attribute__((nonnull)));

/* Function returning complex type */
struct UltimateTest *create_test_struct(int size) 
    __attribute__((malloc, warn_unused_result));

/* Function with nested attribute */
void important_function(void) 
    __attribute__((constructor(101), visibility("hidden")));

/* ==================== FINAL MEGA STRUCT ==================== */

/* One struct to rule them all - contains every bracket pattern */
struct AllPatterns {
    /* 1. Parentheses: function pointer with nested function pointer parameter */
    int (*(*level1)(int (*(*level2)(char (*level3)[10]))(double)))(float);
    
    /* 2. Brackets: multi-dimensional array of the above */
    int (*(*level1_array[2][3])(int (*(*level2_array[2])(char (*level3_array[5])[10]))(double)))(float)[4];
    
    /* 3. Braces: anonymous struct with bit-fields and nested union */
    struct {
        union {
            struct {
                unsigned int f1:1, f2:1, f3:1, f4:29;
            } bits;
            unsigned int raw;
        } flags[2];
        
        /* Nested struct with function pointer array */
        struct {
            void (*actions[5])(struct AllPatterns *);
            int state;
        } controller;
    } internal;
    
    /* 4. Flexible array member of function pointers */
    void (*(*dynamic_funcs[])(int (*(*)(char[10]))(double)))(void);
    
    /* 5. Pointer to array of structs containing unions */
    struct {
        union {
            int x;
            void (*func)(int);
        } u;
        char tag;
    } (*ptr_to_array)[10];
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
