/* test_gengtype_coverage.h
 * Complex type definitions to test gengtype's consume_balanced function
 */

#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[3])(int, char);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags:4;
        unsigned int mode:2;
        long counter:24;
    } __attribute__((packed)) inner_bits;
    
    /* Multi-dimensional array (brackets) */
    double matrix[5][10][2];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(void))[10];
};

/* 2. Function Pointer Declarations with Varied Signatures */
/* Simple function pointer */
typedef void (*simple_callback)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*nested_func_ptr)(char *))(double);

/* Complex function pointer with nested parameters */
void (*signal_handler(
    int sig, 
    void (*handler)(int, void*)
))(int, const char*);

/* Function taking function pointer that returns array pointer */
int process_data(
    int (*generator)(int, int), 
    char (*buffer)[256]
);

/* 3. Multi-dimensional Arrays and Flexible Array Members */
struct ArrayContainer {
    int fixed[5][10][3];           /* Multi-dimensional */
    char *string_array[20];        /* Array of pointers */
    int flexible[];                /* Flexible array member */
};

/* 4. Nested Anonymous Structs/Unions and Bit-fields */
struct BitFieldStruct {
    /* Anonymous union with bit-fields */
    union {
        struct {
            unsigned char a:2;
            unsigned char b:3;
            unsigned char c:3;
        } bits;
        unsigned char full;
    } byte1;
    
    /* Nested anonymous struct */
    struct {
        long x:16;
        long y:16;
        long z:16;
        long w:16;
    } __attribute__((packed));
    
    /* Another level of nesting */
    struct {
        union {
            int i;
            float f;
        } value;
        int tag:4;
    } tagged_union;
};

/* 5. Macro Expansions Generating Brackets */
#define PTR_FUNC(T) T (*(*)(T))(T)
#define ARRAY_TYPE(T, N) T (*)[N]
#define NESTED_PTR(T) T (*(*(*)(void))[5])(void)

/* Use the macros in declarations */
PTR_FUNC(int) complex_function_ptr;
ARRAY_TYPE(char, 10) string_array_ptr;
NESTED_PTR(double) ultra_nested_ptr;

/* Macro generating struct with all bracket types */
#define COMPLEX_STRUCT(name) \
    struct name { \
        void (*(*funcs[2]))(int); \
        int (*matrix)[3][4]; \
        union { \
            int x; \
            long y; \
        } data; \
    }

COMPLEX_STRUCT(MacroGenerated);

/* 6. Attribute Syntax with Parentheses */
struct __attribute__((aligned(16), packed)) AlignedStruct {
    int data[4];
    char padding;
} __attribute__((deprecated("Use NewStruct instead")));

/* Function with attributes */
int __attribute__((noinline, hot)) 
optimized_function(
    int __attribute__((unused)) param1,
    char *__attribute__((nonnull)) param2
) __attribute__((warn_unused_result));

/* Variable with attribute containing parentheses */
int important_var __attribute__((section(".data.important"), used));

/* 7. Include All Bracket Types in Single Declaration */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*signal(
        int sig, 
        void (*func)(int, const char*)
    ))(int);
    
    /* Array of function pointers with nested parameters */
    int (*pfa[2])(
        int (*callback)(char[10], int),
        void *context
    );
    
    /* Anonymous union with bit-field */
    union {
        int as_int;
        struct {
            unsigned int low:16;
            unsigned int high:16;
        } as_bits;
    } value;
    
    /* Multi-dimensional pointer array */
    int *(*(*nested_ptr_array[3])[2])[5];
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        void (*action)(void);
    } actions[];
} __attribute__((aligned(32)));

/* Additional edge cases */

/* Typedef combining all bracket types */
typedef struct {
    int (*(*get_processor(void))[5])(
        char (*input_buffer)[256],
        int (*filter)(int)
    );
    union {
        int x[10];
        struct {
            long a:10;
            long b:10;
            long c:12;
        } bits;
    } data;
} UltimateProcessor __attribute__((may_alias));

/* Nested anonymous structs in union */
union DeepNest {
    struct {
        struct {
            int a;
            struct {
                char x:2;
                char y:2;
                char z:4;
            } flags;
        } level1;
    } level2;
    
    long long as_long;
};

/* Function returning pointer to array of function pointers */
int (*(*create_dispatcher(int type))[10])(void);

/* Struct with zero-length array (GCC extension) */
struct ZeroLength {
    int count;
    int data[0];  /* Zero-length array */
};

/* Struct with nested attribute */
struct WithNestedAttribute {
    int field __attribute__((aligned(8), deprecated));
    char *ptr __attribute__((cleanup(free)));
};

/* Final test: Everything combined */
typedef struct __attribute__((packed)) {
    /* Macro expansion with all brackets */
    PTR_FUNC(void) *func_table[5];
    
    /* Anonymous struct with bit-fields */
    struct {
        unsigned int valid:1;
        unsigned int type:3;
        unsigned int size:28;
    };
    
    /* Complex array declaration */
    int (*(*array_of_func_ptrs[3][2])(int))[5];
    
    /* Flexible array member of structs */
    struct {
        int id;
        union {
            int num;
            char *str;
        } value;
    } items[];
} ComprehensiveType;

#endif /* TEST_GENGTYPE_COVERAGE_H */
