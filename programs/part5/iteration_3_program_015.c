/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to trigger
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by gengtype during a GCC build, it should cause
 * the switch to be exercised for multiple enum typekind values.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) /* nothing */
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
__attribute__((noinline)) 
size_t external_checksum(const void* ptr, size_t size) {
    /* Simple checksum to ensure code isn't optimized away */
    const unsigned char* bytes = (const unsigned char*)ptr;
    size_t sum = 0;
    for (size_t i = 0; i < (size > 16 ? 16 : size); i++) {
        sum += bytes[i];
    }
    return sum;
}

/* ========== TYPE DEFINITIONS ========== */

/* Basic scalar types - TYPE_SCALAR */
typedef struct GTY(()) ScalarContainer {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    _Bool bool_field;       /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
} ScalarContainer;

/* String type - TYPE_STRING */
typedef struct GTY(()) StringContainer {
    const char* static_string;  /* TYPE_STRING */
    char* dynamic_string;       /* TYPE_POINTER (but content is string-like) */
} StringContainer;

/* Nested struct - TYPE_STRUCT */
typedef struct GTY(()) InnerStruct {
    int x;
    int y;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct inner;      /* TYPE_STRUCT */
    int extra;
} OuterStruct;

/* User struct - TYPE_USER_STRUCT */
struct GTY(()) ForwardDeclared;  /* Forward declaration */

typedef struct GTY(()) UserStructContainer {
    struct ForwardDeclared* fd_ptr;  /* TYPE_USER_STRUCT via pointer */
    void* opaque;
} UserStructContainer;

struct GTY(()) ForwardDeclared {
    int data;
    UserStructContainer* back_ref;  /* Circular reference */
};

/* Union - TYPE_UNION */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
} DataUnion;

/* Pointer types - TYPE_POINTER */
typedef struct GTY(()) PointerContainer {
    int* int_ptr;                   /* TYPE_POINTER */
    ScalarContainer* scalar_ptr;    /* TYPE_POINTER */
    void (*func_ptr)(void);         /* TYPE_POINTER (function pointer) */
    const volatile int* cv_ptr;     /* TYPE_POINTER with qualifiers */
} PointerContainer;

/* Array types - TYPE_ARRAY */
typedef struct GTY(()) ArrayContainer {
    int fixed_array[10];            /* TYPE_ARRAY (fixed size) */
    int multi_array[5][3];          /* TYPE_ARRAY (multi-dimensional) */
    char string_array[4][20];       /* TYPE_ARRAY of strings */
    struct GTY(()) {
        int len;
        int data[];
    } flexible_array;               /* TYPE_ARRAY (flexible array member) */
} ArrayContainer;

/* Callback type - TYPE_CALLBACK */
typedef int (*Comparator)(const void*, const void*);

typedef struct GTY(()) CallbackContainer {
    Comparator compare_func;        /* TYPE_CALLBACK */
    void (*callback)(int, void*);   /* TYPE_CALLBACK */
    struct GTY(()) {
        void (*nested_callback)(void);
    } nested;
} CallbackContainer;

/* Language struct - TYPE_LANG_STRUCT */
/* Simulating GCC's language-specific structures */
typedef struct GTY(()) LangStructBase {
    int lang_specific_tag;
} LangStructBase;

typedef struct GTY(()) CppStruct {
    LangStructBase base;
    void* cpp_vtable;  /* Simulating C++ vtable */
} CppStruct;

typedef struct GTY(()) JavaStruct {
    LangStructBase base;
    void* java_class_ref;
} JavaStruct;

/* Complex nested structure combining all types */
typedef struct GTY(()) MegaStruct {
    /* Scalar fields */
    int id;
    char category;
    
    /* String field */
    const char* name;
    
    /* Nested struct */
    InnerStruct position;
    
    /* Union field */
    DataUnion value;
    
    /* Pointer fields */
    MegaStruct* next;
    int* data_ptr;
    
    /* Array field */
    float coefficients[8];
    
    /* Callback field */
    void (*processor)(MegaStruct*);
    
    /* Language-specific extension */
    LangStructBase* lang_ext;
    
    /* Anonymous union */
    union {
        int as_int;
        struct {
            short high;
            short low;
        } as_shorts;
    } variant;
    
    /* Bitfields (scalar) */
    unsigned int flags : 4;
    unsigned int status : 2;
    
} MegaStruct;

