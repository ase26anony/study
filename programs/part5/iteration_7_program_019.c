/* test-gengtype-parser.c - Complex type definitions to exercise gengtype parser */

/* Preprocessor macros that expand to delimiter sequences */
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ALIGNED_ATTR __attribute__((aligned(16)))
#define PACKED_ATTR __attribute__((packed))
#define SECTION_ATTR(sect) __attribute__((section(#sect)))

/* Macro that expands to complex array dimensions with operators */
#define COMPLEX_DIM (1 + 2 * 3 - 4)

/* Trigger default case: line continuation inside type definition */
#define MULTI_LINE_MACRO \
    int multi_line_field; \
    char another_field

/* Complex nested type definitions */

/* 1. Struct with function pointer containing nested parentheses */
struct OuterStruct {
    /* Function pointer with attributes inside parentheses */
    int (*callback1)(int x, 
                     /* Comment inside parentheses triggers default case */
                     char y /* embedded comment */);
    
    /* Another function pointer with GNU attributes */
    void (*callback2)(void) ALIGNED_ATTR;
    
    /* Array with complex dimension calculation */
    float matrix[3][COMPLEX_DIM];
    
    /* Nested anonymous struct */
    struct {
        /* Bit-field with unusual syntax */
        unsigned int flags : 4;
        
        /* Union inside anonymous struct */
        union {
            int as_int;
            float as_float;
            /* Pointer to array */
            char (*string_array)[10];
        } data;
    } inner;
};

/* 2. Typedef with all three delimiter types mixed */
typedef struct {
    /* Array of function pointers */
    int (*(*func_table)[5])(int, char);
    
    /* Nested parentheses in function pointer return type */
    struct InnerType* (*(*get_factory)(void))(int);
    
    /* GNU statement expression inside array size */
    char dynamic_buffer[({ int size = 256; size; })];
} ComplexType;

/* 3. Union with deeply nested delimiters */
union DeepNested {
    /* Struct with array of pointers to functions */
    struct {
        void (*(*handlers[3])(int))[2];
        
        /* Macro expansion with line continuation */
        MULTI_LINE_MACRO;
    } struct_part;
    
    /* Another nested type */
    struct {
        /* Attribute with parentheses inside struct */
        int field PACKED_ATTR;
        
        /* Pointer to array with computed size */
        double (*coords)[ARRAY_SIZE(((double[]){1.0, 2.0, 3.0}))];
    } other_part;
};

/* 4. Enum with embedded expressions in initializers */
enum SpecialValues {
    /* Values with parentheses */
    VALUE_A = (1 << 0),
    VALUE_B = (1 << 1),
    /* Value with macro expansion */
    VALUE_C = COMPLEX_DIM,
    /* Value with GNU extension */
    VALUE_D = __builtin_abs(-10)
};

/* 5. Typedef for function returning pointer to array of structs */
typedef struct Node** (*(*NodeFactory)(int depth))[10];

/* 6. Struct with attribute containing string literal */
struct AttributedStruct {
    int id;
    char name[32];
} SECTION_ATTR(.special_section) ALIGNED_ATTR;

/* 7. Complex declaration mixing all delimiter types in one line */
static void (*(*global_callback_array[2])(int, struct {int a; char b;}))(float, 
    /* Comment between parameters triggers default case */
    double) = {NULL, NULL};

/* 8. Type with GNU vector extension */
typedef int v4si __attribute__((vector_size(16)));

/* 9. Struct containing a flexible array member with attribute */
struct FlexibleArray {
    size_t count;
    /* Attribute on flexible array member */
    int data[] PACKED_ATTR;
};

/* 10. Anonymous union/struct with bitfields and attributes */
struct BitFieldStruct {
    /* Multiple bitfields */
    unsigned int a : 1;
    unsigned int b : 2;
    unsigned int c : 3;
    
    /* Anonymous union */
    union {
        int x;
        long y;
    };
    
    /* Attribute on entire struct */
} __attribute__((packed, aligned(8)));

/* 11. Function pointer type with nested attributes */
typedef int (*(*ComplexFuncPtr)(__attribute__((nonnull(1, 2))) void*, 
                                __attribute__((const)) int))[5];

/* 12. Struct with nested type definitions inside */
struct Container {
    /* Type definition inside struct */
    typedef enum { RED, GREEN, BLUE } Color;
    
    /* Another nested typedef */
    typedef struct {
        int x, y;
    } Point;
    
    Color color;
    Point position;
    
    /* Array of nested type */
    Point trajectory[10];
};

/* 13. Macro that expands to complex type with all delimiters */
#define MAKE_COMPLEX_TYPE(name) \
    struct name##_type { \
        int (*methods[2])(void); \
        union { \
            struct { int a; char b; } s; \
            long l; \
        } u; \
    }

/* Use the macro to create a type */
MAKE_COMPLEX_TYPE(Generated);

/* 14. Type with __builtin_choose_expr */
typedef __typeof__(__builtin_choose_expr(1, (int){0}, (float){0})) ChoiceType;

/* 15. Final complex type mixing everything */
typedef union {
    /* Function pointer returning pointer to array */
    struct OuterStruct* (*(*get_structs)(int count))[];
    
    /* Nested struct with all features */
    struct {
        /* Attribute with string argument */
        __attribute__((deprecated("use new_field instead")))
        int old_field;
        
        /* Pointer to function with nested attributes */
        void (*deprecated_callback)(void) 
            __attribute__((deprecated));
        
        /* Two-dimensional array with complex sizing */
        int grid[({ int x = 3; x; })][COMPLEX_DIM];
    } nested;
} UltimateType;

/* Trigger parser with preprocessor directives inside type context */
#ifdef __GNUC__
/* GCC-specific type */
typedef __attribute__((transparent_union)) union {
    int i;
    float f;
} TransparentUnion;
#endif

/* Multi-line comment with unusual characters 
   that might trigger default case when parsed */
/********************************************\
 *  Special: !@#$%^&*()_+-=[]{}|;:'",.<>/?  *
\********************************************/

/* Global variable using complex type */
static ComplexType global_var = {
    .func_table = NULL,
    .get_factory = NULL,
    .dynamic_buffer = {0}
};

/* The file ends here - no main function needed as gengtype only parses types */
