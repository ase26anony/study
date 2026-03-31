#ifndef GC_TYPES_H
#define GC_TYPES_H

/* Force gengtype processing with GCC attributes */
#define GC_TYPE __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* TYPE_SCALAR: Basic scalar types with GC attribute */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_TYPE string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_TYPE callback_t)(int);

/* Forward declarations for cross-references */
struct gc_struct_a;
union gc_union_a;

#endif /* GC_TYPES_H */
