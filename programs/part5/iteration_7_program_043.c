/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (x + 1)
#define ALIGN_SPEC __attribute__((aligned(16)))
#define PACKED_SPEC __attribute__((packed))
#define FUNC_PTR(name) (*name)

/* Macro with unusual characters in expansion */
#define WEIRD_MACRO(x) x /* comment inside macro */ \
    + 1

/* Type 1: Deeply nested struct with all delimiter types */
struct level1 {
    int a;
    struct level2 {
        char b;
        /* Nested anonymous union with attributes */
        union {
            int x;
            double y;
        } PACKED_SPEC;
        
        /* Function pointer with complex arguments */
        int (*(*callback)(int (*)(char), double[ARRAY_SIZE(5)]))(void);
        
        /* Array with computed size containing parentheses */
        float arr[ARRAY_SIZE(sizeof(struct { int temp; }))];
    } inner;
    
    /* Bit-field with unusual size expression */
    unsigned int bits : WEIRD_MACRO(3);
    
    /* Pointer to array of function pointers */
    void (*(*func_array[10])(int, ...))[];
};

/* Type 2: typedef with interdependent delimiters */
typedef struct {
    /* Nested struct inside union */
    union {
        struct {
            int a;
            /* Attribute with parentheses inside struct */
            char b ALIGN_SPEC;
        } s;
        long l;
    } u;
    
    /* Complex function pointer declaration */
    int (*(*signal_handler)(int signum, 
                            void (*old_handler)(int),  /* nested function pointer */
                            const char *msg[][10]))(void *context);
    
    /* Multi-dimensional array with attribute */
    volatile double matrix[3][ARRAY_SIZE(4)] ALIGN_SPEC;
} ComplexType;

/* Type 3: GCC extension with vector types */
typedef int v4si __attribute__((vector_size(16)));

struct VectorStruct {
    /* Vector type inside struct */
    v4si vectors[2];
    
    /* Anonymous struct with vector operations */
    struct {
        v4si (*add)(v4si, v4si);
        v4si (*mul)(v4si, v4si);
    } ops;
    
    /* Pointer to function returning pointer to array of vectors */
    v4si (*(*(*get_vector_array)(void))[10])(void);
};

/* Type 4: Enum with computed values */
enum WeirdEnum {
    VAL1 = sizeof(struct { char c; int i; }),
    VAL2 = (1 << 8) | 0x0F,
    VAL3 = __LINE__,  /* Predefined macro */
    VAL4 = VAL2 + VAL3
};

/* Type 5: Union containing all delimiter types in single member */
union UltimateDelimiterTest {
    /* Single member using all delimiters */
    struct {
        /* Function pointer with array parameter */
        void (*func1)(int (*array_param)[sizeof(struct {int x;})]);
        
        /* Nested anonymous union with bitfields */
        union {
            unsigned int a : 1;
            unsigned int b : sizeof(char[5]);
        };
        
        /* Pointer to array of function pointers returning struct pointers */
        struct VectorStruct *(*(*callback_array[5])(int))[2];
    } s;
    
    /* Alternative: array of complex type */
    ComplexType alt[ARRAY_SIZE(2)];
};

/* Type 6: Typedef chain with attributes between delimiters */
typedef int (*FuncPtrType1)(int, char) ALIGN_SPEC;

typedef FuncPtrType1 (*FuncPtrType2)(FuncPtrType1, 
                                     /* Comment with unusual chars: <>&|^~ */
                                     double) PACKED_SPEC;

typedef struct {
    FuncPtrType2 chain_link;
    
    /* Array with size containing all operators */
    int computed_size[1 + 2 * 3 - 4 / 2 | 0xFF & 0x0F];
} ChainType;

/* Type 7: Forward declarations with attributes */
struct ForwardDecl ALIGN_SPEC;

/* Later definition with nested elements */
struct ForwardDecl {
    /* Nested struct definition */
    struct {
        /* Member with line continuation in initializer (for parser) */
        int value; /* = 1 \
                      + 2 */  /* This comment continues on next line */
        
        /* Function pointer with __attribute__ inside */
        void (*attr_func)(void) __attribute__((deprecated));
    } nested;
};

/* Type 8: Using typeof extension */
struct TypeofExample {
    /* typeof with nested delimiters */
    typeof(int (*[5])(char, double)) func_ptr_array;
    
    /* typeof containing struct definition */
    typeof(struct { 
        int a; 
        union { 
            char b; 
            long c; 
        } u; 
    }) anonymous_member;
};

/* Type 9: Mixed declarations at file scope */
/* Global variable with complex type */
static ComplexType global_var = {0};

/* Function pointer variable */
int (*(*global_callback)(int, ...))(char) = 0;

/* Array of structs with bitfields */
struct BitfieldStruct {
    unsigned a : 1;
    unsigned b : WEIRD_MACRO(2);
    unsigned c : 3 + 4;  /* Expression with operator */
} bitfield_array[] = {
    {1, 2, 3},
    {0, 3, 7}
};

/* Type 10: Final stress test - everything combined */
typedef union {
    struct {
        /* All three delimiters in sequence */
        int (*(*func)(struct { 
            int a; 
            char b[10]; 
        } param))[20];
        
        /* Nested attributes */
        __attribute__((aligned(8))) struct {
            __attribute__((packed)) union {
                short s;
                long l;
            } u;
        } aligned_struct;
    } s;
    
    /* Array with all operators in size */
    char raw_data[sizeof(int) + sizeof(double) - 1 * 2 / 4];
} UltimateType;

/* Additional preprocessor trickery */
#if defined(__GNUC__)
#  define GCC_ATTR(x) __attribute__(x)
#else
#  define GCC_ATTR(x) /* nothing */
#endif

/* Struct using conditional macro */
struct ConditionalAttr {
    int normal_member;
    void (*func_ptr)(void) GCC_ATTR((weak));
} GCC_ATTR((packed));

/* Multiple levels of indirection */
typedef int ****quad_ptr_t;

struct IndirectStruct {
    quad_ptr_t ptrs;
    
    /* Function returning quadruple pointer */
    int ****(*(*get_quad_ptr)(void))(void);
};

/* Line continuations inside type definitions (triggers default case) */
struct WithContinuation {
    int value1; /* = 1 \ */
    int value2; /* + 2 \ */
    int value3; /* + 3 */
    
    /* Multi-line comment with unusual characters */
    char *str; /* Comment with < > & | ^ ~ ` ! @ # $ % * - + = : ; " ' , . ? / */
};

/* Final type: recursive structure */
struct TreeNode {
    struct TreeNode *left;
    struct TreeNode *right;
    
    /* Function pointer operating on the tree */
    void (*visit)(struct TreeNode *, 
                  /* Callback with its own nested structure */
                  void (*callback)(struct { int depth; char *path; } info));
    
    /* Flexible array member */
    int data[];
};
