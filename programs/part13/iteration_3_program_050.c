/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_2D(T, N, M) T[N][M]
#define NESTED_PTR(T) T (*(*)(T (**)(T)))(T)

/* 2. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer with nested parentheses */
    void (*signal_handler)(int sig, void (*cleanup)(void*));
    
    /* Pointer to function returning pointer to function */
    int (*(*complex_func)(int (*callback)(char)))(double);
    
    /* Multi-dimensional arrays */
    int matrix[3][4];
    char strings[5][256];
    
    /* Flexible array member */
    int flexible_array[];
};

/* 3. Union with anonymous struct and bit-fields */
union ComplexUnion {
    struct {
        unsigned int flag1:1;
        unsigned int flag2:3;
        unsigned int flag3:4;
        unsigned int :24;  /* Unnamed bit-field */
    } bits;
    
    struct {
        int (*func_ptr)(int, char);
        long data;
    } nested;
    
    /* Array of function pointers */
    void (*actions[5])(void);
};

/* 4. Struct containing all bracket types in single declaration */
struct UltimateTest {
    /* Combination: array of pointers to functions with nested params */
    int (*(*func_array[2])(int (*)(char[10])))(double);
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:8;
            unsigned int b:8;
            unsigned int c:16;
        } byte_fields;
        unsigned int full;
    } data_union;
    
    /* Multi-dimensional pointer array */
    int *(*(*ptr_matrix[2][3]))[4];
    
    /* Function pointer with attribute */
    void (* __attribute__((aligned(16))) aligned_func)(void);
};

/* 5. Typedef with complex nested parentheses */
typedef int (*(*ComplexCallback)(int (*(*)(char))(double)))(float);

/* 6. Struct with GCC attributes (double parentheses) */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char c;
    int i __attribute__((aligned(16)));
    double d;
} __attribute__((deprecated));

/* 7. More function pointer variations */
/* Pointer to function taking array of function pointers */
void (*process_callbacks(void (*callbacks[])(int), int count))(int);

/* Function returning pointer to array */
int (*get_array_ptr(void))[10];

/* 8. Nested struct definitions */
struct Container {
    struct Inner {
        union {
            int x;
            struct {
                short a;
                short b;
            } parts;
        } value;
        
        /* Array within nested struct */
        int table[2][2];
    } inner;
    
    /* Pointer to nested struct type */
    struct Inner *inner_ptr;
};

/* 9. Using macros to generate complex types */
PTR_FUNC(int) *macro_func_ptr;
ARRAY_2D(double, 5, 5) macro_matrix;

/* 10. Even more complex: function pointer with nested everything */
struct FinalChallenge {
    /* The ultimate test: all brackets deeply nested */
    int (*(*(*ultimate[2])(int (*(*)(char[2][3]))(double)))[3])(float (*(*)(void))[4]);
    
    /* Anonymous struct with bit-fields and function pointer */
    struct {
        unsigned int :4;
        unsigned int mode:3;
        unsigned int error:1;
        void (*error_handler)(int, const char*);
    } status;
    
    /* Flexible array of structs with function pointers */
    struct {
        int id;
        void (*action)(void);
    } actions[];
};

/* 11. Variable declarations with attributes */
extern int global_array[] __attribute__((weak, alias("backup_array")));
static volatile int __attribute__((aligned(32))) aligned_var;

/* 12. Function declarations with complex return types */
int (*(*register_callback(int (*(*cb)(int))(char)))(float))(double);

/* 13. Union with array of pointers to functions with different signatures */
union FunctionUnion {
    int (*int_funcs[3])(int);
    void (*void_funcs[2])(void);
    char *(*str_funcs[4])(const char*, int);
};

/* 14. Struct with nested anonymous structs/unions */
struct DeepNest {
    struct {
        union {
            struct {
                int x;
                int y;
            } point;
            struct {
                long start;
                long end;
            } range;
        } data;
        
        struct {
            unsigned int valid:1;
            unsigned int type:4;
            unsigned int :27;
        } flags;
    } header;
    
    /* Array of unions */
    union {
        int i;
        float f;
        void *p;
    } values[10];
};

/* 15. Edge case: empty brackets */
struct EmptyTest {
    int empty_array[0];  /* Zero-length array */
    struct {} empty_struct;  /* Empty struct */
    union {} empty_union;    /* Empty union */
};

/* 16. Function-like macro with brackets */
#define CREATE_HANDLER(NAME, TYPE) \
    TYPE (*NAME##_handler)(TYPE (*)(TYPE[2]), int)

/* Use the macro */
CREATE_HANDLER(my, int);

/* 17. Pointer to array of function pointers */
typedef void (*(*CallbackDispatcher)(int))[5];
CallbackDispatcher dispatchers[3];

/* 18. Struct with all possible bracket combinations */
struct AllBrackets {
    int a;                          /* default case */
    int b[5];                       /* '[' case */
    int *c;                         /* default case */
    int (*d)(int);                  /* '(' case */
    struct {                        /* '{' case */
        int e;
        int f[3];
    } inner;
    int (*(*g)[2])(int, char);      /* Mixed: '(' and '[' */
    union {                         /* '{' case */
        int h;
        struct {                    /* Nested '{' */
            int i:4;
            int j:4;
        } bits;
    } u;
    int (*(*(*k)(void))[3])(int);   /* Deep nesting */
};

#endif /* TEST_GENGTYPE_COVERAGE_H */
