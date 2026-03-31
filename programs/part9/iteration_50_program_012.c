/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int my_int;
typedef float my_float;
typedef enum { RED, GREEN, BLUE } color_enum;

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

/* TYPE_UNION with volatile member */
union data_union {
    int i;
    float f;
    char str[4];
    volatile long vl;
};

/* TYPE_ARRAY examples */
int int_array[10];
char multi_dim[5][5];
const char const_array[20];

/* TYPE_POINTER examples */
int *int_ptr;
void *void_ptr;
struct packed_struct *struct_ptr;
int **double_ptr;
volatile int * volatile volatile_ptr;

/* TYPE_STRING via string literal */
const char *greeting = "Hello, gengtype!";

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, char *);

/* Complex nested type for deep traversal */
struct nested_container {
    struct packed_struct inner;
    union data_union data;
    point_t *points;
    int (*compare)(struct nested_container *, struct nested_container *);
};

/* GCC-specific pragma */
#pragma pack(push, 1)
struct gcc_packed {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *vp;
} transparent_union_t;

/* Another struct with function pointer array */
struct with_callbacks {
    callback_t callbacks[3];
    comparator_t cmp;
};

/* Global variables to ensure gengtype processes them */
struct packed_struct global_packed = {1, 'a', 3.14f};
point_t global_point = {10, 20};
union data_union global_union = {.i = 42};
struct nested_container global_container;
struct gcc_packed global_gcc_packed = {'x', 100, 50};
transparent_union_t global_transparent;
struct with_callbacks global_callbacks;

/* Function using callback */
static int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int val, char *msg) {
    /* Do nothing for test */
    (void)val;
    (void)msg;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_packed.a = 2;
    prevent_optimization += global_packed.a;
    
    /* Use user struct */
    global_point.x++;
    prevent_optimization += global_point.x;
    
    /* Use union */
    global_union.f = 3.14159f;
    prevent_optimization += (int)global_union.f;
    
    /* Use arrays */
    int_array[0] = 100;
    prevent_optimization += int_array[0];
    multi_dim[2][2] = 'Z';
    
    /* Use pointers */
    int_ptr = &int_array[0];
    prevent_optimization += *int_ptr;
    
    struct_ptr = &global_packed;
    prevent_optimization += struct_ptr->a;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use function pointers */
    global_callbacks.cmp = sample_comparator;
    global_callbacks.callbacks[0] = sample_callback;
    
    if (global_callbacks.cmp) {
        int x = 5, y = 10;
        prevent_optimization += global_callbacks.cmp(&x, &y);
    }
    
    /* Use nested container */
    global_container.inner = global_packed;
    global_container.points = &global_point;
    
    /* Use GCC packed struct */
    global_gcc_packed.a = 'y';
    prevent_optimization += global_gcc_packed.a;
    
    /* Use transparent union */
    global_transparent.intp = int_ptr;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in file scope for more coverage */
static struct {
    int hidden;
    union {
        long l;
        double d;
    } secret;
} file_scope_var = {0, {.l = 123456789}};

/* Array of function pointers */
static int (*func_ptr_array[2])(void) = {NULL, NULL};

/* Const pointer to volatile int */
volatile int * const const_ptr_to_volatile = (volatile int*)0x1000;

/* Complex type chain */
typedef struct chain_node {
    struct chain_node *next;
    struct chain_node *prev;
    void *data;
} chain_node_t;

chain_node_t chain_head = {NULL, NULL, NULL};
