/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct plain_struct user_struct_t;

/* Struct with GCC attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) attributed_struct {
    int id;
    char name[32];
    void *data;
};

/* ========== TYPE_UNION examples ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int *int_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;
int *const const_int_ptr;
volatile int *const const_volatile_int_ptr;

/* Complex pointer types */
struct plain_struct *struct_ptr;
union basic_union *union_ptr;
void (*function_ptr)(void);
int (*array_of_function_ptrs[5])(void);

/* ========== TYPE_ARRAY examples ========== */
int int_array[10];
float float_array[5][5];
char char_3d_array[3][4][5];
struct plain_struct struct_array[2];
union basic_union union_array[8];

/* String literal in initializer (TYPE_STRING) */
const char *string_literal = "Hello, gengtype!";
char string_array[] = "Another string";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*callback_t)(int, float);
typedef void (*complex_callback_t)(struct plain_struct*, union basic_union*);

/* Function pointer returning pointer to struct */
typedef struct attributed_struct* (*struct_factory_t)(int);

/* ========== Complex nested types ========== */
struct nested_container {
    /* TYPE_STRUCT member */
    struct plain_struct inner_struct;
    
    /* TYPE_POINTER to union */
    union basic_union *union_ptr;
    
    /* TYPE_ARRAY of pointers */
    int *ptr_array[7];
    
    /* TYPE_CALLBACK */
    callback_t handler;
    
    /* Nested array of structs */
    struct {
        int tag;
        float value;
    } anonymous_struct_array[4];
};

/* Union with nested struct */
union complex_union {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
        } data;
    } tagged;
    
    struct nested_container container;
    callback_t callback;
};

/* ========== Global variable definitions ========== */
/* Ensure gengtype encounters concrete instances */
scalar_int global_int = 42;
scalar_float global_float = 3.14159f;
color_enum global_color = GREEN;

struct plain_struct global_struct = {1, 2.5f, 'A'};
user_struct_t global_user_struct = {2, 4.7f, 'B'};
struct attributed_struct global_attributed_struct = {100, "test", NULL};

union basic_union global_union = {.as_int = 255};
transparent_union_t global_transparent_union;

struct nested_container global_container = {
    .inner_struct = {3, 6.9f, 'C'},
    .union_ptr = &global_union,
    .ptr_array = {NULL, NULL, NULL, NULL, NULL, NULL, NULL},
    .handler = NULL,
    .anonymous_struct_array = {
        {0, 1.0f},
        {1, 2.0f},
        {2, 3.0f},
        {3, 4.0f}
    }
};

union complex_union global_complex_union = {
    .tagged = {
        .type = 1,
        .data = {.int_val = 999}
    }
};

/* Array initializations */
int global_int_array[5] = {1, 2, 3, 4, 5};
struct plain_struct global_struct_array[2] = {
    {10, 20.5f, 'X'},
    {20, 30.5f, 'Y'}
};

/* ========== Function declarations ========== */
int sample_callback(int a, float b) {
    return a + (int)b;
}

struct attributed_struct* create_struct(int id) {
    static struct attributed_struct instance;
    instance.id = id;
    return &instance;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int++;
    global_float *= 2.0f;
    prevent_optimization += global_color;
    
    /* Use struct types */
    global_struct.x++;
    global_user_struct.y += 1.0f;
    global_attributed_struct.id = 200;
    
    /* Use union types */
    global_union.as_float = 3.14f;
    global_transparent_union.int_ptr = &global_int;
    
    /* Use pointer types */
    if (int_ptr) (*int_ptr)++;
    if (struct_ptr) struct_ptr->x++;
    
    /* Use array types */
    global_int_array[0] = 100;
    global_struct_array[1].z = 'Z';
    
    /* Use string types */
    prevent_optimization += string_literal[0];
    prevent_optimization += string_array[0];
    
    /* Use callback types */
    callback_t cb = sample_callback;
    if (cb) prevent_optimization += cb(1, 2.0f);
    
    struct_factory_t factory = create_struct;
    if (factory) {
        struct attributed_struct *s = factory(300);
        prevent_optimization += s->id;
    }
    
    /* Use nested types */
    global_container.inner_struct.y = 7.8f;
    global_complex_union.container.handler = sample_callback;
    
    /* Use volatile/const qualified pointers */
    if (volatile_int_ptr) prevent_optimization++;
    if (const_string_ptr) prevent_optimization += const_string_ptr[0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* ========== Additional GCC-specific constructs ========== */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    char c;
};
#pragma pack(pop)

/* Struct with vector attribute (GCC extension) */
typedef int v4si __attribute__ ((vector_size (16)));

/* Struct with cleanup attribute */
typedef struct {
    int fd;
} file_handle __attribute__((cleanup));

/* Forward declaration to test TYPE_UNDEFINED? */
struct forward_declared_struct;

/* Opaque pointer type */
typedef struct opaque_struct *opaque_ptr_t;
