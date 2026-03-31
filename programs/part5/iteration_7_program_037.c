/* test-gengtype-parser.c - Complex type definitions to test gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR(sect) __attribute__((section(#sect)))

/* Macro that expands to a complex array dimension with arithmetic */
#define COMPLEX_DIM (1 << 3) /* 8, with comment inside */

/* Trigger default case: unusual characters in macro body */
#define WEIRD_CHARS "/* not a comment start */" \
    "line continuation with backslash \\" \
    "and numeric 123.456e-7"

/* Complex nested type definition 1 */
typedef int (*func_ptr_t)(
    /* Default case trigger: comment between parentheses */
    int arg1, /* comment */
    char *arg2, /* another comment */
    /* Nested parentheses in function pointer argument */
    void (*callback)(int, char)
) ALIGNED_ATTR;

/* Type with all three delimiters deeply nested */
struct outer_struct {
    /* Braces */
    struct {
        /* Parentheses in bit-field */
        unsigned int flags : (sizeof(int) * 8 - 1);
        /* Brackets in array */
        int matrix[3][(2 + 1)];
    } inner PACKED_ATTR;
    
    /* Function pointer with attributes containing parentheses */
    void (*handler)(
        struct outer_struct *self,
        /* Array pointer parameter */
        int (*)[COMPLEX_DIM]
    ) SECTION_ATTR(.text);
    
    /* Union with nested struct containing all delimiters */
    union {
        struct {
            /* Parentheses in sizeof */
            char data[sizeof(long double)];
            /* Nested function pointer */
            func_ptr_t (*get_func)(void);
        } s;
        /* Array of function pointers */
        int (*arr_funcs[((1 << 2) + 1)])(int, char);
    } u;
    
    /* GNU extension: vector type with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
} SECTION_ATTR(.data);

/* Even more complex typedef with mixed delimiters */
typedef struct {
    /* Anonymous union with bitfields */
    union {
        struct {
            unsigned int a : (8 - 1);
            unsigned int b : ((1 << 3) | 1);
        } bits;
        long long value;
    };
    
    /* Pointer to array of function pointers returning struct pointers */
    struct outer_struct *(*(*callbacks)[
        /* Macro expansion with arithmetic */
        ARRAY_SIZE(((int[]){1,2,3,4}))
    ])(
        /* Parameters with default-case triggers */
        int, /* comment between parameters */
        char, /* another */
        ... /* ellipsis */
    );
    
    /* Nested array with computed size */
    double coords[3][
        /* Parentheses with arithmetic */
        (int)(sizeof(double[2]) / sizeof(double))
    ];
} complex_t;

/* Function pointer type with attribute containing parentheses */
typedef void (__attribute__((stdcall)) *stdcall_func_t)(
    int,
    /* Default case: numeric constant with exponent */
    1.234e-5f,
    /* String literal with escaped chars */
    "string with \"quotes\" and \\backslash"
);

/* Union with __attribute__ containing parentheses and unusual chars */
union weird_union {
    /* Attribute with parentheses inside parentheses */
    int x __attribute__((deprecated("use y instead")));
    float y;
    /* Bitfield with parentheses */
    unsigned z : (sizeof(int) * 8);
} __attribute__((aligned((1 << 4))));

/* Struct with macro-expanded array dimensions */
struct macro_struct {
    /* Multiple macro expansions */
    int data[COMPLEX_DIM][ARRAY_SIZE(((int[]){1,2}))];
    /* Function pointer with macro in attribute */
    void (*func)(void) ALIGNED_ATTR;
};

/* Typedef creating circular reference with function pointer */
typedef struct node node_t;
struct node {
    node_t *next;
    /* Function pointer taking pointer to self */
    void (*process)(node_t *self, 
        /* Comment to trigger default case */
        int mode /* mode parameter */
    );
    /* Flexible array member with attribute */
    char data[] PACKED_ATTR;
};

/* Enum with computed values in parentheses */
enum flags {
    FLAG_A = (1 << 0),
    FLAG_B = (1 << 1),
    /* Expression with multiple parentheses */
    FLAG_C = ((1 << 2) | (1 << 3)),
    FLAG_D = (int)(sizeof(long) - 1)
};

/* Final complex declaration mixing everything */
typedef union {
    struct {
        /* Nested anonymous struct */
        struct {
            /* All three delimiters in one member */
            int (*(*func_array)[
                (2 * 2)
            ])(
                struct { int a; char b; } *,
                /* Array parameter */
                int arr[static 3]
            );
        };
        /* Vector type with attribute */
        typedef float v8sf __attribute__((vector_size(32)));
        v8sf vector;
    };
    /* Function pointer with GNU extension: nested functions reference */
    void (*cleanup)(void (*)(void));
} ultimate_t;

/* Trigger default case with line continuations in type definition */
typedef struct { \
    int x; \
    char y; \
} continued_struct_t;

/* Multiple type definitions to ensure repeated parsing */
typedef int simple_t;
typedef simple_t *ptr_t;
typedef ptr_t (*func_ret_ptr_t)(void);
typedef func_ret_ptr_t array_of_funcs_t[10];

/* End of complex type definitions */
