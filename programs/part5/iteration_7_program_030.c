/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targeting consume_balanced() default case and nested delimiters
 */

/* First, define macros that expand to complex delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parentheses */
#define FUNC_ATTR __attribute__((noinline))
#define NESTED_MACRO(x) [(x) + 1]

/* Preprocessor directive inside comment-like context */
#if 0 /* This comment has parentheses () and brackets [] */
#endif

/* Type 1: Struct with deeply nested delimiters */
struct Outer1 {
    /* Function pointer with attributes inside parentheses */
    int (*callback)(char *arg, \
                    /* Line continuation inside parentheses */
                    double data[ARRAY_DIM]) FUNC_ATTR;
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flag:1;
            unsigned int value:31;
        } bits;
        unsigned int raw;
    } u;
    
    /* Array of pointers to functions returning struct pointers */
    struct Inner **(*func_array[10])(int, ...);
};

/* Type 2: Typedef mixing all delimiter types */
typedef union {
    /* Anonymous struct with __attribute__ inside */
    struct {
        long double complex_num __attribute__((aligned(16)));
        volatile short count;
    } __attribute__((packed));
    
    /* Complex array declaration with macro expansion */
    void *ptr_array[ARRAY_DIM][NESTED_MACRO(3)];
} ComplexUnion __attribute__((transparent_union));

/* Type 3: Function pointer type with nested attributes */
typedef int (*(*SignalHandler)(int signum, 
                               /* Comment with special chars: <>&|^~ */
                               void *context))(
    /* Function returning function pointer */
    struct Outer1 *(*factory)(int),
    /* Array parameter with computed size */
    float buffer[sizeof(struct Outer1) + 1]
) __attribute__((deprecated("Use v2")));

/* Type 4: Struct with GNU extensions and nested delimiters */
struct __attribute__((aligned(32))) VectorStruct {
    /* Vector type with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
    
    /* Flexible array member with nested sizeof */
    char data[];
};

/* Type 5: Enum with complex initializers */
enum State {
    INIT = 0,
    /* Value with bitwise operations in parentheses */
    RUNNING = (1 << 0) | (1 << 2),
    PAUSED = (1 << 1),
    /* Macro expansion in enum value */
    STOPPED = ARRAY_DIM
};

/* Type 6: Nested struct/union combination */
struct Container {
    /* Anonymous struct inside union */
    union {
        struct {
            /* Pointer to array of function pointers */
            void (*(*vtable[10])(void))();
            /* Nested parentheses in type cast expression */
            int (*comparator)(const void *, const void *);
        } ops;
        
        /* Bit-field with unusual size */
        struct {
            unsigned long long low:35;
            unsigned long long high:29;
        } __attribute__((packed)) parts;
    } storage;
    
    /* Two-dimensional array with computed dimensions */
    int matrix[ARRAY_DIM + 1][NESTED_MACRO(2) * 2];
};

/* Type 7: Complex typedef with all delimiters intertwined */
typedef struct Node *(*NodeProcessor)(
    /* Nested function pointer parameter */
    int (*visit)(struct Node **, int depth),
    /* Array of structs with bit-fields */
    struct {
        unsigned int tag:4;
        unsigned int length:28;
    } metadata[10],
    /* Variable length array parameter */
    char data[static 10]
) [10] /* Returns pointer to array */;

/* Type 8: Struct with __attribute__ containing parentheses */
struct __attribute__((designated_init)) Config {
    /* Member with multiple attributes */
    const char *name __attribute__((access(read_only, 1))) 
                    __attribute__((nonnull));
    
    /* Complex initializer (for hypothetical extension) */
    int values[3] /* = { [0] = 1, [1] = 2, [2] = 3 } */;
};

/* Type 9: Union with nested anonymous struct containing all delimiter types */
union UltimateTest {
    struct {
        /* Everything together: function pointer returning pointer to array */
        int (*(*(*complex_func)(void))[10])(int, int);
        
        /* Nested union inside struct inside union */
        union {
            struct {
                short x, y;
            } point;
            long coordinates;
        } location;
        
        /* Attribute with parentheses inside struct */
        int field __attribute__((aligned((16))));
    } data;
    
    /* Array with size containing parentheses expression */
    unsigned char raw[sizeof(struct { int a; double b; })];
};

/* Type 10: Forward declaration with attributes */
struct Forward __attribute__((visibility("hidden")));

/* Additional complexity: Macro that expands to type definition */
#define DECLARE_TYPE(name, size) \
    typedef struct name##_s { \
        char buffer[size]; \
        int (*methods[size])(void); \
    } name##_t

/* Use the macro to generate more types */
DECLARE_TYPE(Buffer, ARRAY_DIM);

/* Final test: Type with line continuations inside nested parentheses */
typedef void (* \
    /* Comment between line continuations */ \
    ExtremelyComplexCallback)( \
        struct Outer1 * \
        /* More line continuations */ \
        (*factory)(int), \
        int (* \
            /* Nested function pointer type */ \
            processors[10])(void *, ...) \
    );

/* The file ends without main() - we only need type definitions for gengtype */
