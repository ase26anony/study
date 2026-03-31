/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* First, define macros that expand to complex delimiter sequences */
#define ARRAY_DIM(x) [x + 1]
#define FUNC_ATTR __attribute__((noinline))
#define BITFIELD(x) : x
#define NESTED_PAREN (sizeof(int) + (sizeof(long) - 1))
#define WEIRD_CHARS /* comment with () */ 1.5e-3 \
    continued line

/* Complex struct with all delimiter types */
struct Outer {
    /* Nested anonymous union with struct */
    union {
        struct {
            int x;
            /* Function pointer with attributes inside struct */
            void (*callback)(int, char **) FUNC_ATTR;
        } inner;
        /* Array with macro-expanded dimension containing parentheses */
        float data ARRAY_DIM(NESTED_PAREN);
    };
    
    /* Bit-field with unusual characters in expression */
    unsigned int flags BITFIELD(3 + /* comment */ 2);
    
    /* Pointer to array of function pointers */
    int (*(*func_array)[10])(double, ...);
    
    /* GNU attribute with nested parentheses */
    char aligned_data[64] __attribute__((aligned(32), packed));
};

/* Typedef mixing all delimiter types in one declaration */
typedef struct {
    /* Nested union with anonymous struct */
    union {
        struct {
            /* Complex array declaration with multiple brackets */
            int (*matrix[3][4])(void);
        };
        /* Member with attribute containing parentheses */
        volatile long counter __attribute__((deprecated("use new_counter")));
    };
    
    /* Function pointer returning pointer to array */
    struct Outer* (*(*get_outer)[5])(int, ...);
} ComplexType;

/* Another complex type with deeply nested delimiters */
enum Color { RED = 1, GREEN = 2, BLUE = 4 };

/* Union with nested struct containing all delimiter types */
union Container {
    /* Struct with bit-fields, arrays, and function pointers */
    struct {
        /* Array dimension with arithmetic containing parentheses */
        unsigned char bytes[sizeof(struct Outer) + (8 - 1)];
        
        /* Function pointer with complex return type */
        ComplexType* (*factory)(enum Color, ...);
        
        /* Nested parentheses in bit-field width */
        unsigned int mask BITFIELD((1 << 3) - 1);
    } data;
    
    /* Vector type (GCC extension) with attribute */
    int v4si __attribute__((vector_size(16)));
};

/* Typedef for function pointer with all delimiters */
typedef void (*(*SignalHandler[10])(int signum, 
    void (*old_handler)(int)))(int);

/* Struct with macro expansions inside delimiter sequences */
struct MacroTest {
    /* Use macro that expands to brackets with parentheses inside */
    int array ARRAY_DIM(5 + (2 * 3));
    
    /* Attribute with macro that expands to contain parentheses */
    __attribute__((aligned(NESTED_PAREN))) double aligned_double;
    
    /* Function pointer with attribute containing parentheses */
    int (*compute)(int, float) __attribute__((warn_unused_result));
};

/* Extreme nesting case */
typedef struct {
    union {
        struct {
            int (*(*(*nested_func)[5])(struct MacroTest*))[10];
        };
        struct {
            /* Multiple nested parentheses in type declaration */
            void (*((*callback_array)[3]))(int, ...);
        };
    };
    
    /* Array of pointers to functions returning pointers to arrays */
    char (*(*(*string_table)[20])(int))[256];
} UltraNested;

/* Test case specifically for default/advance() path */
struct DefaultCaseTest {
    /* These should trigger advance() in default case: */
    
    /* 1. Numeric constant with exponent (contains '.', 'e', '-') */
    double value = 1.5e-3;  /* GCC extension: default member initializer */
    
    /* 2. Line continuation inside type definition */
    char long_string[] = "This is a very long string that might be " \
                         "continued across multiple lines";
    
    /* 3. Preprocessor-like comment in middle of declaration */
    int /* #if 0 */ special /* #endif */ : 4;
    
    /* 4. Attribute with string literal containing special chars */
    const char* msg __attribute__((deprecated("message with ()[]{}")));
    
    /* 5. Offsetof with parentheses */
    size_t offset = __builtin_offsetof(struct Outer, flags);
};

/* Function pointer typedef with attributes in parameter list */
typedef int (*Comparator)(const void*, const void*) 
    __attribute__((nonnull(1, 2)));

/* Final complex declaration mixing everything */
struct GrandFinale {
    /* Nested anonymous struct */
    struct {
        /* Pointer to array of function pointers with attributes */
        void (*(*handlers[5])(int))() __attribute__((noreturn));
        
        /* Union within struct within struct */
        union {
            /* Bit-field with complex width expression */
            unsigned int bits BITFIELD((8 * sizeof(int)) - 1);
            /* Array with computed size */
            char bytes[sizeof(ComplexType) + __alignof__(ComplexType)];
        };
    };
    
    /* Member with multiple attributes */
    __attribute__((aligned(64), packed, may_alias)) 
    unsigned char cache_line[64];
    
    /* Complex function pointer member */
    struct DefaultCaseTest* (*(*factory)(enum Color c, 
        void (*log)(const char*)))[10];
};

/* Additional global declarations to ensure parsing */
extern ComplexType* global_var __attribute__((weak));
volatile UltraNested* volatile_ptr;
const SignalHandler default_handlers;

/* GCC vector extensions */
typedef int v8si __attribute__((vector_size(32)));
typedef float v4sf __attribute__((vector_size(16)));

/* Struct using vector types */
struct VectorStruct {
    v8si vectors[4];
    v4sf floats __attribute__((aligned(16)));
};

/* One more for good measure - typedef with nested parentheses */
typedef int (*((*NestedFuncPtr)(int (*)(float), ...)))(double, ...);

/* Empty main to make file compilable (though not needed for gengtype) */
int main(void) {
    return 0;
}
