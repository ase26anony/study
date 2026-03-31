/* gengtype_coverage_test.h
 * Complex type definitions to test consume_balanced() parser in gengtype
 */

#ifndef GENGTYPE_COVERAGE_TEST_H
#define GENGTYPE_COVERAGE_TEST_H

/* 1. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*)(T (*)(T)))(T)

/* 2. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer with nested parentheses */
    void (*signal_handler)(int sig, void (*cleanup)(void*));
    
    /* Pointer to function returning pointer to function */
    int (*(*complex_func_ptr)(int (*)(char*)))(double);
    
    /* Multi-dimensional arrays */
    int matrix[3][4][5];
    
    /* Array of function pointers */
    void (*callbacks[10])(int, char**);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int reserved:28;
        } bits;
        unsigned int raw;
    } status;
    
    /* Flexible array member */
    int flexible_array[];
};

/* 3. Union with deeply nested structures */
union MegaUnion {
    /* Struct with function pointer array */
    struct {
        int (*comparators[5])(const void*, const void*);
        char* (*allocators[2])(size_t);
    } func_table;
    
    /* Another struct with nested parentheses */
    struct {
        void (*(*nested_signal)(int, void (*)(int)))(int);
        int (*(*recursive_ptr)[10])(int (*)(int));
    } signal_handlers;
    
    /* Array of pointers to arrays */
    int *(*(*array_ptr_array)[5])[10];
};

/* 4. Typedef with complex bracketing */
typedef int (*(*ComplexCallback)(int (*)(char[10]), void*))[5];

/* 5. Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(int) int_func_ptr;
    ARRAY_DECL(char*, 20) string_array;
    NESTED_PTR(double) crazy_ptr;
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(32), packed)) AttributedStruct {
    int data __attribute__((aligned(16)));
    void (*func_ptr)(void) __attribute__((nonnull(1)));
    
    /* Nested struct with attribute */
    struct __attribute__((deprecated)) OldStruct {
        int legacy_field;
    } old __attribute__((unused));
};

/* 7. Single declaration combining all bracket types */
struct UltimateTest {
    /* Combination 1: Function pointer with array and nested function */
    void (*(*func_array[3])(int, void (*)(int[2])))(char*);
    
    /* Combination 2: Pointer to array of function pointers */
    int (*(*(*triple_ptr))(int))[5];
    
    /* Combination 3: Anonymous struct with bit-fields and array */
    struct {
        unsigned int:4;
        unsigned int flags:8;
        unsigned int:20;
        int values[10];
    } anonymous;
    
    /* Combination 4: Flexible array of pointers to functions */
    void (*(flexible_funcs[]))(int, ...);
    
    /* Combination 5: Nested union with all bracket types */
    union {
        int (*simple)(int);
        struct {
            char (*(*nested)[5])(void);
            int matrix[2][3];
        } complex;
    } choice;
};

/* 8. More edge cases */
typedef struct EdgeCases {
    /* Empty parentheses (function with no args) */
    void (*no_args)(void);
    
    /* Pointer to array */
    int (*ptr_to_array)[10];
    
    /* Array of pointers */
    int *array_of_ptrs[5];
    
    /* Function returning pointer to array */
    int (*returns_ptr_to_array(int size))[10];
    
    /* Volatile and const qualifiers with brackets */
    volatile const int (*(*volatile_const_ptr)(void))[5];
    
    /* __attribute__ with multiple parentheses */
    int heavily_attributed __attribute__((__aligned__(64), __vector_size__(16)));
} EdgeCases_t;

/* 9. Recursive structure definition */
struct TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    int (*(*data_processor)(struct TreeNode*))(void);
    union {
        int value;
        void (*action)(struct TreeNode*);
    } content;
};

/* 10. Template-like macro for maximum complexity */
#define ULTIMATE_TYPE(T) \
    struct Ultimate_##T { \
        T (*(*get_processor)(T (*)(T[10])))(T); \
        union { \
            T array[5][5]; \
            struct { \
                T (*func_ptr)(T, ...); \
                T nested[3]; \
            } nested_struct; \
        } data; \
        __attribute__((aligned(64))) T aligned_field; \
    }

/* Instantiate the complex template */
ULTIMATE_TYPE(int);
ULTIMATE_TYPE(double);
ULTIMATE_TYPE(char*);

/* 11. Function declarations with complex parameters */
extern void register_callback(
    int (*(*callback_provider)(void))(int, char**),
    void (*error_handler)(const char*, ...)
) __attribute__((nonnull(1)));

extern struct UltimateTest* create_test_instance(
    int size,
    void (*(*initializer)[size])(int)
);

/* 12. Inline function with attribute */
static inline __attribute__((always_inline)) 
int (*(*get_processor_factory(void))[5])(int) {
    return 0;
}

/* 13. Variable declarations using complex types */
extern ComplexCallback global_callback;
extern struct UltimateTest ultimate_instance;
extern EdgeCases_t *edge_case_ptrs[10];

/* 14. One final mega-struct with everything */
struct FinalBoss {
    /* Level 1: Direct nesting */
    struct {
        /* Level 2: Function pointer with array parameter */
        int (*level2_func)(int array[10]);
        
        /* Level 2: Union with bit-fields */
        union {
            struct {
                unsigned int:16;
                unsigned int field:8;
                unsigned int:8;
            } bits;
            int raw;
        } level2_union;
    } level1;
    
    /* Pointer to anonymous struct */
    struct {
        int x;
        int y;
        /* Level 3: Array of function pointers returning pointers to arrays */
        int (*(*level3_array[3])(void))[5];
    } *anonymous_ptr;
    
    /* The ultimate challenge: all brackets in one declaration */
    void (*(*(*ultimate_challenge)[5])(int (*(*)(char[10]))(double), 
                                        struct { int a; int b; }))
         (int, ...) __attribute__((warn_unused_result));
};

#endif /* GENGTYPE_COVERAGE_TEST_H */
