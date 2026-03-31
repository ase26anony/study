#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force gengtype processing with GC attributes */
#define GC_ATTR __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef enum { RED, GREEN, BLUE } GC_ATTR gc_enum_t;

/* TYPE_STRING: String type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR gc_callback_t)(int, float);

/* Forward declarations for complex types */
struct gc_complex_struct;
union gc_complex_union;

/* TYPE_POINTER: Various pointer types */
typedef struct gc_complex_struct* GC_ATTR gc_struct_ptr_t;
typedef union gc_complex_union* GC_ATTR gc_union_ptr_t;
typedef gc_callback_t* GC_ATTR gc_callback_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GC_ATTR gc_int_array_t[10];
typedef struct gc_complex_struct GC_ATTR gc_struct_array_t[5];

/* External references to force cross-TU type analysis */
extern struct gc_complex_struct* global_struct_ref;
extern union gc_complex_union* global_union_ref;

/* Function to prevent optimization */
void use_types(void);

#endif /* GENGYPE_TEST_H */
