/* test_gengtype_coverage.c
 * 
 * A test program to exercise the type enumeration switch in gengtype.cc
 * This defines complex nested structures to trigger all TYPE_* cases
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this would be the actual GTY marker */
#define GTY(x)

/* Forward declarations to create pointer cycles */
struct ComplexStruct;
union NestedUnion;

/* TYPE_SCALAR: Basic scalar types */
GTY(())
struct Scalars {
    int integer;
    char character;
    float floating;
    double double_prec;
    long long_int;
    unsigned int unsigned_int;
    _Bool boolean;
};

/* TYPE_STRING: String types */
GTY(())
struct Strings {
    const char* const_string;
    char* mutable_string;
    char fixed_string[64];
};

/* TYPE_POINTER: Various pointer types */
GTY(())
struct Pointers {
    void* void_ptr;
    int* int_ptr;
    struct Scalars* struct_ptr;
    union NestedUnion* union_ptr;
    int (*func_ptr)(int, char*);  /* Function pointer */
    int* const const_ptr;
    volatile int* volatile_ptr;
};

/* TYPE_ARRAY: Array types */
GTY(())
struct Arrays {
    int simple_array[10];
    char multi_dim[5][5];
    struct Scalars* ptr_array[8];
    float flexible_array[];  /* Flexible array member */
};

/* TYPE_STRUCT: Nested structure */
GTY(())
struct NestedStruct {
    struct Scalars scalars;
    struct Strings strings;
    struct {
        int anonymous_int;
        double anonymous_double;
    } anonymous;
};

/* TYPE_UNION: Union types */
GTY(())
union BasicUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT: More complex user-defined structure */
GTY(())
struct UserStruct {
    struct NestedStruct nested;
    union BasicUnion union_field;
    struct Arrays* array_ptr;
    int tag;
};

/* TYPE_CALLBACK: Structure with function pointers */
GTY(())
struct Callbacks {
    int (*comparator)(const void*, const void*);
    void (*callback)(void* data, int result);
    char* (*string_processor)(char*);
    void (*cleanup)(void);
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
GTY(())
struct LangStruct {
    void* lang_data;
    int lang_tag;
    struct LangStruct* next;
};

/* Complex nested structure with all types */
GTY(())
struct ComplexStruct {
    /* TYPE_SCALAR */
    int id;
    double value;
    
    /* TYPE_STRING */
    const char* name;
    char buffer[256];
    
    /* TYPE_POINTER */
    struct ComplexStruct* self_ptr;
    struct UserStruct* user_struct_ptr;
    union NestedUnion* union_ptr;
    
    /* TYPE_ARRAY */
    int matrix[3][3];
    struct Callbacks* callback_array[4];
    
    /* TYPE_STRUCT */
    struct NestedStruct nested;
    
    /* TYPE_UNION */
    union {
        int int_view;
        float float_view;
        struct {
            char a, b, c, d;
        } char_view;
    } data_union;
    
    /* TYPE_CALLBACK */
    void (*handler)(struct ComplexStruct*);
    
    /* TYPE_LANG_STRUCT */
    struct LangStruct* lang_info;
    
