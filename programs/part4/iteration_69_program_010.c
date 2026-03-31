#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with gengtype */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String pointer type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int);

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_basic_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
};

/* TYPE_USER_STRUCT: Another structure with user attribute */
struct __attribute__((user("GC"), aligned(16))) gc_user_struct {
    gc_int_t counter;
    struct gc_basic_struct* link;
    gc_callback_t handler;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_basic_struct* as_struct;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct gc_basic_struct* __attribute__((user("GC"))) gc_struct_ptr_t;
typedef union gc_data_union* __attribute__((user("GC"))) gc_union_ptr_t;

/* TYPE_ARRAY: Array types */
typedef gc_int_t __attribute__((user("GC"))) gc_int_array_t[10];
typedef struct gc_basic_struct __attribute__((user("GC"))) gc_struct_array_t[5];

/* Forward declarations for cross-references */
struct gc_complex_struct;
union gc_nested_union;

/* TYPE_LANG_STRUCT: More complex structure */
struct __attribute__((user("GC"))) gc_lang_struct {
    gc_int_t tag;
    union {
        gc_int_t int_val;
        gc_float_t float_val;
        struct gc_complex_struct* complex_ptr;
    } data;
    gc_callback_t methods[3];
};

/* Nested type definitions */
struct __attribute__((user("GC"))) gc_complex_struct {
    gc_int_t id;
    union gc_nested_union* nested;
    gc_struct_array_t items;
};

union __attribute__((user("GC"))) gc_nested_union {
    struct gc_lang_struct lang;
    struct gc_complex_struct complex;
    gc_int_array_t numbers;
};

/* Create type aliases to force additional processing */
typedef struct gc_basic_struct __attribute__((user("GC"), alias("gc_basic_struct"))) gc_basic_struct_alias;
typedef union gc_data_union __attribute__((user("GC"), weak)) gc_weak_union_t;

#pragma GCC diagnostic pop

/* External declarations for cross-translation unit references */
extern struct gc_basic_struct global_basic_struct;
extern union gc_data_union global_data_union;
extern gc_int_array_t global_int_array;

/* Function to force type usage */
void use_types(void);

#endif /* GENGYPE_TEST_H */
