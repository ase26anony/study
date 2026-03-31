/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRING examples ========== */
const char* string_literal = "Hello, gengtype!";
static const char static_string[] = "Static string";

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* With GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) attributed_struct {
    volatile int counter;
    const char* name;
    double values[3];
};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;
typedef struct attributed_struct attributed_user_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
    int as_int;
    float as_float;
    char as_char[4];
    void* as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union {
    int* int_ptr;
    void* void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int* int_ptr;
struct plain_struct* struct_ptr;
void* void_ptr;
int** double_ptr;
volatile int* volatile volatile_ptr;
const char* const const_ptr = "constant";

/* ========== TYPE_ARRAY examples ========== */
int int_array[10];
float float_array[5][5];
char char_3d_array[2][3][4];
struct plain_struct struct_array[5];
union data_union union_array[8];

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*simple_callback)(int, float);
typedef void* (*complex_callback)(struct plain_struct*, union data_union);
typedef int (*callback_returning_ptr)(void)** callback_ptr_ptr;

/* Function pointer with attributes */
typedef void __attribute__((noreturn)) (*noreturn_callback)(void);

/* ========== Complex nested types ========== */
struct nested_container {
    /* Struct containing pointer to another struct */
    struct attributed_struct* attr_ptr;
    
    /* Array of unions */
    union data_union unions[4];
    
    /* Pointer to function pointer */
    simple_callback* callback_ptr;
    
    /* Multi-dimensional array of pointers */
    int* ptr_matrix[3][3];
    
    /* Const volatile qualified member */
    const volatile int cv_member;
};

union complex_union {
    struct nested_container container;
    struct {
        callback_returning_ptr cb;
        char flexible_array[];
    } anon_struct;
};

/* ========== GCC pragmas ========== */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* ========== Global variables ========== */
/* TYPE_STRUCT instances */
struct plain_struct global_struct = {1, 2.0f, 'A'};
struct attributed_struct global_attr_struct = {0, "test", {1.0, 2.0, 3.0}};

/* TYPE_USER_STRUCT instances */
user_struct_t global_user_struct = {2, 3.0f, 'B'};

/* TYPE_UNION instances */
union data_union global_union = {.as_int = 42};

/* TYPE_ARRAY instances with initializers */
int initialized_array[5] = {1, 2, 3, 4, 5};
struct plain_struct initialized_struct_array[2] = {
    {1, 1.0f, 'X'},
    {2, 2.0f, 'Y'}
};

/* TYPE_CALLBACK instances */
int sample_callback(int a, float b) { return a + (int)b; }
simple_callback global_callback = sample_callback;

/* Complex global */
struct nested_container global_container = {
    .attr_ptr = &global_attr_struct,
    .unions = {{.as_int = 1}, {.as_float = 2.0f}},
    .cv_member = 99
};

/* ========== External declarations ========== */
extern int external_int;
extern void external_function(void);

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use TYPE_SCALAR */
    scalar_int_t scalar = 10;
    prevent_optimization += scalar;
    
    /* Use TYPE_STRING */
    prevent_optimization += string_literal[0];
    
    /* Use TYPE_STRUCT */
    global_struct.x = 20;
    prevent_optimization += global_struct.x;
    
    /* Use TYPE_USER_STRUCT */
    global_user_struct.y = 30.0f;
    prevent_optimization += (int)global_user_struct.y;
    
    /* Use TYPE_UNION */
    global_union.as_float = 40.0f;
    prevent_optimization += (int)global_union.as_float;
    
    /* Use TYPE_POINTER */
    int_ptr = &scalar;
    prevent_optimization += *int_ptr;
    
    /* Use TYPE_ARRAY */
    int_array[0] = 50;
    prevent_optimization += int_array[0];
    
    /* Use TYPE_CALLBACK */
    if (global_callback) {
        prevent_optimization += global_callback(1, 2.0f);
    }
    
    /* Use complex nested types */
    global_container.ptr_matrix[0][0] = &scalar;
    prevent_optimization += *global_container.ptr_matrix[0][0];
    
    /* Use volatile/const qualified types */
    volatile_ptr = &prevent_optimization;
    prevent_optimization += *volatile_ptr;
    
    /* Use array of structs */
    initialized_struct_array[0].z = 'Z';
    prevent_optimization += initialized_struct_array[0].z;
    
    /* Use union array */
    union_array[0].as_int = 100;
    prevent_optimization += union_array[0].as_int;
    
    /* Use multi-dimensional array */
    float_array[0][0] = 3.14f;
    prevent_optimization += (int)float_array[0][0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional function using callback type */
void use_callback_types(void) {
    /* Function pointer in struct */
    struct with_callback {
        simple_callback cb;
    } s = {sample_callback};
    
    /* Callback returning pointer */
    complex_callback get_ptr = NULL;
    
    /* Pointer to callback */
    simple_callback* cb_ptr = &global_callback;
    
    /* Avoid unused warnings */
    (void)s;
    (void)get_ptr;
    (void)cb_ptr;
}

/* Empty file-scope struct for potential TYPE_UNDEFINED */
struct forward_declared;
struct forward_declared* forward_ptr = NULL;

/* Array of pointers to different types */
void* void_ptr_array[] = {
    &global_struct,
    &global_union,
    string_literal,
    (void*)global_callback
};
