#ifndef TEST_GENGYPE_H
#define TEST_GENGYPE_H

/* Force GCC to process types for garbage collection */
#define GC_ATTR __attribute__((user("GC")))

/* 1. SCALAR TYPES */
typedef int GC_ATTR gc_int_t;
typedef float GC_ATTR gc_float_t;
typedef enum { RED, GREEN, BLUE } GC_ATTR gc_enum_t;

/* 2. STRING TYPE */
typedef char* GC_ATTR gc_string_t;

/* 3. CALLBACK TYPE */
typedef void (*GC_ATTR gc_callback_t)(int, const char*);

/* 4. POINTER TYPES */
typedef gc_int_t* GC_ATTR gc_int_ptr_t;
typedef struct GC_StructA* GC_ATTR gc_structa_ptr_t;

/* 5. ARRAY TYPE */
typedef int GC_ATTR gc_array_t[10];

/* Forward declarations for cross-TU references */
struct GC_StructA;
union GC_UnionA;
struct GC_LangStruct;

/* External variables to force type retention */
extern struct GC_StructA GC_ATTR g_ext_struct;
extern union GC_UnionA GC_ATTR g_ext_union;

/* Function to create type dependencies */
void register_types(void);

#endif /* TEST_GENGYPE_H */
