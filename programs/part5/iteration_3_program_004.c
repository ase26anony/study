/* gengtype-test.c - Complex type definitions to exercise gengtype type enumeration */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this would be the actual GTY marker */
#define GTY(x)

/* Forward declarations */
struct forward_declared;
union forward_declared_union;

/* TYPE_SCALAR: Basic scalar types */
GTY(())
struct Scalars {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_int;
    short short_int;
    unsigned int unsigned_int;
    _Bool boolean;
    int8_t int8;
    int64_t int64;
};

/* TYPE_STRING: String types */
GTY(())
struct Strings {
    const char* constant_string;
    char* mutable_string;
    char fixed_string[32];
    wchar_t* wide_string;
};

/* TYPE_POINTER: Various pointer types */
GTY(())
struct Pointers {
    void* void_ptr;
    int* int_ptr;
    struct Scalars* struct_ptr;
    union forward_declared_union* union_ptr;
    int (*function_ptr)(int, char*);
    void (*void_func_ptr)(void);
    char** double_ptr;
    const volatile int* cv_ptr;
};

/* TYPE_ARRAY: Array types */
GTY(())
struct Arrays {
    int simple_array[10];
    char char_array[256];
    float multi_dim[5][10];
    struct Scalars* pointer_array[20];
    int flexible_array[];
};

/* TYPE_STRUCT: Nested structures */
GTY(())
struct NestedStruct {
    struct {
        int inner_a;
        char inner_b;
        struct {
            int deepest;
        } deepest_struct;
    } anonymous_inner;
    
    struct NamedInner {
        int named_value;
        struct NamedInner* self_ptr;
    } named_inner;
    
    struct Arrays array_member;
};

/* TYPE_UNION: Union types */
GTY(())
union ComplexUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
    struct {
        int union_struct_a;
        char union_struct_b;
    } union_struct;
    char as_array[8];
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
typedef GTY(()) struct UserDefined {
    int user_id;
    char* user_name;
    struct UserDefined* next;
    union ComplexUnion data;
} UserType;

/* TYPE_CALLBACK: Function pointer types (callback types) */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);
typedef char* (*StringProcessor)(const char*, int);

GTY(())
struct WithCallbacks {
    Comparator compare;
    CallbackFunc callback;
    StringProcessor processor;
    void (*array_of_callbacks[5])(void);
};

/* TYPE_LANG_STRUCT: Language-specific structure (simulated) */
/* In real GCC, this would be language-specific structures like tree_node, etc. */
GTY(())
struct LangSpecific {
    int lang_tag;
    void* lang_data;
    struct LangSpecific* (*lang_method)(struct LangSpecific*);
};

/* Complex nested type combining everything */
GTY(())
struct MasterType {
    /* TYPE_SCALAR */
    int master_id;
    
    /* TYPE_STRING */
    const char* master_name;
    
    /* TYPE_POINTER */
    struct MasterType* self_reference;
    struct NestedStruct* nested_ptr;
    union ComplexUnion* union_ptr;
    
    /* TYPE_ARRAY */
    UserType* user_array[50];
    int matrix[10][10];
    
    /* TYPE_STRUCT */
    struct Scalars scalars;
    struct NestedStruct nested;
    
    /* TYPE_UNION */
    union ComplexUnion data_union;
    
    /* TYPE_USER_STRUCT */
    UserType user_data;
    
    /* TYPE_CALLBACK */
    CallbackFunc master_callback;
    
    /* TYPE_LANG_STRUCT */
    struct LangSpecific* lang_struct;
    
    /* Complex nested anonymous struct */
    struct {
        int anonymous_id;
        struct {
            char deep_char;
            void* deep_ptr;
        } deeper;
    } anonymous;
    
    /* Pointer to function returning pointer to array */
    int (*(*complex_func_ptr)(void))[10];
    
    /* Array of function pointers */
    int (*func_ptr_array[5])(int, int);
};

/* Complete the forward declarations */
GTY(())
struct forward_declared {
    int value;
    struct forward_declared* next;
};

GTY(())
union forward_declared_union {
    int as_int;
    struct forward_declared* as_struct;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr, size_t size) {
    volatile size_t result = 0;
    unsigned char* bytes = (unsigned char*)ptr;
    
    /* Simple byte sum to ensure the data is touched */
    for (size_t i = 0; i < (size > 64 ? 64 : size); i++) {
        result += bytes[i];
    }
    
    return result;
}

/* Another external function to use callbacks */
__attribute__((noinline))
void invoke_callback(CallbackFunc func, int value, void* data) {
    if (func) {
        func(value, data);
    }
}

/* Callback implementations */
int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

