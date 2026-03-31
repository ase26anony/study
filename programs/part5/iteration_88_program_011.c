#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;
typedef struct forward_declared *forward_ptr;

/* TYPE_SCALAR: Various scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef _Bool scalar_bool;
typedef __complex__ double complex_scalar;
typedef __builtin_va_list va_list_scalar;
typedef int __attribute__((vector_size(16))) vector_scalar;

/* TYPE_STRING: String types */
typedef const char *string_ptr;
typedef char string_array[32];

/* TYPE_STRUCT: Regular structs */
struct regular_struct {
    int x;
    double y;
    char z;
};

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(4))) packed_struct {
    int a;
    char b;
    double c;
} __attribute__((designated_init));

struct __attribute__((aligned(32))) aligned_struct {
    long long data[8];
};

/* TYPE_UNION: Various unions */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

union __attribute__((packed)) packed_union {
    char bytes[8];
    long long value;
};

/* TYPE_POINTER: Pointer types */
typedef int *int_ptr;
typedef struct regular_struct *struct_ptr;
typedef void (*generic_func_ptr)(void);
typedef const volatile char *cv_ptr;

/* TYPE_ARRAY: Array types */
typedef int fixed_array[10];
typedef int incomplete_array[];
typedef int multidimensional_array[5][7];
extern int external_array[];

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void *user_data, int event_type);
typedef int (*va_func)(int, ...);

/* TYPE_LANG_STRUCT: Language-specific structs */
#ifdef __cplusplus
extern "C++" {
    struct cpp_specific {
        int x;
        virtual ~cpp_specific() {}
    };
    
    class cpp_class {
    public:
        int value;
        cpp_class() : value(0) {}
        virtual void method() {}
    };
}
#else
/* Use GCC attributes for C mode */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
    void *ptr;
};
#endif

/* Complex nested and recursive types */
struct recursive_node {
    int value;
    struct recursive_node *next;  /* Self-referential pointer */
    struct recursive_node *prev;
};

struct container {
    union {
        struct recursive_node *node_ptr;
        int int_value;
    } data;
    
    binary_op operation;
    event_callback callback;
    
    /* Incomplete array as last member */
    int flexible_array[];
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int secret;
    struct container *container_ptr;
};

/* Union containing array of pointers */
union pointer_union {
    struct regular_struct *struct_ptrs[4];
    struct recursive_node *node_ptrs[8];
    void *generic_ptrs[16];
};

/* Callback that takes pointer to struct */
typedef void (*struct_callback)(struct regular_struct *s, int action);

/* Function declarations using the types */
void process_container(struct container *c);
struct_callback get_default_callback(void);

/* Global variables for multi-file testing */
extern struct regular_struct global_struct;
extern union pointer_union global_union;
extern binary_op global_op;

#endif /* VARIED_TYPES_H */
