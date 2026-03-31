/* test-gtype-coverage.c - Comprehensive type coverage for gengtype testing */
/* This file should be placed in gcc/ directory and processed during GCC build */

#include "config.h"
#include "system.h"
#include "coretypes.h"

/* TYPE_UNDEFINED: Forward declaration without definition */
struct opaque_forward_decl GTY(());
typedef struct opaque_forward_decl *opaque_ptr;

/* TYPE_SCALAR: Fundamental scalar types */
typedef int scalar_int GTY(());
typedef char scalar_char GTY(());
typedef long scalar_long GTY(());
typedef _Bool scalar_bool GTY(());
typedef unsigned int scalar_uint GTY(());

/* Enum type (also scalar) */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_enum GTY(());

/* TYPE_STRING: String types */
const char *global_string GTY(()) = "Hello, gengtype!";
char string_array[] GTY(()) = "Test string array";

/* TYPE_POINTER: Various pointer types */
typedef int *int_ptr GTY(());
typedef void *void_ptr GTY(());
typedef const char *const_string_ptr GTY(());

/* TYPE_ARRAY: Array types */
int fixed_array[10] GTY(());
extern int incomplete_array[] GTY(());
typedef int array_of_ints[5] GTY(());

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_fn)(const void *, const void *) GTY((callback));
typedef void (*simple_callback)(void) GTY((callback));

/* TYPE_STRUCT: Regular struct types */
struct simple_struct GTY(()) {
    int field1;
    char field2;
    long field3;
};

struct nested_struct GTY(()) {
    struct simple_struct simple GTY(());
    int_ptr ptr_field GTY(());
    color_enum color_field GTY(());
};

/* Recursive struct with pointer chain */
struct recursive_struct GTY((chain_next("%h.next"))) {
    int data GTY(());
    struct recursive_struct *next GTY(());
    struct recursive_struct *prev GTY((skip));
};

/* Struct with array field */
struct struct_with_array GTY(()) {
    int id GTY(());
    char name[32] GTY(());
    int scores[10] GTY(());
};

/* TYPE_UNION: Union types */
union simple_union GTY(()) {
    int int_val;
    float float_val;
    void *ptr_val;
    char char_val;
};

union complex_union GTY(()) {
    struct simple_struct struct_val GTY(());
    union simple_union union_val GTY(());
    int array_val[4] GTY(());
};

/* TYPE_USER_STRUCT: User-defined struct types with special handling */
/* Use typedef to create user struct type */
typedef struct simple_struct user_struct_type GTY(());

/* Struct with function pointer */
struct struct_with_callback GTY(()) {
    int id GTY(());
    comparator_fn compare GTY((callback));
    simple_callback notify GTY((callback));
};

/* Complex nested type combining multiple type kinds */
struct master_container GTY(()) {
    /* Scalar */
    int counter GTY(());
    
    /* String */
    const char *name GTY(());
    
    /* Pointer */
    struct nested_struct *nested_ptr GTY(());
    
    /* Array */
    struct simple_struct struct_array[5] GTY(());
    
    /* Union */
    union complex_union data_union GTY(());
    
    /* Callback */
    comparator_fn sorter GTY((callback));
    
    /* Pointer to array */
    int (*matrix_ptr)[10] GTY(());
    
    /* Nested anonymous struct */
    struct {
        int x GTY(());
        int y GTY(());
    } point GTY(());
};

/* TYPE_LANG_STRUCT: GCC internal language-specific structures */
/* These typically use GCC-specific attributes or extensions */

/* Vector type (SIMD) - often treated as lang_struct */
typedef int v4si __attribute__((vector_size(16))) GTY(());

/* Struct with vector type */
struct vector_struct GTY(()) {
    v4si data GTY(());
    int size GTY(());
};

/* Tree-like structure mimicking GCC internals */
struct tree_common GTY(()) {
    int code GTY(());
    union tree_node *chain GTY(());
    union tree_node *type GTY(());
};

/* Forward declaration for tree node union */
union tree_node GTY(());

/* Complete the tree node union definition */
union tree_node GTY(()) {
    struct tree_common common GTY(());
    /* In real GCC, there would be many more variants here */
    struct {
        struct tree_common common GTY(());
        int int_value GTY(());
    } int_cst GTY(());
};

/* Array of pointers to different types */
void *type_array[] GTY(()) = {
    (void *)&global_string,
    (void *)fixed_array,
    (void *)&simple_struct_instance
};

/* Global instances to ensure types are used */
struct simple_struct simple_struct_instance GTY(()) = {1, 'A', 100};
struct nested_struct nested_struct_instance GTY(()) = {
    {2, 'B', 200},
    &simple_struct_instance.field1,
    GREEN
};
union simple_union simple_union_instance GTY(()) = {.int_val = 42};
struct master_container container_instance GTY(()) = {
    .counter = 100,
    .name = "Test Container",
    .nested_ptr = &nested_struct_instance,
    .sorter = NULL,
    .matrix_ptr = NULL,
    .point = {10, 20}
};

