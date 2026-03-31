/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex Nested Type Definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*callbacks[5])(void);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flags:4;
        unsigned int mode:2;
        unsigned int :2; /* unnamed bit-field */
        unsigned long long extended:48;
    } __attribute__((packed)) status;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][3][2];
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func)(int))[10];
};

/* 2. Union with deeply nested constructs */
union MegaUnion {
    /* Function pointer with complex signature */
    void (*(*signal_handler)(int, void (*)(int)))(int);
    
    /* Anonymous struct with flexible array member */
    struct {
        int count;
        char data[];
    } flex_struct;
    
    /* Nested array of function pointers */
    int (*func_array[2][3])(char, double);
};

/* 3. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_DECL(T, N) T name[N]
#define NESTED_PTR(T) T (*(*))(T)

/* Use macros to create complex declarations */
PTR_FUNC(int) *simple_func_ptr;
ARRAY_DECL(PTR_FUNC(char), 5) func_ptr_array;

/* 4. Typedef with extremely complex nested types */
typedef struct {
    /* Pointer to function taking function pointer parameter */
    int (*comparator)(int (*)(int, int), int, int);
    
    /* Array of pointers to functions returning pointers to arrays */
    float (*(*compute[2])(int))[5];
    
    /* Nested anonymous union with bit-fields */
    union {
        struct {
            unsigned int a:1;
            unsigned int b:1;
            unsigned int c:14;
        } bits;
        unsigned short word;
    } control;
    
    /* Multi-level pointer with array */
    char *(*(*string_table)[10])(void);
} ComplexType;

/* 5. Struct combining all bracket types in single declaration */
struct UltimateStruct {
    /* The challenge declaration from requirements */
    void (*signal(int sig, void (*func)(int)))(int);
    
    /* Array of function pointers taking function pointers */
    int (*pfa[2])(int (*)(char[10]));
    
    /* Anonymous union with bit-field */
    union {
        int x;
        long y:8;
    } u;
    
    /* Multi-dimensional array */
    int arr[3][2];
    
    /* Pointer to array of function pointers */
    void (*(*func_ptr_array)[5])(int, ...);
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int data __attribute__((aligned(8)));
    void (*func_ptr)(void) __attribute__((deprecated));
} __attribute__((visibility("hidden")));

/* 7. Even more nesting */
struct Level1 {
    struct Level2 {
        struct Level3 {
            int (*(*level3_func)(struct Level3 *))[5];
            union {
                struct {
                    int a:3;
                    int b:5;
                } __attribute__((packed));
                char bytes[1];
            } nested_union;
        } matrix[2][2];
        
        /* Function with array parameter */
        void (*processor)(int buffer[10][20]);
    } *level2_ptr;
    
    /* Complex function pointer signature */
    int (*(*(*crazy_func)(int (*)(int[5])))[10])(void);
};

/* 8. Mix of storage classes and qualifiers */
extern const volatile struct {
    _Atomic int (*atomic_func)(_Atomic int*);
    restrict char * restrict ptr;
    int (*volatile_func)(volatile int*);
} special_vars;

/* 9. Template-like macro for generating nested types */
#define CREATE_NESTED(N) \
    struct Nested##N { \
        int value; \
        struct Nested##N *next; \
        int (*methods[N])(struct Nested##N*); \
    }

CREATE_NESTED(3);
CREATE_NESTED(5);

/* 10. Final complex declaration hitting all cases */
static inline __attribute__((always_inline)) 
int (*(*ultimate_example(
    struct UltimateStruct *s,
    int (*(*callback)(int[2][2]))[3]
))[4])(char (*)[5]) {
    /* Function body would go here in a .c file */
    return 0;
}

/* 11. Enum with complex expressions in initializers */
enum ComplexEnum {
    VALUE1 = sizeof(int[5]),
    VALUE2 = sizeof(struct OuterStruct),
    VALUE3 = (int)((void (*)(void))0)
};

/* 12. Variable declarations using our complex types */
extern ComplexType global_complex;
static struct UltimateStruct static_ultimate = {0};
volatile union MegaUnion *volatile volatile_union_ptr;

#endif /* TEST_GENGTYPE_COVERAGE_H */
