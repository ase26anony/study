/* gengtype-coverage-test.c
 * Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int my_int;                    /* Scalar typedef */
typedef enum { RED, GREEN, BLUE } color_enum;  /* Enum scalar */
typedef float my_float;                /* Floating point scalar */

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    char y;
    double z;
};

/* TYPE_USER_STRUCT - via typedef */
typedef struct {
    int id;
    char name[32];
} user_struct_t;

/* GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
    int int_val;
    float float_val;
    char *string_val;
    void *ptr_val;
};

/* ========== TYPE_POINTER examples ========== */
typedef int* int_ptr_t;
typedef void (*func_ptr_t)(void);
typedef struct plain_struct* struct_ptr_t;
typedef const volatile int* cv_int_ptr_t;

/* ========== TYPE_ARRAY examples ========== */
typedef int int_array_10[10];
typedef char multi_array[5][5];
typedef struct plain_struct struct_array[3];

/* ========== TYPE_STRING examples ========== */
/* String literals in initializers */
const char *greeting = "Hello, gengtype!";
static char default_name[] = "default";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef struct plain_struct* (*factory_t)(void);

/* ========== Complex nested types ========== */
struct complex_nested {
    /* Pointer to struct */
    struct complex_nested *next;
    
    /* Array of unions */
    union data_union values[4];
    
    /* Function pointer */
    callback_t callback;
    
    /* Nested struct */
    struct {
        int depth;
        char label[16];
    } metadata;
    
    /* Pointer to array */
    int (*matrix_ptr)[10];
};

/* ========== Volatile/const qualified types ========== */
volatile int volatile_counter = 0;
const int immutable_value = 42;
volatile int* const volatile_ptr_const = &volatile_counter;
const struct plain_struct const_struct = {1, 'A', 3.14};

/* ========== Global variables using all types ========== */
struct plain_struct global_struct = {10, 'B', 2.718};
user_struct_t global_user_struct = {100, "Test User"};
union data_union global_union = {.int_val = 255};
int_ptr_t global_int_ptr = NULL;
int_array_10 global_array = {0,1,2,3,4,5,6,7,8,9};
multi_array global_multi = {{0}};
comparator_t global_comparator = NULL;
struct complex_nested global_complex = {0};
transparent_union_t global_transparent;

/* ========== Function using callback ========== */
static void sample_callback(int value, void* context) {
    volatile_counter += value;
    (void)context;
}

static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* ========== Main function ========== */
int main(void) {
    /* Use volatile to prevent optimization */
    volatile int prevent_optimization = 0;
    
    /* TYPE_STRUCT usage */
    global_struct.x = 20;
    global_struct.y = 'C';
    prevent_optimization += global_struct.x;
    
    /* TYPE_USER_STRUCT usage */
    global_user_struct.id = 200;
    prevent_optimization += global_user_struct.id;
    
    /* TYPE_UNION usage */
    global_union.int_val = 100;
    prevent_optimization += global_union.int_val;
    
    /* TYPE_POINTER usage */
    int local_var = 30;
    global_int_ptr = &local_var;
    prevent_optimization += *global_int_ptr;
    
    /* TYPE_ARRAY usage */
    global_array[0] = 99;
    prevent_optimization += global_array[0];
    
    global_multi[2][2] = 'X';
    prevent_optimization += global_multi[2][2];
    
    /* TYPE_SCALAR usage */
    my_int scalar = 50;
    color_enum color = GREEN;
    my_float pi = 3.14159f;
    prevent_optimization += scalar + color + (int)pi;
    
    /* TYPE_STRING usage */
    prevent_optimization += greeting[0];
    prevent_optimization += default_name[0];
    
    /* TYPE_CALLBACK usage */
    global_comparator = sample_comparator;
    int nums[] = {5, 2, 8, 1};
    
    /* Call through function pointer if initialized */
    if (global_comparator) {
        prevent_optimization += global_comparator(&nums[0], &nums[1]);
    }
    
    /* Complex nested type usage */
    global_complex.callback = sample_callback;
    global_complex.values[0].int_val = 77;
    global_complex.metadata.depth = 3;
    prevent_optimization += global_complex.values[0].int_val;
    
    /* Volatile/const usage */
    prevent_optimization += immutable_value;
    volatile_counter++;
    
    /* Transparent union usage */
    int transparent_val = 999;
    global_transparent.int_ptr = &transparent_val;
    prevent_optimization += *global_transparent.int_ptr;
    
    /* Packed struct usage */
    struct packed_struct packed = {'Z', 1234, 99};
    prevent_optimization += packed.b;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* ========== Additional declarations in different linkage ========== */
static struct {
    int hidden;
    union data_union secret;
} static_data = {0};

/* External declaration to test cross-file type references */
extern void external_function(struct plain_struct*);

/* GCC pragma for alignment */
#pragma pack(push, 1)
struct pragma_packed {
    char a;
    int b;
};
#pragma pack(pop)

/* Multiple pointer indirections */
typedef int**** complex_ptr_t;

/* Array of function pointers */
typedef void (*func_array_t[5])(void);

/* Struct with bitfields (GCC extension) */
struct with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 24;
};

/* Anonymous struct/union (C11/GCC) */
struct container {
    struct { int x; int y; } point;
    union { int id; long token; };
};
