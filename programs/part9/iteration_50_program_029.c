/* test_gengtype_coverage.c
 * Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) attributed_struct {
    int id;
    char name[20];
    double value;
};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;
typedef struct attributed_struct attributed_user_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
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
typedef int* int_ptr_t;
typedef struct plain_struct* struct_ptr_t;
typedef void (*void_func_ptr_t)(void);

/* Complex pointer chain */
typedef int*** triple_ptr_t;

/* ========== TYPE_ARRAY examples ========== */
typedef int int_array_10_t[10];
typedef char char_matrix_5x5_t[5][5];
typedef struct plain_struct struct_array_t[3];

/* ========== TYPE_STRING examples ========== */
/* String literals used in initializers */
const char* greeting = "Hello, World!";
static const char* messages[] = {"Error", "Warning", "Info"};

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef struct plain_struct* (*struct_factory_t)(void);

/* ========== Complex nested types ========== */
struct nested_container {
    /* TYPE_STRUCT containing TYPE_POINTER */
    struct plain_struct* child;
    
    /* TYPE_ARRAY of TYPE_UNION */
    union data_union items[10];
    
    /* TYPE_CALLBACK */
    comparator_t compare;
    
    /* TYPE_POINTER to TYPE_ARRAY */
    int (*matrix_ptr)[5][5];
};

/* ========== Volatile and const qualifiers ========== */
volatile int volatile_counter = 0;
const int read_only_value = 42;
volatile int* volatile volatile_int_ptr;
int* const const_int_ptr = NULL;
const volatile int cv_qualified = 100;

/* ========== Global variable definitions ========== */
/* Ensure gengtype encounters concrete instances */

/* TYPE_SCALAR */
scalar_int_t global_int = 42;
color_enum global_color = GREEN;

/* TYPE_STRUCT */
struct plain_struct global_struct = {1, 3.14f, 'A'};
struct attributed_struct global_attributed = {100, "test", 99.99};

/* TYPE_USER_STRUCT */
user_struct_t global_user_struct = {2, 2.718f, 'B'};

/* TYPE_UNION */
union data_union global_union = {.as_int = 255};

/* TYPE_POINTER */
int_ptr_t global_int_ptr = &global_int;
struct_ptr_t global_struct_ptr = &global_struct;
void_func_ptr_t global_func_ptr = NULL;
triple_ptr_t global_triple_ptr = NULL;

/* TYPE_ARRAY */
int_array_10_t global_array = {0,1,2,3,4,5,6,7,8,9};
char_matrix_5x5_t global_matrix = {{0}};
struct_array_t global_struct_array = {{0}};

/* TYPE_STRING - already defined above */

/* TYPE_CALLBACK */
comparator_t global_comparator = NULL;

/* Complex nested type */
struct nested_container global_container = {
    .child = &global_struct,
    .compare = NULL,
    .matrix_ptr = &global_matrix
};

/* Function pointer with complex return type */
struct_factory_t global_factory = NULL;

/* ========== Function using callback ========== */
static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int value, void* context) {
    *(int*)context = value * 2;
}

static struct plain_struct* create_struct(void) {
    static struct plain_struct instance = {0, 0.0f, '\0'};
    return &instance;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use TYPE_SCALAR */
    global_int++;
    global_color = BLUE;
    prevent_optimization += global_int;
    
    /* Use TYPE_STRUCT */
    global_struct.x = 10;
    global_struct.y = 20.5f;
    prevent_optimization += global_struct.x;
    
    /* Use TYPE_USER_STRUCT */
    global_user_struct.z = 'X';
    prevent_optimization += global_user_struct.z;
    
    /* Use TYPE_UNION */
    global_union.as_float = 3.14159f;
    prevent_optimization += (int)global_union.as_float;
    
    /* Use TYPE_POINTER */
    if (global_int_ptr) {
        *global_int_ptr = 100;
        prevent_optimization += *global_int_ptr;
    }
    
    /* Use TYPE_ARRAY */
    global_array[0] = 999;
    global_matrix[0][0] = 'Z';
    prevent_optimization += global_array[0] + global_matrix[0][0];
    
    /* Use TYPE_STRING */
    prevent_optimization += greeting[0];
    
    /* Use TYPE_CALLBACK */
    global_comparator = sample_comparator;
    int a = 5, b = 10;
    if (global_comparator) {
        prevent_optimization += global_comparator(&a, &b);
    }
    
    callback_t cb = sample_callback;
    int result = 0;
    if (cb) {
        cb(21, &result);
        prevent_optimization += result;
    }
    
    global_factory = create_struct;
    if (global_factory) {
        struct_ptr_t new_struct = global_factory();
        if (new_struct) {
            prevent_optimization += new_struct->x;
        }
    }
    
    /* Use complex nested type */
    global_container.compare = sample_comparator;
    if (global_container.matrix_ptr) {
        (*global_container.matrix_ptr)[1][1] = 'M';
        prevent_optimization += (*global_container.matrix_ptr)[1][1];
    }
    
    /* Use volatile/const types */
    volatile_counter = prevent_optimization;
    if (volatile_int_ptr) {
        *volatile_int_ptr = volatile_counter;
    }
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional GCC pragma for alignment */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Another GCC attribute example */
struct __attribute__((aligned(16), may_alias)) overaligned_struct {
    long long data[2];
};

/* Static instances to ensure they're processed */
static struct packed_struct static_packed = {0};
static struct overaligned_struct static_overaligned = {{0}};