char* sample_processor(const char* str, int len) {
    static char buffer[256];
    for (int i = 0; i < len && str[i]; i++) {
        buffer[i] = str[i] + 1;
    }
    return buffer;
}

int main(void) {
    /* Declare instances of all complex types */
    struct Scalars scalars_instance = {0};
    struct Strings strings_instance = {0};
    struct Pointers pointers_instance = {0};
    struct Arrays arrays_instance = {.flexible_array = {0}};
    struct NestedStruct nested_instance = {0};
    union ComplexUnion union_instance = {0};
    UserType user_instance = {0};
    struct WithCallbacks callbacks_instance = {0};
    struct LangSpecific lang_instance = {0};
    struct MasterType master_instance = {0};
    struct forward_declared forward_instance = {0};
    union forward_declared_union forward_union_instance = {0};
    
    /* Initialize callback pointers */
    callbacks_instance.compare = sample_comparator;
    callbacks_instance.callback = sample_callback;
    callbacks_instance.processor = sample_processor;
    
    /* Initialize some data */
    scalars_instance.integer = 42;
    scalars_instance.character = 'A';
    scalars_instance.floating = 3.14159f;
    
    strings_instance.constant_string = "Hello, gengtype!";
    strings_instance.mutable_string = (char*)"Mutable string";
    
    /* Create pointer relationships */
    pointers_instance.struct_ptr = &scalars_instance;
    pointers_instance.union_ptr = &union_instance;
    pointers_instance.function_ptr = NULL;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        arrays_instance.simple_array[i] = i * i;
    }
    
    /* Set up self-referential structures */
    user_instance.user_id = 1001;
    user_instance.next = &user_instance; /* Self-reference */
    
    master_instance.self_reference = &master_instance;
    master_instance.nested_ptr = &nested_instance;
    master_instance.master_callback = sample_callback;
    
    /* Compute sizeof for all types - forces compiler to consider them */
    size_t total_size = 0;
    
    total_size += sizeof(struct Scalars);
    total_size += sizeof(struct Strings);
    total_size += sizeof(struct Pointers);
    total_size += offsetof(struct Arrays, flexible_array); /* Don't include flexible array */
    total_size += sizeof(struct NestedStruct);
    total_size += sizeof(union ComplexUnion);
    total_size += sizeof(UserType);
    total_size += sizeof(struct WithCallbacks);
    total_size += sizeof(struct LangSpecific);
    total_size += sizeof(struct MasterType);
    total_size += sizeof(struct forward_declared);
    total_size += sizeof(union forward_declared_union);
    
    /* Take addresses of everything to prevent optimization */
    volatile void* addr_keepalive;
    
    addr_keepalive = &scalars_instance;
    addr_keepalive = &strings_instance;
    addr_keepalive = &pointers_instance;
    addr_keepalive = &arrays_instance;
    addr_keepalive = &nested_instance;
    addr_keepalive = &union_instance;
    addr_keepalive = &user_instance;
    addr_keepalive = &callbacks_instance;
    addr_keepalive = &lang_instance;
    addr_keepalive = &master_instance;
    addr_keepalive = &forward_instance;
    addr_keepalive = &forward_union_instance;
    
    /* Take addresses of members */
    addr_keepalive = &scalars_instance.integer;
    addr_keepalive = &strings_instance.constant_string;
    addr_keepalive = &pointers_instance.function_ptr;
    addr_keepalive = &arrays_instance.simple_array[0];
    addr_keepalive = &nested_instance.anonymous_inner;
    addr_keepalive = &union_instance.as_int;
    addr_keepalive = &user_instance.next;
    addr_keepalive = &callbacks_instance.callback;
    addr_keepalive = &lang_instance.lang_method;
    addr_keepalive = &master_instance.complex_func_ptr;
    
    /* Use callbacks to ensure they're not optimized away */
    int callback_data = 0;
    invoke_callback(sample_callback, 21, &callback_data);
    
    /* Use comparator */
    int a = 5, b = 10;
    int comparison = sample_comparator(&a, &b);
    
    /* Use string processor */
    char* processed = sample_processor("test", 4);
    
    /* Compute checksums to touch the data */
    size_t checksum = 0;
    checksum += compute_checksum(&scalars_instance, sizeof(scalars_instance));
    checksum += compute_checksum(&master_instance, sizeof(master_instance));
    
    /* Print results to prevent dead code elimination */
    printf("Total type size: %zu bytes\n", total_size);
    printf("Callback result: %d\n", callback_data);
    printf("Comparison: %d\n", comparison);
    printf("Checksum: %zu\n", checksum);
    printf("Processed string starts with: %c\n", processed[0]);
    
    return 0;
}
