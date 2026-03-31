/* test_gengtype_switch.c - Complex type definitions to exercise gengtype switch cases */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers for compilation - in real GCC these are processed by gengtype */
#define GTY(x) 

/* Forward declarations */
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_SCALAR examples */
GTY(()) struct Scalars {
    int integer;
    char character;
    float floating;
    double double_precision;
    _Bool boolean;
    long long_int;
    unsigned long unsigned_long_int;
};

/* TYPE_STRING example */
GTY(()) struct WithStrings {
    const char *constant_string;
    char *mutable_string;
    const char *const constant_string_array[];
};

/* TYPE_STRUCT with nesting */
GTY(()) struct OuterStruct {
    int id;
    
    /* Nested struct - TYPE_STRUCT */
    struct {
        int x;
        int y;
    } point;
    
    /* Another struct type */
    struct InnerStruct {
        float data;
        struct InnerStruct *self_ptr;  /* TYPE_POINTER to same type */
    } inner;
    
    /* Reference to forward declared struct */
    struct forward_declared_struct *fwd_ref;
};

/* TYPE_UNION examples */
GTY(()) union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    
    struct {
        char type;
        char data[16];
    } as_struct;
};

/* TYPE_USER_STRUCT - a struct that will be marked as user-defined */
typedef GTY(()) struct UserDefined {
    int tag;
    union DataUnion data;
    struct UserDefined *next;  /* Linked list */
} UserDefinedStruct;

/* TYPE_ARRAY examples */
GTY(()) struct WithArrays {
    /* Fixed size array */
    int fixed_array[10];
    
    /* Multi-dimensional array */
    double matrix[3][3];
    
    /* Array of pointers */
    struct OuterStruct *ptr_array[5];
    
    /* Flexible array member (C99) */
    char flexible_array[];
};

/* TYPE_POINTER examples including function pointers */
typedef int (*comparison_func)(const void *, const void *);

GTY(()) struct WithPointers {
    /* Basic pointers */
    int *int_ptr;
    struct OuterStruct *struct_ptr;
    union DataUnion *union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer - TYPE_CALLBACK */
    comparison_func compare;
    
    /* Pointer to pointer */
    void **void_ptr_ptr;
    
    /* Const pointer */
    const char *const const_ptr;
};

/* TYPE_CALLBACK - Function pointer types */
GTY(()) struct CallbackContainer {
    /* Different function pointer signatures */
    void (*void_func)(void);
    int (*int_func)(int, int);
    char *(*string_func)(const char *);
    struct OuterStruct *(*struct_func)(int);
    
    /* Array of function pointers */
    comparison_func func_array[4];
};

/* TYPE_LANG_STRUCT - Simulating language-specific structure */
#ifdef __cplusplus
#define LANG_SPECIFIC
#else
#define LANG_SPECIFIC
#endif

GTY(()) struct LangSpecific {
    int base_type;
    /* This would normally have language-specific fields */
    void *lang_data;
};

/* Complete the forward declarations */
struct forward_declared_struct {
    int magic;
    struct forward_declared_union *link;
};

union forward_declared_union {
    int as_int;
    struct forward_declared_struct *as_struct;
};

/* Complex nested type combining everything */
GTY(()) struct MasterType {
    enum {
        KIND_SCALAR,
        KIND_STRUCT,
        KIND_UNION,
        KIND_ARRAY,
        KIND_POINTER
    } kind;
    
    union {
        struct Scalars scalars;
        struct OuterStruct outer;
        union DataUnion data_union;
        struct WithArrays arrays;
        struct WithPointers pointers;
    } value;
    
    /* Self-referential pointer */
    struct MasterType *self;
    
    /* Array of various types */
    void *generic_array[8];
    
    /* Union with bitfields */
    union {
        struct {
            unsigned int flag1 : 1;
            unsigned int flag2 : 2;
            unsigned int flag3 : 3;
        } bits;
        unsigned int all_flags;
    } flags;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void *ptr, size_t size) {
    volatile size_t result = 0;
    unsigned char *bytes = (unsigned char *)ptr;
    
    /* Simple byte sum to ensure the data is accessed */
    for (size_t i = 0; i < size && i < 64; i++) {
        result += bytes[i];
    }
    
    return result;
}

/* Another external function that takes various type pointers */
__attribute__((noinline))
void process_types(
    struct Scalars *s,
    struct WithStrings *ws,
    struct OuterStruct *os,
    union DataUnion *du,
    UserDefinedStruct *uds,
    struct WithArrays *wa,
    struct WithPointers *wp,
    struct CallbackContainer *cc,
    struct LangSpecific *ls,
    struct MasterType *mt
) {
    /* Take addresses and compute sizes to force type analysis */
    volatile size_t sizes[] = {
        sizeof(*s),
        sizeof(*ws),
        sizeof(*os),
        sizeof(*du),
        sizeof(*uds),
        sizeof(*wa),
        sizeof(*wp),
        sizeof(*cc),
        sizeof(*ls),
        sizeof(*mt),
        sizeof(struct forward_declared_struct),
        sizeof(union forward_declared_union)
    };
    
    /* Compute offsetof for various members */
    volatile ptrdiff_t offsets[] = {
        offsetof(struct OuterStruct, inner),
        offsetof(struct WithArrays, fixed_array),
        offsetof(struct WithPointers, compare),
        offsetof(struct MasterType, value)
    };
    
    /* Prevent these from being optimized away */
    (void)sizes;
    (void)offsets;
}

