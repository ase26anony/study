/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* Preprocessor macros that expand to delimiter-containing expressions */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_SPEC __attribute__((aligned(16)))
#define PACKED_STRUCT __attribute__((packed)) struct
#define COMPLEX_EXPR (1 << 2) | (3 & 4)

/* Trigger default case with unusual characters in type definitions */
typedef int (*func_ptr1)(int a, /* comment with () inside */ int b);
typedef int (*func_ptr2)(int a, \
                         int b);  /* line continuation inside parentheses */

/* Complex nested delimiter structure */
struct level1 {
    /* Mix all delimiter types in single declaration */
    int (*complex_array_ptr[((2+3)*4)])[5][6];
    
    /* Nested anonymous union with attributes */
    union {
        struct {
            long long bitfield : 8 /* unusual : character */;
            unsigned char data[256];
        } ALIGN_SPEC;
        
        /* Function pointer with attributes inside struct */
        void (*signal_handler)(int sig, 
                               void* context 
                               __attribute__((unused))) __attribute__((noreturn));
    };
    
    /* Array with complex size expression containing parentheses */
    double matrix[ARRAY_SIZE(((int[]){1,2,3,4}))][10];
};

/* Interdependent type definitions with GCC extensions */
typedef union PACKED_STRUCT {
    /* Nested struct with bitfields and array */
    struct {
        unsigned int flags : 4;
        unsigned int : 4;  /* unnamed bitfield - unusual syntax */
        int values[COMPLEX_EXPR];
    } bits;
    
    /* Vector type (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vector_data;
    
    /* Pointer to function returning pointer to array */
    char (*(*callback)(void))[10];
} packed_union_t;

/* Even more complex type with deeply nested delimiters */
typedef struct outer {
    /* Member with all three delimiters in declaration */
    void (*(*signal_table[/* array comment */10])(int, ...))[3];
    
    /* Nested anonymous struct with macro expansion */
    struct {
        /* Attribute with parentheses inside array dimension */
        int aligned_buffer[16] ALIGN_SPEC;
        
        /* Pointer to array of function pointers */
        int (*(*func_array)[(2+3)])();
        
        /* Union containing struct with bitfields */
        union {
            struct {
                unsigned : 16;  /* more unusual syntax */
                signed value : 15;
            };
            int raw;
        } data;
    } inner;
    
    /* Macro with nested parentheses in type */
    typeof(((int*)0)[0]) type_of_expr;
} outer_t;

/* Function pointer type with attributes between parameters */
typedef int (__attribute__((const)) *math_func)(
    double x __attribute__((unused)),
    double y
) __attribute__((nonnull(1)));

/* Structure with array of complex function pointers */
struct callback_registry {
    /* Array where each element is a function pointer to a function
       that returns a pointer to an array of structs */
    struct outer* (*(*handlers[/* comment with [ ] */8])(
        int event_id,
        void* data
    ))[];
    
    /* Nested switch-like syntax in comments to potentially confuse parser */
    /* switch(state) { case 0: break; default: advance(); } */
    
    /* Bitfield with complex expression */
    unsigned int status : (sizeof(int)*8 - 4);
};

/* Typedef with all delimiter types mixed */
typedef void (*(*complex_type)(
    struct outer** param1[][10],
    int (*param2)(int, int[])
))[][(2+3)*4];

/* Union with anonymous struct containing nested parentheses */
union final_union {
    struct {
        /* Function pointer with nested attributes */
        __attribute__((deprecated)) 
        void (*old_func)(
            int,
            __attribute__((may_alias)) char*
        );
        
        /* Multi-dimensional array with computed size */
        int matrix3d[ARRAY_SIZE(((int[][2]){{1,2},{3,4}}))][2][2];
    };
    
    /* Another complex pointer type */
    complex_type ct_ptr;
};

/* Global variable using complex type */
static packed_union_t global_var = { .bits = { .flags = 0xF } };

/* Additional type to ensure multiple parsing passes */
enum {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    VALUE_C = (1 << 2) | (1 << 3)  /* expression with | operator */
} __attribute__((packed));

/* Final complex declaration with everything mixed */
__attribute__((visibility("default")))
extern struct callback_registry* (*(*get_registry)(void))[];
