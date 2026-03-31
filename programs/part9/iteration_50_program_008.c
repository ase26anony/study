/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* TYPE_STRUCT with attributes (potential TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(4))) packed_struct {
    int id;
    char name[20];
    float value;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct packed_struct user_struct_t;

/* TYPE_UNION with GCC extensions */
union __attribute__((transparent_union)) data_union {
    int int_val;
    float float_val;
    void *ptr_val;
    char str_val[16];
};

/* TYPE_ARRAY examples */
typedef int matrix_3x3[3][3];
typedef char string_array[5][32];

/* TYPE_POINTER examples */
typedef int *int_ptr;
typedef void (*func_ptr)(void);
typedef struct packed_struct *struct_ptr;

/* TYPE_CALLBACK - function pointer types */
typedef int (*comparator)(const void *, const void *);
typedef void (*callback_fn)(int, const char *);

/* TYPE_STRING context */
const char *const greeting = "Hello, gengtype!";

/* Complex nested type for deep traversal */
struct nested_container {
    struct packed_struct *items;      /* Pointer to struct */
    union data_union variants[10];    /* Array of unions */
    matrix_3x3 transform;             /* 2D array */
    callback_fn notify;               /* Function pointer */
    const char *description;          /* String pointer */
};

/* Volatile and const qualified types */
volatile int *const volatile_ptr = (volatile int*)0x1000;
const struct packed_struct *const const_struct_ptr = NULL;

/* GCC pragma example */
#pragma pack(push, 1)
struct packed_explicit {
    char flag;
    int count;
    double data;
};
#pragma pack(pop)

/* Global variable definitions (ensure gengtype sees instances) */
struct packed_struct global_struct = {1, "test", 3.14f};
union data_union global_union = {.int_val = 42};
matrix_3x3 global_matrix = {{1,2,3},{4,5,6},{7,8,9}};
string_array global_strings = {"one", "two", "three", "four", "five"};
struct nested_container global_container = {
    .items = &global_struct,
    .variants = {{.int_val = 1}, {.float_val = 2.0f}},
    .transform = {{1,0,0},{0,1,0},{0,0,1}},
    .notify = NULL,
    .description = "Test container"
};

/* Function pointer table (array of callbacks) */
static callback_fn callbacks[] = {NULL, NULL, NULL};

/* External declaration (simulates multi-file scenario) */
extern void external_function(struct nested_container *);

/* Minimal main to ensure code is syntactically valid */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use each major type category */
    global_struct.id = prevent_optimization;
    global_union.int_val++;
    
    int_ptr ptr = &global_struct.id;
    *ptr = 100;
    
    global_matrix[0][0] = 99;
    global_strings[0][0] = 'A';
    
    if (global_container.notify) {
        global_container.notify(1, "test");
    }
    
    /* Use function pointer type */
    comparator cmp = NULL;
    (void)cmp;
    
    /* Use enum scalar */
    color_t col = RED;
    (void)col;
    
    /* Use volatile pointer */
    if (volatile_ptr) {
        prevent_optimization = *volatile_ptr;
    }
    
    return prevent_optimization;
}

/* Callback function definition */
static void sample_callback(int id, const char *msg) {
    (void)id;
    (void)msg;
}

/* Function returning pointer to struct */
struct packed_struct *get_struct_ptr(void) {
    return &global_struct;
}

/* Function taking function pointer */
void register_callback(callback_fn fn) {
    callbacks[0] = fn;
}
