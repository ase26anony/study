/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* First, define macros that expand to complex delimiter sequences */
#define ARRAY_DIM(x) [x * 2 + 1]
#define ATTR_PACKED __attribute__((packed))
#define FUNC_PTR(name) (*name)
#define NESTED_EXPR (1 << 2 | 3)

/* Preprocessor directive inside a comment-like context */
#if 0
/* This won't be compiled but will be seen by the preprocessor */
#define UNUSED_MACRO
#endif

/* Complex typedef with all delimiter types */
typedef int (*callback_func_t)(
    /* Default case trigger: numeric constant inside parentheses */
    42,
    /* Nested parentheses with unusual character */
    (int)(*)(char **, ...),
    /* Array dimension with calculation */
    int ARRAY_DIM(5)
) ATTR_PACKED;

/* Struct with deeply nested delimiter sequences */
struct container {
    /* Function pointer member with attributes */
    void (*operation)(
        struct container *self,
        /* Nested array parameter */
        int matrix[3][4],
        /* GNU attribute inside parameter list */
        int flags __attribute__((aligned(8)))
    ) ATTR_PACKED;
    
    /* Union with bit-fields and array */
    union {
        struct {
            /* Bit-field with complex expression */
            unsigned int flags : NESTED_EXPR;
            /* Array of function pointers */
            int (*handlers[5])(void);
        } bits;
        
        /* Anonymous struct with nested parentheses */
        struct {
            /* Pointer to array */
            int (*array_ptr)[10];
            /* Multi-dimensional array with calculation */
            double grid[ARRAY_DIM(2)][3];
        };
    } data;
    
    /* Nested struct declaration */
    struct {
        /* Line continuation inside default case context \
           (backslash triggers default case) */
        long \
        value;
        
        /* Complex attribute syntax */
        char *buffer __attribute__((aligned(16), packed));
    } inner;
};

/* Another complex type with mixed delimiters */
typedef union variant ATTR_PACKED {
    /* Array of structs containing function pointers */
    struct {
        int (*compare)(const void *, const void *);
        void (*destroy)(void *);
    } ops[10];
    
    /* Nested parentheses in type cast expression */
    int (*((*complex_ptr)))(char);
    
    /* Empty braces with comment (triggers default case for '/') */
    struct {} /* empty */;
} variant_t;

/* Enum with computed values */
enum states {
    IDLE = 0,
    /* Expression with parentheses */
    ACTIVE = (1 << 0),
    /* Expression with brackets (array size calc) */
    BUSY = sizeof(int[10]),
    /* Default case trigger: floating point in enum value */
    ERROR = 0xFF
};

/* Extremely complex declaration on one line */
static const struct {
    int (*((*((*nested_func_ptr[2]))(int[][5]))(void)))(char *);
    union {
        struct {
            int x;
        } s;
        long l;
    } u[sizeof(int*) == 8 ? 10 : 5];
} global_var = {0};

/* Function pointer type with GNU extension */
typedef void (*__gnuc_va_list)(
    /* Varargs with attribute */
    __attribute__((aligned(__BIGGEST_ALIGNMENT__))) ...
);

/* Struct using vector extension (GCC-specific) */
typedef struct {
    /* Vector type with parentheses in size */
    int v __attribute__((vector_size(16 * sizeof(int))));
    /* Nested switch-case like syntax in comment */
    /* switch(state) { case 1: break; default: advance(); } */
} vector_struct;

/* Macro expansion creating complex type */
#define CREATE_COMPLEX_TYPE(name, size) \
    typedef struct name { \
        int (*callback)(int (*)(int[size]), ...); \
        char data[size]; \
    } name##_t

/* Use the macro */
CREATE_COMPLEX_TYPE(my_type, 100);

/* Final complex declaration with everything mixed */
volatile const struct root_type {
    /* Nested anonymous union-struct */
    union {
        struct {
            /* Pointer to function returning pointer to array */
            int (*(*get_matrix)(void))[10][20];
            /* __attribute__ with parentheses */
            unsigned char pad __attribute__((aligned(32)));
        };
        /* Array of complex pointers */
        void *(*ptr_array[5])(int, ...);
    };
    
    /* Bit-field with macro expansion */
    unsigned int mode : ARRAY_DIM(1)[0];
    
    /* Conditional expression in array size */
    double samples[sizeof(long) == 8 ? 1024 : 512];
    
    /* Function pointer with nested attributes */
    void (* __attribute__((noreturn)) fatal_error)(
        const char *msg __attribute__((nonnull)),
        /* Comment inside parameter list */ /* line 1 \
           continued line */ \
        int code
    );
} *global_root_ptr;

/* Trigger default case with various characters */
struct char_test {
    /* These characters in comments/strings trigger default case: */
    /* ~!@#$%^&*_+-=|\:;"'<>,.?/` */
    char *str = "string with \backslash and \x7F";
    int hex = 0xDEADBEEF;
    float fp = 3.14159e-10;
};

/* Multiple levels of nested parentheses */
typedef int (*(*(*deep_nested)(int (*(*)(int[3]))[2]))(void))(char **);

/* Empty file ending with backslash (triggers default case) \
*/
