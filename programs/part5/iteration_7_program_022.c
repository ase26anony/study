/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR __attribute__((section(".data")))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro that expands to a complex array dimension with parentheses */
#define COMPLEX_DIM (1 + 2 * 3 - (4 >> 1))

/* Trigger default case: numeric constants and operators inside type contexts */
#define WEIRD_CONST 0xDEADBEEFUL
#define WEIRD_EXPR (1 << 8 | 3)

/* ========== Type Definitions with Nested Delimiters ========== */

/* Type 1: Function pointer with nested parentheses and attributes */
typedef void (*complex_func_ptr_t)(
    int param1, 
    /* Comment inside parentheses to trigger default case */
    char param2[/* weird comment */ COMPLEX_DIM],
    /* Line continuation inside parentheses - triggers default case */
    struct inner_struct {\
        int a;\
        char b;\
    } param3,
    /* Attribute with parentheses inside argument list */
    void (*nested_callback)(int, char) ALIGN_ATTR
) SECTION_ATTR;

/* Type 2: Struct with all delimiter types mixed */
struct outer_struct {
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            unsigned int flag3 : WEIRD_EXPR; /* Macro with operators */
        } bits;
        unsigned long value;
    } PACKED_ATTR;
    
    /* Array of function pointers */
    complex_func_ptr_t (*callbacks[/* size with comment */ 10])(
        /* Nested parameter with array */
        int matrix[3][4],
        /* Another function pointer parameter */
        void (**operations)(void)
    );
    
    /* Pointer to array with computed size */
    int (*dynamic_array)[ARRAY_SIZE(((int[]){1,2,3,4}))];
    
    /* GNU extension: vector type */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
    
    /* Nested struct with attribute containing parentheses */
    struct {
        int x ALIGN_ATTR;
        int y;
    } inner ALIGN_ATTR;
};

/* Type 3: Deeply nested type with all delimiters */
typedef union {
    /* Multi-dimensional array with complex dimensions */
    int (*deep_array[2][/* comment */3])[4][5];
    
    /* Function returning pointer to array of structs */
    struct deep_struct {
        /* Anonymous struct inside */
        struct {
            /* Bit-field with complex expression */
            unsigned bits : (8 * sizeof(int) - 1);
            /* Array in struct */
            char str[WEIRD_EXPR + 1];
        };
        
        /* Pointer to function with nested attributes */
        void (*(*signal_handler[3])(
            int sig, 
            /* Attribute in parameter */
            const char *msg __attribute__((nonnull))
        ))();
    } *(*get_deep_struct(void))[];
    
    /* Another union member with line continuation */
    long \
        long_var_name_with_continuation;
} ultra_complex_t MAYBE_UNUSED;

/* Type 4: Interdependent types with circular references */
struct node;
typedef struct node node_t;

struct graph {
    /* Forward declaration used in array */
    struct node *nodes[100];
    
    /* Function pointer using forward-declared type */
    void (*visit)(node_t *n, int depth);
    
    /* Nested struct with pointer to parent */
    struct iterator {
        struct graph *graph;
        node_t *current;
        
        /* Method-like function pointer */
        node_t *(*next)(struct iterator *self);
    } iter;
};

struct node {
    int id;
    /* Two-dimensional flexible array member */
    struct graph *edges[][10];
    
    /* Anonymous union with bit-fields and normal fields */
    union {
        struct {
            unsigned visited : 1;
            unsigned processed : 1;
            /* Remaining bits */
            unsigned : (sizeof(unsigned)*8 - 2);
        };
        unsigned int flags;
    };
};

/* Type 5: Extreme nesting with all delimiter types */
typedef int (*(*(*insane_nesting_t)[/* array size */5])(
    /* Parameter with nested array/function */
    void *(**handlers[3])(
        /* Another level */
        char data[256],
        /* Size parameter with expression */
        size_t len __attribute__((aligned(sizeof(void*))))
    ),
    /* Struct parameter */
    struct config {
        /* Array with attribute */
        int values[10] ALIGN_ATTR;
        /* Nested union */
        union {
            /* Function pointer in union */
            void (*action)(void);
            /* Array in union */
            char buffer[256];
        } u;
    } cfg
))[10])();

