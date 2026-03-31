/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 3)  /* Contains parentheses and shift operator */
#define ATTR_ALIGN __attribute__((aligned(16)))  /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* 2. Complex nested type with all delimiter types */
struct level1 {
    /* Default case trigger: numeric constant inside struct */
    int x = 42;  /* '=' triggers default case */
    
    /* Mixed delimiters: function pointer with attributes */
    void (*callback)(int (*)(char **), ...) ATTR_ALIGN;
    
    /* Nested array with computed size */
    char buffer[ARRAY_DIM * 2];  /* Macro expands to parentheses */
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned flag1 : 1;
            unsigned flag2 : 2 /* Missing semicolon to test error recovery */;
        } bits;
        int value;
    } ATTR_ALIGN;  /* Attribute after union */
};

/* 3. Typedef with deeply nested parentheses */
FUNC_PTR_TYPEDEF(complex_handler_t);

/* 4. Structure containing all delimiter types in single member */
typedef struct {
    /* Multi-dimensional array with function pointers */
    complex_handler_t (*matrix[ARRAY_DIM][ARRAY_DIM])(struct level1 ***);
    
    /* Nested anonymous struct with GNU extensions */
    struct {
        /* Vector type with attribute */
        int __attribute__((vector_size(16))) vec;
        
        /* Flexible array member with attribute */
        char data[] __attribute__((packed));
    } container;
    
    /* Union with nested switch-like syntax in comments */
    union {
        /* Comment with parentheses: (test) and brackets: [test] */
        float f;
        /* Line continuation in comment: \
           continues here */
        double d;
    } u;
} mega_struct_t;

/* 5. Function pointer returning pointer to array of structs */
typedef struct level1* (*(*complex_func)(int, ...))[ARRAY_DIM];

/* 6. Enum with computed values */
enum {
    VAL1 = (1 << 0),  /* Parentheses with shift */
    VAL2 = sizeof(struct level1[2]),  /* sizeof with brackets */
    VAL3 = __builtin_offsetof(mega_struct_t, container.vec)  /* Nested parentheses */
};

/* 7. Nested struct with attribute containing parentheses */
struct outer {
    struct inner {
        int x;
    } __attribute__((aligned((16)))) i;  /* Double parentheses */
    
    /* Array of function pointers with GNU attribute */
    void (*actions[3])(void) __attribute__((deprecated));
};

/* 8. Type with __extension__ and nested braces */
typedef __extension__ struct {
    long long big __attribute__((mode(DI)));
    
    /* Anonymous struct with bit-fields and attributes */
    struct {
        unsigned a : 4 __attribute__((packed));
        unsigned b : 12 /* Embedded comment: (test [array]) */;
    };
} gnu_ext_struct;

/* 9. Complex declaration mixing everything */
static volatile mega_struct_t* (*(*global_callback)(  
    /* Parameter with attribute */
    int __attribute__((unused)) param1,
    /* Array parameter */
    char *argv[static 3]
))[ARRAY_DIM] = 0;  /* Initializer with numeric constant */

/* 10. Union with nested struct containing all delimiters */
union ultimate {
    struct {
        /* Function returning pointer to array */
        int (*(*func)(void))[10];
        
        /* Nested union with anonymous struct */
        union {
            struct {
                char c;
            };
            int i;
        } nested;
        
        /* Attribute on bit-field */
        unsigned flags : 8 __attribute__((packed));
    } s;
    
    /* Array of complex type */
    gnu_ext_struct extended[2];
};

/* 11. Macro that expands to partial type definition */
#define PARTIAL_TYPE volatile struct { int x; }

/* Use macro inside typedef */
typedef PARTIAL_TYPE partial_t;

/* 12. Final complex type using previously defined types */
typedef union ultimate* (*(*final_type)(
    partial_t **,  /* Pointer to macro-defined type */
    mega_struct_t (*)[ARRAY_DIM]  /* Pointer to array */
))(void);

/* The file ends without main() - we only need type definitions */
