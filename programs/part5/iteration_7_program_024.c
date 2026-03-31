/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* First, define some macros that expand to include various characters */
#define ARRAY_SIZE(x) (x + 1) /* This comment inside macro def */
#define FUNC_ATTR __attribute__((noinline))
#define COMPLEX_DIM (2 * 3 + 1)
#define WEIRD_CHARS "/* not a comment */" // Line comment in macro

/* Preprocessor directive inside would be tricky, but we can use line continuation */
#define MULTI_LINE_MACRO \
    int x; \
    /* comment with () */ \
    char y

/* Type 1: Struct with deeply nested delimiters */
struct Outer1 {
    /* Trigger default case with numeric constant inside braces */
    int a = 42; /* GCC extension - initializer in struct */
    
    /* Function pointer with attributes inside parentheses */
    void (*func_ptr1)(int, char) FUNC_ATTR;
    
    /* Nested anonymous struct with bit-field */
    struct {
        unsigned int flag:1;
        /* Array with complex dimension calculation */
        float matrix[COMPLEX_DIM][ARRAY_SIZE(5)];
    } inner;
    
    /* Union inside struct */
    union {
        long l;
        /* Pointer to array of function pointers */
        int (*(*arr_func_ptr)[3])(void);
    } u;
};

/* Type 2: Typedef with all delimiter types mixed */
typedef struct {
    /* Nested parentheses in function pointer return type */
    struct Inner2* (*(*complex_func)(int (*)(char)))[10];
    
    /* GCC attribute with parentheses inside braces */
    unsigned char data[16] __attribute__((aligned(16), packed));
    
    /* Anonymous union with macro expansion */
    union {
        MULTI_LINE_MACRO;
        /* Weird characters in string literal (triggers default case) */
        const char* str = WEIRD_CHARS;
    };
} ComplexType;

/* Type 3: Even more complex nested structure */
struct Level1 {
    struct Level2 {
        struct Level3 {
            /* Array of pointers to functions returning pointers to arrays */
            int (*(*(*level3_array[2])())[5])();
            
            /* Nested switch-like syntax in comments to test default case */
            /* case '(': would be parsed as normal text */
            /* default: advance(); */
        } l3;
        
        /* Vector type (GCC extension) */
        typedef int v4si __attribute__((vector_size(16)));
        v4si vectors[4];
        
        /* Pointer to volatile const array */
        volatile const char *(*volatile ptr_array)[];
    } l2;
    
    /* Zero-length array (GCC extension) */
    int flexible_array[];
};

/* Type 4: Enum with complex initializers */
enum WeirdEnum {
    /* Initializer with parentheses */
    VAL1 = (1 << 3),
    
    /* Initializer with braces (GCC extension) */
    VAL2 = { 0xDEADBEEF },
    
    /* Multiple characters that aren't delimiters */
    VAL3 = '\\' + '\n' + '\t'
};

/* Type 5: Function pointer type with everything */
typedef void (*(**(*signal(int sig, void (*func)(int)))(double))[])();

/* Type 6: Struct with __attribute__ containing nested parentheses */
struct __attribute__((aligned(
    /* Nested calculation in attribute argument */
    sizeof(long double)
))) AlignedStruct {
    /* Anonymous struct with bitfields and array */
    struct {
        /* Bitfield with complex expression */
        unsigned int bits : (sizeof(int) * 8 - 1);
        
        /* Multi-dimensional array with macro */
        int arr[ARRAY_SIZE(2)][ARRAY_SIZE(3)];
    };
    
    /* Function pointer with __attribute__ inside parameter list */
    int (*attr_func)(int __attribute__((unused)), ...);
};

/* Type 7: Nested type definitions with line continuations */
typedef \
    struct { \
        union { \
            int (*funcs[5])(void); \
            struct { \
                char *(*str_func)(char **); \
            } s; \
        } u; \
    } \
    NestedTypedef;

/* Type 8: Using typeof (GCC extension) with nested delimiters */
struct TypeofExample {
    /* typeof with nested parentheses */
    typeof(*(int (*)[5])0) array_ref;
    
    /* typeof with struct definition inside */
    typeof(struct { int x; double y; }) anon_struct;
};

/* Type 9: Designated initializers style in type (GCC extension) */
struct DesignatedLike {
    int a : 5;
    int b : 3;
    int array[10];
} __attribute__((packed));

/* Type 10: Final complex monster type */
typedef union {
    /* Nested function pointer with all delimiters */
    int (*(*(*nested_fp)(struct { int x; }))[][5])(char (*)(void));
    
    /* Struct with anonymous members */
    struct {
        /* __attribute__ with multiple sets of parentheses */
        int data __attribute__((deprecated("message", "extra")));
        
        /* Array with computed size containing special chars in comment */
        /* The // comment inside might trigger default case */
        char buf[sizeof(int) + /* random () */ 10];
    };
    
    /* Empty declaration with just semicolons */
    ;;
    
    /* Just some raw characters that aren't delimiters */
    /* 1234567890!@#$%^&*_+-=|\;:'",<.>/?~` */
} UltimateType;

/* Additional global declarations to ensure more parsing */
extern UltimateType global_var;
static const volatile ComplexType static_var = {0};

/* Function prototype with complex return type */
struct Outer1* (*get_complex(int))(void);

/* Multiple type definitions in one using comma */
typedef int INT, *PINT, (*FPINT)(void), ARRINT[10];

/* K&R style function declaration (older style) */
int old_style_func(param1, param2)
    int param1;
    char *param2;
{
    return 0;
}

/* Main function is irrelevant for gengtype but keeps file compilable */
int main(void) {
    return 0;
}
