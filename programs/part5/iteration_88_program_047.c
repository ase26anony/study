#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* Forward declarations to create TYPE_UNDEFINED initially */
struct opaque;
struct forward_declared;
union forward_union;

/* TYPE_SCALAR: typedefs for scalar types */
typedef int my_int;
typedef char my_char;
typedef float my_float;
typedef double my_double;
typedef _Bool my_bool;

/* GNU extensions for scalar types */
typedef __complex__ double my_complex;
typedef int __attribute__((vector_size(16))) my_vector;
typedef __builtin_va_list my_va_list;

/* TYPE_STRING */
typedef const char* my_string;

/* TYPE_POINTER */
typedef int* int_ptr;
typedef struct opaque* opaque_ptr;
typedef void (*generic_func_ptr)(void);

/* TYPE_ARRAY */
typedef int int_array[10];
typedef int incomplete_array[];
typedef int (*array_of_ptrs)[5];

/* TYPE_CALLBACK (function pointer types) */
typedef int (*binary_op)(int, int);
typedef void (*event_callback)(void* user_data, int event_type);
typedef int (*variadic_func)(int, ...);

/* TYPE_STRUCT with various attributes */
struct __attribute__((packed)) packed_struct {
    char c;
    int i;
    double d;
};

struct __attribute__((aligned(32))) aligned_struct {
    long long data;
    char padding;
};

struct __attribute__((designated_init)) designated_init_struct {
    int id;
    char name[32];
    float value;
};

/* TYPE_USER_STRUCT with complex attributes */
struct __attribute__((packed, aligned(8))) user_struct {
    int tag;
    union {
        int i;
        float f;
        void* p;
    } data;
    char name[16];
};

/* TYPE_UNION */
union data_union {
    int as_int;
    float as_float;
    double as_double;
    void* as_ptr;
    char as_bytes[8];
};

/* Recursive and nested types */
struct recursive_node {
    int value;
    struct recursive_node* next;  /* TYPE_POINTER to same struct */
    struct recursive_node* prev;
};

struct container {
    int id;
    struct recursive_node* nodes;
    union data_union storage;
    binary_op operation;
};

/* Struct with incomplete array as last member */
struct flexible_array {
    size_t count;
    int data[];  /* TYPE_ARRAY (incomplete) */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int secret;
    void* handle;
};

/* Forward declared struct definition */
struct forward_declared {
    int value;
    struct forward_declared* self_ref;
};

/* Forward declared union definition */
union forward_union {
    struct forward_declared* fd;
    struct opaque* op;
    int value;
};

/* Complex nested type with array of pointers */
struct graph_node {
    int id;
    struct graph_node** neighbors;  /* Pointer to array of pointers */
    size_t neighbor_count;
};

/* Callback that uses our types */
typedef void (*graph_traversal_cb)(struct graph_node*, void* context);

/* Extern C++ block for potential TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    struct cpp_like_struct {
        int x, y;
        void method() {}
    };
    
    class SimpleClass {
    private:
        int value;
    public:
        SimpleClass(int v) : value(v) {}
        int getValue() const { return value; }
    };
}
#endif

/* Transaction-safe struct (GCC extension) */
struct __attribute__((transaction_safe)) transaction_struct {
    int counter;
    void (*increment)(struct transaction_struct*);
};

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    unsigned int padding : 24;
};

#endif /* VARIED_TYPES_H */
