/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int my_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef float my_float;

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(4))) packed_struct {
    my_int a;
    char b;
    float c;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    int y;
} point_t;

/* Another struct with complex nesting */
struct container {
    point_t point;
    struct packed_struct* packed_ptr;
    int data[5];
};

/* TYPE_UNION */
union data_union {
    int i;
    float f;
    char str[4];
    void* ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) trans_union {
    int* int_ptr;
    void* void_ptr;
} trans_union_t;

/* TYPE_ARRAY with multiple dimensions */
int multi_array[3][4][5];
char string_array[][10] = {"hello", "world", "test"};

/* TYPE_STRING - string literals in initializers */
const char* messages[] = {
    "Error message 1",
    "Warning message 2",
    "Info message 3"
};

/* TYPE_POINTER variations */
int* int_ptr;
volatile int* volatile volatile_int_ptr;
const char* const const_string_ptr = "constant string";
void** void_ptr_ptr;
struct container* container_ptr;
int (*array_ptr)[10];

/* TYPE_CALLBACK - function pointers */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef struct container* (*factory_t)(void);

/* Complex nested callback */
typedef void (*event_handler_t)(int event, void* data, callback_t completion);

/* Struct with all types combined */
struct mega_struct {
    /* TYPE_SCALAR */
    my_int id;
    color_enum color;
    
    /* TYPE_STRUCT */
    point_t position;
    
    /* TYPE_UNION */
    union data_union value;
    
    /* TYPE_ARRAY */
    float matrix[3][3];
    
    /* TYPE_POINTER */
    struct mega_struct* next;
    void* user_data;
    
    /* TYPE_CALLBACK */
    callback_t notify;
    
    /* TYPE_STRING */
    const char* name;
};

/* Global variables to ensure gengtype sees them */
struct packed_struct global_packed = {1, 'A', 3.14f};
point_t global_point = {10, 20};
union data_union global_union = {.i = 42};
struct container global_container = {
    .point = {5, 6},
    .packed_ptr = &global_packed,
    .data = {1, 2, 3, 4, 5}
};
struct mega_struct global_mega = {
    .id = 100,
    .color = GREEN,
    .position = {15, 25},
    .value = {.f = 2.718f},
    .matrix = {{1,0,0},{0,1,0},{0,0,1}},
    .next = NULL,
    .user_data = &global_point,
    .notify = NULL,
    .name = "MegaStruct"
};

/* Function pointer variables */
comparator_t global_comparator = NULL;
event_handler_t global_handler = NULL;

/* Array of pointers */
void* ptr_array[10];

/* Const volatile qualified pointer */
volatile const int* const volatile crazy_ptr = (volatile const int*)0x1000;

/* Using #pragma pack for potential TYPE_LANG_STRUCT */
#pragma pack(push, 1)
struct packed_explicit {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Function using the types */
static int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct types */
    global_point.x = 30;
    global_point.y = 40;
    prevent_optimization += global_point.x;
    
    /* Use union */
    global_union.i = 100;
    prevent_optimization += global_union.i;
    
    /* Use array */
    multi_array[0][0][0] = 1;
    prevent_optimization += multi_array[0][0][0];
    
    /* Use pointers */
    int local_int = 50;
    int_ptr = &local_int;
    prevent_optimization += *int_ptr;
    
    /* Use string */
    prevent_optimization += messages[0][0];
    
    /* Use callback */
    global_comparator = compare_ints;
    int nums[2] = {5, 3};
    if (global_comparator) {
        prevent_optimization += global_comparator(&nums[0], &nums[1]);
    }
    
    /* Use function pointer directly */
    int result = 0;
    callback_t cb = sample_callback;
    if (cb) {
        cb(21, &result);
        prevent_optimization += result;
    }
    
    /* Use nested struct */
    global_mega.position.x = 99;
    prevent_optimization += global_mega.position.x;
    
    /* Use array of pointers */
    ptr_array[0] = &local_int;
    prevent_optimization += *(int*)ptr_array[0];
    
    /* Use packed struct */
    struct packed_explicit packed = {'X', 123, 456};
    prevent_optimization += packed.b;
    
    /* Use transparent union */
    trans_union_t tu;
    int tu_value = 999;
    tu.int_ptr = &tu_value;
    prevent_optimization += *tu.int_ptr;
    
    return prevent_optimization != 0 ? 0 : 1;
}
