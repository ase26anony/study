/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* 2. Trigger default case with unusual characters in nested contexts */
FUNC_PTR_TYPEDEF(complex_handler_t) /* This expands with parentheses */

/* 3. Complex struct with all delimiter types mixed */
struct level1 {
    /* Default case trigger: numeric constant with unusual format */
    int flags : 3 /* bitfield */;
    
    /* Nested parentheses in function pointer with attribute */
    void (*callback)(int (*)(char), float) ATTR_ALIGN;
    
    /* Array with complex dimension calculation */
    char buffer[ARRAY_DIM + sizeof(void*)];
    
    /* Anonymous union with nested struct */
    union {
        struct {
            /* Nested brackets in multi-dimensional array */
            double matrix[2][3];
            
            /* Pointer to array of function pointers */
            int (*(*func_array)[5])(void);
        } inner;
        
        /* Another way to trigger default: line continuation */
        long \
        continued_member;
    };
    
    /* GNU extension: vector type */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
};

/* 4. Typedef with deeply nested delimiters */
typedef struct level1* (*factory_func_t)(
    /* Arguments with attributes and macros */
    int count ATTR_ALIGN,
    /* Array parameter with computed size */
    char data[ARRAY_DIM * 2]
) /* Comment between delimiters to trigger default case */;

/* 5. Union with nested everything */
union mega_union {
    /* Case labels in nested context (not switch, but looks like one) */
    struct {
        int case1:1;
        int case2:2;
        int default:3;  /* 'default' token inside struct */
    } bits;
    
    /* Function pointer returning pointer to array */
    int (*(*get_matrix)(void))[4][4];
    
    /* Nested anonymous struct with attribute */
    struct {
        __attribute__((packed)) struct {
            char a;
            int b;
        } packed_pair;
    };
};

/* 6. Enum with computed values (contains operators) */
enum complex_enum {
    VAL1 = (1 << 0),  /* Parentheses with shift operator */
    VAL2 = sizeof(struct level1),
    VAL3 = VAL1 | VAL2  /* Binary operator */
};

/* 7. Type with GNU statement expression (extension) */
typedef typeof(({ int x = 5; &x; })) int_ptr_t;

/* 8. Struct with designated initializers in type (GNU extension) */
struct designated {
    int array[10];
    struct {
        int x;
        int y;
    } point;
} __attribute__((aligned(32)));

/* 9. Complex function pointer type with nested attributes */
typedef void (*(*signal_handler_factory)(
    int sig,
    /* Attribute on parameter */
    void (*old)(int) __attribute__((deprecated))
))(
    /* Function returning function pointer */
    int, 
    ... /* Variadic arguments */
) __attribute__((nonnull(1)));

/* 10. Multiple levels of indirection with arrays */
typedef int (*(*(*complex_array)[10])(float))[20];

/* 11. Struct with zero-length array (GNU extension) */
struct flex_array {
    int length;
    char data[0];  /* Zero-length array */
};

/* 12. Nested type definitions with all delimiters in one */
typedef union {
    struct {
        /* Pointer to function returning pointer to array */
        int (*(*(*func)(void))[])(char);
        
        /* Multi-dimensional array with attribute */
        volatile short grid[3][3] __attribute__((aligned(8)));
    } s;
    
    /* Anonymous union inside union */
    union {
        /* __builtin_constant_p in array size */
        char dynamic[__builtin_constant_p(1) ? 10 : 20];
    };
} ultimate_type_t;

/* 13. Type with __extension__ (GNU feature) */
__extension__ typedef long long very_long;

/* 14. Struct with alignment attribute containing expression */
struct aligned_with_expr {
    char c;
} __attribute__((aligned((sizeof(void*) > 4) ? 8 : 4)));

/* 15. Final complex declaration mixing everything */
static ultimate_type_t* (*(*global_factory)(
    int version,
    /* Nested attribute with parentheses */
    __attribute__((format(printf, 2, 3))) const char *fmt,
    ...
))[3] = { 0, 0, 0 };

/* Note: No main() needed - gengtype only parses type definitions */
