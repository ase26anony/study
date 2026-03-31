/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2)  /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16)))
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)
#define NESTED_MACRO(x) { .field = (x) }

/* 2. Complex typedef with all delimiter types */
typedef int (*complex_func_t)(
    struct inner **ptr_array[ARRAY_DIM],  /* Mix of *, [], and () */
    union {
        long long ll;
        double d;
    } ATTR_ALIGN  /* Attribute inside function arguments */
);

/* 3. Struct with deeply nested delimiter sequences */
struct outer_struct {
    /* Default case trigger: numeric constant inside struct */
    int flags : 3;  /* Bit-field with colon (unexpected char) */
    
    /* Nested anonymous union with attributes */
    union ATTR_ALIGN {
        /* Function pointer member with complex return type */
        char *(*callback)(
            int param1, 
            /* Comment inside parentheses triggers default case */
            float param2[2][2]  /* Multi-dimensional array */
        ) ATTR_ALIGN;
        
        /* Array of pointers to functions */
        void (*func_array[3])(
            /* Line continuation inside parentheses: \
               This backslash should trigger default case */
            struct { 
                int x; 
                /* Nested braces with attribute */
                char y __attribute__((packed)); 
            } *
        );
    } data;
    
    /* Mixed delimiters in single declaration */
    int (*matrix_ptr)[ARRAY_DIM][ARRAY_DIM + 1];  /* Arithmetic in array dim */
};

/* 4. Union with GNU extensions and macro expansions */
union __attribute__((transparent_union)) gnu_union {
    /* Vector type extension */
    typedef int v4si __attribute__((vector_size(16)));
    
    /* Nested struct with all delimiter types */
    struct {
        /* Complex declarator: pointer to array of function pointers */
        void (*(*signal_handlers)[10])(int, ...);
        
        /* Macro expansion creating braces */
        int initialized NESTED_MACRO(42);
    } nested;
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned int a:1, b:2, c:3;
        /* Default case: comma separated bitfields */
    };
};

/* 5. Function pointer type with attributes between delimiters */
typedef void (ATTR_ALIGN *handler_func_t)(
    /* Multiple attributes in parameter list */
    const char *msg __attribute__((nonnull(1))),
    ...  /* Ellipsis - unexpected token */
) __attribute__((noreturn));

/* 6. Extreme nesting example */
typedef struct {
    /* Triple nesting: function returning pointer to array of structs */
    struct level1 {
        struct level2 {
            struct level3 {
                int (*(*get_table)[/* Comment with operators: 1+2 */])(
                    char c,
                    /* Backslash line continuation: \
                       triggers default case */
                    float f
                );
            } *l3;
            int array[sizeof(struct level3*)];
        } (*l2)[5];
    } *l1;
} deeply_nested_t;

/* 7. Enum with complex initializers */
enum complex_enum {
    VALUE_A = (1 << 0),  /* Parentheses with shift operator */
    VALUE_B = (int){0},  /* Compound literal - braces */
    VALUE_C = sizeof(struct outer_struct[/* Empty brackets */]),
    VALUE_D = (int)((void*)0)  /* Multiple parentheses */
};

/* 8. Type with __builtin_offsetof containing nested delimiters */
typedef struct container {
    struct outer_struct os;
    union gnu_union gu;
    char raw_data[__builtin_offsetof(struct outer_struct, data)];
} container_t;

/* 9. Variable declarations using complex types */
complex_func_t global_func ATTR_ALIGN;
static deeply_nested_t *static_nested = &(deeply_nested_t){
    .l1 = &(struct level1){
        .l2 = &(struct level2[5]){ [0] = { .l3 = NULL } }
    }
};

/* 10. Final complex declaration mixing everything */
volatile const struct outer_struct* (*(*volatile complex_array)[10])(
    int,
    ...  /* Ellipsis in middle of declaration */
) = {
    [0] = NULL,
    /* Macro with line continuation \
       spanning multiple lines */
    [ARRAY_DIM - 1] = (void*)0
} ATTR_ALIGN;