int main(void) {
    /* Declare instances of all complex types */
    struct Scalars scalars_instance = {
        .integer = 42,
        .character = 'A',
        .floating = 3.14f,
        .double_precision = 2.71828,
        .boolean = 1,
        .long_int = 1000L,
        .unsigned_long_int = 2000UL
    };
    
    struct WithStrings strings_instance = {
        .constant_string = "Hello, World!",
        .mutable_string = "Mutable String"
    };
    
    struct OuterStruct outer_instance = {
        .id = 1,
        .point = { .x = 10, .y = 20 },
        .inner = { .data = 1.5f, .self_ptr = NULL }
    };
    
    union DataUnion union_instance = {
        .as_int = 255
    };
    
    UserDefinedStruct user_struct_instance = {
        .tag = 1,
        .data = { .as_int = 100 },
        .next = NULL
    };
    
    struct WithArrays arrays_instance = {
        .fixed_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10},
        .matrix = {
            {1.0, 0.0, 0.0},
            {0.0, 1.0, 0.0},
            {0.0, 0.0, 1.0}
        }
    };
    
    struct WithPointers pointers_instance = {
        .int_ptr = &scalars_instance.integer,
        .struct_ptr = &outer_instance,
        .union_ptr = &union_instance,
        .compare = NULL,
        .const_ptr = "Constant String"
    };
    
    struct CallbackContainer callback_instance = {
        .void_func = NULL,
        .int_func = NULL,
        .string_func = NULL,
        .struct_func = NULL,
        .func_array = {NULL, NULL, NULL, NULL}
    };
    
    struct LangSpecific lang_instance = {
        .base_type = 99,
        .lang_data = NULL
    };
    
    struct MasterType master_instance = {
        .kind = KIND_STRUCT,
        .value = { .outer = outer_instance },
        .self = NULL,
        .flags = { .all_flags = 0x0F }
    };
    
    struct forward_declared_struct fwd_struct_instance = {
        .magic = 0xDEADBEEF,
        .link = NULL
    };
    
    union forward_declared_union fwd_union_instance = {
        .as_int = 0xCAFEBABE
    };
    
    /* Link the instances to create references */
    outer_instance.fwd_ref = &fwd_struct_instance;
    user_struct_instance.next = &user_struct_instance;  /* Self-reference */
    master_instance.self = &master_instance;
    pointers_instance.compare = (comparison_func)&compute_checksum;
    
    /* Process all types to ensure they're analyzed */
    process_types(
        &scalars_instance,
        &strings_instance,
        &outer_instance,
        &union_instance,
        &user_struct_instance,
        &arrays_instance,
        &pointers_instance,
        &callback_instance,
        &lang_instance,
        &master_instance
    );
    
    /* Compute and print a checksum using all types */
    size_t total_checksum = 0;
    
    total_checksum += compute_checksum(&scalars_instance, sizeof(scalars_instance));
    total_checksum += compute_checksum(&strings_instance, sizeof(strings_instance));
    total_checksum += compute_checksum(&outer_instance, sizeof(outer_instance));
    total_checksum += compute_checksum(&union_instance, sizeof(union_instance));
    total_checksum += compute_checksum(&user_struct_instance, sizeof(user_struct_instance));
    total_checksum += compute_checksum(&arrays_instance, sizeof(struct WithArrays));
    total_checksum += compute_checksum(&pointers_instance, sizeof(pointers_instance));
    total_checksum += compute_checksum(&callback_instance, sizeof(callback_instance));
    total_checksum += compute_checksum(&lang_instance, sizeof(lang_instance));
    total_checksum += compute_checksum(&master_instance, sizeof(master_instance));
    total_checksum += compute_checksum(&fwd_struct_instance, sizeof(fwd_struct_instance));
    total_checksum += compute_checksum(&fwd_union_instance, sizeof(fwd_union_instance));
    
    /* Also compute checksums of specific members */
    total_checksum += compute_checksum(arrays_instance.fixed_array, sizeof(arrays_instance.fixed_array));
    total_checksum += compute_checksum(arrays_instance.matrix, sizeof(arrays_instance.matrix));
    
    /* Print result to prevent optimization */
    printf("Type analysis checksum: %zu\n", total_checksum);
    printf("Sizes of types:\n");
    printf("  Scalars: %zu\n", sizeof(scalars_instance));
    printf("  WithStrings: %zu\n", sizeof(strings_instance));
    printf("  OuterStruct: %zu\n", sizeof(outer_instance));
    printf("  DataUnion: %zu\n", sizeof(union_instance));
    printf("  UserDefinedStruct: %zu\n", sizeof(user_struct_instance));
    printf("  WithArrays: %zu\n", sizeof(struct WithArrays));
    printf("  WithPointers: %zu\n", sizeof(pointers_instance));
    printf("  CallbackContainer: %zu\n", sizeof(callback_instance));
    printf("  LangSpecific: %zu\n", sizeof(lang_instance));
    printf("  MasterType: %zu\n", sizeof(master_instance));
    
    return 0;
}
