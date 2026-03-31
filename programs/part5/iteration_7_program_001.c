/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR __attribute__((section(".data")))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro that expands to something with unusual characters */
#define COMPLEX_DIM (1 << 3) /* Contains shift operator and comment */

/* 2. Complex typedef with all delimiter types */
typedef int (*complex_func_t)(
    int arg1, /* Comment inside parentheses */
    char *arg2[], /* Array parameter */
    struct inner {
        int x;
        union {
            long a;
            double b;
        } data;
    } *arg3
) ALIGNED_ATTR;

/* 3. Struct with deeply nested delimiters */
struct outer_struct PACKED_ATTR {
    /* Function pointer member with attributes */
    void (*callback)(
        int, 
        char, 
        /* Nested anonymous struct in parameter */
        struct { 
            short s; 
            /* Bit field with unusual value */
            unsigned int bits : 3 + 2; /* Contains '+' operator */
        } 
    ) SECTION_ATTR;
    
    /* Array of pointers to functions */
    complex_func_t (*func_array[])(void);
    
    /* Union with nested struct containing array */
    union {
        struct {
            int matrix[3][4]; /* 2D array */
            /* Pointer to array with computed size */
            float (*dynamic_array)[ARRAY_SIZE(matrix[0])];
        } s;
        long long raw_data;
    } data_union;
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned int flag1 : 1;
        unsigned int flag2 : 2 | 1; /* Contains '|' operator */
        unsigned int : 0; /* Unnamed bitfield */
    } flags;
};

/* 4. Type with GNU extensions */
typedef int v4si __attribute__((vector_size(16)));
typedef __attribute__((aligned(32))) struct {
    v4si vectors[2];
    /* Nested function pointer with __attribute__ */
    void (* __attribute__((noreturn)) error_handler)(int);
} simd_struct_t;

/* 5. Complex declaration mixing all delimiters */
static struct container {
    /* Pointer to function returning pointer to array */
    struct outer_struct *(*(*getter)(int index))[10];
    
    /* Array of function pointers with computed size */
    int (*handlers[COMPLEX_DIM])(struct container *);
    
    /* Anonymous union with attribute */
    union {
        /* Nested struct with array of structs */
        struct nested {
            enum { RED, GREEN, BLUE } color;
            /* Multi-dimensional array with attribute */
            unsigned char pixels[16][16] ALIGNED_ATTR;
        } nested_struct;
        
        /* Another path with different delimiters */
        struct {
            /* Function pointer array */
            void (*actions[5])(void);
            /* Pointer to const array */
            const int *(*get_values)(void)[];
        } alt;
    } MAYBE_UNUSED;
} global_container;

/* 6. Even more complex typedef chain */
typedef struct outer_struct *(*factory_func)(
    /* Parameter with attribute */
    int count __attribute__((unused)),
    /* Array parameter with size expression */
    char *names[count > 0 ? count : 1]  /* Ternary operator inside brackets */
);

/* 7. Type with line continuation in macro expansion */
#define MULTILINE_MACRO \
    struct { \
        int x; \
        char y; \
    }

typedef MULTILINE_MACRO multiline_struct_t;

/* 8. Declaration with all delimiters in one line (almost) */
static void (*signal_handlers[3])(int, siginfo_t *, void *) = { NULL, NULL, NULL };

/* 9. Struct with attribute containing parentheses */
struct with_attributes {
    int field1 __attribute__((deprecated("Use field2 instead")));
    int field2 __attribute__((aligned(8), packed));
    
    /* Nested type with pointer to array of function pointers */
    struct {
        int (**operations)(void);
        /* Array with GNU range extension */
        unsigned char buffer[0..255];
    } ops;
};

/* 10. Final complex type definition with everything */
typedef union ultimate_union {
    /* Anonymous struct */
    struct {
        /* Function returning pointer to function */
        int (*(*get_func)(int))(char *);
        
        /* Array of structs containing unions */
        struct {
            union {
                int i;
                /* Pointer to array with attribute */
                float (*array)[10] __attribute__((aligned(32)));
            } value;
            char tag;
        } items[20];
    } data;
    
    /* Another anonymous struct with bitfields */
    struct {
        unsigned long long low : 32;
        unsigned long long high : 32;
        /* Empty bitfield */
        unsigned long long : 0;
    } bits;
    
    /* Simple array fallback */
    unsigned char raw[64];
} ultimate_union_t;

/* 11. Forward declarations that might be parsed */
struct forward_decl;
typedef struct forward_decl *(*forward_factory)(void);

/* 12. Enum with computed values */
enum complex_enum {
    VALUE1 = 1 << 0,  /* Shift operator */
    VALUE2 = 1 << 1,
    VALUE3 = VALUE1 | VALUE2,  /* Bitwise OR */
    VALUE4 = (1 + 2) * 3,  /* Arithmetic in parentheses */
    VALUE5 = sizeof(struct outer_struct)  /* sizeof operator */
};

/* 13. Variable declarations using complex types */
static complex_func_t global_func = NULL;
static struct outer_struct instance = {0};
static ultimate_union_t global_union;

/* Note: No main() needed - this file is for gengtype parsing only */
