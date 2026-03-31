#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* Scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* String type */
typedef char* GC_TYPE gc_string_t;

/* Callback type */
typedef void (*GC_TYPE callback_t)(int);

/* Forward declarations for cross-TU references */
struct gc_struct_a;
union gc_union_a;

#endif
