#ifndef TEST_GENGTYPE_H
#define TEST_GENGTYPE_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* Scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* String type */
typedef char* GC_TYPE gc_string_t;

/* Callback type */
typedef void (*GC_TYPE callback_func_t)(int, void*);

/* Forward declarations for cross-references */
struct gc_nested_struct;
union gc_complex_union;

#endif /* TEST_GENGTYPE_H */