/* Type 6: Using __builtin types and attributes */
typedef __builtin_va_list va_list_t;
typedef __attribute__((transparent_union)) union {
    int i;
    float f;
    /* Pointer to function with ... */
    void (*func)(int, ...);
} transparent_union_t;

/* Type 7: Struct with all possible GNU extensions */
struct gnu_extensions {
    /* Vector types */
    typedef float v8sf __attribute__((vector_size(32)));
    v8sf vectors[2];
    
    /* Aligned attribute with expression */
    char aligned_buffer[256] __attribute__((aligned(32)));
    
    /* Packed struct inside */
    struct __attribute__((packed)) packed_struct {
        char a;
        int b;
        char c;
    } packed;
    
    /* Section attribute */
    int counter __attribute__((section(".persistent")));
    
    /* Cleanup attribute (triggers parentheses) */
    FILE *logfile __attribute__((cleanup(fclose)));
    
    /* Mode attribute */
    typedef int __attribute__((mode(SI))) int32_mode_t;
    int32_mode_t mode_var;
};

/* Type 8: Complex typedef with line continuations and comments */
typedef unsigned \
    int \
    (*callback_array_t[/* size with operators */ 1 + 2 * 3]) \
    ( \
        /* Parameter with array of pointers */ \
        void *args[], \
        /* Size parameter with bit operations */ \
        size_t count __attribute__((aligned(8))) \
    );

/* Type 9: Enum with complex expressions in values */
enum weird_enum {
    VALUE_A = 1 << 0,
    VALUE_B = 1 << 1,
    VALUE_C = (1 << 2) | (1 << 3),
    VALUE_D = WEIRD_CONST & 0xFF,  /* Macro with hex constant */
    VALUE_E = sizeof(struct outer_struct) / 2
};

/* Type 10: Final complex type mixing everything */
typedef struct {
    /* Nested anonymous struct */
    struct {
        /* Function pointer returning pointer to array */
        int (*(*get_matrix)(int rows, int cols))[];
        
        /* Union with bit-fields and array */
        union {
            struct {
                unsigned : 4;  /* Unnamed bit-field */
                unsigned field1 : 4;
                unsigned field2 : 8;
            };
            unsigned short all_fields;
            char bytes[2];
        } PACKED_ATTR;
    };
    
    /* Pointer to function with nested attributes */
    void (*(*signal)(int, ...))() 
        __attribute__((format(printf, 2, 3))) 
        __attribute__((nonnull(1)));
    
    /* Flexible array member with attribute */
    long data[] ALIGN_ATTR;
} ultimate_type_t;

/* Global variables using these complex types */
static complex_func_ptr_t global_func_ptr = 0;
static struct outer_struct global_struct = {0};
static ultimate_type_t *global_ultimate_ptr = 0;
static enum weird_enum global_enum = VALUE_A;

/* Additional tricky cases */

/* Macro that expands to something with all delimiters */
#define TRICKY_TYPE(type) \
    typedef struct { \
        type (*process)(type[], int (*compare)(type, type)); \
        type data[COMPLEX_DIM]; \
    } tricky_##type##_t

/* Instantiate the macro */
TRICKY_TYPE(int);
TRICKY_TYPE(double);

/* Type with __attribute__ containing parentheses and commas */
typedef int (special_func_t)(int, char**)
    __attribute__((warning("This is a special function")))
    __attribute__((deprecated("Use new_func instead")));

/* Array type with size containing parentheses and operators */
typedef char buffer_t[(1 + 2) * (3 + 4) - sizeof(int)];

/* Struct with member initialized to expression containing parentheses */
struct with_initializer {
    int size;
    char *name;
} global_with_init = {
    .size = (int)(sizeof(struct outer_struct) + 100),
    .name = (char[]){'t', 'e', 's', 't', '\0'}
};

/* Final type: everything combined */
typedef struct {
    /* All previous types as members */
    tricky_int_t tricky_int;
    tricky_double_t tricky_double;
    buffer_t buffer;
    special_func_t *special_func;
    
    /* Nested anonymous union with bit-fields spanning multiple lines */
    union {
        struct {
            unsigned bit_field_with_continuation \
                : 8;
            unsigned another_bit_field \
                : 8;
        };
        unsigned short combined;
    };
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*(*func_array[3])(void))[10])(int, int);
} mega_composite_t;

/* End of complex type definitions */
