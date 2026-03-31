/* test-gengtype-parser.c - Complex type definitions to exercise gengtype parser */

/* First, define macros that expand to delimiter sequences */
#define ARRAY_DIM(x) [((x) > 0 ? (x) : 1)]
#define ATTRIB_PACKED __attribute__((packed))
#define ALIGN_SPEC __attribute__((aligned(16)))
#define FUNC_ATTRS __attribute__((noreturn, noinline))

/* Macro with unusual characters in expansion */
#define WEIRD_MACRO(x) /* comment with () */ sizeof(x) \
    + 0xDEADBEEF /* hex constant */ \
    + 0b1010 /* binary literal (GCC extension) */

/* Complex nested type 1: Function pointer returning array pointer */
typedef int (*(*complex_func_ptr_t)(char **argv, int argc))
    ARRAY_DIM(10) ATTRIB_PACKED;

/* Type with all delimiters mixed */
struct outer_struct {
    /* Nested anonymous union with struct */
    union {
        struct {
            int x: WEIRD_MACRO(5); /* Bit-field with macro expansion */
            long y;
        } inner;
        double data[20];
    } variant;
    
    /* Function pointer member with attributes */
    void (*callback)(int, ...) FUNC_ATTRS;
    
    /* Pointer to array of function pointers */
    int (*(*func_array)[5])(float, double);
    
    /* Nested struct with GNU attributes */
    struct __attribute__((packed, aligned(8))) packed_struct {
        char c;
        /* Array dimension with parenthesized expression */
        int arr[WEIRD_MACRO(2) * 3];
        
        /* Anonymous union inside packed struct */
        union {
            short s;
            /* Vector type (GCC extension) */
            int v __attribute__((vector_size(16)));
        };
    } packed_member;
};

/* Another complex type using all delimiters in one declaration */
typedef union {
    /* Struct containing array of pointers to functions */
    struct {
        /* Multi-dimensional array with computed size */
        void *matrix[WEIRD_MACRO(3)][(2 + 3) * 4];
        
        /* Function returning pointer to array */
        int (*(*get_matrix)(void))[10];
    } data;
    
    /* Function pointer with complex return type */
    struct outer_struct (*(*make_struct)(int count, ...)) 
        __attribute__((malloc));
} mega_union_t;

/* Enum with unusual values */
enum weird_enum {
    VAL1 = 0x1 << 0,  /* Bit shift in initializer */
    VAL2 = sizeof(struct outer_struct),  /* sizeof in enum */
    VAL3 = (int)(3.14159 * 1000),  /* Cast in initializer */
    /* Line continuation with backslash - unusual character */
    VAL4 = VAL1 \
         | VAL2  /* This backslash should trigger default case */
};

/* Type with deeply nested parentheses */
typedef int (*(*(*deep_nested_fp)(int (*(*)(double))[3]))
    (char *))[5];

/* Struct with attribute containing parentheses */
struct attributed {
    int field1 __attribute__((deprecated("Use field2 instead")));
    
    /* Array with GNU range extension */
    int ranged_array[1..10] __attribute__((aligned(32)));
    
    /* Nested function pointer with __attribute__ inside */
    void (*(*attr_func)(void))() __attribute__((warn_unused_result));
};

/* Union with anonymous struct containing bitfields */
union bitfield_union {
    struct {
        unsigned int a: 4;
        unsigned int b: 4;
        unsigned int c: 8;
        unsigned int d: 16;
    } bits;
    unsigned int value;
};

/* Type definition with macro expanding to nested delimiters */
#define NESTED_MACRO(x) { .data = { [0] = (x) } }
struct with_macro_init {
    struct { int data[5]; } inner;
};

/* Global instance with macro initialization */
struct with_macro_init global_instance = NESTED_MACRO(42);

/* Function pointer type with attributes between asterisks */
typedef int (* __attribute__((const)) 
    const_func_ptr)(int, int);

/* Final ultra-complex declaration using all features */
static volatile const struct outer_struct * const *
    (*(*ultimate_func)(register int r, 
                       struct { int x; double y; } s,
                       /* Empty union in parameter */
                       union { } empty))
    [sizeof(mega_union_t) / 2] 
    __attribute__((section(".data"))) = 0;

/* Additional type to ensure more parsing */
typedef __attribute__((may_alias)) struct {
    /* Flexible array member */
    int flex_array[];
} aliasable_struct;

/* GCC vector types */
typedef int v4si __attribute__((vector_size(16)));
typedef float v8sf __attribute__((vector_size(32)));

/* Struct with vector member */
struct with_vector {
    v4si vec_data;
    /* Array of vectors */
    v8sf big_vecs[4];
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    long *longp;
    void *voidp;
} transparent_union_t;

/* Struct with transparent union parameter in function pointer */
struct has_transparent {
    int (*method)(transparent_union_t arg);
};

/* Cleanup attribute (triggers parentheses parsing) */
struct with_cleanup {
    char *buffer __attribute__((cleanup(free)));
};

/* Alias with asm label (contains '@' - unusual character) */
int aliased_var __asm__("aliased_var@v1") = 42;

/* Multiple declarators in one declaration */
int multi1, *multi2, multi3[10], (*multi4)(void);

/* __builtin_choose_expr in type context */
typedef __typeof__(__builtin_choose_expr(1, int, double))
    chosen_type;

/* Struct with __builtin_offsetof */
struct offset_test {
    int a;
    char b;
    double c;
};

/* Use offsetof in array dimension */
char offset_array[__builtin_offsetof(struct offset_test, c)];

/* Final check: ensure file ends with something normal */
int dummy = 0;
