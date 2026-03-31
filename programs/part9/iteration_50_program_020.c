/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* TYPE_STRING example */
const char* greeting = "Hello, gengtype!";

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) packed_struct {
    my_int id;
    color_t color;
    char name[20];
    volatile int status;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct packed_struct user_struct_t;

/* TYPE_UNION with GCC extensions */
union __attribute__((transparent_union)) data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_str[16];
};

/* TYPE_ARRAY examples */
int matrix[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
user_struct_t struct_array[5];
const volatile char cv_buffer[100];

/* TYPE_POINTER examples */
int* int_ptr;
struct packed_struct* struct_ptr;
void* generic_ptr;
int (*array_ptr)[3];
const volatile int* const cv_ptr = (const volatile int*)0x1000;

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void*, const void*);
typedef user_struct_t* (*factory_t)(int id, color_t c);

/* Complex nested type for deep traversal */
struct nested_container {
    union data_union data;
    struct packed_struct* items;
    int (*sort)(struct packed_struct*, size_t, comparator_t);
    factory_t create_item;
    struct nested_container* next;
};

/* GCC-specific pragma for alignment */
#pragma pack(push, 1)
struct gcc_packed {
    char flag;
    int value;
    double data;
} __attribute__((aligned(16)));
#pragma pack(pop)

/* More complex type chains */
typedef union data_union* union_ptr_t;
typedef union_ptr_t (*getter_t)(struct nested_container*);

/* Global instances */
struct packed_struct global_struct = {1, RED, "test", 0};
union data_union global_union = {.as_int = 42};
struct nested_container* container_chain;
comparator_t global_comparator = NULL;
getter_t global_getter = NULL;
struct gcc_packed packed_instance;

/* Function using callback */
static int compare_ids(const void* a, const void* b) {
    const struct packed_struct* pa = a;
    const struct packed_struct* pb = b;
    return pa->id - pb->id;
}

/* Factory function */
static user_struct_t* create_packed_struct(int id, color_t c) {
    static user_struct_t instance;
    instance.id = id;
    instance.color = c;
    instance.status = 1;
    return &instance;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    my_int counter = 0;
    my_float temperature = 23.5f;
    color_t current_color = GREEN;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use struct */
    global_struct.id = 100;
    global_struct.status = prevent_optimization;
    
    /* Use union */
    global_union.as_float = 3.14f;
    
    /* Use arrays */
    matrix[0][0] = counter;
    cv_buffer[0] = 'A';
    
    /* Use pointers */
    int_ptr = &counter;
    struct_ptr = &global_struct;
    generic_ptr = &global_union;
    
    /* Use callback */
    global_comparator = compare_ids;
    if (global_comparator) {
        prevent_optimization += 1;
    }
    
    /* Use factory */
    factory_t factory = create_packed_struct;
    user_struct_t* new_struct = factory(2, BLUE);
    if (new_struct) {
        prevent_optimization += new_struct->id;
    }
    
    /* Use nested types */
    struct nested_container local_container = {
        .data = {.as_int = 99},
        .items = &global_struct,
        .sort = NULL,
        .create_item = create_packed_struct,
        .next = NULL
    };
    
    /* Use packed struct */
    packed_instance.flag = 'X';
    packed_instance.value = prevent_optimization;
    
    /* Use function pointer chain */
    if (global_getter) {
        union_ptr_t up = global_getter(&local_container);
        if (up) prevent_optimization += up->as_int;
    }
    
    /* Ensure all arrays are referenced */
    for (int i = 0; i < 5; i++) {
        struct_array[i].id = i;
    }
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional declarations in file scope */
static const char* const messages[] = {
    "Type",
    "coverage",
    "test",
    NULL  /* TYPE_ARRAY of TYPE_STRING */
};

/* Opaque forward declaration for pointer testing */
struct opaque;
extern struct opaque* external_handle;

/* Multi-dimensional pointer array */
int* pointer_array[4][2];

/* Const callback */
typedef int (*const const_callback_t)(void);
const_callback_t const_cb = NULL;
