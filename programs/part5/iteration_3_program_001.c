/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by GCC's gengtype utility during a build,
 * it should trigger counts for:
 * - TYPE_SCALAR, TYPE_STRING, TYPE_STRUCT, TYPE_USER_STRUCT
 * - TYPE_UNION, TYPE_POINTER, TYPE_ARRAY, TYPE_CALLBACK
 * - TYPE_LANG_STRUCT, and potentially TYPE_UNDEFINED
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing our test structures */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to ensure types are referenced */
__attribute__((noinline)) 
void reference_types(void* ptr) {
    /* Do nothing but prevent optimization */
    (void)ptr;
}

/* ========== TYPE DEFINITIONS ========== */

/* Basic scalar types - will trigger TYPE_SCALAR */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
} ScalarTypes;

/* String type - will trigger TYPE_STRING */
typedef struct GTY(()) StringType {
    const char* string_field;      /* TYPE_STRING */
    char* mutable_string;          /* TYPE_STRING */
    const char* const const_string; /* TYPE_STRING */
} StringType;

/* Nested struct - will trigger TYPE_STRUCT */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    float inner_float;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;           /* TYPE_STRUCT */
    int outer_data;
} OuterStruct;

/* User struct - will trigger TYPE_USER_STRUCT */
struct GTY(()) UserDefined {
    int user_id;
    char user_name[32];
};

typedef struct UserDefined UserDefined;

/* Union type - will trigger TYPE_UNION */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
} DataUnion;

/* Complex union with struct members */
typedef union GTY(()) ComplexUnion {
    struct GTY(()) {
        int type;
        char data[16];
    } structured;
    DataUnion simple;
    long long big_value;
} ComplexUnion;

/* Pointer types - will trigger TYPE_POINTER */
typedef struct GTY(()) PointerStruct {
    int* int_ptr;                  /* TYPE_POINTER to scalar */
    ScalarTypes* struct_ptr;       /* TYPE_POINTER to struct */
    void* void_ptr;                /* TYPE_POINTER */
    const char* const* ptr_to_string_ptr; /* Nested pointer */
    struct PointerStruct* self_ptr; /* Self-referential pointer */
} PointerStruct;

/* Array types - will trigger TYPE_ARRAY */
typedef struct GTY(()) ArrayStruct {
    int fixed_array[10];           /* Fixed-size array */
    char multi_dim[5][5];          /* Multi-dimensional array */
    float* dynamic_array;          /* Pointer to array */
    int flexible_array[];          /* Flexible array member */
} ArrayStruct;

/* Callback/function pointer - will trigger TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

typedef struct GTY(()) CallbackStruct {
    callback_func handler;         /* TYPE_CALLBACK */
    simple_callback init;          /* TYPE_CALLBACK */
    void (*complex_callback)(struct CallbackStruct*, int); /* TYPE_CALLBACK */
} CallbackStruct;

/* Language-specific struct - will trigger TYPE_LANG_STRUCT */
/* Simulating GCC's internal lang-specific types */
typedef struct GTY(()) LangSpecificBase {
    int lang_magic;
    void* lang_data;
} LangSpecificBase;

/* Nested type simulating GCC tree nodes */
typedef struct GTY(()) TreeLike {
    int code;
    union GTY(()) {
        long int_cst;
        double real_cst;
        struct GTY(()) {
            const char* ptr;
            int length;
        } string_cst;
    } u;
    struct TreeLike* chain;
} TreeLike;

/* ========== COMPLEX NESTED STRUCTURE ========== */

/* Master structure containing all types */
typedef struct GTY(()) MasterContainer {
    /* Scalar fields */
    ScalarTypes scalars;
    
    /* String fields */
    StringType strings;
    
    /* Struct fields */
    OuterStruct nested_struct;
    
    /* User struct */
    UserDefined* user_struct_ptr;
    
    /* Union fields */
    DataUnion simple_union;
    ComplexUnion complex_union;
    
    /* Pointer fields */
    PointerStruct pointers;
    
    /* Array fields */
    ArrayStruct arrays;
    
    /* Callback fields */
    CallbackStruct callbacks;
    
    /* Language-specific fields */
    LangSpecificBase lang_struct;
    TreeLike* tree_node;
    
    /* Self-reference */
    struct MasterContainer* next;
    
    /* Mixed array of different types */
    void* mixed_array[5];
    
    /* Function pointer array */
    callback_func callback_array[3];
    
    /* Nested anonymous struct */
    struct GTY(()) {
        int anonymous_id;
        union GTY(()) {
            int anon_int;
            float anon_float;
        } anon_union;
    } anonymous;
} MasterContainer;

