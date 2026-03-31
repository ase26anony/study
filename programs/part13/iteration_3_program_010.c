/* gengtype-test.h - Complex type definitions to test consume_balanced() */
#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* 1. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T [n]
#define NESTED_PTR(T) T (*(*))(T)
#define ATTR_ALIGNED(n) __attribute__((aligned(n)))

/* 2. Function pointer declarations with varied signatures */
/* Simple function pointer */
typedef void (*simple_func_ptr)(int);

/* Pointer to function returning pointer to function */
typedef int (*(*complex_func_ptr)(int (*)(char[10])))(double);

/* Nested parentheses in parameters */
typedef void (*signal_handler)(int sig, void (*func)(int));

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayStruct {
    int multi_dim[3][4][5];
    char strings[10][256];
    int flexible_array[];
};

/* 4. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer with array */
    int (*func_array[5])(int, char);
    
    /* Nested struct with bit-fields */
    struct {
        unsigned int flag1:1;
        unsigned int flag2:3;
        unsigned int reserved:28;
    } bitfield_struct;
    
    /* Union inside struct */
    union {
        int int_val;
        float float_val;
        struct {
            char c;
            short s;
        } nested;
    } data_union;
    
    /* Pointer to array of function pointers */
    int (*(*ptr_to_func_array)[10])(void);
    
    /* Array of pointers to functions returning pointers */
    void *(*(*func_ptr_array[3])(int))[2];
};

/* 5. Highly complex single declaration combining all bracket types */
struct UltimateType {
    /* Function returning function pointer */
    void (*(*signal(int sig, void (*handler)(int)))(int))(int);
    
    /* Array of function pointers with complex signatures */
    int (*(*pfa[2])(int (*)(char[10])))(double);
    
    /* Nested anonymous union with bit-field */
    union {
        int x;
        struct {
            unsigned long y:8;
            unsigned long z:24;
        } bits;
    } anonymous_union;
    
    /* Multi-dimensional array */
    int matrix[3][3][3];
    
    /* Pointer to flexible array member in nested struct */
    struct {
        int count;
        int data[];
    } *flex_struct_ptr;
};

/* 6. Attribute syntax with parentheses */
struct ATTR_ALIGNED(16) AlignedStruct {
    double data[4];
    int flags;
} ATTR_ALIGNED(32);

/* 7. Using macros to create complex types */
typedef PTR_FUNC(int) int_func_ptr_t;
typedef ARRAY_DECL(PTR_FUNC(char), 5) func_ptr_array_t;

/* 8. Even more nesting */
struct DeeplyNested {
    /* Array of structs containing unions containing structs */
    struct {
        union {
            struct {
                int a;
                int b[2][2];
            } s1;
            struct {
                float f;
                double d[3];
            } s2;
        } u;
    } nested_array[4];
    
    /* Function pointer with deeply nested parameters */
    void (*(*deep_func)(int (*(*)(char(*)[10]))(double)))(float);
};

/* 9. Typedef with complex declarator */
typedef int (*(*(*complex_typedef)(int))[5])(void);

/* 10. Struct with all possible bracket combinations */
struct AllBrackets {
    int simple;                    /* default case */
    int array[10];                 /* '[' case */
    int (*func)(int);              /* '(' case */
    struct {                       /* '{' case */
        int x;
        int y;
    } point;
    int (*func_array[5])(int[2]);  /* Mixed: '[' then '(' */
    struct {
        union {
            int a;
            long b;
        } u;
        int arr[3];
    } nested;                      /* Nested '{' and '[' */
};

/* 11. Test default case triggers (non-bracket characters) */
/* These should trigger the default case in consume_balanced */
struct DefaultCaseTest {
    int a;          /* ';' after identifier */
    char *b;        /* '*' pointer */
    const int c;    /* 'const' keyword */
    volatile long d; /* 'volatile' keyword */
    unsigned e:4;   /* ':' in bit-field */
    int f, g;       /* ',' separator */
};

/* 12. Function declarations with attributes */
void __attribute__((noreturn)) fatal_error(void);
int __attribute__((format(printf, 1, 2))) debug_printf(const char *fmt, ...);

/* 13. Complex initialization (if gengtype processes initializers) */
#ifdef PROCESS_INITIALIZERS
struct InitExample {
    int values[3];
    struct {
        int x;
        int y;
    } point;
} init_example = {
    .values = {1, 2, 3},      /* Nested braces */
    .point = {.x = 10, .y = 20}
};
#endif

/* 14. Enum with complex expressions in initializers */
enum ComplexEnum {
    ENUM_A = (1 << 0),        /* Parentheses in expression */
    ENUM_B = (ENUM_A << 1),   /* More parentheses */
    ENUM_C = sizeof(int[5])   /* sizeof with brackets */
};

/* 15. Final stress test: everything combined */
typedef struct {
    union {
        struct {
            int (*(*func_ptr)(int (*)(char[10])))(double);
            struct {
                unsigned bits:16;
                unsigned more:16;
            } flags;
        } s;
        long long raw;
    } u ATTR_ALIGNED(8);
    
    PTR_FUNC(void) callback;
    int matrix[2][2];
    void (*handlers[3])(int, ...);
} StressTestType ATTR_ALIGNED(64);

#endif /* GENGYPE_TEST_H */
