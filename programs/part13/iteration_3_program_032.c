/* test_gengtype_coverage.h */
#ifndef TEST_GENGTYPE_COVERAGE_H
#define TEST_GENGTYPE_COVERAGE_H

/* 1. Complex nested type definitions with all bracket types */
struct OuterStruct {
    /* Function pointer array with nested parentheses */
    int (*func_array[5])(int, char);
    
    /* Nested struct with bit-fields (braces) */
    struct {
        unsigned int flag1:1;
        unsigned int flag2:3;
        unsigned int flag3:4;
    } bitfield_struct;
    
    /* Multi-dimensional array (brackets) */
    double matrix[3][4][5];
    
    /* Union containing anonymous struct */
    union {
        struct {
            int x;
            long y;
        } point;
        char data[16];
    } data_union;
};

/* 2. Function pointers with complex signatures */
typedef void (*SignalHandler)(int sig);
typedef int (*ComplexFunc)(int (*callback)(char[10]), void *context);

/* Pointer to function returning pointer to function */
typedef int (*(*FuncFactory)(int))(void);

/* 3. Multi-dimensional arrays and flexible array members */
struct ArrayContainer {
    int fixed[5][10];
    int flexible[];
};

struct NestedArrays {
    char *string_array[3][20];
    struct ArrayContainer *containers[2];
};

/* 4. Nested anonymous structs/unions with bit-fields */
struct AnonymousContainer {
    /* Anonymous union */
    union {
        struct {
            unsigned int a:8;
            unsigned int b:8;
            unsigned int c:8;
            unsigned int d:8;
        } bytes;
        unsigned int full;
    } union_data;
    
    /* Anonymous struct */
    struct {
        long id:24;
        long type:8;
        long value:32;
    };
};

/* 5. Macro expansions generating brackets */
#define PTR_FUNC(T) T (*)(T)
#define ARRAY_TYPE(T, N) T [N]
#define NESTED_PTR(T) T (*(*))(T)

/* Using the macros */
typedef PTR_FUNC(int) IntFuncPtr;
typedef ARRAY_TYPE(char, 100) CharArray100;
typedef NESTED_PTR(double) NestedDoubleFuncPtr;

struct MacroStruct {
    IntFuncPtr func_ptr;
    CharArray100 buffer;
    NestedDoubleFuncPtr nested_func;
};

/* 6. GCC attributes with parentheses */
struct __attribute__((aligned(16), packed)) AttributedStruct {
    int x __attribute__((aligned(8)));
    char y __attribute__((deprecated));
    long z;
} __attribute__((visibility("default")));

typedef __attribute__((const)) int (*ConstFuncPtr)(int, int)
    __attribute__((nonnull(1, 2)));

/* 7. Single complex declaration combining all bracket types */
struct UltimateType {
    /* Complex function pointer declaration */
    void (*signal_handler(int sig, 
                         void (*handler)(int, void*),
                         void *data))(int);
    
    /* Array of function pointers with nested parameters */
    int (*func_ptrs[3])(int (*)(char[10]), 
                       struct UltimateType *);
    
    /* Nested union with anonymous struct and bit-fields */
    union {
        struct {
            unsigned int version:4;
            unsigned int flags:12;
            unsigned int reserved:16;
        } header;
        unsigned int raw;
        struct {
            char *name;
            int value;
        } data;
    } metadata;
    
    /* Multi-dimensional flexible array member */
    int dynamic_matrix[][5][10];
    
    /* Pointer to array */
    int (*ptr_to_array)[10];
    
    /* Function returning pointer to array */
    int (*(*get_matrix)(int rows))[10];
} __attribute__((packed));

/* Additional edge cases */

/* Nested parentheses in function typedef */
typedef int (*(*(*TripleIndirect)(int))(int))(int);

/* Complex array of function pointers */
typedef void (*(*CallbackRegistry[10])(int event))(void *data);

/* Struct with all bracket types in members */
struct AllBrackets {
    int (*func)(int[5]);                    /* () and [] */
    struct { int x; } nested;               /* {} */
    int array[2][3];                        /* [][] */
    union { char c; int i; } value;         /* {} */
    void (*actions[3])(void);               /* () and [] */
};

/* Template-like macro for generic function pointers */
#define GENERIC_FUNC(name, ret, ...) \
    typedef ret (*name##_ptr_t)(__VA_ARGS__)

GENERIC_FUNC(ProcessInt, int, int *, size_t);
GENERIC_FUNC(ProcessData, void, const void *, size_t, void *);

/* Forward declarations that should be processed */
struct ForwardDecl;
typedef struct ForwardDecl *ForwardPtr;

/* Enum with last value for array size */
enum Constants {
    MAX_SIZE = 100,
    BUFFER_LEN = 256
};

/* Variable declarations using complex types */
extern struct UltimateType global_ultimate;
extern ConstFuncPtr global_const_func;
extern CallbackRegistry global_callbacks;

/* Inline function with attributes */
static inline __attribute__((always_inline)) 
int process_value(int x __attribute__((unused))) {
    return x * 2;
}

/* Final complex nested type */
typedef struct {
    struct {
        int (*compare)(const void *, const void *);
        void (*free)(void *);
    } ops;
    union {
        struct {
            int *data;
            size_t size;
        } array;
        struct {
            char *key;
            void *value;
        } map;
    } container;
} GenericContainer __attribute__((aligned(32)));

#endif /* TEST_GENGTYPE_COVERAGE_H */
