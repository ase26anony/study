/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during GCC build, it should trigger
 * all TYPE_* cases in the switch statement.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC build this would
 * be the actual garbage collector annotation */
#define GTY(x) 

/* Forward declarations to create complex type dependencies */
struct forward_declared_struct;
union forward_declared_union;

/* ========== TYPE_SCALAR triggers ========== */
GTY(())
struct scalar_types {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_value;
    short short_value;
    unsigned int unsigned_integer;
    _Bool boolean;
    int8_t int8;
    int64_t int64;
};

/* ========== TYPE_STRING triggers ========== */
GTY(())
struct string_types {
    const char *constant_string;
    char *mutable_string;
    const char *const constant_string_array[3];
};

/* ========== TYPE_POINTER triggers ========== */
/* Function pointer type for TYPE_CALLBACK */
typedef void (*callback_func)(int, void*);

GTY(())
struct pointer_types {
    /* Simple pointers */
    int *int_ptr;
    char **char_ptr_ptr;
    
    /* Pointer to struct */
    struct scalar_types *scalar_ptr;
    
    /* Pointer to union */
    union forward_declared_union *union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointers (TYPE_CALLBACK) */
    callback_func callback;
    void (*void_callback)(void);
    int (*int_callback)(double, float);
    
    /* Pointer to pointer chain */
    void ***triple_ptr;
    
    /* Self-referential pointer */
    struct pointer_types *next;
};

/* ========== TYPE_ARRAY triggers ========== */
GTY(())
struct array_types {
    /* Fixed-size arrays */
    int fixed_array[20];
    char char_array[50];
    float float_array[5][10];  /* Multi-dimensional */
    
    /* Array of pointers */
    void *pointer_array[15];
    
    /* Array of structs */
    struct scalar_types struct_array[8];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ========== TYPE_STRUCT triggers ========== */
/* Nested struct definitions */
GTY(())
struct inner_struct_a {
    int inner_data;
    char inner_char;
};

GTY(())
struct inner_struct_b {
    double inner_double;
    struct inner_struct_a *link_to_a;
};

GTY(())
struct nested_struct_types {
    /* Direct nesting */
    struct {
        int anonymous_member;
        float anonymous_float;
    } anonymous_struct;
    
    /* Named nested structs */
    struct inner_struct_a inner_a;
    struct inner_struct_b inner_b;
    
    /* Array of nested structs */
    struct inner_struct_a inner_array[5];
    
    /* Pointer to nested struct */
    struct inner_struct_b *inner_ptr;
};

/* ========== TYPE_UNION triggers ========== */
GTY(())
union simple_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

GTY(())
union complex_union {
    struct {
        int type_tag;
        union {
            int int_value;
            double double_value;
            char *string_value;
        } data;
    } tagged;
    
    unsigned char raw_bytes[16];
    
    struct scalar_types as_struct;
};

/* ========== TYPE_USER_STRUCT triggers ========== */
/* This would typically be user-defined structs with special handling */
typedef GTY(()) struct scalar_types user_scalar_t;
typedef GTY(()) struct nested_struct_types user_nested_t;

GTY(())
struct container_for_user_structs {
    user_scalar_t user_scalar;
    user_nested_t *user_nested_ptr;
};

/* ========== TYPE_LANG_STRUCT triggers ========== */
/* Language-specific struct - simulate with attributes */
struct GTY(()) lang_specific_struct {
    int lang_data;
    void *lang_pointer;
} __attribute__((aligned(16)));

/* ========== Complete the forward declarations ========== */
GTY(())
struct forward_declared_struct {
    int data;
    struct forward_declared_struct *next;
    union forward_declared_union *associated_union;
};

GTY(())
union forward_declared_union {
    int int_data;
    struct forward_declared_struct *struct_ptr;
    double double_data;
};

/* ========== Ultimate mega-struct that contains everything ========== */
GTY(())
struct master_type {
    /* Scalar types */
    struct scalar_types scalars;
    
    /* String types */
    struct string_types strings;
    
    /* Pointer types */
    struct pointer_types pointers;
    
