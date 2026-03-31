/* test-gengtype-coverage.c - Complex type definitions to exercise gengtype parser */

/* First, define some macros that expand to delimiter-heavy expressions */
#define ARRAY_DIM(x) [(x) + 1]
#define ATTR_PACKED __attribute__((packed))
#define FUNC_PTR(name) (*name)
#define NESTED_EXPR (1 << (sizeof(int)*8 - 1))

/* Trigger default case with unusual characters in macro expansions */
#define WEIRD_CHARS /* comment inside macro */ \
  (void)0; // line continuation with comment

/* Complex typedef with all delimiter types */
typedef int (*func_ptr_t1)(int (*callback)(char **argv[]), 
                           struct { 
                             int x; 
                             /* Default case trigger: numeric constant */
                             double y 3.14159e-10;
                           } *data);

/* Struct with deeply nested delimiters */
struct outer_struct {
    /* Nested parentheses in function pointer */
    void (*complex_func)(
        int, 
        /* Comment between parentheses - triggers default case */
        /* This should make parser call advance() on '/' and '*' */
        struct inner { 
            char c; 
            /* Attribute with parentheses inside struct */
            int i ATTR_PACKED; 
        } *
    );
    
    /* Array with complex dimension containing parentheses */
    int matrix ARRAY_DIM( (2+3)*4 );
    
    /* Union with anonymous struct containing bit-field */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            /* Trigger default case with preprocessor-like token */
            #if 0
            int never_compiled;
            #endif
        } bits;
        long long value;
    } u;
    
    /* Nested array of function pointers */
    int (*func_array[ (sizeof(void*) + 3) & ~3 ])(
        char *str,
        /* Default case: numeric with exponent */
        float f 1.0e+10,
        ...
    );
};

/* Another complex type mixing all delimiters */
typedef struct {
    /* Pointer to array of structs */
    struct element (*elements[])[10];
    
    /* Function returning pointer to array */
    int (*(*get_matrix)(void))[5][5];
    
    /* GNU extension: vector type */
    typedef int v4si __attribute__ ((vector_size (16)));
    v4si vectors[4];
    
    /* Anonymous union with attribute */
    union ATTR_PACKED {
        char bytes[16];
        /* Nested parentheses in cast expression */
        int *as_ints;
    };
} container_t;

/* Extreme nesting example */
typedef void (*(**(*complex_nest)( 
    int (*(*arg1)[/* bracket with comment */5])(char),
    struct {
        union {
            /* All three delimiters in one member */
            int (*(*func)(int[3], ...))[2];
            /* Default case: backslash in comment (won't appear post-preprocessing) */
            /* But preprocessor might leave whitespace */
        } u;
        /* Attribute with multiple parentheses */
        short s __attribute__((aligned((1+2)*4)));
    } arg2
))(void))(int);

/* Enum with complex initializers */
enum weird_enum {
    ZERO,
    ONE = (1 << 0),
    /* Expression with parentheses and brackets */
    TWO = sizeof(int[ (1+1) ]),
    /* Default case: floating point in integer context (should parse then error) */
    THREE = 3.0,
};

/* GCC-specific extension: __builtin_va_list */
typedef __builtin_va_list va_list_t;

/* Struct using typeof extension */
struct with_typeof {
    /* typeof with nested parentheses */
    typeof(*(int (*)[5])0) array_ref;
    
    /* __alignof__ with parentheses */
    char padding[__alignof__(long double)];
};

/* Function pointer with attributes in parameter */
typedef int (__attribute__((stdcall)) *stdcall_func_t)(
    /* Parameter with attribute */
    int arg1 __attribute__((unused)),
    /* Empty parentheses then comment */
    void /* comment */,
    /* Array parameter with static keyword */
    int arg3[static 5]
);

/* Union containing all delimiter types in one member */
union everything {
    struct {
        /* Mixed delimiters: function pointer array */
        void (*(*vtable[10])(int))[3];
        
        /* Nested braces with semicolons inside */
        struct { int a; char b; float c; } nested;
        
        /* Macro expansion creating complex type */
        int ARRAY_DIM( sizeof(struct {int x;}) ) array;
    } s;
    
    /* Raw data with bit-field */
    struct {
        unsigned int : 4;  /* Unnamed bit-field */
        unsigned int field: 4;
        /* Default case: multiple semicolons */
        ;;;;
    } bits;
};

/* Typedef that uses all three delimiters sequentially */
typedef int (*(*((*signal_handler)(int signum, 
                                   void (*oldact)(int), 
                                   /* Comment between commas */
                                   /* triggers default case */
                                   sa_handler_t))[2])(float))[5];

/* Final test: macro that expands to something with all delimiters */
#define ULTIMATE_TYPE(type) \
    union { \
        type (*func)(type (*)(type[2]), ...); \
        struct { type array[(sizeof(type)+3)]; } s; \
    }

/* Use the macro */
ULTIMATE_TYPE(double) ultimate_var;

/* Trigger parsing of multiple top-level declarations */
extern struct outer_struct *global_ptr;

/* Line continuation test inside type (preprocessor removes but parser sees tokens) */
typedef \
   int \
   multiline_type;

/* Empty declaration with just semicolons (triggers default case repeatedly) */
;;

/* End of complex type definitions */
