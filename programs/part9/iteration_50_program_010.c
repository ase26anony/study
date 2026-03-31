/* test_gengtype_coverage.c
 * Comprehensive type declarations to exercise gengtype's type system
 */

#include <stddef.h>

/* ===== TYPE_SCALAR examples ===== */
typedef int scalar_int;
typedef float scalar_float;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ===== TYPE_STRING example ===== */
const char *greeting = "Hello, gengtype!";

/* ===== TYPE_STRUCT examples ===== */
struct plain_struct {
    int id;
    float value;
    char name[32];
};

/* ===== TYPE_USER_STRUCT via typedef ===== */
typedef struct {
    int x, y;
    double magnitude;
} user_struct_t;

/* ===== TYPE_UNION examples ===== */
union data_union {
    int as_int;
    float as_float;
    char as_char[4];
    void *as_ptr;
};

/* ===== TYPE_POINTER examples ===== */
int *int_ptr;
struct plain_struct *struct_ptr;
void *void_ptr;
const volatile int *const volatile_qualified_ptr;

/* ===== TYPE_ARRAY examples ===== */
int int_array[10];
char char_matrix[5][5];
struct plain_struct struct_array[3];
union data_union union_array[8];

/* ===== TYPE_CALLBACK (function pointer) examples ===== */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, void *);
typedef struct plain_struct *(*factory_t)(void);

/* ===== TYPE_LANG_STRUCT via GCC attributes ===== */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    char c;
};

union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
};

struct __attribute__((aligned(64))) overaligned_struct {
    double data[8];
};

/* ===== Complex nested types ===== */
struct nested_container {
    /* Pointer to struct */
    struct plain_struct *child;
    
    /* Array of pointers */
    void *ptr_array[5];
    
    /* Nested union */
    union {
        int option_a;
        struct {
            float x, y;
        } option_b;
    } choice;
    
    /* Function pointer member */
    comparator_t compare;
    
    /* Pointer to array */
    int (*matrix_ptr)[5][5];
    
    /* Self-referential pointer */
    struct nested_container *next;
};

/* ===== Volatile and const qualified types ===== */
volatile int volatile_counter;
const double pi_approx = 3.14159;
volatile const char *const volatile_string = "constant";
int *const const_pointer = NULL;
volatile int *volatile volatile_pointer;

/* ===== Global variable definitions ===== */
struct plain_struct global_struct = {1, 3.14f, "test"};
user_struct_t global_user_struct = {10, 20, 35.7};
union data_union global_union = {.as_int = 42};
struct nested_container global_container = {0};
struct packed_struct global_packed = {'A', 1234, 'B'};
struct overaligned_struct global_aligned = {{0}};

/* ===== Function pointer variables ===== */
comparator_t global_comparator = NULL;
callback_t global_callback = NULL;
factory_t global_factory = NULL;

/* ===== Array initializations ===== */
int initialized_array[5] = {1, 2, 3, 4, 5};
char *string_array[] = {"first", "second", "third"};
struct plain_struct initialized_structs[] = {
    {1, 1.1f, "one"},
    {2, 2.2f, "two"},
    {3, 3.3f, "three"}
};

/* ===== Complex type chain ===== */
typedef union data_union *(*complex_callback_t)(struct nested_container **, int[][5]);

/* ===== External declaration (simulating multi-file) ===== */
extern void external_function(void);

/* ===== Function definitions ===== */
int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int value, void *context) {
    volatile int *target = (volatile int*)context;
    *target = value;
}

struct plain_struct *sample_factory(void) {
    static struct plain_struct instance = {999, 99.9f, "factory"};
    return &instance;
}

/* ===== Main function to ensure all types are referenced ===== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Reference scalar types */
    scalar_int si = 100;
    scalar_float sf = 3.14f;
    color_enum ce = GREEN;
    prevent_optimization += si + (int)sf + ce;
    
    /* Reference string */
    prevent_optimization += greeting[0];
    
    /* Reference struct types */
    global_struct.id = 2;
    global_user_struct.x = 30;
    prevent_optimization += global_struct.id + global_user_struct.x;
    
    /* Reference union */
    global_union.as_float = 3.14f;
    prevent_optimization += (int)global_union.as_float;
    
    /* Reference pointers */
    int_ptr = &si;
    struct_ptr = &global_struct;
    prevent_optimization += *int_ptr + struct_ptr->id;
    
    /* Reference arrays */
    int_array[0] = 100;
    char_matrix[0][0] = 'X';
    prevent_optimization += int_array[0] + char_matrix[0][0];
    
    /* Reference function pointers */
    global_comparator = sample_comparator;
    global_callback = sample_callback;
    global_factory = sample_factory;
    
    if (global_comparator) {
        int nums[2] = {5, 3};
        prevent_optimization += global_comparator(&nums[0], &nums[1]);
    }
    
    if (global_callback) {
        global_callback(42, &prevent_optimization);
    }
    
    if (global_factory) {
        struct_ptr = global_factory();
        prevent_optimization += struct_ptr->id;
    }
    
    /* Reference nested types */
    global_container.child = &global_struct;
    global_container.choice.option_a = 77;
    global_container.compare = sample_comparator;
    prevent_optimization += global_container.choice.option_a;
    
    /* Reference packed/aligned structs */
    global_packed.a = 'C';
    global_aligned.data[0] = 1.0;
    prevent_optimization += global_packed.a + (int)global_aligned.data[0];
    
    /* Reference qualified types */
    volatile_counter = 99;
    prevent_optimization += volatile_counter;
    
    /* Reference initialized arrays */
    prevent_optimization += initialized_array[0] + string_array[0][0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* ===== Additional complex type for good measure ===== */
struct ultra_complex {
    /* Pointer to function returning pointer to array */
    int (*(*func_ptr_array[3])(void))[10];
    
    /* Union containing struct containing union */
    union {
        struct {
            union {
                int a;
                float b;
            } inner;
            char *name;
        } data;
        long long big_value;
    } mega_union;
    
    /* Const volatile qualified function pointer */
    int (*const volatile cv_func_ptr)(volatile int *);
    
    /* Flexible array member (GCC extension) */
    int flex_array[];
};
