/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Targeting lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro with unusual characters in expansion */
#define COMPLEX_DIM (1 << 2) /* Contains << operator */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* 2. Complex nested type definitions */

/* First: Struct with all delimiter types mixed */
struct OuterStruct {
    /* Nested anonymous union with attributes */
    union {
        int x;
        double y;
    } ALIGN_ATTR;
    
    /* Function pointer with complex arguments */
    int (*callback)(
        struct Inner {  /* Nested struct definition inside parentheses */
            int a;
            char b;
        } *inner,  /* Pointer to nested struct */
        int arr[/* Comment inside brackets */ 10],
        void (*nested_cb)(char, short, long)
    );
    
    /* Array with macro-expanded size containing bit shift */
    unsigned char data[ARRAY_SIZE(((int[]){1,2,3,4})) + COMPLEX_DIM];
    
    /* Bit-field with unusual width expression */
    unsigned int flags : (sizeof(int)*8 - 2);
} PACKED_ATTR;

/* 3. Typedef with deeply nested delimiters */
typedef struct {
    /* Union inside struct */
    union {
        /* Function pointer returning pointer to array */
        int (*(*func_array[3])(void))[5];
        
        /* Nested struct with attribute */
        struct {
            char *name;
            int id;
        } ALIGN_ATTR entries[2];
    } data;
    
    /* Pointer to function with complex return type */
    struct Node *(*(*get_node)(int level))(
        char *name,
        int params[...]  /* C99 variadic array */
    );
} ComplexType;

/* 4. Enum with embedded expressions */
enum SpecialValues {
    VAL1 = (1 << 0),  /* Bit shift in parentheses */
    VAL2 = (1 << 1),
    VAL3 = (1 << 2) | (1 << 3),  /* Bitwise OR */
    VAL4 = sizeof(struct OuterStruct)  /* sizeof in initializer */
};

/* 5. Function pointer typedef with all delimiters */
typedef void (*(*SignalHandler[5])(
    int signum,
    const char */* Comment between asterisks */msg,
    struct {
        pid_t pid;
        void *context;
    } *info
))(
    void *data,
    int options[...]  /* Another variadic array */
);

/* 6. Union with anonymous struct and unusual members */
union WeirdUnion {
    struct {
        /* Array of function pointers */
        int (*callbacks[3])(
            /* Multi-line argument list
             * with comments */
            int a,  // Line comment inside parentheses
            char b
        );
        
        /* Pointer with attribute */
        volatile char * ALIGN_ATTR ptr;
    };
    
    /* 2D array with computed dimensions */
    float matrix[(2+3) * 2][sizeof(double) * 2];
    
    /* Nested union */
    union {
        long double ld;
        __int128_t i128;  /* GCC extension */
    } extended;
};

/* 7. Struct using GNU vector extension */
typedef struct {
    /* Vector types with attributes */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
    
    /* Function pointer with vector argument */
    v4si (*vec_func)(v4si a, v4si b, int mask);
    
    /* Anonymous struct with bit-fields */
    struct {
        unsigned int a : 4;
        unsigned int b : (8 - 1);  /* Expression in bit-field */
        unsigned int c : 1;
    };
} VectorStruct;

/* 8. Complex declaration with macros and attributes */
FUNC_PTR_TYPEDEF(CommandFunc) MAYBE_UNUSED;

/* 9. Struct with __attribute__ containing nested parentheses */
struct AttributedStruct {
    int value;
    char *name;
} __attribute__((designated_init, 
                 access(read_only, 
                        /* Nested attribute argument */
                        (1), 
                        (2)
                       )
                ));

/* 10. Final mega-complex typedef */
typedef union {
    /* Case 1: Array of pointers to functions returning structs */
    struct Result {
        int status;
        char error_msg[256];
    } (*(*handlers[10])(void *arg))[3];
    
    /* Case 2: Function taking array of function pointers */
    void (*executor)(
        int (*ops[])(int, int),
        int count,
        /* Nested anonymous union parameter */
        union {
            int i;
            float f;
        } param
    );
    
    /* Case 3: Pointer to array of structs with bit-fields */
    struct {
        unsigned int flag : 1;
        unsigned int value : 31;
    } (*bit_fields)[(1 << 5) + 1];
} UltimateUnion ALIGN_ATTR;

/* 11. Additional edge cases */

/* Type with line continuation in macro expansion */
#define MULTILINE_ARGS(x, y) x \
    + y  /* Backslash triggers default case */

struct WithContinuation {
    int value MULTILINE_ARGS(1, 2);
};

/* Type with string literal in array size (GCC extension) */
struct WithStringSize {
    char data[sizeof("test") - 1];
};

/* Multiple attributes with commas and spaces */
struct MultiAttr {
    short s;
    long l;
} __attribute__((packed, aligned(8), deprecated));

/* 12. Global variables using complex types */
ComplexType global_var = {0};
UltimateUnion *union_ptr = 0;
volatile VectorStruct vs MAYBE_UNUSED;

/* The file ends here - no main function needed as gengtype only parses types */
