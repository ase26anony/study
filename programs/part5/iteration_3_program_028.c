/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, it should trigger
 * all TYPE_* cases in the switch statement.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent dead code elimination */
#define KEEP_ALIVE(x) do { \
    volatile void* _ptr = (void*)&(x); \
    (void)_ptr; \
} while(0)

/* External function to prevent optimization */
extern void use_type_info(void* ptr, size_t size) __attribute__((noinline));

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    unsigned int uint_field;
} ScalarTypes;

/* TYPE_STRING: String types */
typedef struct GTY(()) StringTypes {
    const char* const_string;
    char* mutable_string;
    char fixed_string[64];
} StringTypes;

/* TYPE_STRUCT: Simple structure */
typedef struct GTY(()) SimpleStruct {
    int id;
    char name[32];
} SimpleStruct;

/* TYPE_UNION: Union type */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
    char as_string[8];
} DataUnion;

/* TYPE_USER_STRUCT: User-defined structure with GTY marker */
struct GTY(()) UserDefinedBase {
    int base_value;
    char base_name[16];
};

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    /* Pointers to different types */
    int* int_ptr;
    char** string_ptr_ptr;
    struct UserDefinedBase* user_struct_ptr;
    void* void_ptr;
    
    /* Function pointer (TYPE_CALLBACK) */
    int (*compare_func)(const void*, const void*);
    void (*callback_func)(int, char*);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer chain */
    void**** deep_ptr;
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    /* Fixed-size arrays */
    int int_array[50];
    char char_array[100];
    float float_array[20];
    
    /* Multi-dimensional arrays */
    double matrix[3][3];
    char cube[2][4][8];
    
    /* Array of pointers */
    void* ptr_array[16];
    
    /* Array of structures */
    SimpleStruct struct_array[5];
    
    /* Flexible array member (C99) */
    int flexible_array[];
} ArrayTypes;

/* TYPE_STRUCT with nested types */
typedef struct GTY(()) ComplexNested {
    /* Scalar fields */
    int id;
    
    /* String field */
    const char* description;
    
    /* Nested structure */
    struct GTY(()) InnerStruct {
        int inner_id;
        double inner_value;
        struct GTY(()) DeepInner {
            char deep_char;
            int deep_array[4];
        } deep;
    } inner;
    
    /* Union field */
    DataUnion data;
    
    /* Pointer to another instance (self-referential) */
    struct ComplexNested* next;
    
    /* Array field */
    int scores[10];
    
    /* Function pointer */
    void (*process)(struct ComplexNested*);
    
    /* Array of function pointers */
    int (*operations[5])(int, int);
} ComplexNested;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
/* In GCC, this would be marked with language-specific GTY options */
typedef struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) LangChain {
    struct LangChain* next;
    struct LangChain* prev;
    int lang_specific_data;
    void* lang_pointer;
} LangChain;

/* TYPE_CALLBACK: More function pointer types */
typedef struct GTY(()) CallbackContainer {
    /* Various callback signatures */
    size_t (*get_size)(void*);
    int (*filter)(const char*);
    void* (*allocator)(size_t);
    void (*deallocator)(void*);
    
    /* Nested callback */
    void (*nested_callback)(int (*)(void), void*);
} CallbackContainer;

/* Master structure containing all type kinds */
typedef struct GTY(()) TypeMaster {
    /* TYPE_SCALAR */
    ScalarTypes scalars;
    
    /* TYPE_STRING */
    StringTypes strings;
    
    /* TYPE_STRUCT */
    SimpleStruct simple;
    
    /* TYPE_USER_STRUCT */
    struct UserDefinedBase user_struct;
    
    /* TYPE_UNION */
    DataUnion union_data;
    
    /* TYPE_POINTER */
    PointerTypes pointers;
    
    /* TYPE_ARRAY */
    ArrayTypes arrays;
    
    /* TYPE_CALLBACK */
    CallbackContainer callbacks;
    
    /* TYPE_LANG_STRUCT */
    LangChain lang_chain;
    
    /* TYPE_STRUCT (nested complex) */
    ComplexNested nested;
    
    /* Self-referential pointer */
    struct TypeMaster* self;
    
    /* Array of various pointers */
    void* mixed_ptrs[8];
} TypeMaster;

