/* test_gengtype_coverage.c
 * 
 * This test defines complex data structures to exercise all type kinds
 * in gengtype.cc's switch statement (lines 182-213).
 * 
 * Compilation for gengtype testing:
 * 1. Place in GCC source tree where gengtype processes files
 * 2. Compile with: gcc -c -O0 -g -fdump-tree-all -ffat-lto-objects test_gengtype_coverage.c
 * 3. Or integrate into GCC build with GTY markers
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation outside GCC build */
#ifndef GTY
#define GTY(x) 
#endif

/* Function pointer type for TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);

/* Complex nested type definitions */

/* Basic struct with scalars (TYPE_SCALAR) */
struct GTY(()) ScalarStruct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;   /* TYPE_SCALAR */
    _Bool bool_field;      /* TYPE_SCALAR */
};

/* Struct with string (TYPE_STRING) */
struct GTY(()) StringStruct {
    const char* GTY((skip)) string_ptr;  /* TYPE_STRING */
    char fixed_string[64];               /* TYPE_ARRAY of TYPE_SCALAR */
};

/* Nested struct (TYPE_STRUCT) */
struct GTY(()) OuterStruct {
    struct ScalarStruct scalar_member;   /* TYPE_STRUCT */
    struct StringStruct* string_member;  /* TYPE_POINTER */
};

/* Union type (TYPE_UNION) */
union GTY(()) DataUnion {
    int as_int;
    float as_float;
    void* as_pointer;
    struct ScalarStruct as_struct;
};

/* User-defined struct (TYPE_USER_STRUCT) */
typedef struct GTY(()) {
    int id;
    char name[32];
} UserDefinedStruct;

/* Array-heavy struct (TYPE_ARRAY) */
struct GTY(()) ArrayStruct {
    int matrix[3][3];                    /* TYPE_ARRAY */
    struct ScalarStruct* ptr_array[10];  /* TYPE_ARRAY of TYPE_POINTER */
    int flexible_array[];                /* Flexible array member */
};

/* Struct with function pointer (TYPE_CALLBACK) */
struct GTY(()) CallbackStruct {
    callback_func handler;               /* TYPE_CALLBACK */
    void* GTY((skip)) user_data;
};

/* Language-specific struct placeholder (TYPE_LANG_STRUCT) */
/* This would normally be a GCC internal language structure */
struct GTY(()) LangStructPlaceholder {
    int lang_specific_field;
};

/* Master struct containing all types */
struct GTY(()) MasterType {
    /* TYPE_SCALAR fields */
    int scalar_int;
    char scalar_char;
    
    /* TYPE_POINTER fields */
    struct ScalarStruct* scalar_ptr;
    struct OuterStruct* outer_ptr;
    union DataUnion* union_ptr;
    UserDefinedStruct* user_struct_ptr;
    struct ArrayStruct* array_ptr;
    struct CallbackStruct* callback_ptr;
    struct LangStructPlaceholder* lang_struct_ptr;
    
    /* TYPE_STRUCT fields */
    struct ScalarStruct nested_scalar;
    struct OuterStruct nested_outer;
    
    /* TYPE_UNION field */
    union DataUnion data_union;
    
    /* TYPE_USER_STRUCT field */
    UserDefinedStruct user_struct;
    
    /* TYPE_ARRAY fields */
    int int_array[5];
    struct ScalarStruct struct_array[2];
    
    /* TYPE_CALLBACK field */
    callback_func direct_callback;
    
    /* TYPE_STRING field */
    const char* GTY((skip)) message;
    
    /* Nested anonymous struct/union */
    struct GTY(()) {
        int anonymous_field;
        union GTY(()) {
            int anon_union_int;
            float anon_union_float;
        } anonymous_union;
    } anonymous_struct;
    
    /* Pointer to array */
    int (*array_pointer)[10];
    
    /* Multi-dimensional pointer array */
    int* pointer_array[4][4];
    
