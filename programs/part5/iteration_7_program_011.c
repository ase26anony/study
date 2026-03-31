/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define MAYBE_UNUSED __attribute__((unused))

/* 2. Complex nested type with all delimiter types */
typedef int (*complex_func_ptr_t)(
    /* Default case trigger: comment inside parentheses */
    struct inner_struct {
        int x;
        /* Line continuation inside nested context: \
           This backslash should trigger default case */
        int y;
    } *arg1,
    /* Mixed delimiters in array parameter */
    int (*callback)(int matrix[3][4], void *data),
    /* Attribute with parentheses inside function parameter */
    volatile const char *str ALIGNED_ATTR
);

/* 3. Struct with deeply nested delimiter sequences */
struct outer_struct {
    /* Function pointer member with nested attributes */
    void (*operation)(
        int param1,
        /* Numeric constant with unusual format (0x prefix) */
        long param2 /* 0xDEADBEEF */,
        ... /* Ellipsis operator */
    ) PACKED_ATTR;
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            /* Default case: numeric constant inside bit-field */
            unsigned int flags : 29; /* 0x1FFFFFF */
        } bits;
        unsigned int all_flags;
    } flags_union;
    
    /* Array with macro-expanded size containing arithmetic */
    int dynamic_array[ARRAY_SIZE((int[]){1,2,3,4}) + 1];
    
    /* Nested struct with function pointer returning array pointer */
    struct {
        int (**(*get_matrix)(void))[][4];
        /* Attribute on function pointer inside anonymous struct */
        float (*compute)(double) __attribute__((const));
    } nested;
};

/* 4. Union with GCC vector extension */
typedef union vector_union {
    /* GCC vector type with parentheses */
    int __attribute__((vector_size(16))) v4si;
    /* Nested array inside union */
    struct {
        int arr[2][2];
        /* Comment with special characters: <>&|^~ */
        int extra; /* <tag> & reference */
    } as_struct;
} vector_union_t;

/* 5. Enum with embedded attributes */
enum complex_enum {
    VALUE_A = 0,
    VALUE_B = 1 << 0, /* Bit shift inside enum */
    VALUE_C = 1 << 1,
    /* Attribute on enum value (GCC extension) */
    VALUE_D __attribute__((deprecated)) = 1 << 2
};

/* 6. Type definition mixing all delimiter types in one declaration */
typedef struct container** (*(*factory_func)(
    int mode,
    /* Default case: preprocessor directive inside parameter list */
    #ifdef DEBUG
    const char *debug_name,
    #endif
    /* Complex array dimension with parentheses */
    int dimensions[][(2 + 3) * 4]
))[/* Empty comment */] (struct {
    /* Nested anonymous struct */
    union {
        /* Function pointer with nested attributes */
        void (*(*signal_handler)(int sig, void *ctx))(
            /* Multiple parameters with different delimiters */
            const char *msg[10],
            struct { int code; char desc[50]; } *details
        ) __attribute__((noreturn));
        /* Alternative union member */
        long double complex_number;
    } u;
    /* Bit-field with computed width */
    unsigned int control : sizeof(int) * 8 - 4;
});

/* 7. Variable declarations using complex types */
complex_func_ptr_t global_func MAYBE_UNUSED;
struct outer_struct instance ALIGNED_ATTR;
vector_union_t vectors[10];

/* 8. Function with nested type in parameter */
static void process_data(
    /* Pointer to array of function pointers */
    int (*(*callbacks[]))(
        /* Nested parameter with attribute */
        const struct outer_struct *data __attribute__((nonnull)),
        /* Array parameter with variable brackets */
        float results[static 5]
    ),
    /* Two-dimensional array with computed bounds */
    int matrix[ARRAY_SIZE(vectors)][sizeof(struct outer_struct) / 4]
) {
    /* Function body not needed for gengtype parsing */
}

/* 9. One more complex typedef for good measure */
typedef union {
    /* Anonymous struct with all delimiter types */
    struct {
        /* Function returning pointer to array */
        int (*(*get_array)(void))[];
        /* Pointer to function with nested struct parameter */
        void (*set_value)(struct { int x; int y; } point);
        /* Multi-dimensional array with attribute */
        volatile char buffer[2][3][4] ALIGNED_ATTR;
    };
    /* Alternative: array of function pointers */
    void (*actions[5])(int, ...);
} ultimate_union_t;

/* 10. Final type with macro expansion creating complex delimiter pattern */
#define WRAP(type) type wrapped_##type
#define NEST(count) WRAP(WRAP(WRAP(int)))

typedef NEST(3) deeply_nested_t;  /* Expands to int wrapped_int wrapped_wrapped_int */

/* The file ends here - no main function needed as gengtype only parses declarations */
