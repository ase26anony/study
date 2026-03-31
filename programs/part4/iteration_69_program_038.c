/* gengtype_test.h - Common type definitions for GC testing */
#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types during gengtype execution */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wattributes"

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef enum { RED, GREEN, BLUE } __attribute__((user("GC"))) gc_color_t;

/* TYPE_STRING: String type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int);

/* TYPE_POINTER: Pointer to GC types */
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
    gc_string_t name;
    gc_float_t value;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* TYPE_USER_STRUCT: User-defined structure with special handling */
struct __attribute__((user("GC"), aligned(16))) gc_user_struct {
    gc_int_t magic;
    gc_string_t data;
    struct gc_base_struct* next;
    gc_callback_t handlers[3];
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* as_struct;
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct __attribute__((user("GC"), transparent_union)) gc_lang_struct {
    gc_int_t lang_id;
    void* lang_data;
    gc_callback_t lang_callback;
};

/* Complex nested type to force deeper analysis */
struct __attribute__((user("GC"))) gc_complex_nested {
    union gc_base_union variant;
    struct gc_user_struct user;
    gc_string_array_t tags;
    struct gc_complex_nested* children[4];
    gc_callback_t (*selector)(struct gc_complex_nested*);
};

/* Create type aliases to increase type graph complexity */
typedef struct gc_base_struct __attribute__((user("GC"), alias("gc_base_struct"))) gc_base_alias_t;
typedef union gc_base_union __attribute__((user("GC"), alias("gc_base_union"))) gc_union_alias_t;

#pragma GCC diagnostic pop

/* Global variables to prevent elimination */
extern struct gc_base_struct gc_global_base __attribute__((used, retain));
extern union gc_base_union gc_global_union __attribute__((used, retain));
extern struct gc_user_struct gc_global_user __attribute__((used, retain));
extern struct gc_lang_struct gc_global_lang __attribute__((used, retain));
extern struct gc_complex_nested gc_global_nested __attribute__((used, retain));

/* Function declarations */
void init_gc_types(void);
void traverse_gc_types(void);

#endif /* GENGYPE_TEST_H */