    /* Self-referential pointer */
    struct MasterType* self_ptr;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t process_types(
    struct MasterType* master,
    struct ScalarStruct* scalar,
    struct StringStruct* string,
    struct OuterStruct* outer,
    union DataUnion* data_union,
    UserDefinedStruct* user_struct,
    struct ArrayStruct* array,
    struct CallbackStruct* callback,
    struct LangStructPlaceholder* lang_struct
) {
    /* Force compiler to consider all types */
    volatile size_t total_size = 0;
    
    /* Take addresses and sizes of all types */
    total_size += sizeof(*master);
    total_size += sizeof(*scalar);
    total_size += sizeof(*string);
    total_size += sizeof(*outer);
    total_size += sizeof(*data_union);
    total_size += sizeof(*user_struct);
    total_size += sizeof(*array);
    total_size += sizeof(*callback);
    total_size += sizeof(*lang_struct);
    
    /* Access members to ensure they're referenced */
    if (master) {
        total_size += master->scalar_int;
        total_size += (size_t)master->scalar_ptr;
        total_size += master->nested_scalar.int_field;
        total_size += master->data_union.as_int;
        total_size += master->user_struct.id;
        total_size += master->int_array[0];
        total_size += (size_t)master->direct_callback;
        total_size += (size_t)master->message;
        total_size += master->anonymous_struct.anonymous_field;
        total_size += (size_t)master->self_ptr;
    }
    
    return total_size;
}

/* Callback function implementation */
int sample_callback(int value, void* data) {
    return value * 2;
}

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarStruct scalar_instance = {
        .int_field = 42,
        .char_field = 'A',
        .float_field = 3.14f,
        .double_field = 2.71828,
        .bool_field = 1
    };
    
    struct StringStruct string_instance = {
        .string_ptr = "Hello, gengtype!",
        .fixed_string = "Fixed string content"
    };
    
    struct OuterStruct outer_instance = {
        .scalar_member = scalar_instance,
        .string_member = &string_instance
    };
    
    union DataUnion union_instance = {
        .as_int = 100
    };
    
    UserDefinedStruct user_struct_instance = {
        .id = 1,
        .name = "TestUserStruct"
    };
    
    /* Note: ArrayStruct with flexible array member needs special handling */
    struct ArrayStruct* array_ptr = NULL;
    
    struct CallbackStruct callback_instance = {
        .handler = sample_callback,
        .user_data = &scalar_instance
    };
    
    struct LangStructPlaceholder lang_struct_instance = {
        .lang_specific_field = 999
    };
    
    /* Master type instance */
    struct MasterType master_instance = {
        .scalar_int = 123,
        .scalar_char = 'X',
        .scalar_ptr = &scalar_instance,
        .outer_ptr = &outer_instance,
        .union_ptr = &union_instance,
        .user_struct_ptr = &user_struct_instance,
        .array_ptr = array_ptr,
        .callback_ptr = &callback_instance,
        .lang_struct_ptr = &lang_struct_instance,
        .nested_scalar = scalar_instance,
        .nested_outer = outer_instance,
        .data_union = union_instance,
        .user_struct = user_struct_instance,
        .int_array = {1, 2, 3, 4, 5},
        .direct_callback = sample_callback,
        .message = "Master type message",
        .anonymous_struct = {
            .anonymous_field = 777,
            .anonymous_union = {
                .anon_union_int = 888
            }
        },
        .self_ptr = &master_instance
    };
    
    /* Initialize struct_array */
    master_instance.struct_array[0] = scalar_instance;
    master_instance.struct_array[1] = scalar_instance;
    
    /* Calculate total size using all types */
    size_t total = process_types(
        &master_instance,
        &scalar_instance,
        &string_instance,
        &outer_instance,
        &union_instance,
        &user_struct_instance,
        array_ptr,
        &callback_instance,
        &lang_struct_instance
    );
    
    /* Additional operations to ensure types are referenced */
    volatile size_t additional_sizes = 0;
    
    /* sizeof operations on all types */
    additional_sizes += sizeof(struct ScalarStruct);
    additional_sizes += sizeof(struct StringStruct);
    additional_sizes += sizeof(struct OuterStruct);
    additional_sizes += sizeof(union DataUnion);
    additional_sizes += sizeof(UserDefinedStruct);
    additional_sizes += sizeof(struct ArrayStruct);
    additional_sizes += sizeof(struct CallbackStruct);
    additional_sizes += sizeof(struct LangStructPlaceholder);
    additional_sizes += sizeof(struct MasterType);
    
    /* Offsetof operations */
    additional_sizes += offsetof(struct MasterType, scalar_ptr);
    additional_sizes += offsetof(struct MasterType, nested_scalar);
    additional_sizes += offsetof(struct MasterType, data_union);
    additional_sizes += offsetof(struct MasterType, user_struct);
    additional_sizes += offsetof(struct MasterType, int_array);
    additional_sizes += offsetof(struct MasterType, direct_callback);
    additional_sizes += offsetof(struct MasterType, message);
    
    /* Pointer arithmetic */
    int* int_ptr = master_instance.int_array;
    additional_sizes += (size_t)(int_ptr + 1);
    
    /* Function pointer call */
    if (master_instance.direct_callback) {
        int result = master_instance.direct_callback(21, NULL);
        additional_sizes += result;
    }
    
    /* Print result to prevent optimization */
    printf("Type analysis coverage test:\n");
    printf("Total processed size: %zu\n", total);
    printf("Additional sizes: %zu\n", additional_sizes);
    printf("Combined: %zu\n", total + additional_sizes);
    
    return 0;
}
