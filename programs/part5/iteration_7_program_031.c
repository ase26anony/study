/* test-gengtype-coverage.c - Complex type definitions to exercise gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* Trigger default case with unusual characters in macro */
#define WEIRD_CHARS /* comment inside macro */ \
    volatile /* backslash continuation */ \
    const

/* Level 1: Basic nested structures */
struct outer1 {
    int a;
    /* Nested anonymous struct with attribute */
    struct {
        char b;
        long c ATTR_ALIGN;
    } inner;
    
    /* Array with macro-expanded dimension */
    float arr[ARRAY_DIM];
};

/* Level 2: Function pointers within structs */
struct func_container {
    /* Function pointer with complex return type */
    struct outer1* (*func1)(int, char*);
    
    /* Nested function pointer with attributes */
    void (*func2)(void) __attribute__((noreturn));
    
    /* Multi-dimensional array of function pointers */
    int (*(*func_array[3])(double))[5];
};

/* Level 3: Deeply nested with all delimiter types */
typedef union {
    struct {
        /* Bit-field with unusual size expression */
        unsigned int flags : (sizeof(int) * 8 - 1);
        
        /* Anonymous union inside struct */
        union {
            int i;
            /* Pointer to array with computed size */
            char (*str)[sizeof(struct outer1) + 10];
        };
        
        /* Function pointer returning pointer to array */
        int (*(*get_matrix)(void))[][10];
    } data;
    
    /* Array of structs containing function pointers */
    struct func_container containers[2];
} mega_union_t;

/* Level 4: GNU extensions with nested attributes */
typedef struct __attribute__((packed, aligned(8))) {
    /* Vector type extension */
    typedef int v4si __attribute__((vector_size(16)));
    
    /* Nested struct with attribute on member */
    struct {
        v4si vec WEIRD_CHARS;
        /* __attribute__ inside parentheses */
        void (*callback)(int) __attribute__((deprecated));
    } vector_data;
    
    /* Flexible array member */
    long flex_array[];
} gnu_ext_struct;

/* Level 5: The ultimate complex declaration mixing everything */
FUNC_PTR_TYPEDEF(ultimate_callback_t);

typedef ultimate_callback_t (*(*complex_factory)(
    /* Arguments with attributes and weird chars */
    int param1 __attribute__((unused)),
    const char *param2 WEIRD_CHARS,
    /* Nested type in parameter */
    struct { int x; double y; } param3,
    /* Empty parameter with just comment */
    void /* intentionally empty */
))[ /* Array dimension with expression */
    (sizeof(mega_union_t) + 0x10) / sizeof(void*)
];

/* Level 6: Enum with computed values */
enum weird_enum {
    VAL1 = (1 << 0),  /* Parentheses with shift */
    VAL2 = sizeof(struct outer1),
    VAL3 = VAL1 | VAL2,  /* Binary operator */
    VAL4 = (int){0}  /* Compound literal in enum */
};

/* Level 7: Struct with anonymous members and nested braces */
struct anonymous_madness {
    struct { union { int a; char b; }; } nested;
    
    /* Designated initializer syntax in type context */
    int array[3] = { [0] = 1, [2] = 3 };
    
    /* Nested switch-like syntax in bit-field */
    unsigned int state : 2 + ({
        int x = 3;
        x;
    });
};

/* Level 8: Multiple typedefs with pointer chains */
typedef int*** triple_ptr_t;
typedef triple_ptr_t (*func_returning_triple_ptr)(void);

/* Level 9: __builtin types */
typedef __builtin_va_list va_list_t;
typedef __typeof__(int[4]) array4_int_t;

/* Level 10: Final monster declaration */
static volatile const struct __attribute__((may_alias)) {
    /* All three delimiters deeply nested */
    union {
        struct {
            int (*(*(*complex_array)[ /* comment inside brackets */ 5])
                 (char (*)[ /* nested brackets */ sizeof(int) ]))
                 (void (*)(int, ...));
        }* ptr;
        
        /* Nested braces with attributes */
        mega_union_t data __attribute__((aligned(32)));
    } u ATTR_ALIGN;
    
    /* Empty struct with just a semicolon */
    struct {} empty;
    
    /* Line continuation inside */
    char multiline_str[ \
        100 \
    ];
} final_struct = { /* Initializer with nested braces */
    .u.data.containers[0].func_array[1] = 0,
    .multiline_str = { /* Array initializer */
        'a', 'b', /* comma separated */
        '\0' /* null terminator */
    }
};

/* Additional declarations to increase parsing events */

/* Typedef with __attribute__ in middle */
typedef struct tag {
    int x;
} __attribute__((packed)) my_struct_t;

/* Function prototype with nested parentheses */
extern void (*signal(int sig, void (*func)(int)))(int);

/* K&R style function declaration for variety */
int old_style_func(p, str)
    int p;
    char *str;
{
    return p;
}

/* Multiple labels and goto (unusual in type context but valid) */
struct with_label {
    int value;
    /* This will trigger default case when parsing */
    __label__ loop_start, loop_end;
};

/* Assembly inline - extreme case for default case */
typedef struct {
    int result;
    asm volatile ("nop" : : : "memory"); /* assembly statement */
} asm_struct __attribute__((unused));

/* The file ends with complex type definitions only */
/* No main function needed - gengtype parses declarations only */
