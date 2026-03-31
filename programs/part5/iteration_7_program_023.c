/* test-gengtype-parser.c - Complex type definitions to test gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains parentheses and shift operator */
#define FUNC_ATTR __attribute__((noinline)) /* Attribute with parentheses */
#define BITFIELD_WIDTH 3:2 /* Contains colon character */

/* Complex nested type definition 1 */
typedef int (*func_ptr1_t)(char *arg1, 
                           /* Comment with unusual chars: <>&|^~ */
                           double arg2[ARRAY_DIM],
                           ... /* Ellipsis triggers default case */
                          ) FUNC_ATTR;

/* Struct with deeply nested delimiters */
struct outer_struct {
    /* Nested anonymous union */
    union {
        /* Function pointer member with all delimiter types */
        void (*(*complex_callback)(
            struct outer_struct *self,
            int matrix[3][4], /* 2D array */
            void (*handler)(int, ...) /* Varargs function pointer */
        ))(int, char **);
        
        /* Array of pointers to functions */
        int (*(*func_array[ARRAY_DIM])(float, ...))[];
        
        /* Bitfield with macro expansion */
        unsigned int flags : BITFIELD_WIDTH;
    };
    
    /* Nested struct with attribute */
    struct __attribute__((packed)) inner_struct {
        /* Pointer to array of structs containing function pointers */
        struct {
            func_ptr1_t callback;
            char name[];
        } (*items)[];
        
        /* Union within struct within struct */
        union {
            /* Complex declaration mixing all delimiters */
            int (*(*(*nested_func)(int (*(*)(int[][5]))()))[10])(void);
            
            /* Default case triggers: line continuation */
            char multi_line_string[] = "Line 1\
Line 2\
Line 3";
        } u;
    } packed_member;
};

/* Type definition with GNU extensions */
typedef int v4si __attribute__((vector_size(16), aligned(16)));

/* Union containing all delimiter types in one declaration */
union mega_union {
    /* Function returning pointer to array of structs */
    struct point {
        int x;
        int y;
        /* Anonymous struct member */
        struct {
            int z;
            /* Nested array with computed size */
            float coords[sizeof(struct point)];
        };
    } (*(*get_points)(int count))[];
    
    /* Complex function pointer declaration */
    union mega_union (*(*self_referential)(
        /* Default case: numeric constant with exponent */
        double scientific = 1.23e-4,
        /* Default case: preprocessor in argument (simulated) */
        int preproc_like = 0xDEADBEEF,
        /* Nested parentheses */
        void (*)((int), (char))
    ))(void);
    
    /* Array with multiple dimensions and attributes */
    unsigned char data[ARRAY_DIM][1 << 3] 
        __attribute__((aligned(8)));
};

/* Enum with unusual values */
enum tricky_enum {
    VALUE1 = (1 << 0), /* Parentheses */
    VALUE2 = 0x0,      /* Hex constant */
    VALUE3 = 0777,     /* Octal constant */
    VALUE4 = 1.2e3,    /* Floating constant - triggers default */
    VALUE5 = '\\',     /* Escape character */
    VALUE6 = L'宽',    /* Wide character */
    VALUE7 = VALUE1 | VALUE2 /* Binary operator */
};

/* Typedef with everything combined */
typedef struct outer_struct* (*(*ultimate_type)(
    /* Mixed delimiters in parameters */
    int param1[({ /* GCC statement expression */
        int x = 5; 
        x * 2; 
    })],
    
    /* Function pointer parameter */
    void (*param2)(struct { int a; int b; }),
    
    /* Varargs with attribute */
    ...
))[sizeof(union mega_union)] __attribute__((deprecated));

/* Additional complex declarations to increase coverage */

/* Nested function pointer type */
typedef int (*(*(*nested_fp)(int))[5])(float, double);

/* Struct with anonymous members and bitfields */
struct bitfield_struct {
    unsigned int a : 5;
    unsigned int : 3; /* Unnamed bitfield */
    signed int b : 10;
    unsigned int c : sizeof(int)*8 - 15;
    
    /* Array of function pointers */
    void (*actions[10])(int, ...);
    
    /* Pointer to self-referential struct */
    struct bitfield_struct *next;
};

/* Union with nested attribute */
union attr_union {
    int x __attribute__((aligned(16), deprecated));
    char y[10] __attribute__((packed));
    
    /* Function pointer with attribute on return type */
    __attribute__((noreturn)) void (*fatal_error)(const char *msg);
};

/* Typedef using typeof extension */
typedef typeof(sizeof(int)) size_type;

/* Struct with flexible array member and attribute */
struct flex_array {
    int count;
    double data[] __attribute__((aligned(8)));
};

/* Final complex declaration mixing everything */
static volatile const struct outer_struct * const * (*(*global_callback)(
    int,
    struct flex_array *,
    ultimate_type
)) = 0;

/* End of type definitions */
