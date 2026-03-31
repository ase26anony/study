/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets the default case and nested delimiter handling
 */

/* Preprocessor macros that expand to delimiter-containing expressions */
#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))
#define ALIGN_SPEC __attribute__((aligned(16)))
#define PACKED_SPEC __attribute__((packed))
#define SECTION_SPEC __attribute__((section(".data")))
#define DEPRECATED __attribute__((deprecated("Don't use this")))

/* Macro that expands to something with unusual characters */
#define WEIRD_MACRO(x) (x + 1) /* comment inside macro expansion */

/* Trigger default case with unusual characters in nested contexts */
typedef int (*func_ptr_with_comment)(int a, /* comment between params */ 
                                     char b, \
                                     /* line continuation above */ \
                                     float c);

/* Complex nested type 1: Struct with all delimiter types */
struct outer_struct {
    /* Nested anonymous union with attributes */
    union {
        struct {
            int x;
            char y;
        } inner ALIGN_SPEC;
        
        /* Bit-field with unusual value */
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 3; /* 3 bits */
            unsigned int : 4; /* padding with colon */
        } bits PACKED_SPEC;
    } data;
    
    /* Array with complex dimension calculation */
    double matrix[3][ARRAY_SIZE((int[]){1,2,3,4})]; /* Default case trigger: numbers and braces */
    
    /* Function pointer with nested attributes */
    void (*callback)(int, ...) DEPRECATED SECTION_SPEC;
    
    /* Pointer to array of function pointers */
    int (*(*complex_array)[5])(void);
};

/* Type 2: Deeply nested parentheses and brackets */
typedef struct node** (*(*tree_visitor)(struct node ***roots[10]))[20];

/* Type 3: Mixed delimiters with GCC extensions */
typedef __attribute__((vector_size(32))) float v8sf __attribute__((aligned(32)));

/* Type 4: Union with nested struct containing all delimiter types */
union container {
    struct {
        /* Nested parentheses in array size */
        char buffer[((sizeof(int) == 4) ? 256 : 128)];
        
        /* Function pointer with attributes between params */
        void* (*allocator)(size_t __attribute__((unused)) size, \
                           int flags);
        
        /* Anonymous struct with bitfields */
        struct {
            unsigned : 16; /* Unnamed bitfield - default case trigger (colon) */
            unsigned counter : 8;
            unsigned : 0; /* Zero-width bitfield - force alignment */
        } status;
    } header PACKED_SPEC;
    
    /* Another nested level */
    struct deeper {
        /* Array of pointers to functions returning pointers to arrays */
        int (*(*(*func_table)[10])(int))[5];
        
        /* Nested union inside struct */
        union {
            /* Macro expansion with parentheses */
            long values[WEIRD_MACRO(5)];
            
            /* Pointer with attribute */
            volatile int* volatile ptr __attribute__((aligned(8)));
        } variant;
    } *deep_ptr;
};

/* Type 5: Extremely complex declaration mixing everything */
typedef union {
    struct {
        /* Nested switch-like syntax in comments to potentially confuse lexer */
        enum { RED = 0xFF0000, GREEN = 0x00FF00, BLUE = 0x0000FF } color;
        
        /* Multi-dimensional array with computed sizes */
        unsigned char pixels[256][256][4] \
            __attribute__((aligned(32)));
    } image;
    
    /* Function pointer with nested attributes and complex return type */
    struct outer_struct* (*(*get_factory)(const char* name \
        __attribute__((nonnull))))(int version, ...) \
        __attribute__((warn_unused_result));
} graphics_resource;

/* Type 6: Template-like macro usage (C doesn't have templates but we can simulate) */
#define DECLARE_CONTAINER(T) \
    typedef struct { \
        T* data; \
        size_t (*size)(void); \
        void (*resize)(size_t new_size __attribute__((unused))); \
    } T##_container

/* Instantiate the macro - will expand with nested delimiters */
DECLARE_CONTAINER(int);
DECLARE_CONTAINER(double);

/* Type 7: Nested attributes and __extension__ */
typedef __extension__ struct {
    long long __ll __attribute__((mode(DI)));
    double __d __attribute__((mode(DF)));
} long_double_union __attribute__((aligned(16)));

/* Type 8: Pointer to array of structs containing function pointers */
struct callback_registry {
    /* Mixed delimiters: [] then () then {} */
    struct {
        char name[50];
        union {
            void (*func1)(void);
            int (*func2)(int, ...);
        } callback;
        int priority;
    } entries[100] SECTION_SPEC;
    
    /* Nested parentheses in attribute */
    int count __attribute__((deprecated("Use size() instead"), unused));
};

/* Type 9: Using __builtin_ types */
typedef __builtin_va_list va_list_wrapper[1];
typedef __builtin_ffs(int) ffs_func __attribute__((const));

/* Type 10: Final complex type with everything interleaved */
typedef int (*(**(* const volatile complex_signal_handler[10])(void))[5])(
    struct outer_struct* param1,
    union container* param2,
    /* Default case triggers: numbers and operators */
    int flags __attribute__((unused)) = (1 << 3) | (1 << 5),
    ... /* Ellipsis - another default case trigger */
) __attribute__((noreturn));

/* Additional declarations to ensure more parsing */
extern struct outer_struct global_instance;
static union container static_container = { .header.buffer = {0} };
const volatile long_double_union cv_union = { .__ll = 0 };

/* Function declarations (not definitions) to add more parentheses */
void register_callback(int (*cb)(int, char**), const char* name);
struct node** flatten_tree(struct node* root, int depth);

/* Line with just special characters that might hit default case */
/* This comment contains: ~!@#$%^&*_+-=|\;:'",<.>/?` */

/* Final type with nested macro expansion inside delimiters */
typedef struct {
    int array[WEIRD_MACRO(10)]; /* Will expand to (10 + 1) */
    struct {
        char* name;
        /* Empty parentheses */
        void (*init)(void);
    } helper;
} final_type_t;

/* End of complex type definitions */
