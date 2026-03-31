/* test-gengtype-coverage.c
 * Complex type definitions to test gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* Preprocessor macros that expand to delimiter-containing expressions */
#define ARRAY_DIM (1 << 2)  /* Contains parentheses and shift operator */
#define ATTR_ALIGN __attribute__((aligned(16)))  /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* Trigger default case with unusual characters in nested contexts */
#define WEIRD_CHARS /* comment with () */ 123UL \
    + 0xDEADBEEF  /* Line continuation and hex constant */

/* Complex typedef with all delimiter types */
typedef struct {
    /* Nested anonymous union with attributes */
    union ATTR_ALIGN {
        int x;
        /* Function pointer member with attributes */
        void (*callback)(int, char**) __attribute__((nonnull(1)));
    } data;
    
    /* Array with macro-expanded dimension containing parentheses */
    char buffer[ARRAY_DIM * 2];
    
    /* Bit-field with unusual width expression */
    unsigned int flags: (sizeof(int) * 8 - 1);
    
    /* Pointer to array of function pointers */
    int (*(*func_table)[ARRAY_DIM])(double, ...);
} MasterStruct ATTR_ALIGN;

/* Another complex type mixing all delimiters */
typedef union {
    struct {
        /* Nested parentheses in array dimension */
        float matrix[3][(2 + 1)];
        
        /* GNU statement expression in type context */
        typeof(({ int y = 5; &y; })) weird_ptr;
    } s;
    
    /* Function returning pointer to array */
    int (*(*get_matrix(void))[3][3])(void);
} ComplexUnion;

/* Function pointer type with attributes between parentheses */
typedef void (ATTR_ALIGN *SpecialFunc)(
    int param1,  /* Parameter with comment: () */
    struct {      /* Anonymous struct parameter */
        char *data[WEIRD_CHARS];  /* Macro with line continuation */
        long double value;
    } param2,
    ...           /* Variadic arguments */
) __attribute__((format(printf, 2, 4)));

/* Struct with deeply nested delimiter combinations */
struct Container {
    /* Pointer to function returning pointer to array of structs */
    struct Element *(*(*lookup)(int id))[ARRAY_DIM];
    
    /* Anonymous struct containing union containing struct... */
    struct {
        union {
            struct {
                /* Nested attributes with parentheses */
                int x __attribute__((deprecated("use y instead")));
                int y;
            } ATTR_ALIGN;
            double d;
        } value;
        
        /* Array of function pointers with complex signature */
        int (*(*handlers[5]))(struct Container *, ...);
    } state;
    
    /* GNU vector type with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
};

/* Enum with computed values containing parentheses */
enum WeirdEnum {
    FIRST = (1 << 0),
    SECOND = (1 << 1) | FIRST,
    THIRD = (SECOND + 1) * 2  /* Arithmetic with parentheses */
};

/* Typedef using all three delimiters in one declaration */
typedef int (*(*(*UltraComplexType)[
    (sizeof(void*) == 8) ? 2 : 1  /* Conditional with parentheses */
])(struct {
    int a; 
    char b[(1 + 2)];  /* Brackets with parentheses inside */
}))(void (*)(int), ...);

/* Macro that expands to type with nested delimiters */
#define MAKE_COMPLEX_TYPE(name) \
    typedef struct name { \
        union { \
            int (*func)(int, [(1){2}]); /* GCC array designator extension */ \
            char str[]; \
        } u; \
        _Static_assert(sizeof(int) == 4, "int must be 4 bytes"); \
    } name##_t

/* Use the macro to generate another complex type */
MAKE_COMPLEX_TYPE(Generated);

/* Final struct with everything combined */
struct FinalTest {
    /* All three delimiters in member declaration */
    void (*(*signal_handlers[3])(
        int sig, 
        /* Nested struct in parameter */
        struct { 
            const char *name; 
            void *data[ARRAY_DIM]; 
        } *info
    ))(void);
    
    /* __attribute__ with nested parentheses and commas */
    unsigned char raw_data[256] 
        __attribute__((aligned(32), 
                       packed, 
                       deprecated("use safe_data instead")));
    
    /* Anonymous bitfield struct */
    struct {
        unsigned int a:1, b:2, c:(8 - 5);
    } flags;
    
    /* Pointer to const volatile function pointer */
    void (* const volatile (* volatile fn_ptr))(void);
};

/* Trigger default case with numeric constants and operators */
static const int defaults[] = {
    (1 + 2) * 3,      /* Parentheses with operators */
    [2] = 0xFF,       /* Designated initializer */
    sizeof(int[2]),   /* sizeof with brackets */
    /* Multi-line initializer with backslash */
    (unsigned long)0x80000000 \
        | 0x40000000,  /* Line continuation in constant */
    (int){0}           /* Compound literal */
};

/* Function prototype with complex return type */
struct Container *(**initialize_system(
    int level,
    ...  /* Variadic for future extension */
))(void);

/* End of complex type definitions */
