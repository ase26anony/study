/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by GCC's gengtype utility during build, it should
 * trigger counts for all type kinds.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers for compilation - in real GCC these would
 * be actual GTY annotations that tell gengtype to process the types */
#define GTY(x)

/* Forward declarations to create pointer cycles */
struct ForwardDeclared;
union ForwardUnion;

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_int;
    unsigned int unsigned_int;
    _Bool boolean;
} ScalarTypes;

/* TYPE_STRING: String types */
typedef struct GTY(()) StringTypes {
    const char* constant_string;
    char* mutable_string;
    char fixed_string[64];
    wchar_t* wide_string;
} StringTypes;

/* TYPE_STRUCT: Regular structure */
typedef struct GTY(()) RegularStruct {
    int id;
    char name[32];
    struct RegularStruct* next;
} RegularStruct;

/* TYPE_USER_STRUCT: User-defined structure */
struct GTY(()) UserDefinedBase {
    int base_value;
    void (*base_func)(void);
};

typedef struct GTY(()) UserStruct {
    struct UserDefinedBase base;
    int extra_data;
    struct UserStruct* sibling;
} UserStruct;

/* TYPE_UNION: Union types */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
    char as_string[8];
} DataUnion;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    /* Simple pointers */
    int* int_ptr;
    char** char_ptr_ptr;
    
    /* Function pointers (TYPE_CALLBACK) */
    int (*func_ptr)(int, char*);
    void (*callback)(void*);
    
    /* Pointer to incomplete type */
    struct ForwardDeclared* forward_ptr;
    
    /* Pointer to union */
    union ForwardUnion* union_ptr;
    
    /* Self-referential pointer */
    struct PointerTypes* self;
    
    /* Pointer array */
    void* ptr_array[10];
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    /* Fixed-size arrays */
    int int_array[100];
    char char_array[256];
    
    /* Multi-dimensional arrays */
    double matrix[10][10];
    
    /* Array of pointers */
    struct RegularStruct* struct_ptr_array[50];
    
    /* Array of arrays */
    int nested_array[5][20];
    
    /* Flexible array member (C99) */
    long flexible_array[];
} ArrayTypes;

/* TYPE_CALLBACK: Function pointer types */
typedef struct GTY(()) CallbackTypes {
    /* Various function signatures */
    int (*compare)(const void*, const void*);
    void (*handler)(int, void*);
    char* (*string_processor)(char*);
    void (*simple_callback)(void);
    
    /* Array of function pointers */
    void (*callbacks[5])(int);
} CallbackTypes;

/* TYPE_LANG_STRUCT: Language-specific structure */
/* In GCC, these are structures with special language-dependent fields */
typedef struct GTY(()) LangStruct {
    /* Language-specific marker */
    enum { C_LANG, CPP_LANG, JAVA_LANG } language;
    
    /* Union for language-specific data */
    union {
        struct {
            int c_specific;
            void* c_pointer;
        } c_data;
        struct {
            const char* class_name;
            long long vtable_offset;
        } cpp_data;
    } lang_data;
    
    /* Generic data */
    void* user_data;
} LangStruct;

/* Complex nested structure to trigger multiple cases */
typedef struct GTY(()) ComplexNested {
    /* Scalar members */
    int id;
    float priority;
    
    /* String member */
    const char* description;
    
    /* Nested struct */
    struct {
        int x, y;
        union {
            int coord_id;
            float coord_value;
        } coord;
    } position;
    
    /* Union member */
    DataUnion data;
    
    /* Pointer to array */
    ArrayTypes* array_data;
    
    /* Array of structs */
    RegularStruct struct_array[5];
    
    /* Pointer to callback */
    CallbackTypes* callbacks;
    
    /* Self-referential pointer for cycles */
    struct ComplexNested* next;
    
    /* Pointer to language struct */
    LangStruct* lang_info;
    
    /* Flexible array of pointers */
    void* dynamic_items[];
} ComplexNested;

/* Forward declared types (now defined) */
struct GTY(()) ForwardDeclared {
    int magic_number;
    struct ForwardDeclared* next;
    ComplexNested* complex_ref;
};

union GTY(()) ForwardUnion {
    int tag;
    void* data;
    struct ForwardDeclared* struct_data;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr, size_t size) {
    /* Simple checksum to ensure code isn't optimized away */
    unsigned char* bytes = (unsigned char*)ptr;
    size_t sum = 0;
    for (size_t i = 0; i < size && i < 64; i++) {
        sum += bytes[i];
    }
    return sum;
}

/* Volatile global to prevent dead code elimination */
volatile size_t global_checksum = 0;

int main(void) {
    /* Declare instances of all complex types */
    ScalarTypes scalars = {0};
    StringTypes strings = {0};
    RegularStruct regular = {0};
    UserStruct user = {0};
    DataUnion data_union;
    PointerTypes pointers = {0};
    ArrayTypes* arrays = NULL;
    CallbackTypes callbacks = {0};
    LangStruct lang_struct = {0};
    ComplexNested* complex = NULL;
    struct ForwardDeclared forward = {0};
    union ForwardUnion forward_union;
    
    /* Take addresses and compute sizes to ensure types are referenced */
    size_t total_size = 0;
    
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringTypes);
    total_size += sizeof(RegularStruct);
    total_size += sizeof(UserStruct);
    total_size += sizeof(DataUnion);
    total_size += sizeof(PointerTypes);
    total_size += sizeof(ArrayTypes);
    total_size += sizeof(CallbackTypes);
    total_size += sizeof(LangStruct);
    total_size += sizeof(ComplexNested);
    total_size += sizeof(struct ForwardDeclared);
    total_size += sizeof(union ForwardUnion);
    
    /* Take addresses to create pointer operations */
    void* addresses[] = {
        &scalars,
        &strings,
        &regular,
        &user,
        &data_union,
        &pointers,
        arrays,
        &callbacks,
        &lang_struct,
        complex,
        &forward,
        &forward_union
    };
    
    /* Compute checksum of addresses to prevent optimization */
    for (int i = 0; i < (int)(sizeof(addresses)/sizeof(addresses[0])); i++) {
        if (addresses[i]) {
            global_checksum += compute_checksum(addresses[i], sizeof(void*));
        }
    }
    
    /* Array operations to trigger array type processing */
    int int_array[100] = {0};
    char* string_array[] = {"hello", "world", "gengtype", "test"};
    struct RegularStruct struct_array[10] = {0};
    
    total_size += sizeof(int_array);
    total_size += sizeof(string_array);
    total_size += sizeof(struct_array);
    
    /* Pointer arithmetic */
    int* int_ptr = int_array;
    int_ptr += 50;
    
    char** string_ptr = string_array;
    string_ptr++;
    
    /* Union field access */
    data_union.as_int = 42;
    data_union.as_float = 3.14159f;
    
    /* Nested structure access */
    regular.id = 1;
    regular.next = &regular;  /* Create cycle */
    
    /* Function pointer assignment */
    callbacks.simple_callback = NULL;
    
    /* Print results to prevent optimization */
    printf("Total type size: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", global_checksum);
    printf("Union as int: %d\n", data_union.as_int);
    printf("Regular struct id: %d\n", regular.id);
    
    return 0;
}
