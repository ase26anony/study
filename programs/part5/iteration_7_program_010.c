/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2)  /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16)))  /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* 2. Trigger default case with unusual characters in nested contexts */
struct Outer1 {
    /* Comment between delimiters { ... } */
    int x ATTR_ALIGN;  /* Attribute with parentheses inside struct */
    
    /* Line continuation inside array dimension */
    char buffer[ARRAY_DIM \
                + 1];  /* Backslash triggers default case */
    
    /* Numeric constant with unusual format */
    double values[3] = {0x1.0p-10, /* Hex float */
                        1.2e-3,    /* Scientific notation */
                        0.5};      /* Regular float */
};

/* 3. Mixed delimiter types in single declaration */
typedef struct {
    /* Nested anonymous union inside struct */
    union {
        int i;
        float f;
        /* Function pointer with attributes */
        void (*callback)(int) __attribute__((noreturn));
    } data;
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_array[5])(void))[10];
    
    /* Bit-field with complex expression */
    unsigned int flags: (sizeof(int)*8 - 1);
} ComplexType;

/* 4. Deeply nested and interdependent delimiters */
FUNC_PTR_TYPEDEF(HandlerFunc);  /* Macro expands to typedef */

struct Container {
    /* Function pointer with nested parentheses */
    HandlerFunc handlers[3];
    
    /* Pointer to array of structs containing unions */
    struct {
        union {
            int a;
            long b[2][3];  /* Multi-dimensional array */
        } u;
        
        /* Nested struct with attribute */
        struct __attribute__((packed)) {
            char c;
            short s;
        } inner;
    } (*nested_ptr)[4];
    
    /* GNU vector type extension */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
};

/* 5. Enum with complex initializers */
enum State {
    INIT = 0,
    RUNNING = (1 << 0) | (1 << 1),  /* Bitwise OR in parentheses */
    STOPPED = 2,
    ERROR = -1  /* Negative number */
};

/* 6. Typedef with all delimiter types intertwined */
typedef union {
    struct {
        /* Function returning pointer to array */
        int (*get_matrix(void))[3][3];
        
        /* Pointer to function with array parameter */
        void (*set_values)(int arr[static 5]);
    } ops;
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned int : 4;  /* Unnamed bitfield */
        unsigned int mode: 3;
        unsigned int error: 1;
    } status;
    
    /* Complex array declaration with size from enum */
    char buffer[RUNNING * 2 + 1];  /* Enum value in expression */
} UltimateType ATTR_ALIGN;

/* 7. Multiple levels of indirection with attributes */
typedef int (*(**(*complex_func_ptr)(double d[const restrict 10]))(void))
    __attribute__((warn_unused_result));

/* 8. Struct with macro expansions inside nested contexts */
struct WithMacros {
    /* Use macro that expands to attribute */
    int aligned_int ATTR_ALIGN;
    
    /* Array dimension from macro */
    float dynamic_array[ARRAY_DIM];
    
    /* Nested struct with macro attribute */
    struct {
        char data[32];
    } __attribute__((aligned(32))) aligned_struct;
};

/* 9. Union containing switch-case like syntax in comments (triggers default) */
union CommentTest {
    int x;
    /* The following comment contains characters that will hit default case:
       switch(state) {
         case 1: break;
         default: break;
       }
       This comment spans multiple lines with various tokens.
    */
    long y;
};

/* 10. Final complex type mixing everything */
typedef struct {
    /* Nested anonymous struct */
    struct {
        /* Pointer to function with nested attributes */
        void (__attribute__((fastcall)) *method)(int, ...);
        
        /* Multi-dimensional array with computed size */
        int matrix[2][(sizeof(long) == 8) ? 4 : 2];
    } base;
    
    /* Union with bitfields and array */
    union {
        struct {
            unsigned int a:2, b:3, c:4;
        } bits;
        unsigned short words[2];
    } packed_data __attribute__((packed));
    
    /* Self-referential pointer */
    struct _self_ref *next;
} FinalType;

/* 11. Global variables using complex types */
ComplexType global_ct = {0};
UltimateType global_ut;
FinalType *global_ft_ptr;

/* 12. Function prototype with complex parameter */
extern void process_all(ComplexType (*input)[], 
                       UltimateType **output,
                       __attribute__((deprecated)) int count);

/* 
 * The file contains no main() function because gengtype only needs
 * type declarations for parsing. The complex nesting of (), [], and {}
 * along with comments, macros, attributes, and unusual characters
 * ensures the parser will:
 * 1. Recursively call consume_balanced for all delimiter types
 * 2. Hit the default case when encountering:
 *    - Backslashes (line continuations)
 *    - Comments (// and /*)
 *    - Attribute syntax
 *    - Macro expansions
 *    - Numeric constants (0x, scientific notation)
 *    - Operators within parentheses
 */
