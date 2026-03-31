/* gengtype-test.h - Complex type definitions to test consume_balanced() */
#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* 1. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, n) T[n]
#define NESTED_PTR(T) T (*(*)(T (**)(T)))(T)

/* 2. Complex nested type definitions with all bracket types */
struct Outer {
    /* Function pointer with nested parentheses */
    void (*signal_handler(int sig, void (*callback)(int)))(int);
    
    /* Multi-dimensional arrays */
    int matrix[5][10][2];
    
    /* Anonymous union with bit-fields */
    union {
        unsigned int flags:4;
        unsigned int status:2;
        struct {
            unsigned char a:1;
            unsigned char b:1;
            unsigned char c:6;
        } bits;
    } u;
    
    /* Pointer to array of function pointers */
    int (*(*func_array[3])(int (*)(char[10])))(double);
    
    /* Nested struct with flexible array member */
    struct Inner {
        char name[32];
        int data[];
    } inner;
    
    /* Complex function pointer declaration */
    char (*(*complex_fp)(int (*)(char[10]), void (**)(double)))[20];
};

/* 3. Union with deeply nested constructs */
union MegaUnion {
    /* Array of pointers to functions returning pointers to arrays */
    int (*(*arr_func[2])(void))[5];
    
    /* Struct containing anonymous struct */
    struct {
        /* Pointer to function with function pointer parameter */
        long (*processor)(int (*filter)(short), char *);
        
        /* Nested array with multiple dimensions */
        unsigned char cube[3][3][3];
        
        /* Bit-field struct */
        struct {
            unsigned int ready:1;
            unsigned int busy:1;
            unsigned int error:6;
        } status;
    } nested;
    
    /* Function pointer returning function pointer */
    void (*(*chained_fp)(int))(char);
};

/* 4. Typedef with complex bracketed types */
typedef int (*(*ComplexCallback)(int (*)(char *), void *))(double);

/* 5. Struct using macro expansions */
struct MacroStruct {
    PTR_FUNC(int) int_func_ptr;
    ARRAY_DECL(double, 10) dbl_array;
    NESTED_PTR(char) crazy_ptr;
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(32), packed)) AttributedStruct {
    int data __attribute__((aligned(16)));
    char buffer[64] __attribute__((aligned(8)));
    
    /* Function with attribute */
    void (* __attribute__((noreturn)) fatal_error)(const char *msg);
    
    /* Nested attributed union */
    union __attribute__((transparent_union)) {
        int i;
        long l;
        void *p;
    } value;
};

/* 7. Extremely complex single declaration combining all bracket types */
struct UltimateType {
    /* Combination of all bracket types in one member */
    void (*(*ultimate_member[2])(
        int (*param1)(char[10][5]), 
        union {
            int x;
            struct { short a; short b:4; } s;
        } param2
    ))(int (*)(char), double[3])[5];
    
    /* Mixed brackets with attributes */
    int (*(* __attribute__((warn_unused_result)) attr_member)(
        __attribute__((nonnull)) char **argv
    ))(void) __attribute__((deprecated));
    
    /* Flexible array of function pointers */
    void (*flex_array[])(int, ...);
};

/* 8. More variations to ensure coverage */
/* Function returning pointer to array */
int (*func_ret_array(int size))[10];

/* Pointer to array of function pointers */
int (*(*ptr_to_func_array)[5])(char *);

/* Const volatile qualified function pointer */
void (* const volatile cv_fp)(int) = 0;

/* 9. Nested anonymous structs/unions */
struct Container {
    /* Anonymous struct */
    struct {
        int x;
        int y;
        
        /* Anonymous union inside anonymous struct */
        union {
            float f;
            double d;
        };
    } point;
    
    /* Direct anonymous union */
    union {
        long id;
        char uuid[16];
    };
};

/* 10. Complex typedef chain */
typedef int (*FuncPtr)(int);
typedef FuncPtr (*FuncPtrGenerator)(char);
typedef FuncPtrGenerator (*MetaGenerator)(double);
typedef MetaGenerator GeneratorArray[3];

/* 11. Struct with all possible bracket combinations */
struct AllBrackets {
    /* Parentheses only */
    int (*simple_fp)(void);
    
    /* Brackets only */
    int array[10][20];
    
    /* Braces only (anonymous struct) */
    struct {
        int a;
        int b;
    };
    
    /* Parentheses + brackets */
    int (*array_of_fp[5])(void);
    
    /* Parentheses + braces */
    struct {
        int (*nested_fp)(void);
    } wrapper;
    
    /* Brackets + braces */
    struct {
        int data[10];
    } array_struct;
    
    /* All three combined */
    struct {
        int (*(*complex[2])(void))[3];
    } ultimate;
};

/* 12. Variable declarations using complex types */
extern struct Outer global_outer;
extern union MegaUnion *mega_ptr;
extern ComplexCallback callbacks[10];

/* 13. Inline function with complex parameters */
static inline void process_complex(
    int (*(*param1)[5])(char *),
    struct __attribute__((packed)) {
        short len;
        char data[];
    } *param2
) {
    /* Function body - triggers more parsing */
    (void)param1;
    (void)param2;
}

/* 14. Additional edge cases */
/* Empty parentheses (function with no parameters) */
void (*empty_params)(void);

/* Multiple attribute groups */
int (*multi_attr_fp)(int) __attribute__((const)) __attribute__((leaf));

/* Array of pointers to functions returning pointers to functions */
void (*(*func_ptr_array[3])(int))(char);

/* 15. Final complex nested type */
struct RecursiveLike {
    /* Self-referential through function pointer */
    void (*method)(struct RecursiveLike *self, 
                   int (*(*callback)(int[2]))[3],
                   union {
                       int code;
                       struct {
                           unsigned error:1;
                           unsigned warning:1;
                           unsigned info:6;
                       } flags;
                   } status);
    
    /* Multi-dimensional flexible array */
    int dynamic_matrix[][5][10];
};

#endif /* GENGYPE_TEST_H */
