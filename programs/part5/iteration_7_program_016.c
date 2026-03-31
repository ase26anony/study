/* test-gengtype-parser.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 3) /* Contains parentheses and shift operator */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* Trigger default case with unusual characters in macro */
#define WEIRD_CONSTANT 0xDEADBEEF /* Hex constant with letters */
#define LINE_CONT \
    int /* Backslash continuation inside type definition */

/* Complex nested type 1: Struct with all delimiter types */
struct Outer1 {
    /* Function pointer with attributes inside struct */
    int (*callback)(char *buffer[ARRAY_DIM], 
                    struct Outer1 ***self) ATTR_ALIGN;
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int flags : 4;
            /* Array with computed size containing parentheses */
            char data[(ARRAY_DIM * sizeof(int)) + 1];
        } ATTR_ALIGN;
        /* Default case trigger: numeric constant inside union */
        long raw_value WEIRD_CONSTANT;
    };
    
    /* Multi-dimensional array with complex dimensions */
    double matrix[ARRAY_DIM][(ARRAY_DIM / 2) + 1];
    
    /* GCC vector extension with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
};

/* Type 2: Deeply nested function pointer type */
typedef void (*(**complex_func_ptr)(int, 
    /* Comment inside parentheses to trigger default case */
    struct {
        int depth;
        char *name;
    }))[/* Array size with arithmetic */ 3 + 2];

/* Type 3: Intermixed delimiters with GNU extensions */
union WeirdUnion {
    /* Nested struct with attribute containing parentheses */
    struct __attribute__((packed, aligned(8))) {
        /* Function pointer returning pointer to array */
        int (*(*get_array)(void))[10];
        
        /* Anonymous union inside struct */
        union {
            /* Pointer to function with complex arguments */
            void (*func)(int (*)(char), float);
            /* Array of pointers to functions */
            int (*arr_func[5])(double, ...);
        };
    } nested;
    
    /* Bitfield with unusual size expression */
    unsigned long bits : (sizeof(int) * 8 - 1);
    
    /* Zero-length array GNU extension */
    char flexible[];
};

/* Type 4: Recursive type definition with all delimiters */
typedef struct TreeNode TreeNode;
struct TreeNode {
    /* Self-referential pointers */
    TreeNode *left, *right;
    
    /* Union containing array of function pointers */
    union {
        /* Mixed delimiters: function returning pointer to array */
        int (*(*operations[3])(TreeNode *))[];
        
        /* Struct with nested parentheses in bitfield */
        struct {
            int tag : (1 << 2); /* Parentheses in bitfield width */
            /* Array with size containing macro expansion */
            char key[ARRAY_DIM + 1];
        };
    } data;
    
    /* Attribute with multiple parentheses pairs */
    int weight __attribute__((aligned((sizeof(double)))));
};

/* Type 5: Extreme nesting with macros */
FUNC_PTR_TYPEDEF(SimpleFunc);
typedef SimpleFunc (*FuncArray[(ARRAY_DIM > 4) ? 2 : 3]);

struct Container {
    /* All three delimiters in one declaration:
     * 1. Outer braces for struct
     * 2. Parentheses for function pointer
     * 3. Brackets for array
     */
    struct {
        FuncArray (*get_funcs)(int param /* default case: comment */);
        
        /* Nested array of structs with bitfields */
        struct {
            unsigned x : 1;
            unsigned y : (3 + 2); /* Parentheses in bitfield */
            unsigned z : 1;
        } points[10][(ARRAY_DIM / 2)];
    } ATTR_ALIGN inner;
    
    /* Line continuation inside type definition */
    LINE_CONT counter;
};

/* Type 6: Using typeof extension with nested delimiters */
typedef typeof(&((struct Outer1*)0)->callback) CallbackPtr;
typedef CallbackPtr (*CallbackRegistry)[/* Empty size */];

/* Type 7: Complex declaration spanning multiple lines */
static volatile const struct {
    /* Function with __attribute__ containing parentheses */
    void (__attribute__((noreturn)) *exit_handler)(int);
    
    /* Anonymous struct with array of pointers to functions
     * returning pointers to arrays */
    struct {
        int (*(*(*lookup)[5])(const char *))[];
    } ATTR_ALIGN;
    
    /* Union with transparent_union attribute */
    union __attribute__((transparent_union)) {
        int *intp;
        char **charpp;
    } u;
} global_var = {
    .exit_handler = 0,
    .u.intp = 0
};

/* Type 8: Designated initializers with nested braces */
typedef struct {
    int a;
    struct {
        char b[3];
        float c;
    } nested;
} InitExample;

/* Initializer with nested braces to test parser */
InitExample init = {
    .a = ARRAY_DIM, /* Macro with parentheses */
    .nested = {
        .b = {'x', /* Comment between braces */ 'y', 'z'},
        .c = 3.14 /* Floating point constant */
    }
};

/* Type 9: __builtin types with attributes */
typedef __builtin_va_list va_list_wrapper;
typedef va_list_wrapper (*va_func)(int, ...) 
    __attribute__((format(printf, 2, 3)));

/* Type 10: Final complex typedef mixing everything */
typedef union {
    /* Struct containing array of function pointers
     * with attributes */
    struct {
        int (__attribute__((const)) *pure_funcs[5])(void);
        
        /* Pointer to array of structs containing unions */
        struct {
            union {
                int i;
                double d;
            } value;
            char tag;
        } (*item_list)[];
    } s;
    
    /* Function pointer returning pointer to function */
    void (*(*(*func_ret_func)(void))(int))(char);
    
    /* Simple member to trigger default case with hex constant */
    unsigned long raw WEIRD_CONSTANT;
} UltimateType ATTR_ALIGN;

/* Additional preprocessor trickery */
#if 1
/* Conditional compilation with nested delimiters */
typedef struct {
    int test;
} ConditionalType;
#else
/* Alternative branch with different delimiter structure */
typedef union {
    float f;
} AlternativeType;
#endif

/* Trigger default case with string literal in array size? */
/* Note: This is invalid C but gengtype might parse it partially */
/* char bad[("test")]; */ /* Would cause error but might hit default case */

/* Multiple type definitions in sequence to increase parsing events */
typedef UltimateType *UltimatePtr;
typedef UltimatePtr (*UltimateFactory)(int count, ...);
typedef UltimateFactory FactoryArray[10];

/* Empty struct/union to test edge cases */
struct Empty { };
union Void { };

/* Final declaration with all delimiter types interleaved */
static FactoryArray (*system_factories)(void) = 0;

/* The file ends with complex type definitions only - no main function
 * as this file is meant for gengtype parsing only */
