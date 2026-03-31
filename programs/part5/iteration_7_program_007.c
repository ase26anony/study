/* test-gengtype-coverage.c
 * Complex type definitions to exercise gengtype parser's consume_balanced function
 */

/* 1. Preprocessor macros that expand to delimiter sequences */
#define ARRAY_DIM (1 << 2) /* Contains shift operator in parens */
#define ATTR_ALIGN __attribute__((aligned(16))) /* Nested parens in macro */
#define FUNC_PTR_TYPEDEF(name) typedef void (*name)(int, ...)

/* 2. Trigger default case with unusual characters in nested contexts */
FUNC_PTR_TYPEDEF(ComplexFuncPtr) ATTR_ALIGN; /* Attribute after typedef */

/* 3. Struct with deeply nested delimiter sequences */
struct Level1 {
    /* Default case trigger: numeric constant with decimal point */
    float f ATTR_ALIGN;
    
    /* Mixed delimiters: function pointer returning array pointer */
    int (*(*callback)(struct Level1 ***, int))[ARRAY_DIM];
    
    /* Nested anonymous union with bitfields */
    union {
        struct {
            unsigned int flag: 1;
            unsigned int /* comment between tokens */ : 7;
        } bits ATTR_ALIGN;
        char raw[sizeof(int)];
    } data;
    
    /* Array of function pointers with GNU attributes */
    void (*handlers[4])(void) __attribute__((deprecated));
};

/* 4. Complex typedef with all delimiter types interdependently */
typedef struct Level1 *(*(*UltimateType)(int (*(*)(float))[3]))[ARRAY_DIM]
    __attribute__((packed, aligned(8)));

/* 5. Union containing switch between all delimiter types */
union MixedDelimiters {
    /* Parentheses in function pointer */
    int (*func_ptr)(char *);
    
    /* Brackets in array with computed size */
    long array[sizeof(struct Level1) + 1];
    
    /* Braces in nested anonymous struct */
    struct {
        /* Line continuation inside type definition \
           (triggers default case for backslash) */
        short \
        multi_line_field;
        
        /* Attribute with nested parentheses */
        double d __attribute__((aligned(32)));
    } nested;
};

/* 6. Enum with embedded expressions in initializers */
enum ComplexEnum {
    VAL1 = (1 << 0),  /* Parentheses with shift operator */
    VAL2 = ARRAY_DIM, /* Macro expansion */
    VAL3 = sizeof(struct Level1[2])  /* sizeof with brackets */
};

/* 7. Type definition with GNU vector extension */
typedef int v4si __attribute__((vector_size(16)));

/* 8. Struct with attribute on nested function pointer */
struct WithNestedAttributes {
    /* Attribute between pointer asterisks */
    char * __attribute__((aligned(8))) * double_ptr;
    
    /* Function pointer with attribute in return type */
    __attribute__((noreturn)) void (*exit_func)(int);
    
    /* Nested array of struct pointers with attribute */
    struct Level1 * ATTR_ALIGN ptr_array[2][3];
};

/* 9. Complex declaration using all features together */
static const volatile struct WithNestedAttributes *(*global_callback)(
    union MixedDelimiters ***, 
    enum ComplexEnum, 
    /* Default case trigger: numeric literal */
    42,  /* Unexpected integer constant between parameters */
    v4si, 
    ...  /* Variadic ellipsis */
) = 0;

/* 10. Typedef with nested anonymous struct/union */
typedef struct {
    union {
        struct {
            int x;
        } s;
        long y;
    } u ATTR_ALIGN;
    
    /* Array with dimension containing parentheses */
    float arr[(ARRAY_DIM) + 1];
    
    /* Function pointer with complex return type */
    struct Level1 *(*(*getter)(void))[2];
} AnonymousTypedef;

/* 11. Macro that expands to partial type definition */
#define DECLARE_CALLBACK(name, type) \
    type (*name)(type (*)(int), type [][ARRAY_DIM])

/* Use macro to create complex declaration */
DECLARE_CALLBACK(macro_callback, struct Level1 *) ATTR_ALIGN;

/* 12. Struct with bitfield containing expression */
struct BitfieldStruct {
    unsigned int a : (1 << 1);  /* Parentheses in bitfield width */
    signed int b : sizeof(char); /* sizeof in bitfield */
    unsigned long c : 1, d : 7; /* Multiple bitfields */
};

/* 13. Forward declaration to test parser recovery */
struct ForwardDecl;

/* 14. Final complex type mixing everything */
typedef union {
    struct BitfieldStruct bs;
    AnonymousTypedef at;
    
    /* Function returning pointer to array of function pointers */
    int (*(*(*func_factory)(void))[3])(char *, ...);
    
    /* Nested attribute with all delimiter types */
    __attribute__((designated_init)) struct {
        int field1;
        int field2[ARRAY_DIM];
    } designated ATTR_ALIGN;
} UltimateUnion ATTR_ALIGN;

/* 15. Global variable with complex type and initializer */
static UltimateUnion global_var = {
    .designated = {
        .field1 = (int)(3.14),  /* Cast with decimal point */
        .field2 = {[0] = VAL1, [ARRAY_DIM-1] = VAL3}
    }
};
