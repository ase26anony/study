#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process types for garbage collection */
#define GC_ATTR __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef enum { RED, GREEN, BLUE } GC_ATTR gc_enum_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR gc_callback_t)(int, void*);

/* TYPE_POINTER: Various pointer types */
typedef gc_int_t* GC_ATTR gc_int_ptr_t;
typedef struct gc_base_struct* GC_ATTR gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GC_ATTR gc_int_array_t[10];
typedef char GC_ATTR gc_char_array_t[256];

/* Forward declarations for cross-references */
struct gc_base_struct GC_ATTR;
union gc_base_union GC_ATTR;

#endif /* GENGYPE_TEST_H */
