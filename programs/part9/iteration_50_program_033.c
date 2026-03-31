/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING example */
const char* greeting = "Hello, gengtype!";

/* TYPE_STRUCT with basic types */
struct plain_struct {
    int id;
    float value;
    char name[32];
    scalar_int custom_int;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    int y;
    struct plain_struct* nested;
} user_struct_t;

/* TYPE_UNION example */
union data_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

/* TYPE_ARRAY examples */
int int_array[10];
float float_matrix[5][5];
char* string_array[3];
struct plain_struct struct_array[4];

/* TYPE_POINTER examples */
int* int_ptr;
void* void_ptr;
struct plain_struct* struct_ptr;
union data_union* union_ptr;
int (*function_ptr)(void);
int** double_ptr;

/* TYPE_CALLBACK (function pointer) */
typedef int (*callback_t)(int, void*);
typedef void (*simple_callback)(void);

/* Complex nested type for deep traversal */
struct complex_nested {
    struct plain_struct base;
    union data_union data;
    user_struct_t* user;
    callback_t handler;
    int (*methods[3])(struct complex_nested*);
    volatile int volatile_member;
    const char* const_name;
};

/* GCC-specific attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    void* void_ptr;
};

/* Volatile and const qualified types */
volatile int volatile_int;
const int const_int = 42;
volatile int* volatile_ptr;
int* const const_ptr = &volatile_int;
const volatile int cv_int = 100;

/* Function pointer returning pointer to struct */
typedef struct plain_struct* (*struct_factory_t)(int);

/* Array of function pointers */
callback_t callback_array[5];

/* Nested array within struct */
struct with_nested_array {
    int matrix[3][3];
    union data_union variants[10];
    struct with_nested_array* next;
};

/* Forward declaration for circular reference */
struct forward_declared;
struct container {
    struct forward_declared* fwd;
    int data;
};

struct forward_declared {
    struct container* parent;
    char name[20];
};

/* Global variables using all types */
struct plain_struct global_struct = {1, 3.14f, "test", 42};
user_struct_t global_user_struct = {10, 20, &global_struct};
union data_union global_union = {.as_int = 100};
struct complex_nested global_complex;
struct packed_struct global_packed = {'A', 123, 456};
struct with_nested_array global_nested_array;

/* Function using callback */
static int sample_callback(int param, void* context) {
    return param + *(int*)context;
}

/* Function returning pointer */
static struct plain_struct* create_struct(int id) {
    static struct plain_struct local;
    local.id = id;
    return &local;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    scalar_int si = 10;
    scalar_float sf = 3.14f;
    color_enum ce = GREEN;
    prevent_optimization += si + (int)sf + ce;
    
    /* Use string */
    const char* local_greeting = greeting;
    prevent_optimization += local_greeting[0];
    
    /* Use struct */
    global_struct.id = 2;
    global_struct.value = 2.718f;
    prevent_optimization += global_struct.id;
    
    /* Use user struct */
    global_user_struct.x = 30;
    prevent_optimization += global_user_struct.x;
    
    /* Use union */
    global_union.as_float = 3.14159f;
    prevent_optimization += (int)global_union.as_float;
    
    /* Use arrays */
    int_array[0] = 1;
    float_matrix[0][0] = 1.0f;
    string_array[0] = "test";
    struct_array[0].id = 1;
    prevent_optimization += int_array[0] + (int)float_matrix[0][0];
    
    /* Use pointers */
    int_ptr = &si;
    struct_ptr = &global_struct;
    union_ptr = &global_union;
    prevent_optimization += *int_ptr;
    
    /* Use function pointer/callback */
    callback_t cb = sample_callback;
    int context = 5;
    int result = cb(10, &context);
    prevent_optimization += result;
    
    /* Use complex nested */
    global_complex.base.id = 99;
    global_complex.handler = cb;
    prevent_optimization += global_complex.base.id;
    
    /* Use packed struct */
    global_packed.a = 'B';
    prevent_optimization += global_packed.a;
    
    /* Use nested array struct */
    global_nested_array.matrix[0][0] = 7;
    prevent_optimization += global_nested_array.matrix[0][0];
    
    /* Use volatile/const */
    volatile_int = 50;
    prevent_optimization += volatile_int + const_int + cv_int;
    
    /* Use double pointer */
    int value = 100;
    int* ptr = &value;
    double_ptr = &ptr;
    prevent_optimization += **double_ptr;
    
    /* Use array of callbacks */
    callback_array[0] = cb;
    if (callback_array[0]) {
        prevent_optimization += callback_array[0](5, &context);
    }
    
    /* Create and use factory */
    struct_factory_t factory = create_struct;
    struct plain_struct* new_struct = factory(99);
    prevent_optimization += new_struct->id;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in file scope for more coverage */
static struct {
    int anonymous_member;
    callback_t handlers[2];
} anonymous_struct = {0, {NULL, NULL}};

/* External declaration to test cross-file references */
extern int external_function(void);

/* Pragmas for GCC-specific handling */
#pragma pack(push, 1)
struct pragma_packed {
    char a;
    int b;
};
#pragma pack(pop)

/* Multiple typedef chain */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 my_int3;

/* Opaque pointer type */
typedef struct opaque* opaque_handle_t;
struct uses_opaque {
    opaque_handle_t handle;
    int count;
};
