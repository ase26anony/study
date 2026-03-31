#ifndef GC_TYPES_H
#define GC_TYPES_H

/* Force gengtype processing with GCC attributes */
#define GC_TYPE __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* Scalar types (TYPE_SCALAR) */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef double GC_TYPE scalar_double_t;

/* Enum type (also TYPE_SCALAR) */
enum GC_TYPE color { RED, GREEN, BLUE };

/* String type (TYPE_STRING) */
typedef char* GC_TYPE string_t;

/* Callback type (TYPE_CALLBACK) */
typedef void GC_TYPE (*callback_t)(int);
typedef int GC_TYPE (*filter_t)(const char*);

/* Pointer types (TYPE_POINTER) */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef void* GC_TYPE generic_ptr_t;

/* Array type (TYPE_ARRAY) */
typedef int GC_TYPE int_array_t[10];
typedef callback_t GC_TYPE callback_array_t[5];

/* Forward declarations for struct/union types */
struct GC_TYPE my_struct;
union GC_TYPE my_union;
struct GC_TYPE nested_container;

#endif /* GC_TYPES_H */
