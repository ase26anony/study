/* test-gengtype-parser.c - Complex type definitions to test gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR(section) __attribute__((section(#section)))

/* Macro that expands to include unusual characters */
#define WEIRD_MACRO(x) /* comment with ()[]{} */ x \
    + 1 /* line continuation with backslash */

/* Complex nested type 1: Function pointer with nested attributes */
typedef int (ALIGN_ATTR *func_ptr_t)(int a, 
                                     /* Unusual character sequence: */
                                     double b[WEIRD_MACRO(5)], 
                                     void *c);

/* Complex nested type 2: Struct with all delimiter types */
struct PACKED_ATTR outer_struct {
    /* Nested parentheses in bit-field */
    unsigned int flags : (sizeof(int) * 8 - 1);
    
    /* Array with computed size containing parentheses */
    func_ptr_t callbacks[ARRAY_SIZE(((int[]){1,2,3,4}))];
    
    /* Anonymous union with nested struct */
    union {
        struct {
            /* Function pointer returning pointer to array */
            int (*(*complex_func)(int n))[];
            
            /* Nested array declaration with attributes */
            char SECTION_ATTR(.data) buffer[256];
        } inner;
        
        /* Another member with all delimiters */
        struct {
            /* Pointer to function taking array of structs */
            void (*handler)(struct { int x; double y; } items[]);
            
            /* Multi-dimensional array with parentheses in size */
            long matrix[3][(2 + 3)];
        } alt;
    } data;
    
    /* Union with GCC vector extension */
    union {
        /* Vector type with attribute */
        int __attribute__((vector_size(16))) vec;
        
        /* Struct with bit-fields and array */
        struct {
            unsigned char a:4, b:4;
            short arr[(sizeof(int) + 1)];
        } parts;
    } PACKED_ATTR vector_union;
};

/* Complex nested type 3: Typedef chain with all delimiters */
typedef struct {
    /* Nested function pointer declaration */
    int (*(*signal_handler[10])(int sig, void *data))(void);
    
    /* Anonymous struct with computed array bounds */
    struct {
        /* Array size with ternary operator */
        double values[sizeof(long) > 4 ? 8 : 4];
        
        /* Pointer to array of function pointers */
        void (*(*func_array)[5])(int);
    };
    
    /* Union containing nested parentheses in bit-field */
    union {
        struct {
            unsigned int a : (1 << 2);
            unsigned int b : (3 + 2);
        } bits;
        long all;
    } flags;
} event_handler_t;

/* Type 4: Deeply nested with macro expansions */
typedef union SECTION_ATTR(.rodata) {
    /* Function returning pointer to struct containing array of function pointers */
    struct {
        int (*(*(*get_operations(void))[3])(int, int))[5];
        
        /* Nested anonymous union */
        union {
            /* Array with size from macro containing parentheses */
            char str[ARRAY_SIZE("test")];
            
            /* Pointer to const array of volatile pointers */
            volatile void * const * volatile *ptr_array;
        };
    } ops;
    
    /* Alternative with all delimiter types mixed */
    struct {
        /* Multi-level pointer with attributes */
        int ALIGN_ATTR ****quad_ptr;
        
        /* Function pointer with complex return type */
        struct outer_struct (*(*factory)(int count))(int, ...);
    } factory;
} mega_union_t;

/* Type 5: Using typeof extension with nested delimiters */
typedef typeof(&(struct {
    /* Nested struct with array of structs containing function pointers */
    struct {
        int (*compare)(const void *, const void *);
        void (*free)(void *);
    } methods[2];
    
    /* Union with bit-fields and array */
    union {
        unsigned char bytes[sizeof(void*)];
        unsigned long word;
    } data;
})) complex_callback_t;

/* Type 6: Multiple levels of indirection with attributes */
typedef int (SECTION_ATTR(.text) *****(*insanely_complex_t)(void))[][2][3];

/* Global variable declarations using complex types */
static event_handler_t global_handler = {0};
static mega_union_t ALIGN_ATTR global_union;

/* Inline function with complex parameter type */
static inline void process_data(struct outer_struct *PACKED_ATTR os,
                                /* Parameter with nested array */
                                int matrix[][(2 * 3 + 1)]) {
    /* Function body not important for parser coverage */
    (void)os;
    (void)matrix;
}

/* Struct with designated initializers (containing parentheses) */
struct with_init {
    int x;
    double y;
    char z[10];
} initialized = {
    .x = (int)(sizeof(void*) * 2),
    .y = 3.14159,
    .z = {'t', 'e', 's', 't', /* comment with {} */ }
};

/* Final complex type: Mix of everything */
typedef struct {
    /* Nested anonymous struct with bit-field using parentheses */
    struct {
        unsigned int a : (8 - 1);
        unsigned int b : 1;
    };
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*func_table[5])(int))[];
    
    /* Union containing struct with nested union */
    union {
        struct {
            /* Pointer to const function with attribute */
            int (SECTION_ATTR(.text) * const method)(void);
            
            /* Anonymous union inside struct inside union */
            union {
                long l;
                double d;
            };
        } s;
        
        /* Array with size containing unusual characters via macro */
        char raw[WEIRD_MACRO(32)];
    } data;
    
    /* GCC attribute with nested parentheses */
    int value __attribute__((deprecated("Use new_value instead")));
} ultimate_type_t;

/* Multiple typedefs to increase parsing events */
typedef ultimate_type_t *ultimate_ptr_t;
typedef ultimate_ptr_t (*generator_t)(int seed);
typedef generator_t factory_array_t[10];

/* Empty main function - file is for parsing only */
int main(void) {
    return 0;
}
