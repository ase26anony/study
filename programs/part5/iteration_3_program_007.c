/* gengtype-test.c - Complex type definitions to exercise gengtype type enumeration */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this would be the actual GTY marker */
#define GTY(x)

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* TYPE_SCALAR triggers */
typedef struct {
    int int_field;              /* TYPE_SCALAR */
    char char_field;            /* TYPE_SCALAR */
    float float_field;          /* TYPE_SCALAR */
    double double_field;        /* TYPE_SCALAR */
    long long_field;            /* TYPE_SCALAR */
    short short_field;          /* TYPE_SCALAR */
    unsigned uint_field;        /* TYPE_SCALAR */
    _Bool bool_field;           /* TYPE_SCALAR */
} GTY(()) ScalarStruct;

/* TYPE_STRING triggers */
typedef struct {
    const char* string1;        /* TYPE_STRING */
    char* string2;              /* TYPE_STRING */
    const char* const string3;  /* TYPE_STRING */
} GTY(()) StringStruct;

/* TYPE_STRUCT and nested structures */
typedef struct InnerStruct {
    int inner_data;
    struct InnerStruct* self_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
} GTY(()) InnerStruct;

/* TYPE_UNION */
typedef union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;           /* TYPE_POINTER */
} GTY(()) DataUnion;

/* TYPE_USER_STRUCT - complex user-defined structure */
typedef struct UserStruct {
    ScalarStruct scalars;       /* TYPE_USER_STRUCT */
    StringStruct strings;       /* TYPE_USER_STRUCT */
    InnerStruct inner;          /* TYPE_USER_STRUCT */
    DataUnion data;             /* TYPE_USER_STRUCT */
    
    /* TYPE_ARRAY with fixed size */
    int fixed_array[10];        /* TYPE_ARRAY */
    
    /* TYPE_ARRAY with pointer decay */
    char char_array[20];        /* TYPE_ARRAY */
    
    /* Multi-dimensional array */
    int matrix[5][5];           /* TYPE_ARRAY of TYPE_ARRAY */
    
    /* Flexible array member */
    int flexible_array[];       /* TYPE_ARRAY */
} GTY(()) UserStruct;

/* TYPE_POINTER variations */
typedef struct PointerStruct {
    /* Various pointer types */
    int* int_ptr;               /* TYPE_POINTER to TYPE_SCALAR */
    char** char_ptr_ptr;        /* TYPE_POINTER to TYPE_POINTER */
    void* void_ptr;             /* TYPE_POINTER */
    const void* const_ptr;      /* TYPE_POINTER */
    volatile int* volatile_ptr; /* TYPE_POINTER */
    
    /* Pointer to struct */
    UserStruct* user_ptr;       /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* Pointer to union */
    DataUnion* union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    
    /* Pointer to array */
    int (*array_ptr)[10];       /* TYPE_POINTER to TYPE_ARRAY */
    
    /* Self-referential pointer */
    struct PointerStruct* next; /* TYPE_POINTER to TYPE_STRUCT */
} GTY(()) PointerStruct;

/* TYPE_CALLBACK - function pointers */
typedef int (*CallbackFunc)(int, char*);  /* TYPE_CALLBACK */

typedef struct CallbackStruct {
    CallbackFunc func_ptr;      /* TYPE_CALLBACK */
    
    /* Multiple callback types */
    void (*void_callback)(void);
    int* (*ptr_ret_callback)(void);
    void (*complex_callback)(struct CallbackStruct*);
    
    /* Array of callbacks */
    CallbackFunc func_array[5]; /* TYPE_ARRAY of TYPE_CALLBACK */
} GTY(()) CallbackStruct;

/* TYPE_LANG_STRUCT - simulate language-specific structure */
typedef struct LangStruct {
    /* Language-specific metadata fields */
    void* lang_data;
    int lang_tag;
    
    /* Nested language structures */
    struct LangStruct* lang_next;
    
    /* Union with language-specific variants */
    union {
        int int_variant;
        void* ptr_variant;
        struct LangStruct* struct_variant;
    } lang_union;
} GTY(()) LangStruct;

/* Complex nested structure combining all types */
typedef struct MasterStruct {
    /* Direct members of various types */
    ScalarStruct scalars;
    StringStruct strings;
    UserStruct user;
    PointerStruct pointers;
    CallbackStruct callbacks;
    LangStruct lang;
    DataUnion union_data;
    
    /* Arrays of different types */
    ScalarStruct scalar_array[3];      /* TYPE_ARRAY of TYPE_USER_STRUCT */
    PointerStruct* pointer_array[5];   /* TYPE_ARRAY of TYPE_POINTER */
    CallbackFunc callback_array[4];    /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Pointer to incomplete type */
    struct ForwardDecl* fwd_ptr;       /* TYPE_POINTER to TYPE_UNDEFINED */
    union ForwardUnion* fwd_union_ptr; /* TYPE_POINTER to TYPE_UNDEFINED */
    
    /* Nested anonymous struct */
    struct {
        int anonymous_data;
        struct MasterStruct* parent;  /* TYPE_POINTER to TYPE_STRUCT */
    } nested;
    
    /* Bitfields (scalar but special) */
    unsigned bitfield1 : 4;
    unsigned bitfield2 : 8;
    unsigned bitfield3 : 1;
} GTY(()) MasterStruct;

