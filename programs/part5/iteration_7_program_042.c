/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 3) /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, char**)

/* 2. Complex type with all delimiter types mixed */
typedef struct /* Default case: comment inside struct definition */
{
    /* Nested parentheses in function pointer with attribute */
    int (*callback)(char *buffer ATTR_ALIGN, ...) __attribute__((deprecated));
    
    /* Array with macro-expanded dimension containing operators */
    unsigned char data[ARRAY_DIM /* Comment between brackets */];
    
    /* Union inside struct with bit-fields */
    union {
        struct {
            int x:8;
            int y:8 /* Missing semicolon to test error recovery */;
        } ATTR_ALIGN;
        long long combined;
    } coord;
    
    /* Pointer to array of function pointers */
    void (*(*func_table)[5])(int, float);
} ComplexStruct ATTR_ALIGN;

/* 3. Type definition with deeply nested delimiters */
typedef int (*(**(*nested_func_ptr)(struct {int a; double b;}))[][8])(void);

/* 4. GNU extensions with unusual characters in nested contexts */
struct __attribute__((packed, aligned(32))) PackedStruct {
    /* Vector type with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
    
    /* Anonymous struct with bit-field containing numeric constant */
    struct {
        unsigned flag1:1 + 3; /* Expression in bit-field width */
        unsigned flag2:2;
    };
    
    /* Function pointer with complex return type */
    struct Inner {
        char *name;
        int id;
    } (*(*get_inner)(void))[3];
};

/* 5. Union with nested array of structs containing function pointers */
union Container {
    struct {
        /* Multi-dimensional array with computed size */
        int matrix[ (2 + 3) * 4 ][ ARRAY_DIM / 2 ];
        
        /* Pointer to function returning pointer to array */
        float (*(*get_matrix_row)(int index))[];
        
        /* Nested anonymous union */
        union {
            char *str;
            void *ptr;
        } data;
    } section;
    
    /* Another struct with __attribute__ containing parentheses */
    struct __attribute__((may_alias)) {
        long long values[2];
    } alias_section;
};

/* 6. Typedef creating circular dependency with pointers */
typedef struct Node Node;
struct Node {
    Node *next ATTR_ALIGN;
    Node *prev;
    void *data;
    
    /* Function pointer member with attribute */
    int (*compare)(const Node *a, const Node *b) 
        __attribute__((nonnull(1, 2)));
};

/* 7. Enum with last comma (C99 extension) */
enum Flags {
    FLAG_A = 1 << 0, /* Shift operator */
    FLAG_B = 1 << 1,
    FLAG_C = 1 << 2,
    FLAG_D = 1 << 3, /* Trailing comma triggers default case */
};

/* 8. Complex function pointer type with nested attributes */
typedef void (*(*signal_handler)(int signum, 
    void (*old_handler)(int) 
    __attribute__((unused)) /* Attribute inside parentheses */
    ))() __attribute__((noreturn));

/* 9. Struct with designated initializers in comments (triggers default case) */
struct Config {
    int timeout /* = 100 */;  /* Comment with equals sign */
    char *path /* = "/tmp" */; /* Comment with slashes and quotes */
};

/* 10. Type containing line continuation */
typedef struct { \
    int x; \
    int y; \
} Point ATTR_ALIGN; /* Backslash-newline in middle of definition */

/* 11. Macro that expands to contain all delimiter types */
#define COMPLEX_TYPE \
    union { \
        struct { \
            int (*func)(int a[5], ...); \
        } s; \
        long l; \
    }

/* Use the macro in a typedef */
typedef COMPLEX_TYPE ComplexUnion;

/* 12. Final complex declaration mixing everything */
static volatile ComplexStruct* (*(*global_table[10])(int, ...))[] = {0};

/* The file ends without main() - we only need type definitions for gengtype */
