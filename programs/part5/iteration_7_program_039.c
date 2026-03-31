/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* Preprocessor macros to create complex expansions */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_16 __attribute__((aligned(16)))
#define PACKED __attribute__((packed))
#define MAYBE_UNUSED __attribute__((unused))

/* Macro that expands to include unusual characters */
#define COMPLEX_DIM (/* comment inside macro */ 16 + 4 \
                     /* line continuation */ \
                     - 2)

/* Trigger default case with unusual characters in nested contexts */
#define WEIRD_CHARS(x) x##_tag /* ## token */ \
                       // C++ style comment in macro

/* Type 1: Struct with deeply nested delimiters */
struct OuterStruct {
    /* Function pointer with attributes and nested parentheses */
    int (*callback)(int (*inner_cb)(char **argv, int argc), 
                    /* Unusual character sequence: */ 
                    float matrix[3][4]) ALIGNED_16;
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int flags : 4;
            unsigned int /* comment between tokens */ : 3;
            unsigned int mode : 2;
        } PACKED;
        unsigned char raw_byte;
    } inner_data;
    
    /* Array with macro-expanded dimension containing comments */
    long double big_array[COMPLEX_DIM][ARRAY_SIZE((int[]){1,2,3})];
    
    /* Pointer to array of function pointers */
    void (*(*func_array)[/* size with comment */ 8])(int, ...);
};

/* Type 2: Typedef mixing all delimiter types */
typedef struct {
    /* Nested braces within parentheses (GNU extension) */
    int (*method)(struct { int x; double y; } param) 
        __attribute__((deprecated("use v2 instead")));
    
    /* Multi-dimensional array with complex sizing expression */
    char buffer[ (1 << 3) /* bit shift */ ][ ({ int x = 5; x + 1; }) ];
    
    /* Union containing struct containing array of pointers */
    union {
        struct {
            void *ptr_array[/* empty */][3];
            /* Default case trigger: numeric constant with exponent */
            double values[2] = {1.0e-10, 2.0E+10};
        } nested;
        long long alternate;
    } choice;
} ComplexType, *PComplexType;

/* Type 3: Function pointer type with extreme nesting */
typedef void (*(**(*signal_handler[/* comment with // slashes */ 4])
               (int signum, 
                /* Struct literal in parameter type */
                const struct sigcontext *(*get_context)(void)))
              (void (*)(int), ...))
              (volatile int *restrict);

/* Type 4: GCC vector extension with attributes */
typedef int v4si __attribute__ ((vector_size (16), 
                                 /* Unusual: attribute with parentheses inside */
                                 aligned(/* nested () */ (16))));

/* Type 5: Anonymous struct/union at file scope (GNU extension) */
struct {
    /* Nested switch of delimiter types in single declaration */
    union {
        /* Array of pointers to functions returning pointers to arrays */
        int (*(*callbacks[({ int y = 3; y; })])(float))[/* dimension */ 2+2];
        
        /* Bit-field struct with __attribute__ inside */
        struct {
            unsigned int a : 1 __attribute__((packed));
            unsigned int b : 7 /* comment with = sign */;
        } bits;
    } data __attribute__((aligned(32)));
    
    /* Macro expansion creating unusual token sequence */
    int WEIRD_CHARS(special)_field;
} global_instance = {
    .data = { .callbacks = { NULL } },
    .special_tag_field = 0xDEADBEEF /* hex constant */
};

/* Type 6: Enum with complex initializers */
enum State {
    INIT = ({ int x = 0; x; }),  /* Statement expression */
    RUNNING = (1 << 8) | (1 << 3),  /* Bit operations */
    /* Default case trigger: floating point in integer context (will be error but parsed) */
    ERROR = 3.14e2,
    STOPPED = ({ enum State s = INIT; s + 1; })
};

/* Type 7: Struct with designated initializers in type definition (GNU) */
struct Config {
    int version;
    union Settings {
        struct {
            int timeout : 16;
            int retries : 8 __attribute__((packed));
        } network;
        long long raw;
    } settings __attribute__((aligned(8)));
    
    /* Function pointer with nested attributes */
    void (*log)(const char *fmt, ...) 
        __attribute__((format(printf, 1, 2), 
                       nonnull(/* empty parens */ ())));
} default_config = {
    .version = 2,
    .settings = { .network = { .timeout = 1000, .retries = 3 } },
    .log = (void (*)(const char *, ...))0
};

/* Type 8: Extreme nesting of all delimiter types */
typedef union UltraNested {
    struct Level1 {
        int (*(*(*level2)[/* array size with comment */ 4])
             (struct Level3 {
                 char data[({ int sz = 256; sz; })];
                 /* Nested anonymous struct */
                 struct {
                     float (*transform)(float[3][3], ...);
                 } ops;
             } *arg))
            (void);
    } start;
    
    /* Array with multiple dimensions using different bracket styles */
    unsigned char bytes[1][2][3][4]
        [/* comment between brackets */ 5]
        [6 /* another comment */];
} UltraNested_t;

/* Type 9: __attribute__ with complex arguments */
struct WithAttributes {
    /* Attribute containing string literal with special chars */
    const char *path __attribute__((access(read_only, 1), 
                                    nonstring));
    
    /* Alignment with expression containing parentheses */
    double aligned_data __attribute__((aligned((16 > 8) ? 16 : 8)));
    
    /* Section attribute with concatenated strings */
    int counter __attribute__((section(".data" ".persistent")));
};

/* Type 10: Final complex declaration mixing everything */
static volatile const struct MasterType {
    /* All three delimiters in sequence: {} containing () containing [] */
    struct {
        void (*(*vtable[((sizeof(void*) == 8) ? 8 : 4)])(int))
            (char *const *argv, int argc);
    } methods;
    
    /* Macro expanding to attribute with __ prefix */
    #ifdef __GNUC__
    unsigned long magic __attribute__((visibility("hidden")));
    #endif
    
    /* Nested type definition inside struct */
    enum { RED, GREEN, BLUE } color : 2;
    
    /* Zero-length array (GNU extension) */
    int flexible_array[];
} master_instance = {
    .methods = { .vtable = { NULL } },
    .magic = 0xCAFEBABE,
    .color = GREEN
};

/* Trigger parsing of all types by referencing them */
typedef struct OuterStruct* (*FactoryFunc)(ComplexType **, UltraNested_t);

/* Empty main to satisfy compiler - not executed by gengtype */
int main(void) {
    return 0;
}
