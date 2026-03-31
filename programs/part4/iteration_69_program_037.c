#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing with GC attributes */
#define GC_TYPE __attribute__((user("GC")))

/* Base scalar types with GC attributes */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef double GC_TYPE scalar_double_t;

/* Enum type - should count as scalar */
typedef enum GC_TYPE {
    STATE_A,
    STATE_B,
    STATE_C
} state_t;

/* String type */
typedef char* GC_TYPE string_ptr_t;

/* Callback type */
typedef void (*GC_TYPE callback_t)(int, void*);

/* Forward declarations for cross-references */
struct gc_struct_a;
union gc_union_a;

#endif /* TEST_GENGYPE_H */