/* ========== MAIN FUNCTION ========== */

int main() {
    size_t total_size = 0;
    size_t checksum = 0;
    
    /* Declare instances of all types */
    ScalarContainer scalars = {0};
    StringContainer strings = {0};
    OuterStruct nested_struct = {0};
    UserStructContainer user_struct = {0};
    DataUnion data_union = {0};
    PointerContainer pointers = {0};
    ArrayContainer arrays = {0};
    CallbackContainer callbacks = {0};
    CppStruct cpp_struct = {0};
    JavaStruct java_struct = {0};
    MegaStruct mega_struct = {0};
    
    /* Initialize forward-declared struct */
    struct ForwardDeclared fd_instance = {42, &user_struct};
    user_struct.fd_ptr = &fd_instance;
    
    /* Take addresses and compute sizes to force type analysis */
    total_size += sizeof(ScalarContainer);
    total_size += sizeof(StringContainer);
    total_size += sizeof(OuterStruct);
    total_size += sizeof(UserStructContainer);
    total_size += sizeof(DataUnion);
    total_size += sizeof(PointerContainer);
    total_size += sizeof(ArrayContainer);
    total_size += sizeof(CallbackContainer);
    total_size += sizeof(CppStruct);
    total_size += sizeof(JavaStruct);
    total_size += sizeof(MegaStruct);
    total_size += sizeof(struct ForwardDeclared);
    
    /* Take addresses to ensure types are referenced */
    KEEP_ALIVE(scalars);
    KEEP_ALIVE(strings);
    KEEP_ALIVE(nested_struct);
    KEEP_ALIVE(user_struct);
    KEEP_ALIVE(data_union);
    KEEP_ALIVE(pointers);
    KEEP_ALIVE(arrays);
    KEEP_ALIVE(callbacks);
    KEEP_ALIVE(cpp_struct);
    KEEP_ALIVE(java_struct);
    KEEP_ALIVE(mega_struct);
    KEEP_ALIVE(fd_instance);
    
    /* Compute checksums to prevent optimization */
    checksum += external_checksum(&scalars, sizeof(scalars));
    checksum += external_checksum(&strings, sizeof(strings));
    checksum += external_checksum(&nested_struct, sizeof(nested_struct));
    
    /* Create pointer chains and arrays */
    MegaStruct* mega_array[3];
    mega_array[0] = &mega_struct;
    mega_array[1] = NULL;
    mega_array[2] = (MegaStruct*)&arrays;
    
    /* Complex member access */
    mega_struct.next = &mega_struct;  /* Self-reference */
    mega_struct.data_ptr = &scalars.int_field;
    mega_struct.processor = NULL;
    mega_struct.lang_ext = (LangStructBase*)&cpp_struct;
    
    /* Array initialization */
    for (int i = 0; i < 10; i++) {
        arrays.fixed_array[i] = i;
    }
    
    /* Union usage */
    data_union.as_int = 0xDEADBEEF;
    mega_struct.variant.as_int = 0xCAFEBABE;
    
    /* More checksums */
    checksum += external_checksum(mega_array, sizeof(mega_array));
    checksum += external_checksum(&data_union, sizeof(data_union));
    
    /* Print results to ensure no dead code elimination */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", checksum);
    printf("Address of mega_struct: %p\n", (void*)&mega_struct);
    printf("Size of MegaStruct: %zu\n", sizeof(MegaStruct));
    
    /* Take address of specific members to ensure they're analyzed */
    printf("Offset of MegaStruct.name: %zu\n", offsetof(MegaStruct, name));
    printf("Offset of MegaStruct.position: %zu\n", offsetof(MegaStruct, position));
    printf("Offset of MegaStruct.value: %zu\n", offsetof(MegaStruct, value));
    printf("Offset of MegaStruct.next: %zu\n", offsetof(MegaStruct, next));
    printf("Offset of MegaStruct.coefficients: %zu\n", offsetof(MegaStruct, coefficients));
    printf("Offset of MegaStruct.processor: %zu\n", offsetof(MegaStruct, processor));
    
    return (int)(checksum % 256);
}
