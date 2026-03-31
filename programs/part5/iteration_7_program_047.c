/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR(sect) __attribute__((section(sect)))

/* Macro that expands to a complex array dimension with operators */
#define COMPLEX_DIM (1 << 3) /* This comment inside macro def */

/* Trigger default case: unusual characters in type definitions */
#define WEIRD_CHARS "/* Not a comment start in string */"

/* Level 1: Basic nested structures */
struct level1 {
    int a;
    struct {
        /* Default case trigger: numeric constant with decimal */
        float b[3.14e-10 ? 10 : 20]; /* Mixed [] and () in dimension */
        char c;
    } inner;
    long d;
};

/* Level 2: Function pointers with attributes */
typedef void (*callback_func_t)(int, char **) ALIGNED_ATTR;

/* Level 3: Nested function pointer in struct */
struct has_funcptr {
    /* Default case: line continuation inside type */
    int (*complex_func)(struct level1 *, \
                       callback_func_t);
    /* Attribute with parentheses inside struct */
    unsigned flags : 4 PACKED_ATTR;
};

/* Level 4: Mixed delimiters in single declaration */
typedef struct {
    /* All three delimiters: {} for struct, () for function, [] for array */
    union {
        struct has_funcptr *(*get_funcptr)(int index);
        void (*set_funcptr)(struct has_funcptr *, int);
    } ops[10];
    
    /* Default case: numeric constant with unusual format */
    double data[0x1FUL + 1];
    
    /* Nested anonymous struct with bitfield */
    struct {
        unsigned int : 16; /* Unnamed bitfield */
        signed int field : 8;
    } PACKED_ATTR;
} mixed_delimiters_t;

/* Level 5: Deeply nested with GCC extensions */
typedef mixed_delimiters_t *(*factory_func[
#ifdef __GNUC__
    /* Conditional compilation inside array size */
    COMPLEX_DIM
#else
    8
#endif
])(int count, ...) /* Variadic function */ SECTION_ATTR(".text.factories");

/* Level 6: Complex array with computed size */
struct with_complex_array {
    /* Default case: preprocessor directive result */
    char buffer[ARRAY_SIZE(((int[]){1,2,3,4}))]; /* Compound literal */
    
    /* Nested function pointer returning pointer to array */
    int (*(*get_matrix)(void))[][10];
    
    /* Union with anonymous struct containing array of function pointers */
    union {
        struct {
            factory_func items;
            /* Default case: backslash in comment (not line continuation) */
            /* This is a comment with \ backslash character */
            int count;
        };
        void *raw;
    };
};

/* Level 7: The ultimate challenge - all delimiters intertwined */
typedef union ultimate_challenge {
    /* {} for union */
    struct {
        /* () for function pointer */
        mixed_delimiters_t *(*(*callbacks[3])(void))(
            /* Parameters with attributes */
            struct with_complex_array *arr
            __attribute__((nonnull(1))),
            /* Array parameter with size */
            int matrix[static 4][4]
        );
        
        /* [] for array of structs containing unions */
        struct {
            union {
                int x;
                double y;
            } data[((2 + 3) * 4) / 2]; /* Expression in array size */
        } items[10];
    };
    
    /* Default case: string literal with special chars */
    char metadata[]; /* Flexible array member */
} ultimate_challenge_t;

/* Level 8: Multiple typedefs with nested attributes */
typedef ultimate_challenge_t **(*complex_meta_func)(
    int,
    /* Nested attribute */
    struct __attribute__((designated_init)) {
        int a;
        int b;
    } *config
) __attribute__((warn_unused_result));

/* Level 9: Template-like macro usage */
#define DECLARE_CONTAINER(type, name) \
    struct container_##name { \
        type *data; \
        int (*compare)(const type *, const type *); \
        void (*print)(const type *, FILE *); \
    }

/* Instantiate macros that create complex types */
DECLARE_CONTAINER(struct with_complex_array, complex_array);
DECLARE_CONTAINER(ultimate_challenge_t, ultimate);

/* Level 10: Forward declarations with attributes */
struct forward_decl __attribute__((visibility("hidden")));
typedef struct forward_decl *(*forward_factory)(void)
    __attribute__((malloc));

/* Final: A type using everything */
typedef struct {
    /* Array of function pointers returning pointers to arrays */
    complex_meta_func (*meta_handlers[])(
        struct container_complex_array *,
        struct container_ultimate *
    );
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 1;
            /* Default case: comment with * and / inside */
            unsigned int reserved : 30; /* Reserved / for future * use */
        };
        unsigned int flags;
    };
    
    /* Flexible array member of structs containing function pointers */
    struct {
        forward_factory factory;
        const char *name;
    } plugins[];
} master_type_t;

/* Trigger default case in various contexts:
 * 1. Numeric constants (3.14e-10, 0x1FUL)
 * 2. String literals with special chars
 * 3. Comments with unusual sequences
 * 4. Preprocessor directives
 * 5. Line continuations
 * 6. Attribute syntax
 * 7. Bitfield specifications
 * 8. Variadic arguments (...)
 * 9. Compound literals
 * 10. Static array indices
 */

/* Additional edge cases */
enum weird_enum {
    VALUE1 = 1 << 0,  /* Bit shift */
    VALUE2 = 2,       /* Simple */
    VALUE3 = 0xFF,    /* Hex */
    VALUE4 = 0777,    /* Octal - default case trigger */
    VALUE5 = VALUE1 | VALUE2  /* Expression */
};

/* Struct with all delimiter types in member declarations */
struct final_test {
    /* Function pointer with complex return type */
    master_type_t *(*(*get_master)(int))(void);
    
    /* Array of pointers to functions */
    void (*(*func_array[5])(int))[10];
    
    /* Nested struct with anonymous union */
    struct {
        union {
            int a;
            struct {
                char *(*get_name)(void);
                void (*set_name)(const char *);
            } name_ops;
        };
        
        /* Default case: floating point in array size (GCC extension) */
        char padding[(int)(3.14159 * 10)];
    } inner;
    
    /* Attribute on struct member */
    volatile int counter __attribute__((aligned(32)));
};

/* End of complex type definitions */
