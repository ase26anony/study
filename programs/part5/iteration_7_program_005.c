/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced()
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)
#define NESTED_PAREN(x) (((x) + 1) * 2) /* Multiple parentheses levels */

/* 2. Complex typedef with all delimiter types */
typedef int (*complex_callback_t)(
    /* Default case trigger: comment inside parentheses */
    struct inner { 
        int x; 
        /* Line continuation inside struct definition: \
           This backslash should trigger default case */
        char y;
    } *arg1,
    /* Array dimension with macro containing operators */
    int matrix[ARRAY_DIM][NESTED_PAREN(3)],
    /* Function pointer parameter */
    void (*helper)(char, float)
) ATTR_ALIGN;

/* 3. Struct with deeply nested delimiter sequences */
struct container {
    /* Bit-field with unusual syntax */
    unsigned int flags : 3;
    
    /* Anonymous union with nested struct */
    union {
        struct {
            /* Array of function pointers */
            complex_callback_t (*callbacks[5])(
                /* Nested: parentheses inside array declaration */
                int (*nested)(char[10]), /* Array parameter */
                ... /* Ellipsis operator */
            );
            
            /* Pointer to array with computed size */
            double (*data)[sizeof(struct container*) + 1];
        } s;
        
        /* Union alternative with GNU attributes */
        struct alt {
            int __attribute__((packed)) packed_member;
            
            /* Vector type extension */
            typedef int v4si __attribute__((vector_size(16)));
            v4si vectors[2];
        } ATTR_ALIGN a;
    };
    
    /* Member with attribute containing parentheses */
    char* name __attribute__((aligned((NESTED_PAREN(2)))));
};

/* 4. Enum with macro expansions in values */
enum error_codes {
    ERR_NONE = 0,
    ERR_PARSE = 1 << 0, /* Bit shift operator */
    ERR_TYPE = ARRAY_DIM, /* Macro expansion */
    ERR_MAX = (1 << 8) - 1 /* Complex expression */
};

/* 5. Function pointer type with nested attributes */
typedef void (__attribute__((stdcall)) *winapi_func_t)(
    /* Parameter with array of structs */
    const struct container* containers[],
    /* Function pointer callback parameter */
    enum error_codes (*validate)(int, ...),
    /* Empty parameter list then ellipsis */
    ...
) __attribute__((deprecated("Use v2 API")));

/* 6. Union with bitfields and unusual characters */
union mixed_data {
    /* Hexadecimal constant (contains 'x' character) */
    unsigned int raw : 0x10;
    
    struct {
        /* Floating point constant with exponent */
        float values[2];
        
        /* Pointer to function returning pointer to array */
        int (*(*get_matrix(void))[ARRAY_DIM])(int, int);
        
        /* Macro with line continuation in member declaration */
        char long_string\
        _name[100];
    } parsed;
    
    /* Anonymous struct with attribute */
    struct __attribute__((packed)) {
        char a;
        /* Default case trigger: numeric constant inside braces */
        int b: 0x1F; /* Hexadecimal with 'F' */
        long c;
    };
};

/* 7. Typedef chain with all delimiter types mixed */
typedef union mixed_data* (*factory_func_t)(
    int count,
    /* Array size as expression with parentheses */
    char buffer[count * sizeof(union mixed_data) + 1],
    /* Nested function pointer type */
    winapi_func_t* callbacks[]
) [/* Empty array size */];

/* 8. Struct with computed field offset using GCC extension */
struct with_offset {
    char prefix;
    
    /* Field with offset attribute containing expression */
    int important __attribute__((offset(
        /* Expression with multiple parentheses levels */
        (NESTED_PAREN(8)) + (int)sizeof(char*)
    )));
    
    /* Flexible array member */
    complex_callback_t flexible[];
};

/* 9. Nested type definition inside function-like macro */
#define DECLARE_NESTED_TYPE(prefix) \
    typedef struct prefix##_inner { \
        /* Type definition inside struct */ \
        typedef enum { YES, NO } bool_t; \
        bool_t flag; \
        /* Pointer to parent type */ \
        struct prefix##_inner* next; \
    } prefix##_t

/* Instantiate the macro-generated type */
DECLARE_NESTED_TYPE(my);

/* 10. Complex array declaration with all delimiters */
static factory_func_t (*global_handlers[3])(void) = {
    /* Initializer list with nested braces */
    [0] = (void*)0,
    [1] = (factory_func_t(*)(void))0xDEADBEEF, /* Cast with hex constant */
    [2] = NULL
};

/* 11. Struct with attribute containing string literal */
struct annotated {
    /* Attribute with string containing parentheses */
    const char* msg __attribute__((warning("This field (obsolete) will be removed")));
    
    /* Member with section attribute */
    int counter __attribute__((section(".data.unusual" /* Comment inside attribute */)));
    
    /* Zero-length array with attribute */
    unsigned char extra[0] __attribute__((aligned(8)));
};

/* 12. Final complex type mixing everything */
typedef struct {
    /* Nested anonymous union */
    union {
        /* Function pointer with complex return type */
        struct container* (*method1)(
            int,
            /* Default case: numeric with decimal point */
            float f = 3.14,
            /* Macro in parameter */
            char str[ARRAY_DIM]
        );
        
        /* Array of pointers to functions returning arrays */
        int (*(*method2[2])(int))[10];
    } u;
    
    /* Bitfield with computed width */
    unsigned int : sizeof(int*) * 8 - 4; /* Expression with pointer */
    
    /* Member with multiple attributes */
    volatile long atomic __attribute__((aligned(64), transparent_union));
} ultimate_type_t;

/* The file contains only type definitions - no executable code.
 * This is intentional as gengtype only processes type information.
 */
