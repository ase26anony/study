/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef _Bool scalar_bool_t;

/* Global scalar variables */
scalar_int_t global_int = 42;
scalar_float_t global_float = 3.14159f;
color_enum global_color = GREEN;
scalar_bool_t global_bool = 1;

/* ========== TYPE_STRUCT examples ========== */
/* Plain struct */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with pointer member */
struct struct_with_pointer {
    int id;
    struct plain_struct* ptr;
    void* opaque;
};

/* Nested struct */
struct outer_struct {
    int outer_id;
    struct inner_struct {
        int inner_x;
        float inner_y;
    } inner;
    struct struct_with_pointer* next;
};

/* Global struct instances */
struct plain_struct global_plain = {1, 2.5f, 'A'};
struct outer_struct global_outer = {100, {10, 20.5f}, NULL};

/* ========== TYPE_USER_STRUCT examples ========== */
/* Typedef creating user-defined struct type */
typedef struct plain_struct user_struct_t;
typedef struct outer_struct complex_user_t;

/* User struct variables */
user_struct_t user_var = {2, 3.14f, 'B'};
complex_user_t complex_user_var = {200, {20, 30.5f}, NULL};

/* ========== TYPE_UNION examples ========== */
union data_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

union nested_union {
    struct {
        int type;
        union data_union data;
    } tagged;
    long long raw;
};

/* Union variables */
union data_union global_union = {.as_int = 255};
union nested_union global_nested_union = {.raw = 0xDEADBEEF};

/* ========== TYPE_POINTER examples ========== */
/* Various pointer types */
int* int_ptr = &global_int;
float* float_ptr = &global_float;
struct plain_struct* struct_ptr = &global_plain;
void* void_ptr = NULL;
int** double_ptr = &int_ptr;
const int* const_ptr = &global_int;
volatile float* volatile_ptr = &global_float;
const volatile char* cv_ptr = "test";

/* Function pointer */
typedef int (*func_ptr_t)(int, float);

/* ========== TYPE_ARRAY examples ========== */
/* Various array types */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][5];
char char_array[3][4][5];
struct plain_struct struct_array[3];
union data_union union_array[2][2];

/* String array (TYPE_STRING context) */
const char* string_array[] = {"hello", "world", "test"};
const char* single_string = "This is a string literal";

/* ========== TYPE_CALLBACK examples ========== */
/* Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* data, int result);
typedef struct plain_struct* (*struct_factory_t)(int);

/* Callback variable */
binary_op_t global_callback = NULL;

/* ========== TYPE_LANG_STRUCT examples ========== */
/* GCC-specific attributes and extensions */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    char c;
} __attribute__((aligned(8)));

struct __attribute__((transparent_union)) transparent_union {
    int as_int;
    float as_float;
};

/* Variable with GCC attributes */
volatile int __attribute__((aligned(16))) aligned_var = 0;

/* ========== Complex nesting examples ========== */
/* Deeply nested type chain */
typedef struct node {
    int value;
    struct node* next;
    struct node* prev;
    void* data;
} node_t;

typedef struct container {
    node_t* head;
    node_t* tail;
    int size;
    void (*processor)(node_t*);
} container_t;

/* Array of function pointers */
typedef int (*math_func_t[5])(int, int);

/* Struct with flexible array member */
struct with_fam {
    int count;
    int data[];
};

/* ========== Volatile/Const qualified types ========== */
volatile int volatile_int = 100;
const int const_int = 200;
volatile const int volatile_const_int = 300;
int* const const_ptr_var = &global_int;
const int* ptr_to_const = &global_int;
volatile int* volatile volatile_ptr_var = &volatile_int;

/* ========== Global variables with initializers ========== */
/* Complex initializer */
struct struct_with_pointer global_complex = {
    .id = 999,
    .ptr = &global_plain,
    .opaque = &global_outer
};

/* Array with designated initializers */
int designated_array[5] = {[0] = 1, [2] = 3, [4] = 5};

/* ========== Function declarations ========== */
/* To ensure TYPE_CALLBACK is processed */
int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }
void process_result(void* data, int result) { (void)data; (void)result; }
struct plain_struct* create_struct(int x) {
    static struct plain_struct s;
    s.x = x;
    s.y = x * 1.5f;
    s.z = 'A' + x;
    return &s;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use each major type category */
    
    /* Scalars */
    global_int++;
    global_float *= 2.0f;
    prevent_optimization += global_color;
    
    /* Structs */
    global_plain.x = 10;
    global_outer.inner.inner_x = 20;
    user_var.y = 3.0f;
    
    /* Unions */
    global_union.as_float = 2.71828f;
    prevent_optimization += (int)global_nested_union.raw;
    
    /* Pointers */
    *int_ptr = 50;
    struct_ptr->z = 'X';
    prevent_optimization += (intptr_t)void_ptr;
    
    /* Arrays */
    int_array[0] = 100;
    float_array[2][2] = 1.5f;
    prevent_optimization += char_array[0][0][0];
    
    /* Strings */
    prevent_optimization += single_string[0];
    
    /* Callbacks */
    global_callback = add;
    if (global_callback) {
        prevent_optimization += global_callback(1, 2);
    }
    
    /* Language structs */
    struct packed_struct ps = {.a = 'X', .b = 42, .c = 'Y'};
    prevent_optimization += ps.b;
    
    /* Complex nesting */
    node_t node = {.value = 1, .next = NULL, .prev = NULL, .data = NULL};
    container_t container = {.head = &node, .tail = &node, .size = 1, .processor = NULL};
    prevent_optimization += container.size;
    
    /* Qualified types */
    volatile_int = 500;
    prevent_optimization += *ptr_to_const;
    
    /* Call function returning struct pointer */
    struct plain_struct* new_struct = create_struct(42);
    prevent_optimization += new_struct->x;
    
    return prevent_optimization == 0 ? 0 : 0;
}

/* ========== Additional declarations in file scope ========== */
/* Static variables with internal linkage */
static struct {
    int secret;
    char code[4];
} static_struct = {1234, {'A', 'B', 'C', 'D'}};

static union {
    long as_long;
    double as_double;
} static_union = {.as_long = 0xCAFEBABE};

/* External declaration (simulating multi-file) */
extern int external_function(void);

/* Pragmas */
#pragma pack(push, 1)
struct packed_with_pragma {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Multiple typedef chains */
typedef int my_int;
typedef my_int my_int2;
typedef my_int2 my_int3;

/* Opaque forward declaration */
struct opaque_struct;
typedef struct opaque_struct* handle_t;

/* Incomplete array type in struct */
struct with_incomplete {
    int len;
    char data[];
};
