/* test-gengtype-parser.c - Complex type definitions to test gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (x + 1)
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define FUNC_PTR_TYPEDEF(name, ret, ...) typedef ret (*name)(__VA_ARGS__)

/* Trigger default case with unusual characters in macro expansions */
#define WEIRD_CONSTANT 0xDEADBEEF /* hex constant */
#define MULTILINE_MACRO \
    int multi_line_var; \
    /* comment inside macro */ \
    char another

/* Level 1: Basic nested structures */
struct level1 {
    int a;
    struct nested_in_level1 {
        char b;
        /* Default case trigger: numeric constant inside braces */
        float c[10 + 5]; /* 10 + 5 triggers advance() on '+' and digits */
    } inner;
    /* GCC attribute with parentheses */
    unsigned long d ALIGN_ATTR;
};

/* Level 2: Union with function pointers */
union level2 {
    /* Function pointer with complex return type */
    struct level1* (*func_ptr1)(int, char);
    
    /* Nested anonymous struct with bit-field */
    struct {
        unsigned int bitfield1 : 4;
        unsigned int bitfield2 : 8;
        /* Array with macro-expanded size */
        int dynamic_array[ARRAY_SIZE(10)];
    } PACKED_ATTR;
    
    /* Another function pointer with attributes */
    void (*__attribute__((noreturn)) fatal_error)(const char* msg);
};

/* Level 3: Typedef with all delimiter types mixed */
typedef struct level1* (*complex_func_ptr_t)(
    int param1, 
    /* Default case: line continuation inside parentheses */
    char param2,\
    /* The backslash should trigger advance() in default case */
    struct level2 param3[ARRAY_SIZE(5)],
    /* Comment inside parameter list */
    void (*callback)(int, int) /* ))) nested parentheses */
) [10]; /* Returns pointer to array */

/* Level 4: Deeply nested with GCC extensions */
struct __attribute__((aligned(32))) outer_container {
    /* Vector type (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    
    /* Anonymous union inside anonymous struct */
    struct {
        union {
            complex_func_ptr_t func_array[5];
            /* Nested array of structs with bit-fields */
            struct {
                unsigned a : 1;
                unsigned b : 2;
                unsigned c : 3;
            } bits[3][2];
        };
        
        /* Pointer to function returning pointer to array */
        int (*(*nested_func_ptr)(void))[10];
    } inner_container;
    
    /* Macro expansion that creates complex type */
    MULTILINE_MACRO;
};

/* Level 5: Even more complex with all delimiters intertwined */
typedef union {
    /* Struct with array of function pointers */
    struct {
        int (*(*callbacks[3])(int, ...))(char*);
        /* Default case: numeric constant inside brackets */
        double matrix[2+3][4*2]; /* + and * trigger advance() */
    } s;
    
    /* Function pointer with nested attributes */
    void (*(* __attribute__((deprecated("use v2 instead"))) old_func)(
        /* Parameter with attribute */
        int __attribute__((unused)) dummy,
        /* Array parameter with weird size */
        char data[WEIRD_CONSTANT & 0xFF]
    ))(int);
    
    /* Anonymous enum inside union */
    enum {
        VALUE1 = 0x1,
        VALUE2 = 0x2,
        /* Default case: hex constant */
        VALUE3 = 0x3 + 0x4  /* + triggers advance() */
    } flags;
} ultimate_type_t;

/* Level 6: Multiple levels of parentheses */
FUNC_PTR_TYPEDEF(
    mega_callback_t,
    struct outer_container*,
    /* Multiple nested function pointer parameters */
    void (*param1)(int, int (*)(char)),
    ultimate_type_t param2[][5],
    /* Empty parameter (void) in middle */
    void,
    /* Final parameter with attribute */
    const char* __attribute__((nonnull(1))) msg
);

/* Level 7: The kitchen sink - everything combined */
struct final_test {
    /* Nested type definitions */
    typedef struct {
        /* Mixed delimiters in single declaration */
        int (*(*complex_array[2])[3])(float, double);
    } nested_typedef;
    
    /* Union with anonymous struct containing array of unions */
    union {
        struct {
            union {
                int a;
                char b;
            } u1, u2[2];
        };
        
        /* Function pointer returning function pointer */
        void (*(*(*func_ception)(void))(int))(char);
    } data;
    
    /* Variable with multiple attributes */
    volatile unsigned char special ALIGN_ATTR PACKED_ATTR;
    
    /* Macro that expands to something with all delimiters */
    #define LOCAL_MACRO(x) { (x), [0] = { .a = 1 } }
    struct { int a; int b; } macro_var = LOCAL_MACRO(42);
};

/* Additional edge cases */

/* Empty struct with attributes */
struct __attribute__((packed)) empty {};

/* Typedef for pointer to array of function pointers */
typedef int (*(*(*triple_ptr)[5])(float))[10];

/* Struct with designated initializers (GCC extension) */
struct with_designated_init {
    int a;
    int b;
    int c;
} global_var = {
    .a = 1,
    .b = 2,  /* comma triggers advance() in default case */
    .c = 3
};

/* Forward declaration inside complex context */
struct forward_decl;
typedef struct forward_decl* (*factory_func)(int size);

/* Final typedef using typeof (GCC extension) */
typedef typeof(global_var) auto_type;

/* Multiple declarations on one line with different delimiters */
int simple, *pointer, array[10], (*func_ptr)(void), value = {0};

/* End of complex type definitions */
