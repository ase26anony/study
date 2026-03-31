/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ===== TYPE_SCALAR examples ===== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ===== TYPE_STRING example ===== */
const char *string_literal = "Hello, gengtype!";

/* ===== TYPE_STRUCT examples ===== */
struct plain_struct {
    int id;
    float value;
    char name[32];
    volatile int counter;
};

/* Struct with GCC attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) attributed_struct {
    char data;
    int number;
    double precision;
} __attribute__((aligned(16)));

/* ===== TYPE_USER_STRUCT example ===== */
typedef struct plain_struct user_struct_t;

/* ===== TYPE_UNION examples ===== */
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

/* ===== TYPE_POINTER examples ===== */
int *int_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;
void *void_ptr;
struct plain_struct *struct_ptr;
int **double_ptr;
volatile int *const volatile_ptr_const = (volatile int*)0x1000;

/* Function pointer type (TYPE_CALLBACK) */
typedef int (*callback_t)(int, float, void*);
typedef void (*simple_callback)(void);

/* ===== TYPE_ARRAY examples ===== */
int int_array[10];
float float_array[5][5];
char char_3d_array[2][3][4];
struct plain_struct struct_array[5];
union basic_union union_array[8];
callback_t callback_array[3];

/* ===== Complex nested types ===== */
struct nested_container {
    /* Pointer to struct */
    struct plain_struct *child_struct;
    
    /* Array of pointers */
    int *ptr_array[5];
    
    /* Pointer to array */
    float (*array_ptr)[5];
    
    /* Nested union */
    union {
        int option_a;
        struct {
            char code;
            int value;
        } option_b;
    } choice;
    
    /* Function pointer member */
    callback_t handler;
    
    /* Pointer to function pointer */
    callback_t *handler_ptr;
    
    /* Array of structs containing unions */
    struct {
        int tag;
        union {
            int num;
            char str[16];
        } data;
    } variant_array[4];
};

/* ===== TYPE_LANG_STRUCT triggers ===== */
/* Use #pragma pack which may generate special type handling */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

/* Struct with vector attribute (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

struct with_vector {
    v4si vec_data;
    int regular;
};

/* ===== Global variable definitions ===== */
scalar_int_t global_int = 42;
scalar_float_t global_float = 3.14159f;
color_enum global_color = GREEN;

struct plain_struct global_struct = {
    .id = 1,
    .value = 2.71828f,
    .name = "test",
    .counter = 0
};

union basic_union global_union = { .as_int = 255 };

struct nested_container global_container = {
    .child_struct = &global_struct,
    .handler = NULL,
    .choice.option_a = 100
};

/* Initialize array elements */
int initialized_array[5] = {1, 2, 3, 4, 5};
struct plain_struct initialized_structs[2] = {
    { .id = 10, .value = 1.0f, .name = "first", .counter = 0 },
    { .id = 20, .value = 2.0f, .name = "second", .counter = 0 }
};

/* ===== Function using callback ===== */
static int sample_callback(int a, float b, void *c) {
    return a + (int)b + (c != NULL);
}

/* Another callback type */
typedef struct plain_struct* (*struct_factory_t)(int id);

static struct plain_struct* create_struct(int id) {
    static struct plain_struct instance;
    instance.id = id;
    return &instance;
}

/* ===== Main function to ensure all types are referenced ===== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    scalar_int_t local_int = global_int + 1;
    scalar_float_t local_float = global_float * 2.0f;
    color_enum local_color = global_color;
    
    /* Use string */
    prevent_optimization += (string_literal[0] == 'H');
    
    /* Use struct */
    global_struct.counter++;
    struct plain_struct local_struct = global_struct;
    
    /* Use union */
    union basic_union local_union = global_union;
    local_union.as_float = 3.14f;
    
    /* Use pointers */
    int_ptr = &local_int;
    struct_ptr = &local_struct;
    void_ptr = (void*)&local_union;
    
    /* Use arrays */
    int_array[0] = local_int;
    float_array[1][1] = local_float;
    char_3d_array[0][0][0] = 'X';
    
    /* Use nested container */
    global_container.child_struct = &global_struct;
    global_container.ptr_array[0] = &local_int;
    global_container.handler = sample_callback;
    
    /* Use callback */
    if (global_container.handler) {
        int result = global_container.handler(1, 2.0f, &local_struct);
        prevent_optimization += result;
    }
    
    /* Use function pointer array */
    callback_array[0] = sample_callback;
    
    /* Use transparent union */
    transparent_union_t tu;
    int special_value = 999;
    tu.int_ptr = &special_value;
    
    /* Use packed struct */
    struct packed_struct packed;
    packed.a = 'Z';
    packed.b = 123;
    
    /* Use vector type */
    struct with_vector vec_struct;
    vec_struct.regular = 456;
    
    /* Ensure everything is used */
    return prevent_optimization == 0 ? 0 : 1;
}

/* ===== External declarations (simulating multi-file) ===== */
extern int external_int;
extern struct plain_struct external_struct;

/* Forward declaration for circular reference */
struct forward_declared;
struct circular_struct {
    struct forward_declared *next;
};

struct forward_declared {
    int data;
    struct circular_struct *prev;
};
