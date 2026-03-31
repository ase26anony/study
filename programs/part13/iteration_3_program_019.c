/* gengtype_test_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 * Specifically targeting lines 341-352 in gengtype-parse.cc
 */

#ifndef GENGTYPE_TEST_COVERAGE_H
#define GENGTYPE_TEST_COVERAGE_H

/* ==================== MACRO EXPANSIONS WITH BRACKETS ==================== */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*))(T)
#define COMPLEX_MACRO(T) T (*(*[2]))(T (*)(T[5]))

/* ==================== FUNCTION POINTERS WITH NESTED PARENTHESES ==================== */

/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*func_ret_func_ptr)(float))(double);

/* Complex nested parentheses */
typedef void (*(*(*deep_nested_ptr)(int (*(*)(char[10]))(long)))(short))(int);

/* Function pointer with array parameter */
typedef void (*array_param_func)(int matrix[3][4]);

/* ==================== STRUCTS WITH ALL BRACKET TYPES ==================== */

/* Struct combining all bracket types in one declaration */
struct MasterStruct {
    /* Parentheses: function pointer with complex signature */
    void (*signal_handler)(int sig, void (*cleanup)(void*));
    
    /* Brackets: multi-dimensional array */
    int multi_array[2][3][4];
    
    /* Braces: anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int :4;  /* Unnamed bit-field */
            unsigned int value:8;
        } bits;
        unsigned int raw;
    } bitfield_container;
    
    /* Flexible array member */
    int flexible_array[];
};

/* ==================== NESTED TYPE DEFINITIONS ==================== */

/* Union containing struct with function pointer array */
union ComplexUnion {
    struct {
        /* Array of function pointers */
        int (*func_array[5])(char *);
        
        /* Pointer to array of function pointers */
        void (*(*ptr_to_func_array)[3])(int);
        
        /* Nested anonymous struct */
        struct {
            /* Function returning pointer to array */
            int (*(*get_array)(void))[10];
            
            /* Complex declaration combining all brackets */
            void (*(*complex_member)(int (*)(char[10])))[2];
        };
    } nested;
    
    /* Another struct with bit-fields */
    struct {
        unsigned char a:2;
        unsigned char b:2;
        unsigned char c:4;
    } small_bits;
};

/* ==================== ATTRIBUTED DECLARATIONS ==================== */

/* Struct with GCC attributes */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func_ptr)(void) __attribute__((noreturn));
    
    /* Nested attributed union */
    union __attribute__((transparent_union)) {
        int i;
        long l;
    } value;
};

/* Function with attributes */
void __attribute__((format(printf, 1, 2))) 
attributed_function(const char *fmt, ...);

/* ==================== USING MACRO EXPANSIONS ==================== */

/* Type using macro that expands to include parentheses */
typedef PTR_FUNC(int) int_func_ptr_t;

/* Array declaration using macro */
ARRAY_DECL(int_func_ptr_t, 5) macro_array;

/* Complex macro expansion */
COMPLEX_MACRO(double) complex_macro_var;

/* ==================== EXTREME NESTING EXAMPLE ==================== */

/* The ultimate test - maximum nesting of all bracket types */
struct UltimateNesting {
    /* Triple parentheses nesting */
    int (*(*(*(*deep_func_ptr)(void))(int))(float))(double);
    
    /* Array of pointers to functions returning pointers to arrays */
    char (*(*(*func_ptr_array[3])(int))[5])(void);
    
    /* Struct containing union containing struct with bit-fields */
    struct {
        union {
            struct {
                unsigned int a:1, b:1, c:1, d:29;
            } bits1;
            struct {
                unsigned int x:4, y:4, z:24;
            } bits2;
            long long all_bits;
        } switcher;
        
        /* Multi-dimensional flexible array member simulation */
        struct {
            size_t count;
            int data[][10][20];  /* Simulated multi-dim flexible array */
        } dynamic;
    } mega_nested;
    
    /* Function pointer with attribute in parameter */
    void (*(*attr_in_param)(int __attribute__((unused))))(void);
};

/* ==================== ADDITIONAL EDGE CASES ==================== */

/* Empty struct */
struct EmptyStruct {};

/* Struct with only bit-fields */
struct OnlyBitfields {
    unsigned int a:1;
    unsigned int b:2;
    unsigned int c:29;
};

/* Struct with array of empty structs */
struct ArrayOfEmpty {
    struct EmptyStruct arr[10];
};

/* Forward declaration to test incomplete types */
struct ForwardDeclared;

/* Struct with pointer to forward declared type */
struct HasForwardPtr {
    struct ForwardDeclared *ptr;
    struct ForwardDeclared *array_ptr[5];
};

/* Now define the forward declared struct */
struct ForwardDeclared {
    int value;
    struct HasForwardPtr *back_ptr;
};

/* ==================== EXTERNAL DECLARATIONS ==================== */

/* External variable with complex type */
extern struct MasterStruct external_master;

/* External function with complex return type */
struct UltimateNesting *(*get_nesting_factory(void))(int) 
    __attribute__((warn_unused_result));

/* ==================== TYPEDEF COMBINATIONS ==================== */

/* Chain of typedefs */
typedef int simple_int;
typedef simple_int *int_ptr;
typedef int_ptr (*func_returning_int_ptr)(void);
typedef func_returning_int_ptr func_ptr_array_t[3];

/* Final complex typedef */
typedef struct {
    func_ptr_array_t funcs;
    struct UltimateNesting *nesting;
    volatile const int (* volatile * const volatile restrict complex_ptr)[10];
} UltimateTypedef;

#endif /* GENGTYPE_TEST_COVERAGE_H */
