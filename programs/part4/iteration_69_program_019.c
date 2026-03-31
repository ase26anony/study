#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process these types during gengtype execution */
#ifdef __GNUC__
#define GC_ATTR __attribute__((user("GC")))
#define USED_ATTR __attribute__((used, retain))
#else
#define GC_ATTR
#define USED_ATTR
#endif

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR scalar_int_t;
typedef float GC_ATTR scalar_float_t;
typedef double GC_ATTR scalar_double_t;

/* TYPE_ENUM (falls under scalar) */
enum GC_ATTR color { RED, GREEN, BLUE };

/* TYPE_STRING: String pointer type */
typedef char* GC_ATTR string_ptr_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR callback_func_t)(int, void*);

/* Forward declarations for complex types */
struct gc_struct_a;
union gc_union_a;

/* TYPE_POINTER: Various pointer types */
typedef struct gc_struct_a* GC_ATTR struct_ptr_t;
typedef union gc_union_a* GC_ATTR union_ptr_t;
typedef callback_func_t* GC_ATTR callback_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GC_ATTR int_array_t[10];
typedef struct gc_struct_a* GC_ATTR ptr_array_t[5];

/* Base structure for TYPE_STRUCT */
struct GC_ATTR gc_base {
    scalar_int_t id;
    string_ptr_t name;
    callback_func_t handler;
};

/* TYPE_UNION */
union GC_ATTR gc_union_a {
    scalar_int_t as_int;
    scalar_float_t as_float;
    string_ptr_t as_string;
    struct gc_base* as_base;
};

/* TYPE_STRUCT with nested types */
struct GC_ATTR gc_struct_a {
    scalar_int_t counter;
    scalar_float_t value;
    string_ptr_t description;
    callback_func_t notify;
    int_array_t scores;
    struct gc_base* base;
    union gc_union_a data;
    struct gc_struct_a* next;  /* Self-referential pointer */
};

/* TYPE_USER_STRUCT: Using typedef with struct */
typedef struct GC_ATTR {
    scalar_int_t x;
    scalar_int_t y;
    string_ptr_t label;
} user_point_t;

/* Complex nested structure for TYPE_LANG_STRUCT simulation */
struct GC_ATTR lang_complex {
    struct {
        scalar_int_t type_id;
        string_ptr_t type_name;
    } type_info;
    
    union {
        struct gc_struct_a* as_struct;
        union gc_union_a* as_union;
        callback_func_t as_callback;
    } data_union;
    
    ptr_array_t references;
};

/* Global variables to force type instantiation */
extern struct gc_struct_a USED_ATTR global_struct;
extern union gc_union_a USED_ATTR global_union;
extern user_point_t USED_ATTR global_user_struct;
extern struct lang_complex USED_ATTR global_lang_struct;

/* Function declarations that use GC types */
callback_func_t register_callback(callback_func_t cb);
void process_struct(struct gc_struct_a* s);
union gc_union_a* create_union(void);

#endif /* GENGYPE_TEST_H */
