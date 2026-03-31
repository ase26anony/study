/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* Preprocessor macros that expand to delimiter-containing expressions */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR __attribute__((section(".data")))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro that expands to include unusual characters */
#define COMPLEX_DIM (1 << 2) /* Comment inside macro expansion */

/* Trigger default case with line continuation and comments */
#define MULTI_LINE_MACRO \
    (x + /* embedded comment */ \
     y) /* more comment */

/* Type 1: Deeply nested function pointer with all delimiter types */
typedef void (*(*complex_funcptr_t)(int, \
                                    char **))[10] \
    ALIGN_ATTR;

/* Type 2: Struct with nested anonymous union containing function pointers */
struct outer_struct {
    int id;
    
    /* Anonymous union with attributes */
    union PACKED_ATTR {
        float f;
        double d;
        
        /* Function pointer member with complex return type */
        int (*(*callback)(struct outer_struct *self, \
                          int (*(*nested)(void))[5]))[10];
    } data;
    
    /* Bit-field with unusual size expression */
    unsigned int flags : (1 + 3) /* comment between tokens */;
    
    /* Array with macro-expanded dimension */
    char buffer[ARRAY_SIZE("test") + 2];
    
    /* Nested struct with attribute */
    struct inner_struct SECTION_ATTR {
        /* Pointer to array of function pointers */
        void (*(*func_array[COMPLEX_DIM])(void))[];
        
        /* Union inside struct inside struct */
        union {
            long long ll;
            
            /* Complex type with all delimiters mixed */
            struct {
                int (*(*method)(int a[3], \
                                struct { int x; } s)) \
                    (float, double);
            } nested;
        } u;
    } inner;
};

/* Type 3: Union containing struct with GCC vector extension */
union vector_union {
    /* GCC vector type extension */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
    
    struct {
        /* Function returning pointer to array */
        int (*(*get_matrix)(void))[4][4];
        
        /* Attribute with parentheses in strange place */
        char str[10] ALIGN_ATTR;
    } s;
};

/* Type 4: Enum with computed values */
enum complex_enum {
    ENUM_A = (1 << 0),  /* Bit shift in initializer */
    ENUM_B = sizeof(int[2]),  /* sizeof with array type */
    ENUM_C = MULTI_LINE_MACRO,  /* Macro expansion */
    ENUM_D = (int){0}  /* Compound literal in enum */
};

/* Type 5: Typedef chain with deeply nested parentheses */
typedef int *(*(*level1_t)(void))(float);
typedef level1_t (*(*level2_t)(level1_t f))(char);
typedef level2_t (*(*level3_t)(int, ...))(double);

/* Type 6: Struct with flexible array member and attributes */
struct flexible_struct {
    int count;
    double values[];
} PACKED_ATTR ALIGN_ATTR;

/* Type 7: Function pointer with nested attribute specifications */
typedef void (*(*attr_funcptr_t)(int a __attribute__((unused)), \
                                 char b __attribute__((deprecated)))) \
    __attribute__((noreturn));

/* Type 8: Complex array type with multiple dimensions */
typedef int (*(*array_3d_t)[10][20])[30];

/* Type 9: Struct with nested struct/unions and all delimiter types */
struct ultimate_test {
    /* Parentheses: function pointer */
    int (*(*func)(void))();
    
    /* Brackets: multi-dimensional array */
    unsigned char pixels[4][4][3];
    
    /* Braces: anonymous struct */
    struct {
        /* Mixed delimiters in single member */
        union {
            /* Array of pointers to functions returning struct pointers */
            struct ultimate_test *(*(*callbacks[5])(int))();
            
            /* Function pointer with array parameter */
            void (*handler)(int params[((void)0, 10)]);  /* comma operator */
        } u;
        
        /* Attribute with complex argument */
        int x __attribute__((aligned((sizeof(long) > 4) ? 8 : 4)));
    } data;
    
    /* Bit-field with complex width containing comment */
    unsigned int mode : 2 /* two bits */ + 1;
};

/* Type 10: Forward declaration in nested context */
struct forward_ref;

typedef struct container {
    /* Pointer to forward-declared struct */
    struct forward_ref *fwd;
    
    /* Self-referential pointer */
    struct container *next;
    
    /* Function using forward declaration */
    void (*process)(struct forward_ref *ref, \
                    struct container *cont[10]);
} container_t;

/* Additional complex declarations to increase parsing events */

/* Nested attribute syntax */
int (*(*global_callback)(int, ...)) \
    __attribute__((format(printf, 2, 3))) = 0;

/* Variable with complex type and initializer */
static struct outer_struct global_instance = {
    .id = (int)((void*)0 + 1),  /* Cast and pointer arithmetic */
    .data = {
        .callback = 0
    },
    .flags = (1 << 0) | (1 << 1),  /* Bit operations */
    .buffer = {'t', 'e', 's', 't', '\0'},
    .inner = {
        .func_array = {0},
        .u = {
            .nested = {
                .method = 0
            }
        }
    }
};

/* Multiple typedefs with increasing complexity */
typedef int simple_int;
typedef simple_int *int_ptr;
typedef int_ptr (*func_returning_int_ptr)(void);
typedef func_returning_int_ptr (*higher_order_func)(int, func_returning_int_ptr);

/* Final type: Everything combined */
typedef struct {
    higher_order_func hof;
    container_t *containers[((int)3.14)];  /* Cast in array size */
    attr_funcptr_t cleanup __attribute__((cleanup));
} final_type_t;

/* The file ends with a comment containing unusual characters: */
/* !@#$%^&*()_+-=[]{}|;':",./<>? */
