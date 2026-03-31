/* test_gengtype_coverage.h - Complex type definitions to test gengtype parser */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(void (*callback)(int, char));
    
    /* Nested union with bit-fields */
    union {
        struct {
            unsigned int flag1:1;
            unsigned int flag2:3;
            unsigned int:4;  /* Unnamed bit-field */
            unsigned int value:8;
        } bits;
        long long combined;
    } nested_union __attribute__((aligned(8)));
    
    /* Multi-dimensional array */
    double matrix[3][4][2];
    
    /* Pointer to function returning pointer to array */
    char (*(*complex_func)(int))[10];
};

/* 2. Function pointer declarations with varied signatures */
typedef void (*SimpleFunc)(void);
typedef int (*FuncReturningFunc)(float (*)(double));
typedef char (*(*FuncWithArrayParam)(int arr[][5]))(void);

/* 3. Multi-dimensional arrays and flexible array members */
struct WithFlexArray {
    int count;
    int data[];  /* Flexible array member */
};

struct MultiDimContainer {
    int (*ptr_array[2][3])(int);
    struct WithFlexArray *flex_structs[4];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct AnonymousContainer {
    struct {
        int x;
        union {
            short s;
            char c[4];
        } u;
    } named;
    
    /* Anonymous struct */
    struct {
        unsigned int a:2;
        unsigned int b:6;
        unsigned int c:8;
    };
    
    /* Anonymous union */
    union {
        float f;
        int i;
    };
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_OF(T, N) T [N]
#define NESTED_PTR(T) T (*(*)(void))[]

typedef PTR_FUNC(int) int_func_ptr_t;
typedef ARRAY_OF(PTR_FUNC(char), 5) func_ptr_array_t;

struct MacroExpanded {
    NESTED_PTR(double) complex_ptr;
    func_ptr_array_t funcs;
    int_func_ptr_t int_funcs[3];
};

/* 6. Attribute syntax with parentheses */
struct __attribute__((packed, aligned(2))) PackedStruct {
    char a;
    int b __attribute__((aligned(4)));
    short c;
} __attribute__((deprecated));

int __attribute__((const)) pure_func(int x) __attribute__((warn_unused_result));

/* 7. Single declaration combining all bracket types */
struct UltimateTest {
    /* Complex function pointer declaration */
    void (*(*signal_handler[2])(int sig, void (*)(int)))(int);
    
    /* Nested array of function pointers */
    int (*(*pfa[3][2])(int (*)(char[10]), float))(double);
    
    /* Anonymous struct with bit-fields */
    struct {
        unsigned int mode:4;
        unsigned int:4;  /* Padding */
        unsigned int flags:8;
    } __attribute__((packed));
    
    /* Flexible array member of pointers */
    struct WithFlexArray *flex_array[];
    
    /* Multi-dimensional array with attribute */
    volatile int restricted_matrix[4][3] __attribute__((aligned(16)));
    
    /* Union containing anonymous struct */
    union {
        struct {
            long long big;
            char small[7];
        };
        double dbl_array[2];
    } data_union;
};

/* Additional complex typedefs */
typedef struct {
    int (*(*level1)(int (*level2)(char (*level3)(short))[5]))[10];
} RecursiveFuncPtr;

/* Template for generating more complex cases */
#define CREATE_COMPLEX_TYPE(name, T) \
    struct name##_struct { \
        T (*process)(T (*input)(T[2]), int); \
        T data[sizeof(T) > 4 ? 2 : 4]; \
    }

CREATE_COMPLEX_TYPE(IntProcessor, int);
CREATE_COMPLEX_TYPE(DoubleProcessor, double);

/* Final test: Everything combined */
typedef struct UltimateTest UltimateTest_t;
typedef UltimateTest_t *(*FactoryFunc)(int count, 
                                       UltimateTest_t (*prototype)(void),
                                       char options[][20]);

/* Global declarations using all constructs */
extern struct OuterStruct global_complex;
extern UltimateTest_t (*global_factory)(FactoryFunc, 
                                        int (*)(char (*)[10]));

#endif /* TEST_GENGTYPE_COVERAGE_H */
