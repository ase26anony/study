/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro that expands to include unusual characters */
#define COMPLEX_DIM (1 << 2) /* Contains shift operator and comment */

/* 2. Base type with attributes (triggers default case for '__' and numbers) */
typedef unsigned long long uint64_t ALIGNED_ATTR;

/* 3. Complex nested structure with all delimiter types */
struct level1 {
    /* Default case trigger: numeric constant inside struct */
    int a[5 /* embedded comment */];
    
    /* Nested parentheses in function pointer */
    void (*callback)(int (*nested_cb)(char **argv, int argc));
    
    /* Mixed delimiters: array of function pointers */
    int (*(*array_of_funcs[COMPLEX_DIM])(void))[3];
    
    /* GCC extension: nested struct with attribute */
    struct __attribute__((packed)) {
        char x;
        int y PACKED_ATTR; /* Attribute after member */
    } nested;
    
    /* Union with bit-field containing shift operator */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2 /* comment with */ + 1;
            unsigned int flags : sizeof(int)*8 - 3;
        } bits;
        uint64_t all_flags;
    } flags_union;
};

/* 4. Typedef with deeply nested parentheses and brackets */
typedef void (*(**complex_type)(struct level1 *s[][2]))(
    int (*param1)[/* empty size */],
    char param2[ARRAY_SIZE(((int*){0}))], /* Macro with nested parentheses */
    ...
); /* Variadic function pointer */

/* 5. Structure containing array with computed size */
struct with_computed_array {
    /* Line continuation and unusual characters */
    double matrix\
[3] /* backslash continuation */ \
[4];
    
    /* Pointer to array of pointers to functions returning pointers */
    struct level1 *(*(*func_table)[/*comment*/2+2])(int, char);
    
    /* Anonymous union with attribute */
    union ALIGNED_ATTR {
        long long ll;
        double d;
        struct level1 *p;
    };
};

/* 6. Enum with embedded expressions in initializers */
enum complex_enum {
    ZERO = 0,
    ONE = 1,
    /* Expression with shift and bitwise operators */
    MASK = (1 << 8) | (1 << 16) | (1 << 24),
    /* Macro expansion */
    SIZE = ARRAY_SIZE(((int[]){1,2,3,4}))
};

/* 7. Type with __attribute__ containing nested parentheses */
typedef int (vector_type[4]) 
    __attribute__((vector_size(16), 
                   aligned(/*nested comment*/16)));

/* 8. Structure with member containing all delimiter types in one declaration */
struct ultimate_nest {
    /* The most complex single declaration:
     * - Outer parentheses for function pointer
     * - Inner brackets for array
     * - Braces for struct in return type
     * - Default case triggers throughout
     */
    struct { 
        int a; 
        char b; 
    } (*(*ultimate_member)(int (*)(char [][/* size */2]), 
                           ...))[3+2 /* expression */];
    
    /* GNU extension: __builtin_va_list */
    __builtin_va_list va;
    
    /* Array with attribute in middle */
    unsigned char data[256] 
        /* Attribute between array and semicolon */
        __attribute__((aligned(32)));
};

/* 9. Forward declaration in middle of parsing */
struct forward_decl;

/* 10. Typedef using typeof with expression */
typedef typeof(((struct level1*)0)->a) level1_array_type;

/* 11. Union with nested anonymous struct containing function pointer array */
union final_union {
    struct {
        /* Multi-dimensional array with computed sizes */
        int (*callbacks[/*macro*/COMPLEX_DIM][2])(void);
        
        /* Pointer to array of structs containing unions */
        struct with_computed_array *(*get_array)(int size);
    };
    
    /* Bit-field spanning multiple lines with line continuation */
    unsigned int \
    wide_bitfield : sizeof(struct ultimate_nest) * 8 \
                    - 8 /* adjustment */;
};

/* 12. Global variable with complex type and initializer */
static struct ultimate_nest global_instance = {
    .ultimate_member = NULL,
    .va = __builtin_va_start,
    .data = {[0 ... 63] = 0xFF, [255] = 0x01}
};

/* 13. Function prototype with nested attributes */
extern void process_data(struct level1 **data_list[][4])
    __attribute__((nonnull(1), 
                   format(printf, 2, 3) /* attribute with multiple params */));

/* 14. Structure with flexible array member and attribute */
struct flexible_struct {
    int count;
    double values[]; /* Flexible array member */
} PACKED_ATTR;

/* 15. Final typedef combining everything */
typedef union final_union *(**(*most_complex_type)(
    struct ultimate_nest (*)[/*dynamic*/ global_instance.data[0] + 1],
    int (*(*)(int, ...))[3] /* function returning pointer to array */
))(
    char const *restrict /* C99 restrict keyword */,
    volatile int /* volatile qualifier */,
    ...
);

/* Note: No main() function needed - gengtype only parses type definitions */
