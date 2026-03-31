#ifndef TEST_GENGTYPE_TYPES_H
#define TEST_GENGTYPE_TYPES_H

/* Force gengtype processing with GCC attributes */
#define GC_ATTR __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef enum { RED, GREEN, BLUE } GC_ATTR gc_color_t;

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_STRUCT: Basic structure */
struct GC_ATTR gc_basic_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
};

/* TYPE_USER_STRUCT: User-defined structure with nested types */
typedef struct GC_ATTR {
    struct gc_basic_struct* parent;
    gc_int_t count;
    gc_float_t data[5];
} gc_user_struct_t;

/* TYPE_UNION: Union type */
union GC_ATTR gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_basic_struct* as_struct;
};

/* TYPE_POINTER: Various pointer types */
typedef gc_user_struct_t* GC_ATTR gc_user_ptr_t;
typedef union gc_data_union* GC_ATTR gc_union_ptr_t;
typedef void* GC_ATTR gc_void_ptr_t;

/* TYPE_ARRAY: Array types */
typedef gc_int_t GC_ATTR gc_int_array_t[10];
typedef struct gc_basic_struct GC_ATTR gc_struct_array_t[5];

/* TYPE_CALLBACK: Function pointer types */
typedef void (*GC_ATTR gc_callback_t)(gc_int_t, gc_string_t);
typedef gc_int_t (*GC_ATTR gc_transform_t)(gc_float_t*, int);

/* Complex nested type for deeper analysis */
struct GC_ATTR gc_complex_nested {
    gc_user_struct_t base;
    union gc_data_union variant;
    gc_callback_t handler;
    gc_int_array_t numbers;
    gc_struct_array_t items;
    struct gc_complex_nested* GC_ATTR next;  /* Self-referential pointer */
};

/* Forward declarations to create type cycles */
struct GC_ATTR gc_forward_decl;
typedef struct gc_forward_decl GC_ATTR gc_forward_t;

struct GC_ATTR gc_forward_decl {
    gc_int_t id;
    gc_forward_t* GC_ATTR partner;
};

/* Alias types to force additional type enumeration */
typedef gc_user_struct_t GC_ATTR gc_alias1_t __attribute__((alias("gc_user_struct_t")));
typedef gc_int_t GC_ATTR gc_alias2_t __attribute__((weak, alias("gc_int_t")));

/* Global variables to prevent elimination */
extern struct gc_basic_struct GC_ATTR g_global_struct __attribute__((used, retain));
extern gc_user_struct_t GC_ATTR g_user_structs[3] __attribute__((used, retain));
extern union gc_data_union GC_ATTR g_data_union __attribute__((used, retain));
extern gc_callback_t GC_ATTR g_current_callback __attribute__((used, retain));

#endif /* TEST_GENGTYPE_TYPES_H */
