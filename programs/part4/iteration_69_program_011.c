/* gengtype_test.h - Common type definitions for triggering gengtype logic */

#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types with GC machinery */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String type */
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
    gc_int_ptr_t data_ptr;
    int __attribute__((user("GC"))) extra_field;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* as_struct;
};

/* TYPE_LANG_STRUCT: Complex nested structure (simulating language-specific type) */
struct __attribute__((user("GC"))) gc_lang_struct {
    union gc_base_union data;
    struct gc_user_struct* user;
    gc_callback_t handlers[3];
    struct {
        gc_int_t x;
        gc_float_t y;
    } __attribute__((user("GC"))) point;
};

/* Create type aliases to force additional type processing */
typedef struct gc_base_struct __attribute__((alias("gc_base_struct"))) gc_base_alias_t;
typedef union gc_base_union __attribute__((weak)) gc_union_alias_t;

#pragma GCC diagnostic pop

/* Global variables to prevent optimization */
extern struct gc_base_struct gc_global_base __attribute__((used, retain));
extern struct gc_user_struct gc_global_user __attribute__((used, retain));
extern union gc_base_union gc_global_union __attribute__((used, retain));
extern struct gc_lang_struct gc_global_lang __attribute__((used, retain));

/* Function declarations */
void init_gc_types(void);
unsigned long calculate_checksum(void);

#endif /* GENGYPE_TEST_H */
