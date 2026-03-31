/* test-gengtype-coverage.c - Complex type definitions to exercise gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR(sect) __attribute__((section(sect)))

/* Macro that expands to include unusual characters */
#define WEIRD_DIM (1 /* comment */ + 2 \
                   + 3)  /* line continuation with backslash */

/* Complex nested type 1: Function pointer with attributes */
typedef int (SECTION_ATTR(".text") *func_ptr_t)(char *buffer, 
                                                int size /* default case trigger */,
                                                void **data);

/* Complex nested type 2: Struct with all delimiter types */
struct PACKED_ATTR container {
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag:1;
            unsigned int count:7 /* comment between bits */;
        } bits;
        unsigned char raw;
    } ALIGNED_ATTR header;
    
    /* Function pointer member with complex return type */
    void (**callback_array[WEIRD_DIM])(struct container *self,
                                       int action,
                                       /* Unusual character in default case: */
                                       float value /* 3.14159 */);
    
    /* Multi-dimensional array with computed size */
    int matrix[ARRAY_SIZE((int[]){1,2,3})][ARRAY_SIZE((int[]){4,5})];
    
    /* Nested struct with attribute containing parentheses */
    struct PACKED_ATTR {
        long timestamp;
        /* Pointer to array of function pointers */
        func_ptr_t (*handlers[2])(int, ...);
    } ALIGNED_ATTR metadata;
};

/* Complex nested type 3: Union with GCC vector extension */
typedef union PACKED_ATTR vector_data {
    /* GCC vector type with attribute */
    int __attribute__((vector_size(16))) v4;
    
    /* Anonymous struct with bit-fields and unusual characters */
    struct {
        unsigned char r, g, b, a /* alpha */;
    } channels;
    
    /* Array with size containing arithmetic */
    float arr[1 + 2 * 3 - 4];
} vector_data_t;

/* Complex nested type 4: Typedef chain with all delimiters */
typedef struct container* (*(*factory_func)[3])(
    int mode,
    /* Default case triggers: numbers and operators */
    unsigned int count /* 0xFF & 0x0F */,
    vector_data_t *(*generator)(void)
);

/* Nested type 5: Enum with computed values */
enum PACKED_ATTR error_codes {
    ERR_NONE = 0,
    ERR_INVALID = 1 << 0,
    ERR_OVERFLOW = 1 << 1,
    ERR_UNDERFLOW = 1 /* comment */ << 2,
    ERR_MAX = (ERR_INVALID | ERR_OVERFLOW | ERR_UNDERFLOW)
};

/* Complex nested type 6: Struct with nested anonymous types */
struct outer {
    /* Anonymous union inside struct */
    union {
        /* Function pointer with nested attributes */
        int (ALIGNED_ATTR *method)(struct outer *,
                                   /* Trigger default with preprocessor */
                                   #if 0
                                   unused_param
                                   #endif
                                   int param);
        
        /* Pointer to array with computed dimensions */
        double (*coords)[sizeof(struct outer) > 64 ? 2 : 3];
    } ALIGNED_ATTR u;
    
    /* Bit-field with unusual spacing */
    unsigned int : 4;  /* Unnamed bit-field */
    unsigned int flag /* flag */ : 1;
    unsigned int : 0;  /* Force alignment */
    
    /* Nested struct with all delimiter types */
    struct {
        /* Array of pointers to functions returning pointers to arrays */
        int (*(*(*callbacks[2])())[3])(void);
        
        /* Union containing struct with bit-fields */
        union {
            struct {
                short x:9, y:7 /* split */;
            } PACKED_ATTR point;
            unsigned short raw;
        } ALIGNED_ATTR position;
    } inner;
};

/* Complex nested type 7: Typedef with deeply nested parentheses */
typedef void (*(*(*signal_handler_registry)[5])(
    int sig,
    /* Numbers and operators trigger default case */
    void (*handler)(int, siginfo_t *, void *),
    unsigned long flags /* 0x80000000UL */
))(int, void *);

/* Type 8: Using typeof with nested expressions */
typedef typeof(*(struct container (*)[WEIRD_DIM])0) container_array_t;

/* Type 9: Struct with __attribute__ containing string literal */
struct SECTION_ATTR(".data.unusual") unusual {
    /* Array dimension with ternary operator */
    char data[sizeof(long) == 8 ? 64 : 32];
    
    /* Function pointer with nested attributes */
    void (PACKED_ATTR *cleanup)(struct unusual *self,
                                /* Unusual character sequence */
                                int flags /* O_RDWR | O_CREAT */);
};

/* Type 10: Final complex typedef mixing everything */
typedef union {
    struct container containers[2];
    vector_data_t vectors[ARRAY_SIZE((int[]){1,2,3,4})];
    factory_func factories;
    
    /* Anonymous struct with bit-fields and comments */
    struct {
        unsigned int : 16;  /* padding */
        unsigned int version:8 /* v1.0 */;
        unsigned int : 8;
    } PACKED_ATTR version_info;
} master_union_t ALIGNED_ATTR;

/* Additional declarations to ensure more parsing */
extern int (*(*global_callback)(int, ...))(void);
static volatile master_union_t * volatile global_union_ptr;

/* Array of function pointers with complex signatures */
static void (*signal_handlers[])(int, siginfo_t *, void *) = {
    NULL,
    /* Comment between array initializers */
    /* SIG_DFL would go here */
};

/* Struct with flexible array member and attributes */
struct PACKED_ATTR flexible {
    size_t count;
    /* Unusual characters in comment: @#$% */
    int data[];  /* Flexible array member */
};

/* Final type: Nested function pointer with all delimiters */
typedef int (*(*(*nested_func_factory)(void))[3])(
    struct container *,
    /* Trigger default case with floating point */
    double matrix[3][3] /* {{1.0, 0.0, 0.0}, ...} */,
    /* Nested attribute */
    __attribute__((deprecated)) int old_param
);