    /* Array types */
    struct array_types *arrays;  /* Pointer to allow flexible array */
    
    /* Struct types */
    struct nested_struct_types nested;
    
    /* Union types */
    union simple_union simple_u;
    union complex_union complex_u;
    
    /* User struct types */
    struct container_for_user_structs user_structs;
    
    /* Language struct */
    struct lang_specific_struct lang_struct;
    
    /* Forward declared types */
    struct forward_declared_struct forward_struct;
    union forward_declared_union forward_union;
    
    /* Self-reference for graph complexity */
    struct master_type *self_ptr;
    
    /* Array of self-references */
    struct master_type *family[5];
    
    /* Callback function */
    callback_func master_callback;
};

/* ========== External function to prevent optimization ========== */
/* Use noinline attribute to ensure function isn't optimized away */
static void __attribute__((noinline)) 
use_all_types(const struct master_type *mt) {
    /* This function does nothing but reference all types */
    volatile int sink = 0;
    
    /* Reference scalars */
    sink += sizeof(struct scalar_types);
    
    /* Reference strings */
    sink += sizeof(struct string_types);
    
    /* Reference pointers */
    sink += sizeof(struct pointer_types);
    
    /* Reference arrays */
    sink += sizeof(struct array_types);
    
    /* Reference structs */
    sink += sizeof(struct nested_struct_types);
    
    /* Reference unions */
    sink += sizeof(union simple_union);
    sink += sizeof(union complex_union);
    
    /* Reference user structs */
    sink += sizeof(struct container_for_user_structs);
    
    /* Reference lang struct */
    sink += sizeof(struct lang_specific_struct);
    
    /* Reference forward declared */
    sink += sizeof(struct forward_declared_struct);
    sink += sizeof(union forward_declared_union);
    
    /* Reference master type */
    sink += sizeof(struct master_type);
    
    /* Prevent unused parameter warning */
    (void)mt;
    (void)sink;
}

/* ========== Main function to force type usage ========== */
int main(void) {
    /* Declare instances of all major types */
    struct scalar_types scalars = {0};
    struct string_types strings = {0};
    struct pointer_types pointers = {0};
    
    /* Allocate array struct with extra space for flexible array */
    struct array_types *arrays = (struct array_types*)
        malloc(sizeof(struct array_types) + 10 * sizeof(int));
    
    struct nested_struct_types nested = {0};
    union simple_union simple_u = {0};
    union complex_union complex_u = {0};
    struct container_for_user_structs user_structs = {0};
    struct lang_specific_struct lang_struct = {0};
    struct forward_declared_struct forward_struct = {0};
    union forward_declared_union forward_union = {0};
    
    /* Master type instance */
    struct master_type master = {0};
    
    /* Initialize pointers */
    master.arrays = arrays;
    master.self_ptr = &master;
    master.master_callback = NULL;
    
    /* Take addresses of everything to ensure they're used */
    volatile void *addresses[] = {
        &scalars,
        &strings,
        &pointers,
        arrays,
        &nested,
        &simple_u,
        &complex_u,
        &user_structs,
        &lang_struct,
        &forward_struct,
        &forward_union,
        &master
    };
    
    /* Calculate total size of all types */
    size_t total_size = 0;
    
    total_size += sizeof(struct scalar_types);
    total_size += sizeof(struct string_types);
    total_size += sizeof(struct pointer_types);
    total_size += sizeof(struct array_types) + 10 * sizeof(int);
    total_size += sizeof(struct nested_struct_types);
    total_size += sizeof(union simple_union);
    total_size += sizeof(union complex_union);
    total_size += sizeof(struct container_for_user_structs);
    total_size += sizeof(struct lang_specific_struct);
    total_size += sizeof(struct forward_declared_struct);
    total_size += sizeof(union forward_declared_union);
    total_size += sizeof(struct master_type);
    
    /* Call function that references all types */
    use_all_types(&master);
    
    /* Print something to prevent optimization */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Number of addressable items: %zu\n", 
           sizeof(addresses) / sizeof(addresses[0]));
    
    /* Cleanup */
    free(arrays);
    
    return 0;
}
