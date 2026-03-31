#ifndef GENGYPE_TEST_H
#define GENGYPE_TEST_H

/* Force GCC to process types for garbage collection */
#define GC_ATTR __attribute__((user("GC")))

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef enum { RED, GREEN, BLUE } GC_ATTR gc_color_t;

/* TYPE_STRING: String type */
typedef char* GC_ATTR gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*GC_ATTR gc_callback_t)(int, float);

/* TYPE_POINTER: Pointer to various types */
typedef gc_int_t* GC_ATTR gc_int_ptr_t;
typedef struct gc_base_struct* GC_ATTR gc_struct_ptr_t;

/* TYPE_ARRAY: Array types */
typedef int GC_ATTR gc_int_array_t[10];
typedef float GC_ATTR gc_float_array_t[5][5];

/* Forward declarations for cross-references */
struct gc_base_struct;
union gc_base_union;
struct gc_complex_struct;

/* TYPE_STRUCT: Basic structure */
struct GC_ATTR gc_base_struct {
    gc_int_t id;
    gc_string_t name;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* TYPE_UNION: Basic union */
union GC_ATTR gc_base_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* as_struct;
};

/* TYPE_USER_STRUCT: More complex user-defined structure */
struct GC_ATTR gc_user_struct {
    struct gc_base_struct base;
    union gc_base_union data;
    gc_int_ptr_t* double_ptr; /* Pointer to pointer */
    struct gc_user_struct* next; /* Self-referential */
    gc_callback_t handlers[3];
};

/* TYPE_LANG_STRUCT: Language-specific structure */
struct GC_ATTR gc_lang_struct {
    struct gc_base_struct base;
    void* lang_data;
    int lang_tag;
};

/* Complex nested type for deeper type graph */
struct GC_ATTR gc_complex_struct {
    struct gc_user_struct users[4];
    union gc_base_union variants[8];
    gc_callback_t (*callbacks)[2];
    struct gc_lang_struct* lang_structs;
};

/* Global variables to force type retention */
extern struct gc_base_struct GC_ATTR global_base_struct 
    __attribute__((used, retain));
extern union gc_base_union GC_ATTR global_base_union 
    __attribute__((used, retain));
extern struct gc_user_struct GC_ATTR global_user_struct 
    __attribute__((used, retain));
extern struct gc_lang_struct GC_ATTR global_lang_struct 
    __attribute__((used, retain));
extern struct gc_complex_struct GC_ATTR global_complex_struct 
    __attribute__((used, retain));

/* Function to create type references */
void init_gc_types(void);
void use_gc_types(void);

#endif /* GENGYPE_TEST_H */