    /* Flexible array member */
    int dynamic_data[];
};

/* Another union type */
GTY(())
union NestedUnion {
    struct Scalars as_struct;
    struct Pointers as_pointers;
    int as_int_array[10];
    struct {
        long header;
        char data[32];
    } packed;
};

/* Global variables to ensure types are used */
GTY(()) struct ComplexStruct* global_complex_ptr = NULL;
GTY(()) union NestedUnion global_union;
GTY(()) struct UserStruct global_user_struct;
GTY(()) struct Callbacks global_callbacks;

/* External function to prevent optimization */
__attribute__((noinline)) 
void use_types(void* ptr1, void* ptr2, void* ptr3) {
    /* Prevent dead code elimination */
    volatile int dummy = 0;
    if (ptr1) dummy++;
    if (ptr2) dummy++;
    if (ptr3) dummy++;
    printf("Dummy use: %d\n", dummy);
}

/* Another external function */
__attribute__((noinline))
size_t compute_total_size(void) {
    size_t total = 0;
    
    /* TYPE_SCALAR structures */
    total += sizeof(struct Scalars);
    
    /* TYPE_STRING structures */
    total += sizeof(struct Strings);
    
    /* TYPE_POINTER structures */
    total += sizeof(struct Pointers);
    
    /* TYPE_ARRAY structures */
    total += sizeof(struct Arrays);
    
    /* TYPE_STRUCT structures */
    total += sizeof(struct NestedStruct);
    
    /* TYPE_UNION structures */
    total += sizeof(union BasicUnion);
    
    /* TYPE_USER_STRUCT structures */
    total += sizeof(struct UserStruct);
    
    /* TYPE_CALLBACK structures */
    total += sizeof(struct Callbacks);
    
    /* TYPE_LANG_STRUCT structures */
    total += sizeof(struct LangStruct);
    
    /* Complex nested structure */
    total += sizeof(struct ComplexStruct);
    
    /* Another union */
    total += sizeof(union NestedUnion);
    
    return total;
}

int main(void) {
    /* Declare instances of all types */
    struct Scalars scalars_instance = {0};
    struct Strings strings_instance = {"Hello", "World", "Buffer"};
    struct Pointers pointers_instance = {0};
    struct Arrays* arrays_ptr = NULL;
    struct NestedStruct nested_instance = {0};
    union BasicUnion union_instance = {0};
    struct UserStruct user_struct_instance = {0};
    struct Callbacks callbacks_instance = {0};
    struct LangStruct lang_struct_instance = {0};
    struct ComplexStruct* complex_ptr = NULL;
    union NestedUnion nested_union_instance = {0};
    
    /* Take addresses to ensure types are referenced */
    volatile void* addresses[] = {
        &scalars_instance,
        &strings_instance,
        &pointers_instance,
        &arrays_ptr,
        &nested_instance,
        &union_instance,
        &user_struct_instance,
        &callbacks_instance,
        &lang_struct_instance,
        &complex_ptr,
        &nested_union_instance
    };
    
    /* Compute sizes of all types */
    size_t sizes[] = {
        sizeof(struct Scalars),
        sizeof(struct Strings),
        sizeof(struct Pointers),
        sizeof(struct Arrays),
        sizeof(struct NestedStruct),
        sizeof(union BasicUnion),
        sizeof(struct UserStruct),
        sizeof(struct Callbacks),
        sizeof(struct LangStruct),
        sizeof(struct ComplexStruct),
        sizeof(union NestedUnion),
        sizeof(int*),
        sizeof(char*),
        sizeof(void*),
        sizeof(int[10]),
        sizeof(float*),
        sizeof(struct ComplexStruct*)
    };
    
    /* Calculate total size (prevents optimization) */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Use the types through external function */
    use_types(&scalars_instance, &strings_instance, &nested_instance);
    use_types(&union_instance, &user_struct_instance, &callbacks_instance);
    
    /* Alternative size computation */
    size_t alt_total = compute_total_size();
    
    /* Print results to prevent optimization */
    printf("Total size: %zu\n", total_size);
    printf("Alternative total: %zu\n", alt_total);
    printf("Address count: %zu\n", sizeof(addresses)/sizeof(addresses[0]));
    
    /* Create some pointer cycles */
    struct ComplexStruct node1 = {0};
    struct ComplexStruct node2 = {0};
    node1.self_ptr = &node2;
    node2.self_ptr = &node1;
    
    /* Use the nodes */
    use_types(&node1, &node2, NULL);
    
    /* Array of different types */
    void* mixed_array[] = {
        &scalars_instance,
        &strings_instance,
        &pointers_instance,
        &nested_instance,
        &union_instance,
        &user_struct_instance
    };
    
    /* Final result */
    size_t final_result = total_size + alt_total + 
                         (size_t)addresses[0] + 
                         (size_t)mixed_array[0];
    
    printf("Final checksum: %zu\n", final_result % 1000);
    
    return (int)(final_result % 256);
}
