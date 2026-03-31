/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* Trigger default case with unusual characters in type definitions */
#define ARRAY_DIM (16 /* comment with () */ + 4)
#define ATTR_ALIGN __attribute__((aligned(16)))
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* Preprocessor directive inside would be problematic, but we can use
 * line continuations within macro definitions */
#define COMPLEX_SIZE \
  1 << 2  /* Bit shift operator in macro */

/* Type 1: Struct with nested delimiters and function pointer */
struct Outer1 {
    int x;
    /* Function pointer with attributes inside parentheses */
    void (*callback)(int (*)(char **), ...) ATTR_ALIGN;
    
    /* Nested anonymous struct with bit-field */
    struct {
        unsigned int flags : 4;
        /* Array with macro-expanded dimension */
        char buffer[ARRAY_DIM];
    } inner;
    
    /* Union with array of function pointers */
    union {
        int (*func_array[COMPLEX_SIZE])(void);
        struct {
            long double ld ATTR_ALIGN;
        } nested;
    } u;
};

/* Type 2: Typedef with all three delimiters interdependently */
typedef struct {
    /* Pointer to array of structs */
    struct Element {
        int id;
        /* Nested parentheses in function pointer return type */
        struct Data *(*get_data)(struct Element *self, int idx);
    } (*element_array)[10];
    
    /* Complex declaration: function returning pointer to array */
    int (*(*complex_func)(int, ...))[5];
    
    /* GCC extension: vector type */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
} Container ATTR_ALIGN;

/* Type 3: Deeply nested with attributes in unusual places */
FUNC_PTR_TYPEDEF(GenericCallback);

struct DeepNested {
    /* Multiple levels of parentheses */
    GenericCallback (*get_callback(void (*)(int)))(int, char);
    
    /* Struct with __attribute__ inside */
    struct __attribute__((packed)) PackedStruct {
        char a;
        int b __attribute__((aligned(8)));
        /* Anonymous union with bit-fields */
        union {
            struct {
                unsigned int x : 3;
                unsigned int y : 5;
            } bits;
            char byte;
        } u;
    } ps;
    
    /* Array with computed size (parentheses with operators) */
    double computed_array[(1 + 2) * 3];
};

/* Type 4: Enum with complex initializers */
enum ComplexEnum {
    VALUE1 = (1 << 0),  /* Parentheses with shift operator */
    VALUE2 = sizeof(struct Outer1),  /* sizeof with type */
    VALUE3 = (int){0},  /* Compound literal */
    VALUE4 = (int)(3.14 * 2)  /* Cast with parentheses */
};

/* Type 5: Mix of all delimiter types in single declaration */
typedef union {
    /* Function pointer array with nested struct parameter */
    void (*handlers[5])(struct {
        int event_type;
        union {
            int ival;
            void *ptr;
        } data;
    } *event);
    
    /* Pointer to array of function pointers returning structs */
    struct Result {
        int status;
        char message[256];
    } (*(*result_getter)[10])(int id);
} UltimateUnion ATTR_ALIGN;

/* Type 6: GNU extension - nested functions (in struct) */
struct WithNested {
    int x;
    /* This creates complex parsing with parentheses */
    int (*comparator)(const void *, const void *);
    
    /* Zero-length array at end */
    char flexible[];
};

/* Type 7: __attribute__ with multiple arguments in nested context */
struct AttributeNest {
    /* Attribute on function pointer inside struct */
    int (__attribute__((const)) *pure_func)(int);
    
    /* Struct with alignment attribute containing array */
    struct __attribute__((aligned(32))) {
        __attribute__((vector_size(32))) float vec[4];
    } aligned_vec;
};

/* Type 8: Complex typedef with all delimiters */
typedef int (*(*ComplexTypedef)[5])(char (*)[10], 
                                    struct { int a; double b; } *);

/* Type 9: Trigger default case with numeric constants and comments */
struct WithNumbers {
    int x;  /* 0xDEADBEEF has letters that aren't delimiters */
    float y;  /* 3.14e-10 contains '.' and '-' */
    double z;  /* 6.02214076e23 */
    
    /* Array dimension with expression */
    char arr[1 + 2 - 3 * 4 / 5 % 6];
    
    /* Function pointer with ellipsis */
    int (*variadic)(int, ...);
};

/* Type 10: Final complex nested type */
struct FinalType {
    /* Nested anonymous struct with union */
    struct {
        union {
            /* Pointer to function returning pointer to array */
            int (*(*func1)(void))[10];
            /* Function taking function pointer as argument */
            void (*func2)(int (*)(int), ...);
        } fptr_union;
        
        /* Multi-dimensional array with attributes */
        int matrix[3][4] __attribute__((aligned(64)));
    } inner;
    
    /* Bit-field with computed width */
    unsigned int bits : (sizeof(int) * 8 - 1);
};

/* Additional declarations to increase parsing events */
typedef struct Outer1 *Outer1Ptr;
typedef Container ContainerArray[5];
typedef enum ComplexEnum (*EnumCallback)(int, void *);

/* Global variable using complex type */
static UltimateUnion global_union = {0};

/* Function prototype with complex parameter */
extern void process_complex(ComplexTypedef ct, 
                           struct WithNumbers wn[static 5]);

/* End of complex type definitions */
