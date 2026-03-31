/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* TYPE_STRUCT with attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) packed_struct {
    int a;
    char b;
    double c;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    int y;
} point_t;

/* TYPE_UNION with volatile qualifier */
union data_union {
    int i;
    float f;
    char str[4];
    volatile long vl;
};

/* TYPE_ARRAY examples */
int int_array[10];
char multi_array[5][5];
const char const_array[20];

/* TYPE_STRING usage */
const char *greeting = "Hello, gengtype!";
char *messages[] = {"msg1", "msg2", "msg3"};

/* TYPE_POINTER examples */
int *int_ptr;
void *void_ptr;
struct packed_struct *struct_ptr;
int **double_ptr;
volatile int *volatile volatile_ptr;
int *const const_ptr = &int_array[0];

/* TYPE_CALLBACK - function pointer */
typedef int (*callback_t)(int, void*);
typedef void (*simple_cb)(void);

/* Complex nested type definitions */
struct container {
    struct packed_struct ps;
    point_t *points;
    union data_union data;
    callback_t handler;
    int (*compute)(int, int);
    char buffer[256];
    struct container *next;
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
typedef union __attribute__((transparent_union)) trans_union {
    int i;
    long l;
} trans_union_t;

/* Another struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int count : 28;
};

/* Array of function pointers */
callback_t callbacks[5];

/* Complex chain of types */
typedef struct node {
    int value;
    struct node *children[4];
    union {
        int int_val;
        float float_val;
    } data;
    enum color color;
} node_t;

/* Global variables using all types */
struct packed_struct global_packed = {1, 'A', 3.14};
point_t global_point = {10, 20};
union data_union global_union = {.i = 42};
struct container global_container;
struct gcc_packed global_gcc_packed = {'X', 100, 50};
struct bitfield_struct global_bitfield = {1, 3, 255};
node_t *global_node = NULL;
trans_union_t global_trans = {.i = 99};

/* Function pointer variable */
callback_t global_callback = NULL;

/* String array */
const char *string_array[] = {
    "first",
    "second",
    "third",
    NULL  /* TYPE_UNDEFINED? NULL pointer might trigger this */
};

/* External declaration (simulating multi-file) */
extern int external_var;

/* Function using callback */
static int sample_callback(int x, void *data) {
    return x * 2;
}

/* Function returning pointer to struct */
struct container* get_container(void) {
    return &global_container;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct types */
    global_point.x = 100;
    global_point.y = 200;
    prevent_optimization += global_point.x;
    
    /* Use union */
    global_union.f = 3.14159f;
    prevent_optimization += (int)global_union.f;
    
    /* Use array */
    int_array[0] = 42;
    prevent_optimization += int_array[0];
    
    /* Use pointers */
    int_ptr = &int_array[0];
    prevent_optimization += *int_ptr;
    
    /* Use function pointer */
    global_callback = sample_callback;
    if (global_callback) {
        prevent_optimization += global_callback(5, NULL);
    }
    
    /* Use nested struct */
    global_container.ps.a = 10;
    global_container.handler = sample_callback;
    prevent_optimization += global_container.ps.a;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use multi-dimensional array */
    multi_array[0][0] = 'Z';
    prevent_optimization += multi_array[0][0];
    
    /* Use bitfield */
    global_bitfield.flag1 = 1;
    prevent_optimization += global_bitfield.count;
    
    /* Use transparent union */
    trans_union_t tu = {.i = 123};
    prevent_optimization += tu.i;
    
    /* Use GCC packed struct */
    global_gcc_packed.a = 'Y';
    prevent_optimization += global_gcc_packed.b;
    
    /* Create node chain */
    node_t nodes[2];
    nodes[0].value = 1;
    nodes[0].color = RED;
    nodes[0].data.int_val = 100;
    nodes[1].value = 2;
    nodes[0].children[0] = &nodes[1];
    prevent_optimization += nodes[0].value;
    
    /* Use double pointer */
    int val = 42;
    int *ptr = &val;
    double_ptr = &ptr;
    prevent_optimization += **double_ptr;
    
    /* Use const pointer */
    prevent_optimization += *const_ptr;
    
    /* Use volatile pointer */
    volatile int vol_val = 99;
    volatile_ptr = &vol_val;
    prevent_optimization += *volatile_ptr;
    
    /* Use array of callbacks */
    callbacks[0] = sample_callback;
    if (callbacks[0]) {
        prevent_optimization += callbacks[0](3, NULL);
    }
    
    /* Use string array */
    prevent_optimization += string_array[0][0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in file scope */
static struct {
    int hidden;
    char secret[10];
} file_scope_struct = {42, "secret"};

/* Function with complex return type */
const volatile int* complex_func(void) {
    static volatile int sv = 999;
    return &sv;
}
