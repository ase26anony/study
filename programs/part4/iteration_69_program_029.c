#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force gengtype processing macros */
#ifdef __cplusplus
extern "C" {
#endif

/* GCC GC attribute to mark types for garbage collector */
#define GC_TYPE __attribute__((user("GC")))

/* 1. SCALAR TYPES */
typedef int GC_TYPE scalar_int_t;
typedef float GC_TYPE scalar_float_t;
typedef enum { RED, GREEN, BLUE } GC_TYPE color_t;

/* 2. STRING TYPE */
typedef char* GC_TYPE gc_string_t;

/* 3. CALLBACK TYPE */
typedef void (*GC_TYPE callback_func_t)(int, void*);

/* 4. POINTER TYPES */
typedef scalar_int_t* GC_TYPE int_ptr_t;
typedef void* GC_TYPE gc_void_ptr_t;

/* 5. BASE STRUCTURE for inheritance-like patterns */
struct GC_TYPE base_struct {
    int id;
    gc_string_t name;
    callback_func_t handler;
};

/* 6. UNION TYPE */
union GC_TYPE data_union {
    scalar_int_t as_int;
    scalar_float_t as_float;
    gc_string_t as_string;
    void* as_ptr;
};

/* 7. ARRAY TYPES */
typedef int GC_TYPE int_array_t[10];
typedef struct base_struct* GC_TYPE struct_ptr_array_t[5];

/* Forward declarations for cross-TU references */
struct GC_TYPE complex_nested;
union GC_TYPE cross_tu_union;

/* Function to force type usage */
void register_types(void);

#ifdef __cplusplus
}
#endif

#endif /* TEST_GENGYPE_H */
