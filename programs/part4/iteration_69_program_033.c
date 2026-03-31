/* test_gengtype_types.c
 * This program defines various GC-tracked types to trigger gengtype's
 * type counting logic during compilation.
 */

/* First, include some headers that might influence type processing */
#include <stddef.h>
#include <stdint.h>

/* Define a macro for GC attributes - using GCC's internal GC marking */
#ifndef GTY
#define GTY(x) __attribute__((user("GC"))) x
#endif

/* Also use retain attribute to ensure types aren't optimized away */
#define RETAIN __attribute__((used, retain))

/* ==================== TYPE DEFINITIONS ==================== */

/* TYPE_SCALAR: Basic scalar types with GC attributes */
typedef GTY(()) int scalar_int_t;
typedef GTY(()) float scalar_float_t;
typedef GTY(()) double scalar_double_t;

/* TYPE_STRING: String pointer types */
typedef GTY(()) char* string_ptr_t;
typedef GTY(()) const char* const_string_ptr_t;

/* TYPE_ENUM (falls under scalar) */
typedef GTY(()) enum color {
    RED,
    GREEN,
    BLUE
} color_enum_t;

/* TYPE_STRUCT: Basic structure */
struct GTY(()) basic_struct {
    int x;
    float y;
    char* name;
};

/* TYPE_USER_STRUCT: More complex user-defined structure */
struct GTY(()) user_struct {
    struct basic_struct* GTY((skip)) ptr;
    int GTY(()) data[10];
    color_enum_t GTY(()) color;
};

/* TYPE_UNION: Union type */
union GTY(()) data_union {
    int i;
    float f;
    char* GTY(()) str;
    struct basic_struct* GTY(()) bs;
};

/* TYPE_POINTER: Various pointer types */
typedef GTY(()) struct basic_struct* struct_ptr_t;
typedef GTY(()) union data_union* union_ptr_t;
typedef GTY(()) void* generic_ptr_t;

/* TYPE_ARRAY: Array types */
typedef GTY(()) int int_array_t[100];
typedef GTY(()) struct basic_struct struct_array_t[50];
typedef GTY(()) union data_union* ptr_array_t[20];

/* TYPE_CALLBACK: Function pointer types */
typedef GTY(()) void (*callback_func_t)(int, void*);
typedef GTY(()) int (*comparator_t)(const void*, const void*);

/* TYPE_LANG_STRUCT: More complex nested structure that might be treated specially */
struct GTY(()) lang_compatible_struct {
    struct GTY(()) inner {
        int depth;
        struct inner* GTY(()) next;
    } *head;
    
    callback_func_t GTY(()) handler;
    int_array_t GTY(()) buffer;
};

/* TYPE_UNDEFINED: Forward declarations that create undefined types */
struct GTY(()) forward_declared_struct;
typedef struct forward_declared_struct* forward_ptr_t;

/* Now define it later to resolve */
struct GTY(()) forward_declared_struct {
    int value;
    forward_ptr_t GTY(()) next;
};

/* ==================== COMPLEX NESTED TYPES ==================== */

/* A complex type that mixes everything */
struct GTY(()) super_complex_type {
    /* Scalar members */
    scalar_int_t id;
    scalar_float_t weight;
    
    /* String member */
    string_ptr_t GTY(()) description;
    
    /* Struct member */
    struct user_struct GTY(()) config;
    
    /* Union member */
    union data_union GTY(()) variant;
    
    /* Pointer members */
    struct_ptr_t GTY(()) parent;
    generic_ptr_t GTY(()) user_data;
    
    /* Array member */
    int_array_t GTY(()) scores;
    
    /* Callback member */
    callback_func_t GTY(()) notify;
    
    /* Nested struct */
    struct GTY(()) metadata {
        int version;
        char* GTY(()) author;
    } meta;
    
    /* Pointer to forward declared type */
    forward_ptr_t GTY(()) chain;
};

/* ==================== GLOBAL VARIABLES ==================== */

/* Global variables to force type instantiation */
RETAIN scalar_int_t global_scalar = 42;
RETAIN string_ptr_t global_string = "Hello, gengtype!";
RETAIN struct basic_struct global_struct = {1, 3.14, "test"};
RETAIN union data_union global_union;
RETAIN struct_ptr_t global_struct_ptr = &global_struct;
RETAIN int_array_t global_array = {1, 2, 3, 4, 5};
RETAIN callback_func_t global_callback = NULL;
RETAIN struct lang_compatible_struct global_lang_struct = {0};
RETAIN struct super_complex_type global_complex = {0};

/* Array of pointers to different types */
RETAIN void* GTY(()) type_ptrs[] = {
    &global_scalar,
    &global_string,
    &global_struct,
    &global_union,
    &global_struct_ptr,
    &global_array,
    &global_callback,
    &global_lang_struct,
    &global_complex,
    NULL
};

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Function using many types to force analysis */
static void GTY(()) process_types(void* GTY(()) data) {
    /* This function's signature itself creates callback types */
    struct super_complex_type* GTY(()) sc = data;
    if (sc) {
        /* Access members to ensure they're used */
        sc->id = 100;
        if (sc->notify) {
            sc->notify(sc->id, sc->user_data);
        }
    }
}

/* Another file to create multiple translation units */
#ifdef MULTI_TU
/* Second translation unit with different types */
struct GTY(()) tu2_struct {
    int tu2_field;
    struct super_complex_type* GTY(()) ref;
};

RETAIN struct tu2_struct tu2_global = {0};
#else
/* Main program continues */
#endif

/* ==================== MAIN PROGRAM ==================== */

#ifndef MULTI_TU
/* Use builtins that require type analysis */
#define ANALYZE_TYPE(t) __builtin_clear_padding(&t)

int main() {
    /* Force analysis of each type through various means */
    
    /* 1. Use sizeof on GC types */
    size_t sizes[] = {
        sizeof(scalar_int_t),
        sizeof(struct basic_struct),
        sizeof(union data_union),
        sizeof(struct_ptr_t),
        sizeof(int_array_t),
        sizeof(callback_func_t),
        sizeof(struct lang_compatible_struct),
        sizeof(struct super_complex_type)
    };
    
    /* 2. Take addresses of GC variables */
    void* addresses[] = {
        &global_scalar,
        &global_string,
        &global_struct,
        &global_union,
        &global_struct_ptr,
        &global_array,
        &global_callback,
        &global_lang_struct,
        &global_complex,
        type_ptrs
    };
    
    /* 3. Use __builtin_clear_padding which requires type layout */
    ANALYZE_TYPE(global_struct);
    ANALYZE_TYPE(global_complex);
    
    /* 4. Create a checksum from addresses to prevent optimization */
    unsigned long checksum = 0;
    for (int i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        checksum ^= (unsigned long)addresses[i];
    }
    
    for (int i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        checksum ^= sizes[i];
    }
    
    /* Simple output to prevent dead code elimination */
    if (checksum != 0) {
        /* Use the types in a way that can't be optimized away */
        global_struct_ptr = &global_struct;
        if (global_string) {
            /* Do nothing, just reference it */
        }
    }
    
    return 0;
}
#endif
