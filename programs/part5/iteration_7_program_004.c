/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define ATTR_SPEC __attribute__((aligned(16), packed))
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* 2. Complex nested type with all delimiter types */
struct level1 {
    /* Default case trigger: numeric constant inside struct */
    int x = 42; /* This '=' should trigger default case */
    
    /* Nested parentheses in function pointer */
    void (*callback)(int (*)(char **), double);
    
    /* Mixed delimiters: array of pointers to functions */
    int (*(*array[ARRAY_DIM])(void))[3];
    
    /* GNU attribute inside struct */
    char data[10] ATTR_SPEC;
    
    /* Nested anonymous union with bit-field */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:2 /* Missing semicolon to test error recovery */ \
            /* Line continuation above should trigger default case */
        } bits;
        long long value;
    };
};

/* 3. Interdependent typedefs with deeply nested delimiters */
typedef struct {
    /* Array dimension with parenthesized expression */
    int matrix[(ARRAY_DIM * 2) + 1][(ARRAY_DIM / 2) - 1];
    
    /* Function pointer returning pointer to array */
    struct level1* (*(*getter)(int index))[][2];
    
    /* Nested struct with attribute */
    struct inner {
        __attribute__((vector_size(16))) float vec[4];
        /* Comment with unusual chars: <>&|^~ */
    } ATTR_SPEC inner_obj;
} complex_t;

/* 4. Macro expansion creating complex type */
FUNC_PTR_TYPEDEF(complex_callback_t);

/* 5. Union with all delimiter types in single member */
union everything {
    /* This single declaration uses (), [], and {} */
    struct {
        complex_callback_t (*handlers[3])(complex_t (*)(int, ...));
        /* Nested function pointer with varargs */
    } nested ATTR_SPEC;
    
    /* Array with computed size containing function pointers */
    void (*(*func_array[((sizeof(struct level1) + 15) & ~15) / 8])(int, ...));
};

/* 6. Type with GNU extension and nested attributes */
typedef __attribute__((mode(SI))) int int32_t __attribute__((aligned(4)));

/* 7. Deeply nested parentheses */
typedef int (*(*(*deep_nested)(int (*(*)(double))[3]))(char **))[5];

/* 8. Struct with macro-generated array bounds */
struct with_macros {
    /* Macro expands to parenthesized expression */
    int arr[ARRAY_DIM];
    
    /* Multiple attributes with parentheses */
    __attribute__((deprecated("use new_field instead"), packed)) int old_field;
    
    /* Pointer to array of function pointers */
    int (*(*(*signal_handlers)[10])(int, ...))[];
};

/* 9. Anonymous struct with bitfields and unusual characters */
struct {
    /* Bitfields with parenthesized expressions */
    unsigned int a : (1 + 1);
    unsigned int b : (2 * 2);
    
    /* Default case triggers: */
    int c = {0}; /* Braces and equals sign */
    float d = 3.14e-10; /* Scientific notation */
    
    /* Nested union in anonymous struct */
    union {
        /* Pointer to function returning pointer to array */
        int (*(*(*func_ptr))(void))[][2];
        
        /* Another struct with all delimiters */
        struct {
            void (*method)(int, float[][*]); /* VLA in parameter */
        };
    } u;
} global_var;

/* 10. Final complex typedef mixing everything */
typedef union everything* (*(*ultimate_type)(
    /* Parameter with attribute */
    __attribute__((nonnull(1))) complex_t *arg1,
    
    /* Array parameter with computed size */
    int arg2[sizeof(union everything) / sizeof(int)],
    
    /* Function pointer parameter */
    int (*compar)(const void *, const void *)
))[] ATTR_SPEC;

/* 11. Additional edge cases */
enum weird_enum {
    VALUE1 = (1 << 0), /* Parentheses with shift */
    VALUE2 = (2 << 1), /* More parentheses */
    VALUE3 = 0xABCD, /* Hex constant */
    VALUE4 = 0777, /* Octal constant */
    VALUE5 = '\\' /* Character literal with backslash */
};

/* 12. Struct with __extension__ and nested attributes */
struct __extension__ gnu_ext_struct {
    /* __extension__ affects parsing */
    __extension__ long long big_num;
    
    /* Nested attribute with multiple arguments */
    __attribute__((format(printf, 2, 3), aligned(32))) 
    char* (*formatter)(char *, const char *, ...);
    
    /* Multi-dimensional array with parenthesized size */
    int deep_array[2][(3 + 2)][((5) * (2))];
};

/* 13. Type definition containing comment between delimiters */
typedef int (* /* comment between '*' and '(' */ callback_type)(
    int param1, /* comment with special chars < > */
    float param2[]
);

/* 14. Forward declarations to test parser state */
struct forward_decl;
typedef struct forward_decl *fwd_ptr_t;

/* 15. Complete the forward declaration with complex internals */
struct forward_decl {
    fwd_ptr_t next;
    /* Circular reference with function pointer */
    void (*processor)(fwd_ptr_t self, int (*(*data)[10]));
    
    /* Anonymous union with bitfields */
    union {
        struct {
            unsigned int : 4; /* Unnamed bitfield */
            unsigned int field : 4;
        };
        char bytes[2];
    };
};

/* The file contains only type definitions - no main function needed
 * as gengtype only parses type information for garbage collector */