/* Complete the forward declarations */
struct ForwardDecl {
    int data;
    MasterStruct* master;  /* TYPE_POINTER to TYPE_USER_STRUCT */
};

union ForwardUnion {
    int as_int;
    struct ForwardDecl* as_struct;  /* TYPE_POINTER to TYPE_STRUCT */
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void* ptr, size_t size) {
    /* Simple checksum computation */
    unsigned char* bytes = (unsigned char*)ptr;
    size_t sum = 0;
    for (size_t i = 0; i < size && i < 64; i++) {
        sum += bytes[i];
    }
    return sum;
}

/* Function using callbacks */
__attribute__((noinline))
int execute_callback(CallbackFunc func, int value, char* str) {
    if (func) {
        return func(value, str);
    }
    return -1;
}

int main(void) {
    /* Declare instances of all complex types */
    ScalarStruct scalar_instance = {0};
    StringStruct string_instance = {"Hello", "World", "Constant"};
    InnerStruct inner_instance = {42, &inner_instance};
    DataUnion union_instance = {.as_int = 100};
    UserStruct* user_ptr = NULL;
    PointerStruct pointer_instance = {0};
    CallbackStruct callback_instance = {0};
    LangStruct lang_instance = {0};
    MasterStruct master_instance = {0};
    struct ForwardDecl fwd_instance = {0};
    union ForwardUnion fwd_union_instance = {0};
    
    /* Take addresses of all instances */
    ScalarStruct* scalar_ptr = &scalar_instance;
    StringStruct* string_ptr = &string_instance;
    InnerStruct* inner_ptr = &inner_instance;
    DataUnion* union_ptr = &union_instance;
    UserStruct** user_ptr_ptr = &user_ptr;
    PointerStruct* pointer_ptr = &pointer_instance;
    CallbackStruct* callback_ptr = &callback_instance;
    LangStruct* lang_ptr = &lang_instance;
    MasterStruct* master_ptr = &master_instance;
    struct ForwardDecl* fwd_ptr = &fwd_instance;
    union ForwardUnion* fwd_union_ptr = &fwd_union_instance;
    
    /* Compute sizeof for all types */
    size_t sizes[] = {
        sizeof(ScalarStruct),
        sizeof(StringStruct),
        sizeof(InnerStruct),
        sizeof(DataUnion),
        sizeof(UserStruct),
        sizeof(PointerStruct),
        sizeof(CallbackStruct),
        sizeof(LangStruct),
        sizeof(MasterStruct),
        sizeof(struct ForwardDecl),
        sizeof(union ForwardUnion),
        sizeof(int*),
        sizeof(CallbackFunc),
        sizeof(int[10]),
        sizeof(char*[5])
    };
    
    /* Compute total size checksum */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Force computation with addresses to prevent optimization */
    volatile size_t checksum = 0;
    checksum += compute_checksum(scalar_ptr, sizeof(*scalar_ptr));
    checksum += compute_checksum(string_ptr, sizeof(*string_ptr));
    checksum += compute_checksum(inner_ptr, sizeof(*inner_ptr));
    checksum += compute_checksum(union_ptr, sizeof(*union_ptr));
    checksum += compute_checksum(pointer_ptr, sizeof(*pointer_ptr));
    checksum += compute_checksum(callback_ptr, sizeof(*callback_ptr));
    checksum += compute_checksum(lang_ptr, sizeof(*lang_ptr));
    checksum += compute_checksum(master_ptr, sizeof(*master_ptr));
    checksum += compute_checksum(fwd_ptr, sizeof(*fwd_ptr));
    checksum += compute_checksum(fwd_union_ptr, sizeof(*fwd_union_ptr));
    
    /* Access array elements */
    int test_array[5][5];
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            test_array[i][j] = i * j;
            checksum += test_array[i][j];
        }
    }
    
    /* Use function pointers */
    CallbackFunc test_callback = NULL;
    checksum += execute_callback(test_callback, 0, NULL);
    
    /* Print results to ensure no dead code elimination */
    printf("Total type size: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", checksum);
    printf("Addresses taken:\n");
    printf("  scalar_ptr: %p\n", (void*)scalar_ptr);
    printf("  string_ptr: %p\n", (void*)string_ptr);
    printf("  inner_ptr: %p\n", (void*)inner_ptr);
    printf("  master_ptr: %p\n", (void*)master_ptr);
    
    /* Create pointer cycles */
    pointer_instance.next = &pointer_instance;
    master_instance.nested.parent = &master_instance;
    fwd_instance.master = &master_instance;
    fwd_union_instance.as_struct = &fwd_instance;
    
    return 0;
}
