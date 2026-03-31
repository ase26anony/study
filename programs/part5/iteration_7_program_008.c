/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define ATTR_SPEC __attribute__((aligned(16)))
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)
#define NESTED_MACRO(x) { .val = (x) }

/* 2. Complex type with all delimiter types mixed */
typedef struct GlobalStruct {
    /* Nested parentheses in function pointer with attribute */
    int (* ATTR_SPEC complex_func_ptr)(char *(*callback)(int[ARRAY_DIM]), ...);
    
    /* Array with computed size containing parentheses */
    double matrix[ARRAY_DIM][(sizeof(int) + 3)];
    
    /* Nested anonymous union within struct */
    union {
        struct {
            /* Bit-field with unusual syntax */
            unsigned int flag: 1;
            /* Line continuation inside struct \
               this comment continues */
            long long big_num;
        } inner;
        /* Array of function pointers */
        void (*func_array[5])(void);
    } ATTR_SPEC;
    
    /* Pointer to array of structs with GNU attributes */
    struct InnerStruct {
        __attribute__((packed)) char data;
        /* Nested parentheses in typeof expression */
        typeof(&data) data_ptr;
    } * ATTR_SPEC ptr_array[];
} GlobalStruct_t;

/* 3. Function pointer typedef with deeply nested delimiters */
FUNC_PTR_TYPEDEF(ComplexFuncPtr);

/* 4. Union with attribute containing unusual characters */
union __attribute__((aligned(32))) WeirdUnion {
    /* Multi-dimensional array with mixed delimiters */
    int (*array_ptr)[3][(2 + 1)];
    
    /* Nested function pointer with varargs */
    void (**nested_fp)(struct { int x; } *, ...);
    
    /* GNU statement expression in initializer position */
    int val;
} ATTR_SPEC;

/* 5. Enum with computed values (contains operators) */
enum SpecialEnum {
    /* These values contain operators that aren't delimiters */
    VALUE_A = (1 << 0),  // Shift operator
    VALUE_B = (2 & 3),   // Bitwise AND
    VALUE_C = (4 | 5),   // Bitwise OR
    VALUE_D = (6 ^ 7)    // Bitwise XOR
};

/* 6. Typedef chain with all delimiter types */
typedef int (*(*NestedFuncPtr)[5])(char *str[((2) + (3))], ...);

/* 7. Struct with designated initializers (contains '=' and '.') */
struct WithInitializer {
    int x;
    double y;
    char z[10];
} instance = {
    .x = (1 + 2) * 3,  /* Expression with operators */
    .y = 3.14,
    .z = { 'a', 'b', /* Embedded comment */ 'c' }
};

/* 8. Complex declaration using typeof and attributes */
typeof(instance) * ATTR_SPEC instance_ptr 
    = &(instance);

/* 9. Function-like macro used in array dimension */
#define DYNAMIC_DIM(x) ((x) + 1)
int dynamic_array[DYNAMIC_DIM(5)];

/* 10. Nested struct with bitfields and unusual spacing */
struct Outer {
    struct /* anonymous */ {
        unsigned a : 1;  /* Colon character triggers default case */
        unsigned b : 2;
        unsigned c : 3;
    } bits;
    
    /* Pointer to function returning pointer to array */
    int (*(* ATTR_SPEC complex_member)(void))[10];
    
    /* Union with nested struct containing array of pointers */
    union {
        struct {
            void * ATTR_SPEC ptr_array[5];
            /* Nested parentheses in sizeof */
            char buffer[sizeof(void *[DYNAMIC_DIM(3)])];
        } data;
        /* Function pointer with __attribute__ inside parentheses */
        int (* ATTR_SPEC alt_func)(int (*(*)[5])());
    } choice;
};

/* 11. Type with __extension__ (GNU extension) */
typedef __extension__ struct {
    /* Vector type (GNU extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
    
    /* Nested anonymous enum */
    enum { E1, E2 = (1 << 1) } enum_val;
} ExtensionStruct;

/* 12. Multiple levels of pointer indirection with attributes */
char * ATTR_SPEC * ATTR_SPEC * ATTR_SPEC triple_ptr;

/* 13. Struct with flexible array member containing function pointers */
struct Flexible {
    int count;
    /* Flexible array member with function pointers */
    void (*handlers[])(struct Flexible *, ...);
};

/* 14. Complex typedef with nested parentheses and brackets */
typedef int (*(*(*ComplexTypedef)[10])(int (*)(char *), ...))[20];

/* 15. Final type with everything combined */
typedef union {
    struct {
        /* All three delimiters in one member declaration */
        int (*(* ATTR_SPEC member1)(int (*)[ARRAY_DIM]))(char *(*)[], ...);
        
        /* Array of structs containing unions containing... */
        struct {
            union {
                /* Default case triggers: numbers and operators */
                int x[(1 + 2 * 3 - 4 / 2)];
                float y;
            } u;
            /* Attribute with multiple arguments */
            __attribute__((deprecated("message"), packed)) char z;
        } array[5];
    } s;
    
    /* Function pointer with nested attribute */
    void (* ATTR_SPEC func_ptr)(__attribute__((unused)) int param);
} UltimateUnion_t;

/* 16. Macro that expands to contain all delimiter types */
#define ULTIMATE_MACRO(type) \
    type (*array[(2+3)])[5]; \
    void (*func)(struct { type x; } *)

/* Use the macro in a struct */
struct UseMacro {
    ULTIMATE_MACRO(int);
    ULTIMATE_MACRO(char);
};

/* 17. Type containing line continuations and comments */
typedef struct WithContinuation {
    int value1; /* Comment with operators: 1 << 2 == 4 */
    int value2; /* Another: 3 & 7 */
    char * ATTR_SPEC \
        multi_line_ptr;  /* Line continuation */
} WithContinuation_t;

/* 18. Forward declarations to test parser recovery */
struct ForwardDecl;
union AnotherForward;

/* 19. Opaque typedef */
typedef struct Opaque *OpaqueHandle;

/* 20. Final global variable with complex initializer */
UltimateUnion_t global_var = {
    .s = {
        .member1 = NULL,
        .array = { [0] = { .u = { .x = {1, 2, 3} }, .z = 'A' } }
    },
    .func_ptr = NULL
};

/* Note: No main() function needed - gengtype only parses type definitions */
