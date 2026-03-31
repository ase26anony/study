/* gc_types.h - Common GC-tracked type definitions */
#ifndef GC_TYPES_H
#define GC_TYPES_H

/* Force gengtype processing with GCC attributes */
#define GC_ATTR __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef double GC_ATTR gc_double_t;

/* TYPE_ENUM (falls under scalar) */
enum gc_enum GC_ATTR {
    GC_ENUM_A,
    GC_ENUM_B,
    GC_ENUM_C
};

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR gc_callback_t)(int, void*);

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_base_union;

#endif /* GC_TYPES_H */
