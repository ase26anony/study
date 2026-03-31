/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser coverage
 * Specifically targets consume_balanced() default case and nested delimiters
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 3)  /* Contains shift operator in parentheses */
#define ATTR_ALIGN __attribute__((aligned(16)))  /* Nested parentheses */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)
#define NESTED_PAREN ((((4))))  /* Multiple nested parentheses */

/* 2. Complex typedef with all delimiter types */
typedef int (*complex_func_ptr_t)(
    /* Default case trigger: comment inside parentheses */
    struct inner_struct {
        int x;
        /* Line continuation inside braces */
        char y \
            [ARRAY_DIM];
    } *arg1,
    /* Mixed delimiters in array argument */
    int (*callback)(int matrix[3][4], void *data),
    /* Attribute inside function parameter */
    volatile const char * ATTR_ALIGN arg3
) ATTR_ALIGN;

/* 3. Struct with deeply nested delimiter sequences */
struct outer_struct {
    /* Function pointer member with nested attributes */
    void (*operation)(
        int param,
        /* GNU attribute in parameter */
        struct { int a; char b; } __attribute__((packed)) *data
    ) __attribute__((deprecated));
    
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*callbacks[ARRAY_DIM])())()[NESTED_PAREN];
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:2;
            /* Default case: numeric constant with exponent */
            double value 3.14e-10;
        };
        /* Array with computed size */
        char buffer[sizeof(struct { int x; double y; })];
    } ATTR_ALIGN;
    
    /* Pointer to array of structs containing function pointers */
    struct element {
        int id;
        /* Complex function pointer type */
        void (*(*signal_handler[2])(int, ...))(
            /* Nested parentheses with operators */
            int (*(*)(int, int))[5]
        );
    } (*elements)[] ATTR_ALIGN;
};

/* 4. Union with macro expansions in type definitions */
union data_union {
    /* Using macro that expands to function pointer typedef */
    FUNC_PTR_TYPEDEF(local_func_ptr_t) member1;
    
    /* Vector type (GCC extension) */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vectors[2];
    
    /* Nested struct with __builtin_offsetof */
    struct {
        char *name;
        /* Default case: offsetof with parentheses */
        size_t offset = __builtin_offsetof(struct outer_struct, elements);
    } meta;
};

/* 5. Enum with complex initializers */
enum error_codes {
    ERR_NONE = 0,
    /* Expression with parentheses */
    ERR_PARSE = (1 << 0) | (1 << 2),
    /* Nested ternary in initializer (GCC extension) */
    ERR_COMPLEX = sizeof(int) == 4 ? 100 : 200,
    /* Macro expansion */
    ERR_MACRO = ARRAY_DIM
};

/* 6. Typedef combining everything */
typedef union {
    /* Anonymous struct with nested braces */
    struct {
        /* Function returning pointer to array */
        int (*get_matrix(void))[][4];
        
        /* Nested switch-like syntax in comment to test default case */
        /* switch(state) {
         *   case 1: break;
         *   default: advance(); break;
         * }
         */
        
        /* Pointer to function with nested attributes */
        void (__attribute__((noreturn)) *panic_handler)(
            const char *fmt,
            ...
        ) __attribute__((format(printf, 1, 2)));
    };
    
    /* Array with GNU range syntax */
    unsigned char raw[1 .. 256];
} mega_type_t ATTR_ALIGN;

/* 7. Variable declarations using complex types */
static complex_func_ptr_t global_callback = ((void*)0);
volatile struct outer_struct ATTR_ALIGN * volatile outer_ptr;
mega_type_t __attribute__((section(".data"))) global_data;

/* 8. Inline assembly in type context (GCC extension) */
typedef struct {
    int value;
    /* Assembly with braces and parentheses */
    void (*asm_op)(void) = ({
        __asm__ volatile ("nop" : : : "memory");
        (void(*)())0;
    });
} asm_struct_t;

/* 9. Nested typeof expressions */
typedef typeof(*((
    /* Complex pointer dereference */
    struct { 
        typeof(int[ARRAY_DIM]) *array_ptr; 
    }*)0
)).array_ptr array_type;

/* 10. Designated initializers in type definition */
struct config {
    int version;
    union {
        struct {
            int flags;
            char name[];
        } v1;
        struct {
            /* Nested designated initializer syntax */
            int .mode = 1,
            .count = ARRAY_DIM
        } v2;
    } data;
};

/* 11. Alias with __attribute__ and pointer to array */
typedef int (__attribute__((const)) *const_fn_ptr)(int) 
    __attribute__((warn_unused_result));

/* 12. Struct with alignment attribute containing another attribute */
struct __attribute__((aligned(
    /* Nested attribute calculation */
    __alignof__(long double)
))) max_align_struct {
    /* Flexible array member */
    long double data[];
};

/* 13. Complex function prototype for parsing */
extern int (*(*register_callback(
    /* Parameter with nested attributes */
    void (__attribute__((noinline)) *handler)(
        int,
        /* Nested struct parameter */
        struct { int line; char *file; } ctx
    ),
    /* Array parameter with static size (C99) */
    int param[static 4]
))[2])();

/* 14. __extension__ usage for GNU extensions */
typedef __extension__ struct {
    /* 128-bit integer */
    __int128 large_int;
    /* Nested anonymous struct */
    struct {
        /* Complex bitfield */
        unsigned: (sizeof(int) * 8) - 1;
        unsigned flag: 1;
    };
} ext_struct_t;

/* 15. Final complex declaration mixing everything */
static const volatile __attribute__((may_alias)) 
union {
    /* Nested function pointer with attributes */
    int (__attribute__((hot)) *hot_func)(
        /* Parameter with array of function pointers */
        void (*hooks[])(void),
        /* Variable length array */
        int size
    );
    
    /* Pointer to array of structs with bitfields */
    struct {
        unsigned a:1, b:1, c:1, d:29;
        /* Zero-length array (GCC extension) */
        int flex[];
    } *bit_structs;
    
    /* Make sure we have unmatched tokens for error recovery testing */
    /* This comment has an unmatched parenthesis: (
     * And an unmatched brace: {
     */
} *ultimate_type ATTR_ALIGN = 0;
