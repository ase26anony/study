/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration logic in gengtype.cc (lines 182-213).
 * When processed by GCC's gengtype utility during build,
 * it should trigger all cases in the switch statement.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile int sink = 0;
    if (p) sink = 1;
}

/* ========== TYPE DEFINITIONS ========== */

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
} ScalarTypes;

/* TYPE_STRING: String type */
typedef struct GTY(()) StringType {
    const char * GTY((tag("0"))) string_field;
    char * GTY((tag("1"))) mutable_string;
} StringType;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct PointerTypes *self_ptr;
    void (*func_ptr)(void);
    int (*int_func_ptr)(int, int);
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    int fixed_array[10];
    char char_array[20];
    float multi_dim[3][4][5];
    struct ArrayTypes *ptr_array[5];
    int flexible_array[];
} ArrayTypes;

/* TYPE_STRUCT: Nested structures */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    double inner_value;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;
    InnerStruct *nested_ptr;
    struct {
        int anonymous_member;
        float anonymous_float;
    } anonymous;
    struct OuterStruct *next;
} OuterStruct;

/* TYPE_UNION: Union types */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    struct {
        int x, y;
    } as_struct;
} DataUnion;

typedef struct GTY(()) UnionContainer {
    DataUnion data;
    int type_tag;
} UnionContainer;

/* TYPE_USER_STRUCT: User-defined structure with callbacks */
typedef int (*comparator_fn)(const void *, const void *);

typedef struct GTY(()) UserDefined {
    const char *name;
    comparator_fn compare;
    void *user_data;
    struct UserDefined *next;
} UserDefined;

/* TYPE_CALLBACK: Function pointer types */
typedef struct GTY(()) CallbackContainer {
    void (*callback)(void *data);
    int (*filter)(const char *);
    void * GTY((skip)) user_data;  /* Skip for GC */
} CallbackContainer;

/* Complex nested type combining everything */
typedef struct GTY(()) SuperComplexType {
    /* Scalar fields */
    int id;
    double weight;
    
    /* String field */
    const char * GTY((tag("0"))) description;
    
    /* Pointer fields */
    struct SuperComplexType *parent;
    void **pointer_array;
    
    /* Array field */
    int scores[8];
    
    /* Nested struct */
    InnerStruct inner;
    
    /* Union field */
    DataUnion variant;
    
    /* Callback field */
    void (*notify)(struct SuperComplexType *);
    
    /* Flexible array member */
    UserDefined *users[];
} SuperComplexType;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef struct GTY(()) LangSpecific {
    void * GTY((length("(%h.length)"))) data;
    unsigned length;
    int lang_tag;
    struct LangSpecific *next;
} LangSpecific;

/* Another level of nesting */
typedef struct GTY(()) Container {
    ScalarTypes scalars;
    StringType strings;
    PointerTypes pointers;
    ArrayTypes arrays;
    OuterStruct structures;
    UnionContainer unions;
    UserDefined user_structs;
    CallbackContainer callbacks;
    SuperComplexType complex;
    LangSpecific lang_struct;
} Container;

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Declare instances of all types */
    volatile ScalarTypes scalars = {0};
    volatile StringType strings = {0};
    volatile PointerTypes pointers = {0};
    volatile ArrayTypes *arrays = NULL;
    volatile OuterStruct structures = {0};
    volatile UnionContainer unions = {0};
    volatile UserDefined user_structs = {0};
    volatile CallbackContainer callbacks = {0};
    volatile SuperComplexType *complex = NULL;
    volatile LangSpecific lang_struct = {0};
    volatile Container container = {0};
    
    /* Take addresses to ensure types are considered */
    void *addresses[] = {
        &scalars,
        &strings,
        &pointers,
        &arrays,
        &structures,
        &unions,
        &user_structs,
        &callbacks,
        &complex,
        &lang_struct,
        &container
    };
    
    /* Compute sizeof for all types */
    size_t sizes[] = {
        sizeof(ScalarTypes),
        sizeof(StringType),
        sizeof(PointerTypes),
        sizeof(ArrayTypes),
        sizeof(OuterStruct),
        sizeof(UnionContainer),
        sizeof(UserDefined),
        sizeof(CallbackContainer),
        sizeof(SuperComplexType),
        sizeof(LangSpecific),
        sizeof(Container),
        sizeof(InnerStruct),
        sizeof(DataUnion)
    };
    
    /* Use pointers to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_pointer((void*)addresses[i]);
    }
    
    /* Calculate checksum based on sizes */
    size_t checksum = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        checksum += sizes[i];
        checksum ^= (checksum << 13);
        checksum ^= (checksum >> 17);
        checksum ^= (checksum << 5);
    }
    
    /* Force compiler to keep everything alive */
    KEEP_ALIVE(scalars);
    KEEP_ALIVE(strings);
    KEEP_ALIVE(pointers);
    KEEP_ALIVE(arrays);
    KEEP_ALIVE(structures);
    KEEP_ALIVE(unions);
    KEEP_ALIVE(user_structs);
    KEEP_ALIVE(callbacks);
    KEEP_ALIVE(complex);
    KEEP_ALIVE(lang_struct);
    KEEP_ALIVE(container);
    
    printf("Type analysis checksum: %zu\n", checksum);
    printf("Number of types defined: %zu\n", 
           sizeof(sizes)/sizeof(sizes[0]));
    
    return 0;
}
