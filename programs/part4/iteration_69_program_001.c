/* gengtype_test.h - Common type definitions for gengtype coverage test */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_TYPE string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_TYPE callback_t)(int);

/* Forward declarations for struct types */
struct GC_TYPE my_struct;
union GC_TYPE my_union;

/* TYPE_ARRAY: Array types */
typedef int GC_TYPE int_array_t[10];
typedef struct my_struct* GC_TYPE struct_ptr_array_t[5];

/* TYPE_POINTER: Pointer typedefs */
typedef struct my_struct* GC_TYPE struct_ptr_t;
typedef union my_union* GC_TYPE union_ptr_t;

/* External variables to force type retention */
extern struct my_struct* GC_TYPE global_struct_ptr;
extern union my_union GC_TYPE global_union_var;

/* Function to create type references */
void register_types(void);

#endif /* GENGYPE_TEST_H */
