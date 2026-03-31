/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* First, define macros that expand to include various delimiters */
#define ARRAY_DIM (1 << 2) /* Contains parentheses and shift operator */
#define FUNC_ATTR __attribute__((noinline)) /* Attribute with parentheses */
#define WEIRD_CHARS "/* Not a comment */" /* String literal with comment-like chars */

/* Trigger default case with unusual characters in macro body */
#define DEFAULT_TRIGGER(x) x ## _suffix /* ## operator */
#define LINE_CONT \
    int /* Backslash continuation */

/* Complex typedef with all delimiter types */
typedef int (*func_ptr_t1)(char *(*callback)(int, \
    double), /* Line continuation inside parentheses */ \
    int array[ARRAY_DIM]) FUNC_ATTR;

/* Struct with deeply nested delimiters */
struct outer_struct {
    /* Default case trigger: numeric constant inside struct */
    int x = 42; /* C++ style initializer (gengtype handles some C++) */
    
    /* Mixed delimiters in single member */
    void (*complex_func[2])(
        struct { 
            int a; 
            union { 
                char *p; 
                int arr[3][2]; 
            } u; 
        } *arg1,
        int (*)(float, double) /* Anonymous function pointer type */
    );
    
    /* Attribute inside nested context */
    int y __attribute__((aligned(16)));
    
    /* Bit-field with unusual size expression */
    unsigned bits : (sizeof(int) * 8 - 1);
    
    /* Nested anonymous struct with macro expansion */
    struct {
        LINE_CONT member;
        int z[ARRAY_DIM][ARRAY_DIM + 1];
    };
};

/* Union with function pointer containing all delimiter types */
union mixed_union {
    /* Function returning pointer to array of structs */
    struct inner_struct {
        char c;
        /* Default case: preprocessor directive-like text in comment */
        /* #if 0 - looks like directive but in comment */
        long l;
    } *(*get_structs(void))[10];
    
    /* Complex array with computed size */
    unsigned char data[sizeof(struct outer_struct) + 
                      offsetof(struct outer_struct, y)];
};

/* Enum with computed values (parentheses) */
enum weird_enum {
    VAL1 = (1 << 0),
    VAL2 = (1 << 1),
    VAL3 = (1 << 2) | (1 << 3), /* Bitwise OR inside parentheses */
    VAL4 = sizeof(int[ARRAY_DIM]) /* sizeof with brackets */
};

/* Typedef chain with all delimiters */
typedef union mixed_union *(*(*complex_typedef)(int))[5];

/* Another complex type definition */
typedef struct {
    /* Nested function pointer with attributes */
    __attribute__((packed)) int (*fp)(
        int a[][3], /* Incomplete array type */
        void (*)(struct outer_struct *),
        ...
    ); /* Variadic function pointer */
    
    /* Anonymous union with bitfields */
    union {
        struct {
            unsigned a : 1;
            unsigned b : (2 + 1); /* Parentheses in bitfield */
        };
        int full;
    } flags;
    
    /* Array of function pointers */
    int (*callbacks[3])(void);
} master_struct_t;

/* GCC vector extension */
typedef int v4si __attribute__((vector_size(16)));

/* Struct using vector type */
struct with_vector {
    v4si vec;
    /* Default case: numeric constant with exponent */
    double d = 1.0e-10; /* C++ style */
    
    /* Pointer to array of pointers to functions */
    void (*(*(*signal)[10])(int))(int);
};

/* Macro expansion inside type definition */
typedef DEFAULT_TRIGGER(mystruct) {
    int x;
    char s[] = WEIRD_CHARS; /* String with comment-like chars */
} my_struct_t;

/* Function prototype with complex parameter */
extern void register_callback(
    int (*cb)(
        master_struct_t **, /* Pointer to pointer */
        int param[(sizeof(master_struct_t) + 15) & ~15] /* Complex array dimension */
    ) __attribute__((nonnull(1)))
);

/* Final complex type mixing everything */
typedef struct {
    /* All three delimiters in one declaration */
    union mixed_union *(*(*factory)(int n))[n] { /* GNU C VLAs in types */
        /* Compound literal style */
        return (union mixed_union *(*)[n])0;
    };
    
    /* Nested type with attributes */
    __attribute__((deprecated)) enum weird_enum (*get_enum)(
        int, 
        /* Comment between parameters */ 
        double
    );
} ultimate_type_t;

/* Trigger default case with line continuations inside parentheses */
int (*weird_func)(
    int a, \
    int b, /* Backslash continuation */ \
    int c \
) = NULL;

/* Multiple declarations to increase parsing events */
struct outer_struct global_var;
union mixed_union *global_ptr;
master_struct_t array_of_structs[10];
complex_typedef func_array[3];

/* Include some C++ style comments and C comments mixed */
// C++ comment
/* C comment with // nested C++ comment */
int dummy = 0; /* Trailing comment with ))) parentheses */

/* Macro that expands to contain all delimiter types */
#define ULTIMATE_MACRO(type) \
    type (*array)[10]; \
    void (*func)(type); \
    struct { type x; } s;

/* Use the macro in a typedef */
typedef ULTIMATE_MACRO(int) macro_type;

/* Final check: ensure file is valid C */
#ifdef __cplusplus
extern "C" {
#endif
    /* Empty main - file not meant to be executed */
    int main(void) { return 0; }
#ifdef __cplusplus
}
#endif
