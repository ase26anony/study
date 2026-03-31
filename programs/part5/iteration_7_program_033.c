/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 * Specifically targets lines 341-352 in gengtype-parse.cc
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGN_SPEC __attribute__((aligned(16)))
#define PACKED_STRUCT __attribute__((packed)) struct
#define FUNC_PTR_TYPEDEF(name, ret, ...) typedef ret (*name)(__VA_ARGS__)

/* 2. Complex nested type definitions with all delimiter types */

/* Type with nested parentheses in function pointer */
typedef int (*complex_func_ptr_t)(
    int arg1, 
    /* Comment inside parentheses to trigger default case */
    char *arg2, 
    /* Line continuation inside parentheses: \
       this should trigger advance() in default case */
    struct inner { int x; double y; } *arg3,
    /* Attribute with parentheses inside argument list */
    void (*callback)(int) __attribute__((nonnull(1)))
);

/* Struct with mixed delimiters */
struct outer_struct {
    /* Array with size calculation using parentheses */
    int matrix[ARRAY_SIZE(((int[]){1,2,3,4}))];
    
    /* Nested anonymous union with attributes */
    union {
        /* Bit-field with unusual syntax */
        unsigned int flags : 4 /* comment in bit-field */;
        
        /* Function pointer member with complex return type */
        struct { 
            int count; 
            char **data; 
        }* (*get_data)(int index, ...);
    } ALIGN_SPEC;
    
    /* Pointer to array of function pointers */
    void (*(*signal_handlers[10])(int, void*))();
    
    /* GCC vector extension with attribute */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[4];
};

/* 3. Deeply nested type with all delimiter types interleaved */
typedef struct {
    /* Multi-dimensional array with computed sizes */
    unsigned char (*pixel_data)[
        (int)(256.0 * /* floating constant */ 1.5)
    ][
        sizeof(struct { int r; int g; int b; })
    ];
    
    /* Union containing struct containing union... */
    union level1 {
        struct level2 {
            union level3 {
                /* Nested function pointer with attributes */
                int (**operations)(
                    /* Parameter with attribute */
                    const char *str __attribute__((format_arg(1))),
                    /* Empty parameter (just void) */
                    void
                ) __attribute__((deprecated));
                
                /* Array of pointers to arrays */
                int *(*(*complex_array)[5])[10];
            } deepest;
            
            /* Enum with hex constants */
            enum { 
                VAL_A = 0x1, 
                VAL_B = 0x2, 
                /* Value with backslash continuation \
                   to trigger default case */
                VAL_C = 0x4 \
            } options;
        } middle;
        
        /* Anonymous struct with bit-fields */
        struct {
            unsigned int a : 1;
            unsigned int b : 2;
            /* Comment between bit-field declarations */
            unsigned int c : 3; /* trailing comment */
        };
    } nested_data PACKED_STRUCT;
} ultra_complex_t;

/* 4. Type using macro that expands to complex delimiter sequences */
FUNC_PTR_TYPEDEF(
    macro_func_ptr,
    struct outer_struct**,
    /* Multiple parameters with different delimiters */
    ultra_complex_t (*)[10],  /* Pointer to array */
    int (*callback)(int, int), /* Function pointer parameter */
    ... /* Variadic argument */
);

/* 5. Struct with attribute containing parentheses inside braces */
struct __attribute__((aligned(
    /* Calculation inside attribute argument */
    sizeof(long double) > 8 ? 16 : 8
))) aligned_struct {
    /* Member with attribute that has parentheses */
    char *name __attribute__((nonnull(1), 
        /* Nested attribute */
        __attribute__((warning("deprecated")))));
    
    /* Zero-length array at end */
    int flexible_array[];
};

/* 6. Complex typedef with nested everything */
typedef union {
    /* Anonymous struct */
    struct {
        /* Pointer to function returning pointer to array */
        int (*(*get_matrix)(void))[][10];
        
        /* Nested struct with bit-fields and union */
        struct {
            union {
                /* __int128 is a GCC extension */
                __int128 large_int;
                
                /* Array of structs containing arrays */
                struct {
                    float coords[3];
                    /* Line continuation inside array initializer (incomplete) \
                       would be in full program */
                } points[100];
            } data;
            
            /* Bit-field spanning multiple lines */
            unsigned int 
                field1 : 1, 
                field2 : 2, 
                /* Comment in middle of bit-field list */
                field3 : 3,
                field4 : 4;
        } container;
    };
    
    /* Function pointer with complex signature */
    void (*(*signal_callback)(int sig, 
        /* Struct parameter */
        struct { 
            void *context; 
            int pid; 
        } info))(
            /* Function pointer return type parameters */
            int, void*, ...
        );
} ultimate_union_t;

/* 7. Forward declarations inside complex type */
typedef struct forward_decl *(*factory_method)(
    /* Parameter with forward declaration */
    struct incomplete *arg,
    /* Another parameter with complex type */
    enum { RED, GREEN, BLUE } color,
    /* Attribute on parameter */
    const char *msg __attribute__((unused))
);

/* 8. Type with __builtin_ types */
typedef __builtin_va_list va_list_t;
typedef __builtin_ms_va_list ms_va_list_t;

/* 9. Struct using typeof extension */
struct with_typeof {
    /* Member with typeof containing parentheses */
    typeof(main) *main_ptr;  /* main is a function */
    
    /* Array sized by sizeof typeof */
    char buffer[sizeof(typeof(int[10]))];
};

/* 10. Final complex declaration mixing everything */
static volatile const ultimate_union_t *(*global_handler)(
    int event_id,
    /* Nested struct literal type in parameter */
    struct { 
        int priority; 
        /* Function pointer inside parameter struct */
        void (*log)(const char*, ...); 
    } *event_data,
    /* Array parameter with static keyword (GCC extension) */
    int param_array[static 10],
    /* Complex attribute */
    ... /* variadic */
) __asm__("global_handler") __attribute__((visibility("hidden")));

/* The file ends with type definitions only - no main function needed
 * as gengtype only parses type declarations */
