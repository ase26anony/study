/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define FUNC_ATTR __attribute__((noinline))
#define NESTED_MACRO(x) [(x) + 1] /* Macro expands to brackets */

/* 2. Complex typedef with all delimiter types */
typedef int (*complex_func_ptr_t)(
    /* Default case trigger: numeric constant inside parentheses */
    42,
    /* Nested parentheses */
    void (*nested_callback)(char **argv[]),
    /* Brackets inside parentheses */
    int matrix[][ARRAY_DIM],
    /* Braces would be invalid here, so use attribute with parentheses */
    struct { int a; double b; } *anonymous_struct_ptr
) FUNC_ATTR;

/* 3. Struct with deeply nested delimiter sequences */
struct outer_struct {
    /* Function pointer member with attributes */
    void (*operation)(
        /* Line continuation inside parentheses - triggers default case */
        int param1, \
        char param2,
        /* Comment inside delimiter sequence */
        /* This comment should trigger default case */
        float param3
    ) __attribute__((aligned(16)));
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_array[ARRAY_DIM])())[];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flags : 4;
            /* Attribute with complex expression */
            int data __attribute__((aligned(8 /* embedded comment */)));
        } s;
        /* Array dimension using macro with parentheses */
        long buffer[NESTED_MACRO(3)];
    } u;
    
    /* GNU extension: vector type in nested context */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
};

/* 4. Intermixed delimiter types in single declaration */
enum {
    /* Enum with computed values containing parentheses */
    VALUE_A = (1 + 2) * 3,
    VALUE_B = sizeof(struct { int x; char y; }),
    /* Default case trigger: floating constant */
    VALUE_C = 3.14
} my_enum;

/* 5. Type definition with macro expansions inside nested delimiters */
typedef struct container {
    /* Macro expands to brackets with parentheses inside */
    int items NESTED_MACRO(5);
    
    /* Pointer to function with attribute containing parentheses */
    void (*cleanup)(void) __attribute__((deprecated("Use destroy() instead")));
    
    /* Nested struct with array of function pointers */
    struct {
        /* Multiple attributes */
        int (*handlers[4])(int, char **) 
            __attribute__((warn_unused_result)) 
            __attribute__((nonnull(2)));
    } nested;
} container_t;

/* 6. Union with all delimiter types mixed */
union mixed_delimiters {
    /* Array of pointers to functions returning struct pointers */
    struct inner_struct *(*(*callbacks[3])(
        /* Default case: special characters in string literal */
        const char *msg = "test{with[delimiters]inside}",
        /* Default case: character constant */
        char delim = '('
    ))[];
    
    /* Function pointer with nested parameter */
    int (*processor)(
        int (*transform)(int input, 
            /* Comment with unusual characters: <>&|^~ */
            void *context /* context pointer */),
        /* Attribute in parameter list */
        void *data __attribute__((aligned(32)))
    );
    
    /* Anonymous struct with bit-field containing expression */
    struct {
        unsigned int config : (sizeof(int) * 8 - 1);
        /* Array with size from enum */
        float samples[VALUE_A];
    };
};

/* 7. Complex function pointer type definition */
typedef void (*(**signal_handler[2])(
    /* Nested parentheses with operators */
    int sig, 
    void (*old_handler)(int),
    /* Empty parentheses then brackets */
    char info[] 
))(
    /* Second parameter list (for returned function pointer) */
    void *context,
    /* Default case: backslash continuation */
    int options\
);

/* 8. Struct with GNU statement expression in bit-field (GCC extension) */
struct with_gnu_extensions {
    /* Traditional bit-field */
    int normal_bitfield : 4;
    
    /* __extension__ to suppress warnings */
    __extension__ unsigned long long large_bitfield : 48;
    
    /* Attribute on bit-field */
    unsigned int aligned_field : 8 __attribute__((aligned(8)));
    
    /* Array in struct */
    volatile int volatile_array[2][3];
};

/* 9. Recursive type definition */
typedef struct tree_node tree_node_t;
struct tree_node {
    tree_node_t *left;
    tree_node_t *right;
    /* Function pointer with complex return type */
    int (*compare)(const tree_node_t *, const tree_node_t *);
    /* Flexible array member */
    char data[];
};

/* 10. Final complex declaration mixing everything */
__attribute__((packed))
struct ultimate_test {
    /* All three delimiters in one member declaration */
    int (*(*array_of_func_ptrs[ 
        /* Size expression with parentheses */
        (ARRAY_DIM > 2) ? ARRAY_DIM : 2 
    ])(
        /* Parameter with default value (GCC extension) */
        struct ultimate_test *self __attribute__((unused)) = 0
    ))[];  /* Returns pointer to array */
    
    /* Nested anonymous struct */
    struct {
        /* Union with bit-fields */
        union {
            int x;
            /* Default case: hex constant */
            long y : 0x10;
        } u;
        
        /* Macro used in array dimension */
        double matrix[2][NESTED_MACRO(1)];
    } inner;
    
    /* Empty declaration with just semicolon - triggers default case */
    ;
} ultimate_test_t;

/* 11. Additional edge cases */
/* Preprocessor directive inside would be invalid, so we use attributes */
struct __attribute__((aligned( 
    /* Nested parentheses in attribute argument */
    (sizeof(long) > 4) ? 16 : 8 
))) aligned_struct {
    /* Member with multiple attributes */
    char *ptr __attribute__((nonnull, 
        /* Nested attribute */
        aligned(32))) 
        __attribute__((deprecated));
};

/* 12. Type definition containing backslash continuation */
typedef unsigned \
long \
long very_long_type;

/* 13. Array with computed size containing all delimiter types */
char computed_array[
    sizeof(struct { 
        int a[3]; 
        char b[(2 + 3)]; 
        double c; 
    }) + 
    (sizeof(int[2]) * 3)
];

/* The file ends with type definitions only - no main function needed */
/* gengtype will parse these type definitions and exercise the parser */
