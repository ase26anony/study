#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process types for garbage collection */
#define GC_ATTR __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef double GC_ATTR gc_double_t;

/* TYPE_ENUM (falls under scalar) */
enum gc_color { RED, GREEN, BLUE } GC_ATTR;

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*gc_callback_t)(int) GC_ATTR;

/* Forward declarations for complex types */
struct gc_nested;
union gc_variant;

/* TYPE_STRUCT: Basic structure */
struct gc_basic_struct GC_ATTR {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    struct gc_nested* next;
};

/* TYPE_UNION: Basic union */
union gc_basic_union GC_ATTR {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
};

/* TYPE_POINTER: Typedef for pointer */
typedef struct gc_basic_struct* GC_ATTR gc_struct_ptr_t;

/* TYPE_ARRAY: Array type */
typedef int GC_ATTR gc_int_array_t[10];

/* Complex nested structure for TYPE_STRUCT */
struct gc_nested GC_ATTR {
    struct gc_basic_struct base;
    union gc_basic_union variant;
    gc_callback_t callback;
    gc_int_array_t numbers;
    struct gc_nested* children[5];
};

/* TYPE_UNION with nested struct */
union gc_complex_union GC_ATTR {
    struct {
        gc_int_t type;
        gc_string_t data;
    } GC_ATTR tagged;
    struct gc_nested nested;
    gc_int_array_t array;
};

/* TYPE_USER_STRUCT: Using struct with typedef */
typedef struct gc_basic_struct gc_user_struct_t GC_ATTR;

/* Self-referential structure */
struct gc_recursive GC_ATTR {
    gc_int_t value;
    struct gc_recursive* GC_ATTR next;
    struct gc_recursive* GC_ATTR prev;
};

/* Force retention of types */
#define RETAIN_TYPE __attribute__((used, retain))

/* Global variables to force type instantiation */
extern struct gc_basic_struct RETAIN_TYPE global_struct;
extern union gc_basic_union RETAIN_TYPE global_union;
extern gc_callback_t RETAIN_TYPE global_callback;
extern gc_int_array_t RETAIN_TYPE global_array;

#endif /* GENGYPE_TEST_H */