/* ========== FUNCTION DEFINITIONS ========== */

/* Callback function implementations */
int sample_callback(int value, void* context) {
    return value * 2;
}

void init_function(void) {
    /* Do nothing */
}

void complex_handler(CallbackStruct* cs, int value) {
    if (cs && cs->handler) {
        cs->handler(value, cs);
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    size_t total_size = 0;
    
    /* Declare instances of all types */
    ScalarTypes scalars = {0};
    StringType strings = {0};
    OuterStruct outer = {0};
    UserDefined user_struct = {0};
    DataUnion data_union = {0};
    ComplexUnion complex_union = {0};
    PointerStruct pointers = {0};
    ArrayStruct arrays = {0};
    CallbackStruct callbacks = {0};
    LangSpecificBase lang_struct = {0};
    TreeLike tree_node = {0};
    MasterContainer master = {0};
    
    /* Initialize string pointers */
    strings.string_field = "Hello, World!";
    strings.mutable_string = "Mutable";
    
    /* Initialize pointers */
    pointers.int_ptr = &scalars.int_field;
    pointers.struct_ptr = &scalars;
    pointers.self_ptr = &pointers;
    
    /* Initialize callbacks */
    callbacks.handler = sample_callback;
    callbacks.init = init_function;
    callbacks.complex_callback = complex_handler;
    
    /* Initialize tree node */
    tree_node.code = 42;
    tree_node.u.string_cst.ptr = "Tree String";
    tree_node.u.string_cst.length = 11;
    
    /* Initialize master container */
    master.scalars = scalars;
    master.strings = strings;
    master.nested_struct = outer;
    master.user_struct_ptr = &user_struct;
    master.simple_union = data_union;
    master.complex_union = complex_union;
    master.pointers = pointers;
    master.arrays = arrays;
    master.callbacks = callbacks;
    master.lang_struct = lang_struct;
    master.tree_node = &tree_node;
    master.next = &master;
    
    /* Take addresses of all instances to ensure they're used */
    void* addresses[] = {
        &scalars, &strings, &outer, &user_struct,
        &data_union, &complex_union, &pointers,
        &arrays, &callbacks, &lang_struct,
        &tree_node, &master
    };
    
    /* Calculate total size of all types */
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringType);
    total_size += sizeof(OuterStruct);
    total_size += sizeof(UserDefined);
    total_size += sizeof(DataUnion);
    total_size += sizeof(ComplexUnion);
    total_size += sizeof(PointerStruct);
    total_size += sizeof(ArrayStruct);
    total_size += sizeof(CallbackStruct);
    total_size += sizeof(LangSpecificBase);
    total_size += sizeof(TreeLike);
    total_size += sizeof(MasterContainer);
    
    /* Take address of structure members */
    reference_types(&scalars.int_field);
    reference_types(&strings.string_field);
    reference_types(&outer.nested.inner_data);
    reference_types(&user_struct.user_name);
    reference_types(&data_union.as_int);
    reference_types(&complex_union.structured.type);
    reference_types(&pointers.int_ptr);
    reference_types(&arrays.fixed_array);
    reference_types(&callbacks.handler);
    reference_types(&lang_struct.lang_magic);
    reference_types(&tree_node.u.string_cst.ptr);
    reference_types(&master.next);
    
    /* Use KEEP_ALIVE to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        KEEP_ALIVE(addresses[i]);
    }
    
    /* Print checksum to ensure code isn't optimized away */
    printf("Type coverage test - Total size: %zu bytes\n", total_size);
    printf("Address checksum: %p\n", (void*)((uintptr_t)addresses[0] + total_size));
    
    /* Execute a callback to ensure function pointers are used */
    if (callbacks.handler) {
        int result = callbacks.handler(21, &callbacks);
        printf("Callback result: %d\n", result);
    }
    
    return 0;
}
