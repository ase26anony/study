/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define FUNC_PTR(name) (*name)

/* 2. Complex nested type with all delimiter types */
typedef int (*(*complex_func_ptr_t)(int, \
    /* Line continuation inside parentheses - triggers default case */ \
    char *))[10] ALIGN_ATTR;

/* 3. Struct with deeply nested delimiters */
struct outer_struct {
    /* Default case trigger: numeric constant inside struct */
    int x = 42;  /* GCC extension - initializer in struct */
    
    /* Mixed delimiters in single member */
    void (*(*signal_handler[3])(int signum, 
        /* Comment inside parentheses - default case */
        void (*old_handler)(int)))(int);
    
    /* Array with complex size expression */
    char buffer[ARRAY_SIZE(((int[]){1,2,3,4}))];
    
    /* Nested anonymous union with attributes */
    union {
        struct {
            int a;
            /* Bit-field with unusual syntax */
            unsigned b : 4 /* no semicolon here */;
        } PACKED_ATTR;
        /* Default case: standalone number */
        3.14159;
        double c;
    } inner_union;
    
    /* Function pointer returning pointer to array */
    float (*(*get_matrix)(void))[4][4];
} PACKED_ATTR;

/* 4. Enum with complex initializers */
enum complex_enum {
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1) | (1 << 2),
    /* Nested parentheses in initializer */
    VALUE_C = ((1 << 3) + (1 << 4)) & 0xFF,
    /* Default case: floating point in enum (GCC extension) */
    VALUE_D = __builtin_choose_expr(0, 3.14, 5)
};

/* 5. Union with GCC vector extension */
typedef union vector_union {
    /* Vector type with attribute */
    int v4si __attribute__((vector_size(16)));
    struct {
        int x, y, z, w;
    };
    /* Array with computed size */
    int arr[sizeof(int[4]) / sizeof(int)];
} vector_union_t;

/* 6. Typedef chain with all delimiter types */
typedef struct node *(*(*node_visitor)(struct node *n, 
    /* Unusual character: @ in comment - default case */
    void *@data))(void);

/* 7. Struct containing nested function pointers with attributes */
struct container {
    /* Multi-level function pointer */
    void (*(*(*callback)(int))(char))(double);
    
    /* Array of function pointers */
    int (*handlers[5])(const char *msg, ...);
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned flag1 : 1;
        unsigned flag2 : 2;
        /* Default case: backslash in comment \ */
        unsigned flag3 : 3;
    };
    
    /* Pointer to array of structs */
    struct inner {
        int id;
        /* Nested union in struct */
        union {
            long l;
            double d;
        } value;
    } (*items)[];
};

/* 8. Complex typedef with macro expansion */
typedef ARRAY_SIZE(((int[]){1,2})) array_size_t;

/* 9. Function-like macro used in type definition */
#define DECLARE_CALLBACK(ret, name, ...) ret (*name)(__VA_ARGS__)

/* Using the macro to create complex type */
DECLARE_CALLBACK(struct container *, 
                 factory_callback,
                 int count,
                 /* Nested parentheses in macro argument */
                 const char *(*name_gen)(int));

/* 10. Final complex type mixing everything */
typedef union {
    /* All three delimiters in one declaration */
    struct {
        int (*(*func_array[2])(float f))[3];
        /* Default case: # in comment */
        char *str #not a directive;
    } s;
    
    /* Array with multiple dimensions */
    unsigned char data[sizeof(struct container)][16];
    
    /* Function pointer with nested attributes */
    __attribute__((noreturn)) void (*(*error_handler)(
        /* Default case: multiple special chars */
        const char *file, int line, ...))();
} ultimate_type_t;

/* 11. Additional edge cases */
/* Type definition with line continuation in middle */
typedef struct weird \
{ \
    int a; \
    /* Nested braces with comment between them */ \
    struct { int b; /* comment */ }; \
} weird_t;

/* 12. Pointer to function returning pointer to array of function pointers */
int (*(*(*(*crazy_ptr)(void))[5])(int))(char);

/* 13. __attribute__ with complex argument */
struct aligned_struct {
    long double ld __attribute__((aligned(32)));
    /* Attribute with nested parentheses */
    short s __attribute__((aligned(__alignof__(long double))));
} __attribute__((packed, aligned(64)));

/* 14. Using __typeof__ with complex expression */
typedef __typeof__((*(ultimate_type_t*)0).s.func_array[0]) func_ptr_array_t;

/* 15. Final declaration to ensure parsing completes */
extern volatile const ultimate_type_t * const global_var;

/* Note: No main function needed - gengtype only processes type definitions */
