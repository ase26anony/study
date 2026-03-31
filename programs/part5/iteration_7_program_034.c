/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* Trigger default case with unusual characters in type definitions */
#define ARRAY_DIM (16 /* comment with () */ + 4)
#define ATTR_SPEC __attribute__((aligned(16)))
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char)

/* Preprocessor directive inside would be problematic, but we can use
 * line continuations and comments within macro expansions */
#define COMPLEX_SIZE \
    (sizeof(struct { \
        int x; \
        char y; \
    }) + 8)

/* Type 1: Struct with nested delimiters and function pointer */
struct outer_struct {
    int simple;
    
    /* Function pointer with attributes - contains () and possibly other chars */
    void (*callback)(int param1, 
                     char param2, 
                     float param3) ATTR_SPEC;
    
    /* Array with complex dimension containing parentheses */
    int matrix[ARRAY_DIM][ARRAY_DIM * 2];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            unsigned int flag3 : 3 /* comment here */;
        } bits;
        unsigned char bytes[4];
    } flags;
    
    /* Pointer to array of function pointers */
    int (*(*func_array)[10])(double, float);
};

/* Type 2: Typedef with all delimiter types mixed */
typedef struct {
    /* Nested struct with array of pointers */
    struct inner {
        void **data_ptrs[COMPLEX_SIZE];
        int (*compare)(const void *, const void *);
    } *inner_ptr;
    
    /* Union containing anonymous struct with bit-fields */
    union {
        struct {
            long : 16;  /* Unnamed bit-field */
            long value : 32;
            long : 16;
        };
        unsigned long long raw;
    } data;
    
    /* Complex function pointer returning pointer to array */
    float (*(*get_data)(int index, 
                        char mode, 
                        void *context))[/* empty size */];
    
    /* GCC vector extension */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
} complex_type_t;

/* Type 3: Even more complex with macro expansions inside */
FUNC_PTR_TYPEDEF(simple_handler_t);

struct container {
    /* Using macro that expands to function pointer typedef */
    simple_handler_t handlers[5];
    
    /* Nested parentheses in array dimensions */
    double weights[(1 << 3) * sizeof(int*)];
    
    /* Anonymous struct with __attribute__ */
    struct {
        int x ATTR_SPEC;
        int y __attribute__((packed));
        int z __attribute__((deprecated));
    } point;
    
    /* Pointer to function returning struct with nested union */
    struct result {
        union {
            int int_val;
            double dbl_val;
            void *ptr_val;
        } value;
        int status;
    } (*(*compute)(int a, int b));
};

/* Type 4: Enum with complex expressions */
enum sizes {
    SMALL = 8,
    MEDIUM = (SMALL * 2) + 4,
    LARGE = (MEDIUM << 2) / 3,
    HUGE = sizeof(struct outer_struct) + COMPLEX_SIZE
};

/* Type 5: Union with nested everything */
union mega_union {
    /* Struct with array of function pointers */
    struct {
        int (*(*ops[5]))(void);
        char name[32];
    } funcs;
    
    /* Another struct with bit-fields and pointer to array */
    struct {
        unsigned int : 4;  /* Padding bits */
        unsigned int count : 12;
        unsigned int : 16;
        int (*data)[];
    } dynamic;
    
    /* Direct array with complex initialization (not for parsing, but for tokens) */
    long long raw[8];
};

/* Type 6: Typedef for pointer to function returning pointer to array */
typedef int (*(*(*triple_indirect_func)(char c, 
                                         int i, 
                                         double d))[10])[20];

/* Type 7: Struct with __attribute__ containing nested parentheses */
struct attributed {
    int field1;
    char field2;
    float field3;
} __attribute__((aligned(32), 
                 packed, 
                 deprecated("Use new_struct instead")));

/* Type 8: Using typeof with nested expressions */
struct type_of_example {
    /* This creates complex parentheses nesting */
    typeof(((struct outer_struct*)0)->matrix[0][0]) element_type;
    typeof(&((struct container*)0)->point) point_ptr_type;
};

/* Type 9: Zero-length array at end of struct (GCC extension) */
struct flexible {
    int count;
    double data[];  /* [] with nothing inside */
};

/* Type 10: Nested structs with all delimiter types in one declaration */
struct ultimate_nesting {
    /* Start with {} */
    struct {
        /* Then [] */
        int array[10];
        /* Then () in function pointer */
        void (*func)(void);
        /* Then {} again for anonymous union */
        union {
            /* () in cast-like expression in bit-field */
            int x : (8 * sizeof(char));
            /* [] in array */
            char y[4];
        };
        /* Mixed [] and () */
        int (*array_of_funcs[5])(int, char);
    } level1;
    
    /* Direct mixed declaration */
    union {
        struct {
            int a;
        } s;
        int b;
    } u ATTR_SPEC;
};

/* Additional complexity: forward declarations inside */
struct forward_decl;

/* Function pointer using forward declaration */
typedef void (*callback_t)(struct forward_decl *fd, int value);

/* Now define it */
struct forward_decl {
    callback_t cb;
    int data;
    /* Self-referential pointer */
    struct forward_decl *next;
};

/* Final type: everything combined */
typedef struct {
    struct outer_struct os;
    complex_type_t ct;
    struct container c;
    union mega_union mu;
    triple_indirect_func tif;
    struct attributed attr;
    struct type_of_example toe;
    struct flexible *flex;
    struct ultimate_nesting un;
    struct forward_decl *fd_list;
    
    /* One last complex member: array of pointers to functions 
     * returning pointers to arrays of structs */
    struct { int x; double y; } (*(*final_member[3])(int param))[5];
} everything_t;

/* Global variable using complex type */
everything_t global_var ATTR_SPEC;

/* Additional tokens to trigger default case:
 * - Numeric constants with different bases (0x, 0b)
 * - Character constants
 * - String literals in comments
 * - Line continuations
 * - More comments with parentheses */
struct with_constants {
    int decimal = 42;
    int hex = 0x2A;
    int binary = 0b101010;  /* GCC extension */
    int with_underscores = 1'000'000;  /* C23/GCC */
    char ch = '\n';
    char tab = '\t';
    char quote = '\'';
    char backslash = '\\';
};

/* End of complex type definitions */
