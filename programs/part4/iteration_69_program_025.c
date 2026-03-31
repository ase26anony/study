#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force GCC to process types for garbage collection */
#define GC_TYPE __attribute__((user("GC")))

/* Basic scalar types with GC attribute */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* String type */
typedef char* GC_TYPE string_t;

/* Callback type */
typedef void (*GC_TYPE callback_t)(int, void*);

/* Forward declarations for cross-references */
struct GC_TYPE nested_struct;
union GC_TYPE complex_union;

#endif