/* Function using callback type */
static int compare_ints(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

/* Initialize callback in struct */
void init_struct_with_callback(struct struct_with_callback *s GTY(())) {
    s->id = 1;
    s->compare = compare_ints;
    s->notify = NULL;
}

/* Complex type graph with mutual recursion */
struct type_a GTY(()) {
    int id GTY(());
    struct type_b *b_link GTY(());
};

struct type_b GTY(()) {
    int id GTY(());
    struct type_a *a_link GTY(());
    struct type_a a_array[3] GTY(());
};

/* Typedef chain leading to scalar */
typedef int base_type GTY(());
typedef base_type level1_type GTY(());
typedef level1_type level2_type GTY(());
typedef level2_type final_scalar_type GTY(());

/* Struct with bitfield (special scalar case) */
struct bitfield_struct GTY(()) {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_field;
};

/* Union with bitfields */
union bitfield_union GTY(()) {
    struct {
        unsigned int a : 8;
        unsigned int b : 8;
        unsigned int c : 8;
        unsigned int d : 8;
    } bits GTY(());
    unsigned int value GTY(());
};

/* Opaque pointer type (pointer to undefined struct) */
typedef struct undefined_struct *opaque_handle GTY(());

/* Self-referential type structure */
struct self_ref GTY(()) {
    int data GTY(());
    struct self_ref *self_ptr GTY(());
    struct self_ref *array_of_self[5] GTY(());
};

/* Mixed array types */
struct array_container GTY(()) {
    int *ptr_array[10] GTY(());
    struct simple_struct struct_ptr_array[5] GTY(());
    union simple_union union_array[8] GTY(());
    void *void_ptr_array[] GTY(());
};

/* Function returning struct by value */
struct simple_struct make_struct(int a, char b, long c) {
    struct simple_struct s = {a, b, c};
    return s;
}

/* Pointer to function returning struct */
struct simple_struct (*struct_maker)(int, char, long) GTY((callback)) = make_struct;

/* Complete the opaque forward declaration */
struct opaque_forward_decl GTY(()) {
    int revealed_data GTY(());
    void *opaque_ptr GTY(());
};

/* Variadic callback type */
typedef void (*log_callback)(const char *format, ...) GTY((callback));

/* Struct with nested anonymous union */
struct anon_union_struct GTY(()) {
    int type GTY(());
    union {
        int int_val GTY(());
        float float_val GTY(());
        void *ptr_val GTY(());
    } value GTY(());
};

/* Const qualified types */
typedef const int const_int GTY(());
typedef volatile char volatile_char GTY(());
typedef const struct simple_struct *const_struct_ptr GTY(());

/* Alignment-specified types */
struct aligned_struct GTY(()) {
    int data GTY() __attribute__((aligned(16)));
    char padding[12];
};

/* Packed struct */
struct packed_struct GTY(()) {
    char a;
    int b;
    char c;
} __attribute__((packed));

/* Make sure all types are referenced to avoid being optimized out */
static void reference_all_types(void) {
    /* Reference each type to ensure they appear in the type system */
    volatile int dummy = 0;
    
    dummy += sizeof(struct opaque_forward_decl);
    dummy += sizeof(scalar_int);
    dummy += sizeof(color_enum);
    dummy += sizeof(global_string[0]);
    dummy += sizeof(int_ptr);
    dummy += sizeof(fixed_array);
    dummy += sizeof(comparator_fn);
    dummy += sizeof(struct simple_struct);
    dummy += sizeof(struct nested_struct);
    dummy += sizeof(struct recursive_struct);
    dummy += sizeof(union simple_union);
    dummy += sizeof(union complex_union);
    dummy += sizeof(user_struct_type);
    dummy += sizeof(struct struct_with_callback);
    dummy += sizeof(struct master_container);
    dummy += sizeof(v4si);
    dummy += sizeof(struct vector_struct);
    dummy += sizeof(struct tree_common);
    dummy += sizeof(union tree_node);
    dummy += sizeof(simple_struct_instance);
    dummy += sizeof(struct type_a);
    dummy += sizeof(struct type_b);
    dummy += sizeof(final_scalar_type);
    dummy += sizeof(struct bitfield_struct);
    dummy += sizeof(union bitfield_union);
    dummy += sizeof(opaque_handle);
    dummy += sizeof(struct self_ref);
    dummy += sizeof(struct array_container);
    dummy += sizeof(struct_maker);
    dummy += sizeof(log_callback);
    dummy += sizeof(struct anon_union_struct);
    dummy += sizeof(const_int);
    dummy += sizeof(struct aligned_struct);
    dummy += sizeof(struct packed_struct);
    
    (void)dummy; /* Suppress unused variable warning */
}
