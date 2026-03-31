/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define FUNC_PTR(name) (*name)

/* 2. Complex nested type with all delimiter types */
typedef int (*complex_func_t)(
    /* Default case trigger: comment inside parentheses */
    struct inner {
        int x;
        /* Line continuation inside struct definition \
           triggers advance() in default case */
        char y;
    } *arg1,
    /* Array with computed size in brackets */
    int arg2[ARRAY_SIZE((int[]){1,2,3})],
    /* Function pointer with attributes */
    void (*callback)(int, char) ALIGNED_ATTR
);

/* 3. Struct with deeply nested delimiters */
struct outer_struct {
    /* Nested anonymous union */
    union {
        /* Bit-field with unusual syntax */
        unsigned int flags : 4;
        /* Array of function pointers */
        complex_func_t func_array[3];
    } data PACKED_ATTR;
    
    /* Pointer to array of structs */
    struct inner (*ptr_array[])(
        /* Nested function pointer type in parameter */
        void (*nested_cb)(char),
        /* Default case: numeric constant with exponent */
        double val = 1.23e-4
    );
    
    /* GNU extension: vector type */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
};

/* 4. Even more complex type mixing all delimiters */
typedef struct outer_struct* (*factory_func)(
    /* Parameter with all delimiter types */
    int config[][ /* comment between brackets */ 2],
    union {
        struct {
            int a;
            char b;
        } s;
        long l;
    } options,
    /* Macro expansion with backslash continuation \
       inside parentheses */
    void (*setup)(struct outer_struct*, \
                  complex_func_t)
) [ /* Array of function pointers */ 3];

/* 5. Enum with complex initializers */
enum complex_enum {
    VALUE_A = (1 << 0),
    VALUE_B = (2 /* comment */ << 1),
    VALUE_C = sizeof(struct { int x; char y; })
};

/* 6. Type definition with GNU statement expression */
typedef typeof(({
    struct outer_struct tmp;
    tmp.data.flags = 3;
    &tmp;
})) auto_struct_ptr_t;

/* 7. Struct with attribute in nested context */
struct attributed {
    /* Attribute on function pointer member */
    void (* __attribute__((noreturn)) fatal_error)(
        const char *msg,
        /* Nested array parameter */
        int codes[] /* No size specified */
    );
    
    /* Flexible array member with attribute */
    int flexible_array[] ALIGNED_ATTR;
} PACKED_ATTR;

/* 8. Union containing all delimiter types */
union mega_union {
    /* Parentheses */
    int (*func_ptr)(int, char);
    
    /* Brackets */
    int matrix[3][ /* between brackets */ 4];
    
    /* Braces */
    struct {
        /* Nested with all three */
        int (*nested[2])(struct { int x; }); /* Missing closing brace intentionally? No, fixed */
    } inner;
    
    /* Default case triggers */
    double scientific = 1.0e-10;  /* Not valid in union, but in C11 anonymous struct */
};

/* 9. C11 anonymous struct/union */
struct c11_example {
    union {
        int x;
        float y;
    };  /* Anonymous union member */
    
    struct {
        char a;
        char b;
    };  /* Anonymous struct member */
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_ptr)(void))[5];
};

/* 10. Final complex typedef with everything */
typedef union {
    /* Nested struct with bitfield */
    struct {
        unsigned int : 4;  /* Unnamed bitfield */
        unsigned int field1 : 8;
        /* Attribute on bitfield (GNU extension) */
        unsigned int field2 : 8 __attribute__((packed));
    } bits;
    
    /* Array of pointers to functions */
    void (*(*func_table[ /* size with expression */ 2+1 ])(
        /* Parameter with default case character: '#' in comment */
        #if 0
        disabled code
        #endif
        int param
    ))[3];
    
    /* Type with __attribute__ containing parentheses */
    char str[32] __attribute__((aligned( /* nested parens */ 8 )));
} ultimate_type_t;

/* 11. Function pointer type with nested attributes */
typedef void (*(*signal_handler_t)(
    int sig,
    /* Nested struct parameter */
    struct siginfo {
        int si_signo;
        /* Union in parameter type */
        union {
            int sival_int;
            void *sival_ptr;
        } si_value;
    } *info,
    void *context
))() __attribute__((noreturn));

/* 12. Struct with computed field offsets */
struct offset_example {
    char c;
    /* Padding calculation in attribute */
    int i __attribute__((aligned(__alignof__(double))));
    /* Array with size from expression containing parentheses */
    double data[(sizeof(long) + sizeof(int) - 1) / sizeof(int)];
};

/* Trigger parsing of all these types */
typedef struct outer_struct type1;
typedef complex_func_t type2;
typedef factory_func type3;
typedef enum complex_enum type4;
typedef auto_struct_ptr_t type5;
typedef struct attributed type6;
typedef union mega_union type7;
typedef struct c11_example type8;
typedef ultimate_type_t type9;
typedef signal_handler_t type10;
typedef struct offset_example type11;

/* Empty main - file is for parsing only */
int main(void) {
    return 0;
}
