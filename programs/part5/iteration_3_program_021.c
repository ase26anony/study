/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types should
 * trigger all TYPE_* cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *ptr) {
    volatile int sink = 0;
    (void)ptr;
    (void)sink;
}

/* ========== TYPE_SCALAR definitions ========== */
struct ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
};

/* ========== TYPE_STRING definitions ========== */
struct StringTypes {
    const char *cstring;
    char *mutable_string;
    const char *const constant_string;
};

/* ========== TYPE_STRUCT definitions ========== */
struct InnerStruct {
    int x;
    double y;
};

struct OuterStruct {
    struct InnerStruct inner;
    int counter;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* In GCC context, user structs might be those with special attributes */
struct GTY(()) UserMarkedStruct {
    int data;
    void *GTY((skip)) skipped_ptr;  /* Marked to be skipped by GC */
};

/* ========== TYPE_UNION definitions ========== */
union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
    char as_bytes[8];
};

struct UnionContainer {
    int type_tag;
    union {
        int int_value;
        float float_value;
        struct InnerStruct struct_value;
    } data;
};

/* ========== TYPE_POINTER definitions ========== */
struct PointerFest {
    int *int_ptr;
    char **double_ptr;
    struct OuterStruct *struct_ptr;
    union DataUnion *union_ptr;
    
    /* Function pointer - might be TYPE_CALLBACK */
    int (*comparator)(const void *, const void *);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to pointer */
    void ***triple_ptr;
};

/* ========== TYPE_ARRAY definitions ========== */
struct ArrayTypes {
    /* Fixed size arrays */
    int fixed_array[100];
    char char_array[256];
    struct InnerStruct struct_array[10];
    
    /* Multi-dimensional arrays */
    int matrix[3][3];
    double cube[2][2][2];
    
    /* Array of pointers */
    void *ptr_array[20];
    
    /* Array of arrays */
    int nested_array[5][7];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*BinaryOp)(int, int);
typedef void (*Callback)(void *user_data, int result);
typedef const char *(*StringGenerator)(void);

struct CallbackContainer {
    BinaryOp arithmetic_op;
    Callback completion_cb;
    StringGenerator name_generator;
    
    /* Array of function pointers */
    void (*handlers[5])(void);
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* In GCC, lang_struct might be language-specific types */
struct GTY(()) LangSpecific {
    /* Simulating tree nodes or other GCC internal types */
    int node_code;
    void *lang_specific_data;
    struct LangSpecific *chain;
};

/* ========== Complex Nested Type ========== */
/* This should trigger multiple type kinds during traversal */
struct GTY(()) SuperComplexType {
    /* TYPE_SCALAR */
    int id;
    float priority;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_STRUCT (nested) */
    struct InnerStruct coordinates;
    
    /* TYPE_UNION */
    union DataUnion variant;
    
    /* TYPE_POINTER */
    struct SuperComplexType *next;
    struct ArrayTypes *array_data;
    
    /* TYPE_ARRAY */
    struct PointerFest *pointer_array[5];
    Callback callbacks[3];
    
    /* Flexible array member (GCC extension) */
    int flexible_array[];
};

/* ========== Even More Complex Nesting ========== */
struct GTY(()) ContainerOfEverything {
    struct ScalarTypes scalars;
    struct StringTypes strings;
    struct OuterStruct outer;
    struct UserMarkedStruct user_struct;
    struct UnionContainer union_container;
    struct PointerFest pointers;
    struct ArrayTypes arrays;
    struct CallbackContainer callbacks;
    struct LangSpecific lang_struct;
    struct SuperComplexType *complex;
};

/* ========== Function Definitions ========== */
int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
void dummy_callback(void *data, int result) { (void)data; (void)result; }
const char *get_name(void) { return "test"; }

/* ========== Main Function ========== */
int main(void) {
    /* Declare instances of all complex types */
    volatile struct ScalarTypes scalars = {0};
    volatile struct StringTypes strings = {0};
    volatile struct OuterStruct outer = {0};
    volatile struct UserMarkedStruct user_struct = {0};
    volatile union DataUnion data_union;
    volatile struct UnionContainer union_container = {0};
    volatile struct PointerFest pointers = {0};
    volatile struct ArrayTypes arrays = {0};
    volatile struct CallbackContainer callbacks = {0};
    volatile struct LangSpecific lang_struct = {0};
    volatile struct SuperComplexType *complex_ptr = NULL;
    volatile struct ContainerOfEverything container = {0};
    
    /* Initialize function pointers */
    callbacks.arithmetic_op = add;
    callbacks.completion_cb = dummy_callback;
    callbacks.name_generator = get_name;
    
    /* Take addresses of everything to ensure types are considered */
    void *addresses[] = {
        &scalars,
        &strings,
        &outer,
        &user_struct,
        &data_union,
        &union_container,
        &pointers,
        &arrays,
        &callbacks,
        &lang_struct,
        &complex_ptr,
        &container,
        &scalars.int_field,
        &outer.inner,
        &arrays.matrix[0][0],
        &pointers.comparator
    };
    
    /* Compute sizeof all types to force type analysis */
    size_t type_sizes[] = {
        sizeof(struct ScalarTypes),
        sizeof(struct StringTypes),
        sizeof(struct OuterStruct),
        sizeof(struct UserMarkedStruct),
        sizeof(union DataUnion),
        sizeof(struct UnionContainer),
        sizeof(struct PointerFest),
        sizeof(struct ArrayTypes),
        sizeof(struct CallbackContainer),
        sizeof(struct LangSpecific),
        sizeof(struct SuperComplexType),
        sizeof(struct ContainerOfEverything),
        sizeof(int*),
        sizeof(int(*)[10]),
        sizeof(int(*)(int, int))
    };
    
    /* Calculate a checksum from sizes and addresses to prevent optimization */
    size_t checksum = 0;
    
    /* Mix addresses and sizes in computation */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        checksum += (size_t)addresses[i];
    }
    
    for (size_t i = 0; i < sizeof(type_sizes)/sizeof(type_sizes[0]); i++) {
        checksum += type_sizes[i];
    }
    
    /* Use external function to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_pointer((void*)addresses[i]);
    }
    
    /* Print result to ensure no dead code elimination */
    printf("Type analysis checksum: %zu\n", checksum);
    printf("Number of type sizes computed: %zu\n", 
           sizeof(type_sizes)/sizeof(type_sizes[0]));
    
    /* Additional operations to ensure type usage */
    if (callbacks.arithmetic_op) {
        int result = callbacks.arithmetic_op(10, 20);
        printf("Callback result: %d\n", result);
    }
    
    /* Access array elements */
    arrays.matrix[0][0] = 42;
    arrays.char_array[0] = 'A';
    
    /* Chain pointer operations */
    pointers.triple_ptr = NULL;
    if (pointers.array_ptr) {
        (*pointers.array_ptr)[0] = 100;
    }
    
    /* Force usage of all types through KEEP_ALIVE */
    KEEP_ALIVE(&scalars);
    KEEP_ALIVE(&strings);
    KEEP_ALIVE(&outer);
    KEEP_ALIVE(&user_struct);
    KEEP_ALIVE(&data_union);
    KEEP_ALIVE(&union_container);
    KEEP_ALIVE(&pointers);
    KEEP_ALIVE(&arrays);
    KEEP_ALIVE(&callbacks);
    KEEP_ALIVE(&lang_struct);
    KEEP_ALIVE(&complex_ptr);
    KEEP_ALIVE(&container);
    
    return (int)(checksum % 256);
}
