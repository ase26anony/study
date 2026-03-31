/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (x + 1)
#define FUNC_ATTR __attribute__((noinline))
#define BITFIELD_WIDTH 3
#define ALIGN_SPEC __attribute__((aligned(16)))

/* Macro that expands to include unusual characters */
#define WEIRD_CHARS /* comment inside macro */ 42 \
    + 1 /* line continuation with comment */

/* Level 1: Basic nested structures */
struct level1 {
    int a;
    struct {
        /* Nested anonymous struct with all delimiter types */
        int (*func_ptr1)(int (*)(int[ARRAY_SIZE(5)]), struct { int x; });
        /* Array with computed size containing parentheses */
        char arr1[ARRAY_SIZE(2 * (3 + 4))];
    } inner;
    
    /* Trigger default case with numeric constant inside braces */
    union {
        int x;
        float y;
    } u /* 3.14159 */ ;  /* Numeric constant after brace */
};

/* Level 2: Function pointer with complex return type */
typedef int (*(*complex_func_ptr)(void))[10];

/* Level 3: Struct with deeply nested delimiters */
struct level3 {
    /* Mix all three delimiters in one declaration */
    void (*(*signal_handler)(int signum, 
                             void (*old_handler)(int),  /* Nested function pointer */
                             const char *msg
                            ))(int);
    
    /* Array of pointers to functions returning struct pointers */
    struct level1 *(*(*callbacks[5])(int, ...))(void);
    
    /* Bit-field with attribute */
    unsigned int flags: BITFIELD_WIDTH __attribute__((packed));
    
    /* GNU extension: vector type */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
};

/* Level 4: Union containing everything */
union master_union {
    /* Parentheses with unusual content inside */
    int (*weird_ptr)(int a /* comment */, ... /* ellipsis */);
    
    /* Brackets with macro expansion */
    double matrix[ARRAY_SIZE(WEIRD_CHARS)][ARRAY_SIZE(2)];
    
    /* Braces with attributes between members */
    struct {
        __attribute__((deprecated)) int old_field;
        int new_field ALIGN_SPEC;
    } nested_struct;
    
    /* Function pointer array with nested attributes */
    void (*(*func_array[3])(int)) 
        __attribute__((warn_unused_result));
};

/* Level 5: Typedef chain with all delimiter types */
typedef union master_union **(*(*typedef_chain)(
    int param1[sizeof(struct level3)],  /* Array parameter */
    void (*param2)(struct { int a; }),  /* Function pointer param */
    ...                                 /* Variadic */
))[5] /* Array of 5 */;

/* Level 6: Struct with inline assembly in declaration (GCC extension) */
struct with_asm {
    int data;
    /* asm constraint contains parentheses and brackets */
    register int reg_var __asm__("r12") __attribute__((unused));
    
    /* Nested type definition inside struct */
    enum {
        VALUE_A = 0x1,
        VALUE_B = 0x2,
        /* Binary literal with underscores */
        VALUE_C = 0b0101_0101
    } options;
};

/* Level 7: Complex declaration mixing everything */
static const volatile struct level3 *(*(*volatile const ultimate)(
    struct with_asm (*arg1)[/* empty size */],
    /* Function pointer returning array pointer */
    int (*(*arg2)(void))[],
    /* Anonymous struct parameter */
    struct { 
        _Complex double z;  /* Complex number type */
        _Bool b:1 __attribute__((packed));
    }
))[10] ALIGN_SPEC = { 0 };

/* Level 8: Template-like macro usage (C doesn't have templates but we can simulate) */
#define DECLARE_PAIR(T1, T2) \
    struct pair_##T1##_##T2 { \
        T1 first; \
        T2 second; \
        /* Function pointer member */ \
        int (*compare)(struct pair_##T1##_##T2 *, struct pair_##T1##_##T2 *); \
    }

/* Instantiate macros with complex types */
DECLARE_PAIR(struct level1 *, int (*[5])(void));

/* Level 9: Struct with designated initializers in type (GNU extension) */
struct with_designated {
    int array[10];
    struct {
        int x;
        int y;
    } point;
} __attribute__((designated_init));

/* Level 10: Final monster declaration */
__attribute__((visibility("hidden")))
static inline struct {
    /* Nested anonymous union */
    union {
        /* Pointer to function returning pointer to array of pointers to functions */
        int (*(*(*(*func_factory)(int n))[])())[];
        
        /* Reference to previous type */
        typeof(ultimate) *self_ref;
        
        /* Zero-length array with attribute */
        char flexible[] __attribute__((aligned(8)));
    } u;
    
    /* Bit-field spanning multiple lines */
    unsigned int 
        field1:1, 
        field2:2, 
        field3:BITFIELD_WIDTH,
        field4:sizeof(int)*8 - 6;
        
    /* __builtin_choose_expr with parentheses */
    int chosen __attribute__((
        __builtin_choose_expr(
            sizeof(void*) == 8,
            aligned(8),
            aligned(4)
        )
    ));
} final_type = {
    .u = { .self_ref = &ultimate },
    .field1 = 1,
    .field2 = 3,
    .field3 = 7
};

/* Additional triggers for default case */

/* Line continuation inside type definition */
struct cont_test {
    int x\
y;  /* Backslash triggers default case */
    
    /* Comment with parentheses inside */
    int z /* (commented paren) */;
    
    /* Numeric constants of various forms */
    enum {
        HEX = 0xDEADBEEF,
        OCTAL = 0777,
        FLOAT = 3.14e-10,
        /* Binary with separator (C23/GCC) */
        BINARY = 0b1101'0101
    } numbers;
};

/* Attribute with nested parentheses */
typedef int heavily_attributed 
    __attribute__(( 
        aligned( 
            sizeof(long) /* comment inside attribute */ 
        ), 
        packed, 
        section(".special") 
    ));

/* Empty declaration with just semicolons */
;;;;  /* Multiple semicolons should trigger advance() */

/* K&R style function definition (old-style) */
int old_style_func(param1, param2)
    int param1;
    struct level1 *param2;
{
    return param1;
}

/* Ensure file ends with type definitions, not main() */
/* No main function - this file is for parsing only */
