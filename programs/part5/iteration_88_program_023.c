#ifndef VARIED_TYPES_H
#define VARIED_TYPES_H

#include <stdarg.h>

/* TYPE_UNDEFINED: Forward declarations */
struct opaque;
struct forward_declared;

/* TYPE_SCALAR: Various scalar typedefs */
typedef char byte_t;
typedef short int16_t;
typedef int int32_t;
typedef long long int64_t;
typedef float float32_t;
typedef double float64_t;
typedef _Bool bool_t;
typedef __complex__ double complex_double;
typedef __complex__ float complex_float;

/* GCC vector extensions for scalar classification */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));

/* TYPE_STRING: String typedef */
typedef const char* string_t;

/* TYPE_USER_STRUCT: Structs with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
};

struct __attribute__((designated_init)) designated_init_struct {
    int field1;
    char field2;
    double field3;
};

/* TYPE_STRUCT: Regular structs */
struct point {
    int x;
    int y;
    int z;
};

struct data_node {
    int id;
    char name[32];
    struct data_node *next;  /* Recursive pointer */
    struct data_node *prev;
};

/* TYPE_UNION: Unions */
union variant {
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
    void *ptr_val;
};

union tagged_union {
    struct {
        int type;
        char data[16];
    } tagged;
    struct {
        long long value;
        void *pointer;
    } raw;
};

/* TYPE_ARRAY: Various array types */
typedef int matrix_3x3[3][3];
typedef char buffer_t[256];

/* Incomplete array type */
struct flexible_array {
    int count;
    double data[];  /* TYPE_ARRAY with incomplete size */
};

/* TYPE_POINTER: Pointer typedefs */
typedef int* int_ptr_t;
typedef struct point* point_ptr_t;
typedef void (*generic_callback_t)(void);

/* TYPE_CALLBACK: Function pointer types */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*event_handler_t)(int event_id, void* user_data);
typedef char* (*formatter_t)(va_list args);

/* Complex callback with struct parameter */
typedef void (*data_processor_t)(struct data_node* node, int operation);

/* Recursive struct with callback */
struct recursive_struct {
    int value;
    struct recursive_struct *self;  /* Recursive pointer */
    void (*process)(struct recursive_struct*);  /* Callback field */
};

/* Opaque struct definition (after forward declaration) */
struct opaque {
    int magic;
    void *data;
    struct forward_declared *link;
};

/* Forward declared struct definition */
struct forward_declared {
    int id;
    struct opaque *back_link;
};

/* Nested complex type */
struct container {
    union variant storage;
    struct point coordinates;
    matrix_3x3 transform;
    comparator_t compare_func;
    struct flexible_array *flex_array;
};

/* Array of pointers to unions */
typedef union variant* variant_ptr_array[10];

/* Multi-dimensional array of function pointers */
typedef event_handler_t event_handler_matrix[5][5];

/* C++ specific structures for TYPE_LANG_STRUCT */
#ifdef __cplusplus
extern "C++" {
    class cpp_class {
    private:
        int private_data;
    public:
        virtual void method() = 0;
        virtual ~cpp_class() {}
    };
    
    struct __attribute__((transaction_safe)) transaction_safe_struct {
        int value;
        void update(int new_value);
    };
}
#endif

/* Attribute to potentially create TYPE_LANG_STRUCT */
struct __attribute__((transaction_safe)) gcc_transaction_struct {
    long counter;
    void (*increment)(struct gcc_transaction_struct*);
};

/* Builtin types */
typedef __builtin_va_list va_list_t;

#endif /* VARIED_TYPES_H */
