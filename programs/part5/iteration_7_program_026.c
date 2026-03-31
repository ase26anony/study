/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 */

/* First, define some macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2)  /* Contains shift operator in parentheses */
#define FUNC_ATTR __attribute__((noinline))
#define NESTED_MACRO(x) [(x) + 1]

/* Preprocessor directive inside a comment area */
#if 0
/* This won't be compiled but will be seen by the preprocessor */
#endif

/* Type 1: Struct with deeply nested delimiters */
struct Outer1 {
    /* Function pointer with attributes inside parentheses */
    int (*func_ptr1)(int a, 
                     /* Comment with unusual chars: @#$% */
                     char b, 
                     ...) FUNC_ATTR;
    
    /* Array with macro-expanded dimension containing operators */
    float matrix[ARRAY_DIM][ARRAY_DIM * 2];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2 /* Missing semicolon here to test recovery */;
        } bits;
        /* Line continuation inside union \
           (backslash triggers default case) */
        long long combined;
    };
    
    /* Pointer to array of function pointers */
    void (*(*callbacks[NESTED_MACRO(3)])(void))[2];
};

/* Type 2: Typedef mixing all delimiter types */
typedef struct {
    /* Nested parentheses in function pointer return type */
    struct Inner2* (*(*get_factory)(const char *name))(int, int);
    
    /* Complex array declaration with attributes */
    volatile int (*signal_handlers[10]) 
        __attribute__((aligned(16))) 
        (int signum, void *context);
    
    /* GNU extension: vector type with nested braces */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
    
    /* Macro with nested brackets in array dimension */
    char buffer[NESTED_MACRO(ARRAY_DIM)][NESTED_MACRO(5)];
} ComplexType;

/* Type 3: Union with attribute in nested context */
union __attribute__((packed)) DataUnion {
    /* Function pointer with __attribute__ inside parameter list */
    void (*callback)(int param __attribute__((unused)), 
                     /* Numeric constant with exponent 1e-5 */
                     float tolerance);
    
    /* Anonymous struct with bit-fields and array */
    struct {
        unsigned char header[4];
        /* Bit-field spanning multiple lines \
           with line continuation */
        unsigned int payload_size : 24;
        unsigned int checksum : 8;
    } packet;
    
    /* Pointer to multidimensional array */
    double (*grid)[][10];
};

/* Type 4: Enum with complex initializers */
enum State {
    IDLE = 0,
    /* Initializer with parentheses */
    PROCESSING = (1 << 0),
    /* Initializer with brackets (GNU extension) */
    WAITING = {2},
    /* Initializer with braces */
    ERROR = {3, 4, 5}[0]
};

/* Type 5: Extremely nested single declaration */
int (*(*(*nested_func_ptr)(struct Outer1 *param1, 
                           /* Comment with special chars: &|^~ */
                           ComplexType **param2))
      [ARRAY_DIM])(union DataUnion ***param3, 
                   enum State (*param4)(void)) 
      __attribute__((warn_unused_result, 
                     deprecated("Use new_api instead")));

/* Type 6: Struct with all delimiter types in one member */
struct UltimateTest {
    /* This has: () for function, [] for array, {} for struct */
    struct {
        int (*methods[5])(void);
        union {
            char *(*factory)(int);
            void (*destructor)(void);
        } ops;
    } __attribute__((aligned(64))) vtable[2][3];
    
    /* Mix of macros and attributes */
    ComplexType* (*generators[NESTED_MACRO(2)])
        (int seed __attribute__((const)), 
         /* Floating point in default arg (GNU extension) */
         float prob = 1.0/3.0);
};

/* Type 7: Typedef with GNU typeof extension */
typedef typeof(&((struct UltimateTest*)0)->vtable[0][0]) 
    VTablePtrType;

/* Additional test: preprocessor directive in the middle of a type */
struct Broken {
    int x;
#ifdef TEST_FLAG
    /* Conditional compilation inside struct */
    int y;
#else
    long y;
#endif
    int z;
};

/* Final test: Backslash continuation inside array dimension */
extern char global_buffer\
        [100 /* Comment with () parentheses */]\
        [ARRAY_DIM + 3];

/* End of complex type definitions */
