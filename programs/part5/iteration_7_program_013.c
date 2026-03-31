/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define MAYBE_UNUSED __attribute__((unused))

/* 2. Complex nested type with all delimiter types */
typedef int (*complex_func_ptr_t)(
    struct inner_struct **, /* Nested pointer */
    int (*)(char, float),   /* Function pointer parameter */
    int array[][10],        /* 2D array */
    ...                     /* Variadic - triggers default case */
) ALIGNED_ATTR;

/* 3. Struct with deeply nested delimiters and unusual characters */
struct outer_struct {
    /* Default case triggers: numeric constant with decimal point */
    float f1 = 3.14159;  /* GCC extension - initializer in struct */
    
    /* Nested parentheses in bit-field */
    unsigned int bf : (sizeof(int) * 8 - 1);
    
    /* Array with complex size calculation */
    char dynamic_array[ARRAY_SIZE(((int[]){1,2,3,4}))];
    
    /* Union inside struct with attributes */
    union {
        struct {
            /* Function pointer array */
            void (*callbacks[5])(int, float);
            
            /* Nested array of pointers to functions */
            complex_func_ptr_t (*func_table[3][2])();
        } nested_struct;
        
        /* Anonymous struct with line continuation */
        struct { \
            int x; \
            /* Comment between tokens triggers default case */ \
            int y; /* Another comment */ \
        } PACKED_ATTR;
    } data_union;
    
    /* Pointer to array of structs */
    struct inner_struct (*ptr_array)[10];
    
    /* GNU attribute with parentheses */
    int special_value __attribute__((deprecated("Use new_value instead")));
};

/* 4. Type with macro expansions inside delimiter sequences */
typedef struct {
    /* Macro expands to attribute with parentheses */
    char buffer[256] ALIGNED_ATTR;
    
    /* Complex array dimension with macro and arithmetic */
    int matrix[ARRAY_SIZE(((int[]){1,2})) + 2][ARRAY_SIZE(((int[]){3,4,5}))];
    
    /* Nested function pointer with attribute */
    void (*signal_handler)(int sig, 
                          void (*old_handler)(int) /* Nested function pointer */
                          ) __attribute__((noreturn));
} container_t;

/* 5. Enum with computed values (triggers default case for operators) */
enum complex_enum {
    VAL_A = 1 << 0,     /* Bit shift operator */
    VAL_B = 1 << 1,
    VAL_C = (VAL_A | VAL_B),  /* Binary operator */
    VAL_D = sizeof(struct outer_struct),  /* sizeof operator */
    VAL_E = __LINE__     /* Predefined macro */
};

/* 6. Union with anonymous struct and unusual characters */
union mixed_union {
    /* Vector type (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vector_data;
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned : 4;  /* Unnamed bitfield */
        unsigned flag1 : 1;
        unsigned flag2 : 1;
        /* Line continuation in bitfield declaration */
        unsigned long \
        reserved : 26;
    };
    
    /* Array of function pointers */
    int (*operations[5])(union mixed_union *, int);
};

/* 7. Extremely complex typedef with all delimiter types mixed */
typedef union {
    struct {
        /* Nested array of pointers to functions returning pointers to arrays */
        int (*(*(*callchain[3])())[10])(float, double);
        
        /* Pointer to array of structs containing unions */
        struct {
            union {
                int i;
                float f;
                void *p;
            } value;
            char tag;
        } (*variant_array)[];
    } level1;
    
    /* Direct declaration with all delimiters */
    void (*(*signal_handlers[5])(int, ...))(int);
} ultimate_type_t;

/* 8. Struct with __attribute__ containing string literal */
struct attributed_struct {
    /* Attribute with string literal (contains quotes) */
    const char *message __attribute__((access(read_only, 1, 2))) = "Test\"string\"with\\escapes";
    
    /* Designated initializer (GCC extension in type context) */
    int point[2] = { .x = 1, .y = 2 };  /* Triggers default case for '.' */
    
    /* Nested anonymous union with packed attribute */
    union {
        struct {
            short s;
            char c;
        } PACKED_ATTR;
        int i;
    };
} MAYBE_UNUSED;

/* 9. Function pointer type with nested attributes */
typedef void (*(*factory_func)(
    const char *name __attribute__((nonnull)),  /* Attribute on parameter */
    /* Variadic argument with attribute */
    ... __attribute__((sentinel))
))() __attribute__((warn_unused_result));

/* 10. Final complex declaration mixing everything */
static volatile ultimate_type_t *global_var 
    __attribute__((section(".data"))) 
    __attribute__((aligned(32))) = 
    &(ultimate_type_t){ 
        .level1 = { 
            .callchain = { NULL, NULL, NULL } 
        } 
    };

/* Note: No main function needed - gengtype only parses type definitions */
