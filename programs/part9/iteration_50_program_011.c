/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ==================== BASIC TYPE DECLARATIONS ==================== */

/* TYPE_SCALAR examples */
typedef int my_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef float my_float;

/* TYPE_STRING example */
const char* greeting = "Hello, gengtype!";

/* TYPE_POINTER examples */
typedef int* int_ptr;
typedef void* generic_ptr;

/* ==================== STRUCT/UNION DECLARATIONS ==================== */

/* TYPE_STRUCT with attributes */
struct __attribute__((packed, aligned(8))) packed_struct {
    my_int id;
    char name[32];
    color_enum color;
    volatile int status;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x, y;
    float z;
} point_3d;

/* TYPE_UNION with volatile member */
union data_union {
    int int_val;
    float float_val;
    char char_val;
    volatile long volatile_val;
};

/* TYPE_ARRAY examples */
typedef int matrix[10][10];
typedef char string_array[5][64];

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator)(const void*, const void*);
typedef void (*callback_func)(int, void*);

/* ==================== COMPLEX NESTED TYPES ==================== */

/* Struct containing pointer to another struct */
struct node {
    int data;
    struct node* next;
    struct node* prev;
};

/* Union within struct */
struct variant_data {
    int type;
    union {
        int int_data;
        float float_data;
        char* string_data;
    } value;
};

/* Array of function pointers */
static callback_func callbacks[10];

/* Pointer to array */
typedef int (*array_ptr)[10];

/* ==================== GCC-SPECIFIC EXTENSIONS ==================== */

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int* int_ptr;
    void* generic_ptr;
} transparent_union_t;

/* Struct with vector attribute (GCC extension) */
struct vector_data {
    int size;
    float values[4] __attribute__((vector_size(16)));
};

/* Packed struct with bitfields */
struct __attribute__((packed)) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 5;
    int value : 24;
};

/* ==================== VARIABLE DEFINITIONS ==================== */

/* Global variables of each type */
struct packed_struct global_packed = {1, "test", GREEN, 0};
point_3d global_point = {10, 20, 3.14};
union data_union global_union = {.int_val = 42};
matrix global_matrix = {{0}};
string_array global_strings = {"one", "two", "three", "four", "five"};
struct node global_node = {100, NULL, NULL};
struct variant_data global_variant = {1, {.int_data = 999}};
transparent_union_t global_transparent = {.generic_ptr = NULL};
struct vector_data global_vector = {4, {1.0, 2.0, 3.0, 4.0}};
struct bitfield_struct global_bitfield = {1, 2, 10, 1000};

/* Pointer variables */
int_ptr global_int_ptr = NULL;
generic_ptr global_generic_ptr = &global_point;
array_ptr global_array_ptr = NULL;
struct node* global_node_ptr = &global_node;

/* Const and volatile qualified pointers */
const int* const_ptr = (const int*)&global_packed.id;
volatile int* volatile volatile_ptr = (volatile int*)&global_packed.status;
const volatile char* const_volatile_ptr = "constant";

/* Function pointer variable */
comparator global_comparator = NULL;

/* ==================== FUNCTION DECLARATIONS ==================== */

/* Callback function implementations */
int int_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void* data) {
    *(int*)data = value * 2;
}

/* Function returning pointer to struct */
struct node* create_node(int data) {
    static struct node static_node;
    static_node.data = data;
    return &static_node;
}

/* Function taking array parameter */
void process_matrix(int mat[10][10]) {
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            mat[i][j] = i * j;
        }
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_point.x = 100;
    global_point.y = 200;
    prevent_optimization += global_point.x;
    
    /* Use union */
    global_union.float_val = 3.14159f;
    prevent_optimization += (int)global_union.float_val;
    
    /* Use array */
    global_matrix[0][0] = 1;
    process_matrix(global_matrix);
    prevent_optimization += global_matrix[5][5];
    
    /* Use string array */
    prevent_optimization += global_strings[0][0];
    
    /* Use pointers */
    if (global_int_ptr) {
        prevent_optimization += *global_int_ptr;
    }
    
    global_node_ptr = create_node(42);
    prevent_optimization += global_node_ptr->data;
    
    /* Use function pointer */
    global_comparator = int_comparator;
    int a = 5, b = 10;
    if (global_comparator) {
        prevent_optimization += global_comparator(&a, &b);
    }
    
    /* Use callback */
    callbacks[0] = sample_callback;
    int result = 0;
    if (callbacks[0]) {
        callbacks[0](21, &result);
        prevent_optimization += result;
    }
    
    /* Use variant data */
    switch (global_variant.type) {
        case 1:
            prevent_optimization += global_variant.value.int_data;
            break;
    }
    
    /* Use GCC extension types */
    prevent_optimization += global_bitfield.flag1;
    prevent_optimization += global_bitfield.value;
    
    /* Use qualified pointers */
    prevent_optimization += *const_ptr;
    if (volatile_ptr) {
        prevent_optimization += *volatile_ptr;
    }
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* ==================== ADDITIONAL COMPLEX TYPES ==================== */

/* Multi-level pointer */
typedef int*** triple_ptr;

/* Array of pointers to functions returning pointers */
typedef struct node* (*node_factory_array[5])(int);

/* Struct with flexible array member (C99) */
struct flex_array {
    size_t length;
    int data[];
};

/* Anonymous struct/union (C11) */
struct anonymous_example {
    struct {
        int x, y;
    };
    union {
        int id;
        float score;
    };
};

/* Static assertions for type checking */
_Static_assert(sizeof(color_enum) == sizeof(int), "enum size mismatch");
_Static_assert(sizeof(struct packed_struct) <= 64, "struct too large");
