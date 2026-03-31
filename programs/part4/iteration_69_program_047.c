#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing with GCC attributes */
#define GC_ATTR __attribute__((user("GC")))
#define GC_USED __attribute__((used, retain))

/* Base scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef double GC_ATTR gc_double_t;

/* Enum type (scalar) */
enum gc_color { RED, GREEN, BLUE } GC_ATTR;

/* String type */
typedef char* GC_ATTR gc_string_t;

/* Callback type (function pointer) */
typedef void (*gc_callback_t)(int) GC_ATTR;

/* Forward declarations for cross-references */
struct gc_nested_struct;
union gc_complex_union;

/* Simple struct */
struct gc_simple_struct GC_ATTR {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
};

/* Union type */
union gc_simple_union GC_ATTR {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
};

/* Array type wrapper */
struct gc_array_container GC_ATTR {
    gc_int_t data[10];
    gc_float_t matrix[3][3];
};

/* Pointer types */
typedef struct gc_simple_struct* GC_ATTR gc_struct_ptr_t;
typedef union gc_simple_union* GC_ATTR gc_union_ptr_t;

/* Complex nested type definitions */
struct gc_complex_nested GC_ATTR {
    struct gc_simple_struct inner_struct;
    union gc_simple_union inner_union;
    gc_struct_ptr_t ptr_to_struct;
    gc_callback_t callback;
    struct gc_complex_nested* next;  /* Self-referential pointer */
};

/* Language-specific struct simulation */
struct gc_lang_struct GC_ATTR {
    void* lang_specific;
    int lang_tag;
};

/* External references to force cross-TU type analysis */
extern struct gc_simple_struct GC_USED global_gc_struct;
extern union gc_simple_union GC_USED global_gc_union;

#endif /* TEST_GENGYPE_H */
