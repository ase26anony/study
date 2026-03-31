/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (x + 1)
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define FUNC_PTR(name) (*name)

/* Macro with unusual characters in expansion */
#define WEIRD_MACRO(x) /* comment */ x \
    + 1 /* line continuation with backslash */

/* Trigger default case: numeric constants and comments inside delimiters */
#define COMPLEX_DIM WEIRD_MACRO(5) /* This expands with comments and backslash */

/* Type 1: Struct with deeply nested delimiters */
struct OuterStruct {
    /* Function pointer with attributes inside parentheses */
    int (ALIGN_ATTR *func_ptr1)(int, char);
    
    /* Array with complex dimension calculation */
    float data[ARRAY_SIZE(10)];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            /* Default case trigger: numeric constant inside braces */
            unsigned int flags : 29 /* comment */;
        } PACKED_ATTR;
        unsigned int raw;
    };
    
    /* Pointer to array of function pointers */
    void (*(*callbacks[5])(int, void (*)(char)))(double);
};

/* Type 2: Typedef with all three delimiter types mixed */
typedef struct {
    /* Nested parentheses in function pointer return type */
    struct Inner {
        int x;
        double y;
    } *(*(*complex_func)(int (*)(char), /* comment between args */ float))[10];
    
    /* Multi-dimensional array with macro expansion */
    long matrix[COMPLEX_DIM][ARRAY_SIZE(3)];
    
    /* Union containing struct with attribute */
    union {
        struct {
            short a;
            long b;
        } ALIGN_ATTR nested;
        char bytes[16];
    } data_union;
} ComplexType;

/* Type 3: GCC extension with vector types and attributes */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

struct VectorStruct {
    /* Vector type inside struct */
    v4si vectors[4];
    
    /* Function pointer returning vector type */
    v8sf (*vector_op)(v4si, v8sf);
    
    /* Nested struct with alignment attribute containing array */
    struct {
        char buffer[256];
        int (*processor)(char * /* comment with slash */, int);
    } ALIGN_ATTR __attribute__((packed)) io_block; /* Multiple attributes */
};

/* Type 4: Extremely complex declaration mixing all delimiters */
typedef union {
    /* Anonymous struct with bit-fields and function pointers */
    struct {
        /* Function pointer with complex argument list */
        void (*(*signal_handler)(int signum, 
                                 void (*old_handler)(int), /* nested parens */
                                 const char *msg))(void);
        
        /* Array of pointers to functions returning pointers to arrays */
        int (*(*func_array[3])(float))[10];
        
        /* Nested union inside struct */
        union {
            struct {
                /* Default case trigger: preprocessor directive simulated content */
                int x /* #if 0 would be here */;
                double y;
            };
            long long combined;
        } PACKED_ATTR inner_union;
    };
    
    /* Another struct member with all delimiters */
    struct {
        /* Pointer to function returning pointer to struct */
        struct VectorStruct *(*(*get_vector)(void))(int);
        
        /* Multi-level array with parentheses in size expression */
        char data[ARRAY_SIZE(2 + 3)][(5 * 2)];
        
        /* Macro with line continuation inside array dimension */
        int test[WEIRD_MACRO(2)];
    } alt;
} MonsterUnion;

/* Type 5: Enum with complex initializers */
enum ComplexEnum {
    VALUE1 = (1 << 0),
    VALUE2 = (1 << 1) | (1 << 2), /* Bitwise OR inside parentheses */
    VALUE3 = sizeof(struct OuterStruct), /* sizeof with nested type */
    VALUE4 = (int)((double)3.14159 * 100.0) /* Nested casts */
};

/* Type 6: Function pointer type with attributes between parameters */
typedef void (*SpecialCallback)(
    int param1 __attribute__((unused)), /* attribute between parens */
    char *param2,
    ... /* variadic - ellipsis triggers default case */
);

/* Type 7: Struct with nested anonymous structs/unions and attributes */
struct AttributeTest {
    /* __attribute__ inside nested context */
    struct {
        int x;
        double y;
    } __attribute__((packed)) nested1;
    
    union {
        int a;
        float b;
    } __attribute__((aligned(8))) nested2;
    
    /* Function pointer with attribute on return type */
    __attribute__((noreturn)) void (*exit_func)(int);
};

/* Type 8: Array of structs containing function pointers with complex signatures */
struct CallbackRecord {
    const char *name;
    int (*validate)(const void *data, size_t len);
    void (*cleanup)(void *ctx, int status);
} callback_table[] = {
    {"test1", NULL, NULL},
    {"test2", NULL, NULL} /* Trailing comma */
};

/* Type 9: Typedef for pointer to array of function pointers */
typedef int (*(*(*ThreeStarFunc)(void))[5])(char, float);

/* Type 10: Struct with all delimiter types in one member declaration */
struct UltimateTest {
    /* This declaration uses (), [], and {} all together */
    union {
        struct {
            int (*(*func_ptr_arr[3])(int))[2];
        } s;
        void *ptr;
    } u __attribute__((aligned(32)));
    
    /* Direct trigger for default case: numeric literal inside brackets */
    char raw_bytes[256 /* half */];
    
    /* Comment and backslash in macro expansion inside array dimension */
    int weird_array[WEIRD_MACRO(1)];
};

/* Additional complexity: forward declarations inside delimiter sequences */
struct ForwardDecl;

typedef struct {
    struct ForwardDecl *(*get_forward)(void);
    void (*set_forward)(struct ForwardDecl *);
} ForwardContainer;

/* Actual definition */
struct ForwardDecl {
    int id;
    ForwardContainer container;
};

/* Final type: Nested typedefs with attributes */
typedef struct Base {
    int value;
} BaseType;

typedef BaseType * (*FactoryFunc)(int) __attribute__((malloc));

/* The file ends with complex type definitions only - no main function
 * as this file is meant to be parsed by gengtype, not compiled and executed.
 */
