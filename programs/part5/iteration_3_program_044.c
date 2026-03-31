/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by gengtype during GCC build, it should trigger:
 * - TYPE_UNDEFINED
 * - TYPE_SCALAR
 * - TYPE_STRING
 * - TYPE_STRUCT
 * - TYPE_USER_STRUCT
 * - TYPE_UNION
 * - TYPE_POINTER
 * - TYPE_ARRAY
 * - TYPE_CALLBACK
 * - TYPE_LANG_STRUCT
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation */
#define GTY(x) __attribute__((annotate("gty")))

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations to create complex type relationships */
struct forward_declared;
typedef struct forward_declared *forward_ptr_t;

/* ========== TYPE_SCALAR definitions ========== */
GTY(()) struct scalar_types {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
    signed char s_char_field;
    unsigned int uint_field;
    size_t size_field;
};

/* ========== TYPE_STRING definitions ========== */
GTY(()) struct string_types {
    const char *const_string;
    char *mutable_string;
    const char *const array_of_strings[3];
};

/* ========== TYPE_STRUCT definitions ========== */
GTY(()) struct inner_struct {
    int data;
    char tag;
};

GTY(()) struct nested_struct {
    struct inner_struct inner;
    struct scalar_types scalars;
    int extra_data;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct with tag */
typedef GTY(()) struct user_tagged_struct {
    int user_id;
    char *user_name;
    struct user_tagged_struct *next;
} user_struct_t;

/* ========== TYPE_UNION definitions ========== */
GTY(()) union variant_data {
    int as_int;
    float as_float;
    double as_double;
    char *as_string;
    struct inner_struct as_struct;
};

GTY(()) struct union_container {
    int type_tag;
    union variant_data data;
};

/* ========== TYPE_POINTER definitions ========== */
GTY(()) struct pointer_galore {
    /* Simple pointers */
    int *int_ptr;
    char **string_ptr_ptr;
    
    /* Struct pointers */
    struct nested_struct *struct_ptr;
    struct pointer_galore *self_ptr;  /* Recursive pointer */
    
    /* Union pointer */
    union variant_data *union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Opaque pointer (forward declared) */
    forward_ptr_t opaque_ptr;
};

/* ========== TYPE_ARRAY definitions ========== */
GTY(()) struct array_types {
    /* Fixed-size arrays */
    int fixed_array[20];
    char char_array[50];
    struct inner_struct struct_array[5];
    
    /* Multi-dimensional arrays */
    int matrix[3][3];
    char strings[4][32];
    
    /* Array of pointers */
    int *ptr_array[8];
    struct nested_struct *struct_ptr_array[4];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*comparator_fn)(const void *, const void *);
typedef void (*callback_fn)(int, void *);
typedef char *(*string_generator_fn)(void);

GTY(()) struct callback_container {
    comparator_fn compare;
    callback_fn notify;
    string_generator_fn generate;
    
    /* Array of function pointers */
    void (*handlers[5])(void);
    
    /* Function pointer returning function pointer */
    callback_fn (*get_callback)(int);
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Language-specific structures - these would normally be defined
 * in language-specific headers in GCC */
GTY(()) struct lang_specific_base {
    int lang_specific_tag;
    void *lang_data;
};

/* Simulate a language struct with virtual methods */
GTY(()) struct cplusplus_class {
    struct lang_specific_base base;
    void (**vtable)(void);
    int member_count;
};

/* ========== Complex nested type ========== */
GTY(()) struct master_type {
    /* Include all type kinds */
    struct scalar_types scalars;
    struct string_types strings;
    struct nested_struct nested;
    user_struct_t user_struct;
    struct union_container union_data;
    struct pointer_galore pointers;
    struct array_types arrays;
    struct callback_container callbacks;
    struct cplusplus_class lang_struct;
    
    /* Self-reference for graph traversal */
    struct master_type *next;
    struct master_type *prev;
    
    /* Anonymous union inside struct */
    union {
        int anon_int;
        float anon_float;
    };
    
    /* Bitfields (scalar special case) */
    unsigned int flags : 4;
    unsigned int status : 2;
    
    /* Zero-length array (GCC extension) */
    int zero_array[0];
};

/* ========== TYPE_UNDEFINED trigger ========== */
/* Incomplete/undefined type usage */
struct forward_declared {
    int magic;
    struct undefined_type *undefined_ptr;  /* This should trigger TYPE_UNDEFINED */
};

/* External function to prevent optimization */
__attribute__((noinline)) 
static size_t compute_checksum(void *ptr, size_t size) {
    volatile char *p = (volatile char *)ptr;
    size_t sum = 0;
    
    /* Simple byte sum to ensure the data is accessed */
    for (size_t i = 0; i < size && i < 64; i++) {
        sum += p[i];
    }
    
    return sum;
}

/* Function to take addresses and prevent dead code elimination */
__attribute__((noinline))
static void reference_all_types(
    struct master_type *mt,
    struct scalar_types *sc,
    struct string_types *st,
    struct nested_struct *ns,
    user_struct_t *us,
    struct union_container *uc,
    struct pointer_galore *pg,
    struct array_types *at,
    struct callback_container *cc,
    struct cplusplus_class *lc
) {
    /* Force all types to be considered by taking addresses */
    volatile void *dummy;
    
    dummy = (void*)mt;
    dummy = (void*)sc;
    dummy = (void*)st;
    dummy = (void*)ns;
    dummy = (void*)us;
    dummy = (void*)uc;
    dummy = (void*)pg;
    dummy = (void*)at;
    dummy = (void*)cc;
    dummy = (void*)lc;
    
    (void)dummy;  /* Suppress unused warning */
}

int main(void) {
    /* Declare instances of all complex types */
    static struct scalar_types scalars = {
        .int_field = 42,
        .char_field = 'A',
        .float_field = 3.14f,
        .double_field = 2.71828,
        .bool_field = 1,
        .long_field = 123456789L,
        .short_field = 1234,
        .s_char_field = 'Z',
        .uint_field = 999,
        .size_field = sizeof(struct scalar_types)
    };
    
    static struct string_types strings = {
        .const_string = "Hello, gengtype!",
        .mutable_string = "Mutable string",
        .array_of_strings = { "one", "two", "three" }
    };
    
    static struct nested_struct nested = {
        .inner = { .data = 100, .tag = 'I' },
        .scalars = { .int_field = 200 },
        .extra_data = 300
    };
    
    static user_struct_t user_struct = {
        .user_id = 1,
        .user_name = "Test User",
        .next = NULL
    };
    
    static struct union_container union_data = {
        .type_tag = 1,
        .data = { .as_int = 42 }
    };
    
    static struct pointer_galore pointers = {
        .int_ptr = &scalars.int_field,
        .string_ptr_ptr = &strings.mutable_string,
        .struct_ptr = &nested,
        .self_ptr = NULL,
        .union_ptr = &union_data.data,
        .array_ptr = NULL,
        .opaque_ptr = NULL
    };
    pointers.self_ptr = &pointers;  /* Self-reference */
    
    static struct array_types arrays = {
        .fixed_array = {1, 2, 3, 4, 5},
        .char_array = "Test array data",
        .struct_array = {{1, 'A'}, {2, 'B'}, {3, 'C'}, {4, 'D'}, {5, 'E'}},
        .matrix = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}},
        .strings = {"row1", "row2", "row3", "row4"},
        .ptr_array = {&scalars.int_field, &scalars.int_field},
        .struct_ptr_array = {&nested, &nested, &nested, &nested}
        /* flexible_array omitted in static init */
    };
    
    static struct callback_container callbacks = {
        .compare = NULL,
        .notify = NULL,
        .generate = NULL,
        .handlers = {NULL, NULL, NULL, NULL, NULL},
        .get_callback = NULL
    };
    
    static struct cplusplus_class lang_struct = {
        .base = { .lang_specific_tag = 0xABCD, .lang_data = NULL },
        .vtable = NULL,
        .member_count = 10
    };
    
    static struct master_type master = {
        .scalars = scalars,
        .strings = strings,
        .nested = nested,
        .user_struct = user_struct,
        .union_data = union_data,
        .pointers = pointers,
        .arrays = arrays,
        .callbacks = callbacks,
        .lang_struct = lang_struct,
        .next = NULL,
        .prev = NULL,
        .anon_int = 0,
        .flags = 0xF,
        .status = 0x3
        /* zero_array has no initializer */
    };
    
    /* Create a chain for graph traversal */
    static struct master_type master2;
    master.next = &master2;
    master2.prev = &master;
    
    /* Reference all types to prevent optimization */
    reference_all_types(
        &master,
        &scalars,
        &strings,
        &nested,
        &user_struct,
        &union_data,
        &pointers,
        &arrays,
        &callbacks,
        &lang_struct
    );
    
    /* Compute checksum of all sizeof results to ensure
     * all type information is required */
    size_t total_size = 0;
    
    total_size += sizeof(struct scalar_types);
    total_size += sizeof(struct string_types);
    total_size += sizeof(struct nested_struct);
    total_size += sizeof(user_struct_t);
    total_size += sizeof(struct union_container);
    total_size += sizeof(struct pointer_galore);
    total_size += sizeof(struct array_types);
    total_size += sizeof(struct callback_container);
    total_size += sizeof(struct cplusplus_class);
    total_size += sizeof(struct master_type);
    
    /* Also compute checksum of actual data */
    size_t data_checksum = 0;
    data_checksum += compute_checksum(&master, sizeof(master));
    data_checksum += compute_checksum(&scalars, sizeof(scalars));
    data_checksum += compute_checksum(&strings, sizeof(strings));
    
    /* Print result to prevent complete optimization */
    printf("Type analysis test program\n");
    printf("Total type size sum: %zu bytes\n", total_size);
    printf("Data checksum: %zu\n", data_checksum);
    printf("All complex types defined and referenced.\n");
    
    return 0;
}
