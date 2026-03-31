/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* Preprocessor macros that expand to delimiter-containing expressions */
#define ARRAY_DIM (1 << 2)  /* Contains parentheses and shift operator */
#define ATTR_ALIGN __attribute__((aligned(16)))  /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* Trigger default case with unusual characters in macro */
#define WEIRD_CHARS /* comment inside macro */ \
    __attribute__((deprecated("message with ()[]{}")))

/* Level 1: Basic nested structures */
struct outer1 {
    int a;
    /* Nested anonymous struct with array */
    struct {
        char buf[ARRAY_DIM];
        long *pointers[10];
    } inner;
    
    /* Function pointer with attributes */
    int (*callback)(const char *msg, ...) ATTR_ALIGN;
};

/* Level 2: Complex typedef with all delimiter types */
typedef struct {
    /* Bit-field with parentheses in expression */
    unsigned int flags : (sizeof(int) * 8 - 1);
    
    /* Union inside struct */
    union {
        /* Array of function pointers */
        void (*handlers[5])(void);
        
        /* Nested struct with computed array size */
        struct {
            double matrix[3][3];
            char *(*get_name)(int idx);
        } data;
    } u;
    
    /* Pointer to array of structs */
    struct outer1 (*items[])[10];
} ComplexType ATTR_ALIGN;

/* Level 3: Deeply nested function pointer type */
typedef char *(*(*DeepFuncPtr)(int (*compar)(const void *, const void *)))
               [ARRAY_DIM]
               (struct { int x; double y; } *param);

/* Level 4: GNU extensions with nested attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    /* Vector type with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
    
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned int a : 4;
            unsigned int b : (8 - 4);
        } bits;
        unsigned char byte;
    } WEIRD_CHARS;  /* Macro expands with comments and attributes */
    
    /* Function pointer with nested attributes */
    void (*__attribute__((noreturn)) fatal_error)(
        const char *fmt, 
        /* Default case trigger: numeric constant with unusual format */
        ... /* 0x1.0p-10 floating hex */ 
    );
};

/* Level 5: Intermixed delimiters in single declaration */
static union {
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_array[3])(float f))[][10];
    
    /* Struct containing all delimiter types */
    struct {
        /* Parentheses in sizeof */
        char buffer[sizeof(struct outer1) + 10];
        
        /* Brackets in array declaration */
        ComplexType *(*get_complex)(int idx)[ARRAY_DIM];
        
        /* Braces for initializer (in declaration) */
        int initialized = { 0 };
        
        /* Nested attributes with parentheses */
        __attribute__((format(printf, 1, 2)))
        void (*logger)(const char *, ...);
    } mixed;
} global_var WEIRD_CHARS;

/* Level 6: Type with macro expansions creating delimiter sequences */
FUNC_PTR_TYPEDEF(SimpleFuncPtr);

typedef SimpleFuncPtr (*FactoryFunc)(
    /* Parameter with array and function pointer */
    void *(**handlers[])(int, char),
    
    /* Attribute between parameters - triggers default case */
    __attribute__((unused)) int count,
    
    /* Empty [] for flexible array member in parameter type */
    struct { int len; char data[]; } *flex
);

/* Level 7: Multiple nested levels in one type */
typedef struct Node {
    /* Self-referential pointer */
    struct Node *children[ARRAY_DIM];
    
    /* Union with anonymous struct containing function pointer */
    union {
        struct {
            /* Complex function pointer type */
            int (*(*get_matrix)(void))[][10];
            
            /* Nested parentheses in cast-like expression */
            unsigned long mask : (unsigned long)(-1) >> 1;
        } s;
        
        /* Array with computed size containing pointers */
        void *(*callbacks[(sizeof(void*) * 8)]);
    } data;
    
    /* Attribute with string containing delimiters */
    char *name __attribute__((deprecated("Replace with {new_name}")));
} Node;

/* Level 8: Extreme nesting */
typedef void (*(*(*UltraNested)(
    /* Parameter with all delimiters */
    struct {
        int (*compare[2])(int a, int b);
        union {
            char str[];
            int num;
        } value;
    } *param,
    
    /* Array of arrays */
    int matrix[][10],
    
    /* Function pointer parameter */
    void (*callback)(Node *n)
))[5])(int);

/* Level 9: Enum with complex expressions */
enum {
    /* Expressions with parentheses */
    VALUE_A = (1 << 0),
    VALUE_B = (ARRAY_DIM * 2),
    
    /* Hex constant with p exponent (triggers default case) */
    VALUE_C = 0x1.0p+0,
    
    /* Character constant with escape */
    VALUE_D = '\n'
};

/* Level 10: Final complex declaration combining everything */
static struct {
    /* All three delimiters deeply nested */
    UltraNested (*get_ultra)(int mode) 
        __attribute__((warn_unused_result, 
                      deprecated("Use {new_get_ultra} instead")));
    
    /* Flexible array member with complex type */
    struct {
        Node node;
        ComplexType data;
    } items[];
} registry = {
    .get_ultra = NULL
};

/* Additional triggers for default case */

/* Line continuation inside type definition */
typedef struct { \
    int x; \
    char y; \
} ContinuedStruct;

/* Numeric constants in bit-field width */
struct WithWeirdBitfield {
    unsigned int a : 0x1;  /* Hex constant */
    unsigned int b : 07;   /* Octal constant */
    unsigned int c : 0b1;  /* Binary constant (GCC extension) */
};

/* Comments and preprocessor in tricky places */
struct WithComments {
    int normal;  /* Regular comment */
    int (*func)(/* Comment inside parentheses */ int x, /* another */ int y);
    int array[10 /* comment between brackets */];
};

/* Empty delimiters */
typedef void (*EmptyFunc)();
typedef int EmptyArray[];
struct EmptyStruct {};

/* GCC statement expression in type declaration (extension) */
typedef typeof(({ int x = 5; &x; })) IntPtrType;

/* __builtin_ types */
typedef __builtin_va_list VaList;
typedef __builtin_ffs FfsType;

/* Multiple attributes with nested parentheses */
struct MultiAttr {
    int field 
        __attribute__((aligned(32))) 
        __attribute__((deprecated))
        __attribute__((vector_size(16)));
};

/* End of type definitions - no main function needed for gengtype */
