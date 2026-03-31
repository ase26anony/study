#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with gengtype machinery */
#pragma GCC GCC
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

/* TYPE_POINTER: Pointer to various types */
typedef gc_int_t* __attribute__((user("GC"))) gc_int_ptr_t;
typedef struct gc_base_struct* __attribute__((user("GC"))) gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int __attribute__((user("GC"))) gc_int_array_t[10];
typedef gc_string_t __attribute__((user("GC"))) gc_string_array_t[5];

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_base_union;

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* TYPE_USER_STRUCT: Another structure type */
struct __attribute__((user("GC"))) gc_user_struct {
    struct gc_base_struct* base;
    gc_int_ptr_t data;
    int __attribute__((user("GC"))) extra_field;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* as_struct;
};

/* Nested structure with union */
struct __attribute__((user("GC"))) gc_complex_struct {
    gc_int_t type;
    union {
        gc_int_t int_val;
        gc_float_t float_val;
        gc_string_t string_val;
    } __attribute__((user("GC"))) data;
    struct gc_complex_struct* __attribute__((user("GC"))) next;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct __attribute__((user("GC"))) __attribute__((lang_struct)) gc_lang_struct {
    gc_int_t lang_id;
    void* __attribute__((user("GC"))) lang_data;
};

/* Create type aliases to force additional processing */
typedef struct gc_base_struct __attribute__((alias("gc_base_struct"))) gc_base_alias_t;
typedef union gc_base_union __attribute__((weak)) gc_union_alias_t;

#pragma GCC diagnostic pop

/* Global variables to prevent elimination */
extern struct gc_base_struct __attribute__((used)) __attribute__((retain)) global_base;
extern union gc_base_union __attribute__((used)) __attribute__((retain)) global_union;
extern gc_int_array_t __attribute__((used)) __attribute__((retain)) global_array;

/* Function to force type references */
void reference_all_types(void);

#endif /* GENGYPE_TEST_H */