/* External function definition to prevent optimization */
void use_type_info(void* ptr, size_t size) {
    /* This function is never actually called in this test,
     * but its existence prevents the compiler from optimizing
     * away the type information */
    (void)ptr;
    (void)size;
}

int main(void) {
    /* Declare instances of all complex types */
    ScalarTypes scalars = {0};
    StringTypes strings = {"constant", "mutable", "fixed"};
    SimpleStruct simple = {42, "test"};
    struct UserDefinedBase user_struct = {100, "user_base"};
    DataUnion union_data = {.as_int = 42};
    PointerTypes pointers = {0};
    ArrayTypes arrays = {0};
    CallbackContainer callbacks = {0};
    LangChain lang_chain = {0};
    ComplexNested nested = {0};
    TypeMaster master = {0};
    
    /* Initialize function pointers (dummy values) */
    pointers.compare_func = NULL;
    pointers.callback_func = NULL;
    nested.process = NULL;
    
    /* Take addresses of all instances */
    KEEP_ALIVE(scalars);
    KEEP_ALIVE(strings);
    KEEP_ALIVE(simple);
    KEEP_ALIVE(user_struct);
    KEEP_ALIVE(union_data);
    KEEP_ALIVE(pointers);
    KEEP_ALIVE(arrays);
    KEEP_ALIVE(callbacks);
    KEEP_ALIVE(lang_chain);
    KEEP_ALIVE(nested);
    KEEP_ALIVE(master);
    
    /* Compute sizeof for all types */
    size_t total_size = 0;
    
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringTypes);
    total_size += sizeof(SimpleStruct);
    total_size += sizeof(struct UserDefinedBase);
    total_size += sizeof(DataUnion);
    total_size += sizeof(PointerTypes);
    total_size += sizeof(ArrayTypes);
    total_size += sizeof(CallbackContainer);
    total_size += sizeof(LangChain);
    total_size += sizeof(ComplexNested);
    total_size += sizeof(TypeMaster);
    
    /* Take addresses of specific members to ensure they're referenced */
    total_size += sizeof(scalars.int_field);
    total_size += sizeof(strings.const_string);
    total_size += sizeof(simple.name);
    total_size += sizeof(user_struct.base_name);
    total_size += sizeof(union_data.as_string);
    total_size += sizeof(pointers.deep_ptr);
    total_size += sizeof(arrays.matrix);
    total_size += sizeof(callbacks.get_size);
    total_size += sizeof(lang_chain.next);
    total_size += sizeof(nested.operations);
    total_size += sizeof(master.self);
    
    /* Simulate external usage to prevent optimization */
    use_type_info(&master, total_size);
    
    /* Print checksum to ensure code isn't optimized away */
    printf("Type coverage test - Total size sum: %zu\n", total_size);
    printf("This program defines types that should exercise all cases in gengtype switch:\n");
    printf("- TYPE_SCALAR: %zu\n", sizeof(ScalarTypes));
    printf("- TYPE_STRING: %zu\n", sizeof(StringTypes));
    printf("- TYPE_STRUCT: %zu\n", sizeof(SimpleStruct));
    printf("- TYPE_USER_STRUCT: %zu\n", sizeof(struct UserDefinedBase));
    printf("- TYPE_UNION: %zu\n", sizeof(DataUnion));
    printf("- TYPE_POINTER: %zu\n", sizeof(PointerTypes));
    printf("- TYPE_ARRAY: %zu\n", sizeof(ArrayTypes));
    printf("- TYPE_CALLBACK: %zu\n", sizeof(CallbackContainer));
    printf("- TYPE_LANG_STRUCT: %zu\n", sizeof(LangChain));
    
    return 0;
}
