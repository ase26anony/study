/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (x + 1)
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define FUNC_PTR(name) (*name)

/* Macro that expands to include unusual characters */
#define WEIRD_CHARS /* comment */ 42 \
                    + 1

/* Trigger default case: numeric constants and comments inside delimiters */
typedef int (*func_ptr1)(int a /* comment with ()[]{} */, 
                         char b /* more () */,
                         float c[WEIRD_CHARS]);

/* Complex nested type 1: Struct with all delimiter types */
struct level1 {
    /* Nested anonymous union with attributes */
    union {
        int x;
        double y;
    } ALIGN_ATTR;
    
    /* Function pointer with complex return type */
    struct level2 *(*callback)(int (*)(char *), 
                               void (*[ARRAY_SIZE(5)])(void));
    
    /* Array with computed size containing nested struct */
    struct {
        unsigned int flags : 3;  /* bit field */
        int data[ARRAY_SIZE(10)];
    } entries[ARRAY_SIZE(20)];
    
    /* Mixed delimiters in single declaration */
    void (*(*signal_handler[2])(int sig, 
                                void *ctx /* ()[]{} */)) 
                                (const char *msg, ...);
} PACKED_ATTR;

/* Type 2: Deeply nested parentheses and brackets */
typedef void (*(*(*complex_func_factory)(const char *name))
               (int argc, 
                char *argv[] /* [] inside () */))
               (void (*)(), 
                int matrix[3][4]);

/* Type 3: Union with GNU extensions */
union __attribute__((transparent_union)) weird_union {
    /* Vector type (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec_data;
    
    /* Nested struct with attribute on member */
    struct {
        long double ld ALIGN_ATTR;
        short s : 5;  /* bit field with unusual size */
    } parts;
    
    /* Array of function pointers */
    int (*func_array[ARRAY_SIZE(3+2)])(char (*)[10], ...);
};

/* Type 4: Interleaved delimiters with macro expansions */
struct outer {
    /* Default case trigger: line continuation inside parentheses */
    enum { 
        VAL1 = 1 << 0, \
        VAL2 = 1 << 1,  /* backslash in enum */
        VAL3 = 0x10 + \
               0x20
    } flags;
    
    /* All three delimiters in one member */
    union inner (*(*get_inner)(struct outer *self, 
                               int idx))[ARRAY_SIZE(5+ /* comment */ 2)];
    
    /* Attribute with parentheses inside struct */
    __attribute__((deprecated("Use new_field instead")))
    int old_field;
};

/* Type 5: Recursive structure with function pointers */
typedef struct tree_node {
    struct tree_node *left, *right;
    /* Function pointer returning pointer to array */
    int (*(*operation)(int a, int b))[ARRAY_SIZE(3)];
    
    /* Anonymous struct with bitfields */
    struct {
        unsigned : 4;  /* unnamed bitfield */
        unsigned visited : 1;
        unsigned depth : 27;
    } state;
    
    /* Nested union with array of pointers to functions */
    union {
        void (*action1)(void);
        int (*action2)(char *str, ...);
        struct tree_node *(*action3)(int, ...);
    } actions[2];
} tree_node_t;

/* Type 6: Extreme nesting */
typedef int (*(*(*(*level4)(double d))(float f))(char c))(short s);

/* Type 7: Struct with __attribute__ containing all delimiters */
struct attributed {
    /* Attribute with nested parentheses */
    int field1 __attribute__((aligned( 
        sizeof(long double) /* comment with {} */ 
    )));
    
    /* Array dimension with macro containing backslash */
    char field2[ARRAY_SIZE( \
        10 /* default case trigger */ \
    )];
    
    /* Function pointer with attribute */
    void (*method)(struct attributed *self) 
        __attribute__((nonnull(1), warn_unused_result));
};

/* Type 8: Mix of everything in typedef */
typedef volatile const struct {
    /* Anonymous union with bitfields */
    union {
        struct {
            unsigned char a : 2;
            unsigned char b : 3;
            unsigned char c : 3;
        } bits;
        unsigned char byte;
    } data;
    
    /* Pointer to array of function pointers */
    int (*(*(*complex)[5])(void))[10];
    
    /* GNU statement expression in type? (via macro trick) */
    #ifdef __GNUC__
    typeof(*(1 ? NULL : (int (*)[ARRAY_SIZE(5)])0)) *gnu_ptr;
    #endif
} ultimate_type_t;

/* Type 9: For triggering default case with unusual characters */
struct trigger_default {
    /* Numeric constants with different bases */
    int dec = 42;           /* Not valid in C89, but gengtype might see it */
    int hex = 0x2A;         /* 0x prefix */
    int oct = 052;          /* 0 prefix */
    float fp = 3.14e-10;    /* scientific notation */
    
    /* Characters that aren't delimiters */
    char quote = '"';       /* double quote */
    char apos = '\'';       /* single quote */
    char backslash = '\\';  /* escape */
    char question = '?';    /* ternary operator */
    char colon = ':';       /* bitfield/ternary */
    char semicolon = ';';   /* statement end */
    char comma = ',';       /* separator */
    char dot = '.';         /* member access */
    char arrow[2] = "->";   /* string literal */
    
    /* Preprocessor-like tokens in comments */
    /* #if 0
     * #endif
     * #include <stdio.h>
     */
};

/* Type 10: Final complex declaration using all techniques */
typedef union {
    struct {
        /* Nested function pointer with attributes */
        __attribute__((always_inline)) 
        inline int (*calc)(int x, int y) {
            return x + y;  /* Function definition inside struct - GCC extension */
        }
        
        /* Flexible array member */
        int flex[];
    } s;
    
    /* Array with weird dimension calculation */
    unsigned char raw[sizeof(struct s) + 
                      (sizeof(int) * 
                       ARRAY_SIZE(/* nested comment () */10))];
} final_union_t __attribute__((packed, aligned(32)));

/* Multiple global declarations to increase parsing events */
extern struct level1 *global_ptr;
static ultimate_type_t static_var;
volatile tree_node_t *volatile_node;

/* Function prototype with complex parameter */
void process_data(int (*(*callback)(int))[5],
                  struct attributed *attr,
                  ...) __attribute__((sentinel));

/* Main function (not important for gengtype, but keeps file valid) */
int main(void) {
    return 0;
}
