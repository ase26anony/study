#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force gengtype processing with GCC attributes */
#ifdef __GNUC__
#define GC_ATTR __attribute__((user("GC")))
#define USED_ATTR __attribute__((used))
#define RETAIN_ATTR __attribute__((retain))
#else
#define GC_ATTR
#define USED_ATTR
#define RETAIN_ATTR
#endif

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef double GC_ATTR gc_double_t;

/* TYPE_ENUM (counted as scalar) */
enum gc_enum GC_ATTR {
    GC_ENUM_A,
    GC_ENUM_B,
    GC_ENUM_C
};

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR gc_callback_t)(int, void*);

/* Forward declarations for complex types */
struct gc_complex_struct;
union gc_complex_union;

#endif /* GENGYPE_TEST_H */
