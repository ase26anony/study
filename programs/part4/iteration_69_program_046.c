#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* Various type categories to trigger different switch cases */

/* TYPE_SCALAR */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
enum color { RED, GREEN, BLUE } GC_TYPE;

/* TYPE_STRING */
typedef char* GC_TYPE string_t;

/* TYPE_CALLBACK */
typedef void (*GC_TYPE callback_t)(int, float);

/* TYPE_POINTER */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef struct gc_struct* GC_TYPE struct_ptr_t;

/* TYPE_ARRAY */
typedef int GC_TYPE int_array_t[10];
typedef struct gc_struct GC_TYPE struct_array_t[5];

/* Forward declarations for cross-references */
struct gc_struct GC_TYPE;
union gc_union GC_TYPE;

#endif /* TEST_GENGYPE_H */
