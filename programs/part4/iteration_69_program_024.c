/* gengtype_test.h - Common type definitions for GC testing */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types for GC */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String pointer type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* Forward declarations for structs/unions */
struct gc_nested;
union gc_complex_union;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_basic_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_callback_t callback;
};

/* TYPE_USER_STRUCT: Another structure type */
struct __attribute__((user("GC"))) gc_user_struct {
    struct gc_basic_struct* parent;
    struct gc_nested* child;
    int __attribute__((user("GC"))) extra_data;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_simple_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
};

/* TYPE_POINTER: Typedef for pointer */
typedef struct gc_basic_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array type */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];

/* Complex nested structure */
struct __attribute__((user("GC"))) gc_nested {
    struct gc_basic_struct base;
    union gc_simple_union data;
    gc_int_array_t numbers;
    struct gc_nested* __attribute__((user("GC"))) next;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct __attribute__((user("GC"))) gc_lang_struct {
    void* __attribute__((user("GC"))) lang_data;
    int lang_type;
};

/* Complex union with nested struct */
union __attribute__((user("GC"))) gc_complex_union {
    struct gc_basic_struct struct_part;
    struct gc_user_struct user_part;
    struct gc_lang_struct lang_part;
    gc_int_array_t array_part;
};

/* Create type aliases to force additional processing */
typedef struct gc_basic_struct __attribute__((alias("gc_basic_struct"))) gc_basic_struct_alias;
typedef union gc_simple_union __attribute__((weak)) gc_weak_union;

#pragma GCC diagnostic pop

/* Global variables to prevent optimization */
extern struct gc_basic_struct __attribute__((used, retain)) global_gc_struct;
extern union gc_complex_union __attribute__((used, retain)) global_gc_union;
extern gc_int_array_t __attribute__((used, retain)) global_gc_array;

/* Function declarations */
void init_gc_types(void);
unsigned long calculate_checksum(void);

#endif /* GENGYPE_TEST_H */
