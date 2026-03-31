/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING example */
const char* greeting = "Hello, gengtype!";

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) packed_struct {
    scalar_int id;
    scalar_float value;
    char name[32];
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    int y;
    struct packed_struct* next;  /* TYPE_POINTER inside */
} point2d;

/* TYPE_UNION with volatile qualifier */
union data_union {
    volatile int as_int;
    float as_float;
    void* as_pointer;
    char as_bytes[8];
};

/* TYPE_ARRAY examples */
int int_array[10];
char char_matrix[5][5];
point2d point_array[4];

/* Complex nested TYPE_STRUCT */
struct nested_container {
    struct packed_struct header;
    union data_union payload;
    point2d points[3];
    void* (*processor)(void*);  /* TYPE_CALLBACK */
};

/* TYPE_POINTER variations */
int* int_ptr;
const int* const_int_ptr;
volatile int* volatile_int_ptr;
const volatile int* const_volatile_int_ptr;
struct nested_container* container_ptr;
void* void_ptr;

/* Function pointer TYPE_CALLBACK */
typedef int (*comparator_t)(const void*, const void*);
typedef void* (*allocator_t)(size_t);
typedef struct nested_container* (*factory_t)(int);

/* GCC-specific pragma for potential TYPE_LANG_STRUCT */
#pragma pack(push, 1)
struct gcc_packed {
    char flag;
    int count;
    double precision;
} __attribute__((aligned(16)));
#pragma pack(pop)

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* generic_ptr;
} transparent_union_t;

/* Complex chain of types */
struct chain_node {
    int data;
    struct chain_node* next;  /* Self-referential pointer */
    union data_union variant;
};

/* Array of pointers to unions */
union data_union* union_ptr_array[5];

/* Const pointer to array */
int (*const array_ptr_const)[10];

/* Volatile function pointer */
void (*volatile signal_handler)(int);

/* Global variables using all types */
scalar_int global_int = 42;
scalar_float global_float = 3.14159f;
color_enum global_color = GREEN;
struct packed_struct global_packed = {1, 2.5f, "test"};
point2d global_point = {10, 20, &global_packed};
union data_union global_union = {.as_int = 100};
struct nested_container global_container;
comparator_t global_comparator = NULL;
allocator_t global_allocator = NULL;
struct gcc_packed global_gcc_packed = {'A', 99, 45.67};
transparent_union_t global_transparent;
struct chain_node global_chain = {0, NULL, {.as_int = 0}};

/* Initialize arrays */
int global_int_array[5] = {1, 2, 3, 4, 5};
char global_char_matrix[2][3] = {{'a', 'b', 'c'}, {'d', 'e', 'f'}};

/* String array */
const char* messages[] = {"msg1", "msg2", "msg3"};

/* Function using callback */
void register_callback(void* (*cb)(void*)) {
    global_container.processor = cb;
}

/* Simple functions for callbacks */
static void* simple_alloc(size_t size) {
    return (void*)(size_t)size;  /* Simplified for example */
}

static int int_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int += 1;
    global_float *= 2.0f;
    prevent_optimization += global_color;
    
    /* Use struct types */
    global_point.x = global_int;
    global_packed.value = global_float;
    prevent_optimization += global_packed.id;
    
    /* Use union */
    global_union.as_float = 3.14f;
    prevent_optimization += (int)global_union.as_float;
    
    /* Use arrays */
    int_array[0] = global_int;
    char_matrix[0][0] = 'X';
    prevent_optimization += int_array[0] + char_matrix[0][0];
    
    /* Use pointers */
    int_ptr = &global_int;
    container_ptr = &global_container;
    prevent_optimization += *int_ptr;
    
    /* Use function pointers */
    global_allocator = simple_alloc;
    global_comparator = int_comparator;
    
    if (global_allocator) {
        void* ptr = global_allocator(100);
        prevent_optimization += (int)(size_t)ptr;
    }
    
    /* Initialize and use nested container */
    global_container.header = global_packed;
    global_container.points[0] = global_point;
    register_callback(simple_alloc);
    
    /* Use GCC-specific struct */
    global_gcc_packed.count = prevent_optimization;
    
    /* Use transparent union */
    global_transparent.int_ptr = &global_int;
    
    /* Build a chain */
    struct chain_node node2 = {2, NULL, {.as_int = 2}};
    global_chain.next = &node2;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use volatile pointer */
    if (signal_handler) {
        /* Would call handler if set */
    }
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional type in separate "translation unit" section */
extern struct external_type {
    long long big_int;
    _Complex double complex_val;
} external_var;

/* Force undefined reference for gengtype to see */
struct undefined_struct;
typedef struct undefined_struct* undefined_ptr_t;

/* Multiple levels of pointer indirection */
typedef int**** complex_ptr_t;

/* Array of function pointers */
void (*func_array[5])(void);

/* Const method pointer (C++ like, but in C) */
typedef int (*const const_method_ptr)(void);
