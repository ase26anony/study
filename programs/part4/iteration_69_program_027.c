/* gengtype_test.h - Common type definitions for triggering gengtype analysis */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with GC machinery */
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

/* TYPE_STRUCT: Basic struct types */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* TYPE_USER_STRUCT: More complex nested struct */
struct __attribute__((user("GC"))) gc_user_struct {
    struct gc_base_struct base;
    struct gc_user_struct* __attribute__((user("GC"))) next;
    union gc_base_union* __attribute__((user("GC"))) data;
    gc_struct_ptr_t ptr_array[3];
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* __attribute__((user("GC"))) as_struct;
};

/* TYPE_LANG_STRUCT: Complex language-specific structure */
struct __attribute__((user("GC"))) gc_lang_struct {
    struct gc_user_struct user;
    union gc_base_union variant;
    gc_callback_t handlers[2];
    struct {
        gc_int_t x;
        gc_float_t y;
    } __attribute__((user("GC"))) point;
};

/* External variables to force type retention */
extern struct gc_base_struct __attribute__((used, retain)) global_base;
extern struct gc_user_struct __attribute__((used, retain)) global_user;
extern union gc_base_union __attribute__((used, retain)) global_union;
extern struct gc_lang_struct __attribute__((used, retain)) global_lang;

/* Function to create type references */
void __attribute__((used)) reference_all_types(void);

#pragma GCC diagnostic pop

#endif /* GENGYPE_TEST_H */
