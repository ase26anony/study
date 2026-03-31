/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;
typedef double my_double;

/* TYPE_STRUCT with basic members */
struct basic_struct {
    int x;
    float y;
    char z;
    double w;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct basic_struct my_struct_t;

/* TYPE_UNION */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

/* TYPE_ARRAY examples */
typedef int int_array_10[10];
typedef char char_matrix[5][5];
typedef float three_d_array[3][3][3];

/* TYPE_POINTER examples */
typedef int *int_ptr;
typedef struct basic_struct *struct_ptr;
typedef void (*void_func_ptr)(void);
typedef union data_union *union_ptr;

/* TYPE_STRING - string literals */
const char *greeting = "Hello, World!";
static const char *messages[] = {"Error", "Warning", "Info"};

/* TYPE_CALLBACK - function pointers */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, void *);

/* Complex nested type for deep traversal */
struct complex_nested {
    struct basic_struct inner;
    union data_union data;
    int_array_10 numbers;
    char_matrix matrix;
    struct complex_nested *next;  /* Self-referential pointer */
    void_func_ptr cleanup;
};

/* TYPE_LANG_STRUCT - GCC extensions */
struct __attribute__((packed, aligned(8))) gcc_struct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((visibility("hidden")));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *intp;
    void *voidp;
    char *charp;
} transparent_union_t;

/* Volatile and const qualified types */
typedef volatile int volatile_int;
typedef const char *const_string_ptr;
typedef volatile const int *restrict volatile_const_ptr;

/* #pragma pack directive */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    double c;
};
#pragma pack(pop)

/* Global variables of each type */
struct basic_struct global_struct = {1, 2.0f, 'A', 3.14};
my_struct_t user_struct = {2, 3.0f, 'B', 2.71};
union data_union global_union = {.i = 42};
int_array_10 global_array = {0,1,2,3,4,5,6,7,8,9};
char_matrix global_matrix = {{'a','b','c','d','e'}};
int_ptr global_int_ptr = &global_array[0];
struct_ptr global_struct_ptr = &global_struct;
comparator_t global_comparator = NULL;
callback_t global_callback = NULL;
struct complex_nested global_complex = {
    .inner = {10, 20.0f, 'X', 30.0},
    .data = {.f = 3.14f},
    .numbers = {100},
    .matrix = {{'X'}},
    .next = NULL,
    .cleanup = NULL
};
struct gcc_struct global_gcc_struct = {'Z', 999, 888.0};
transparent_union_t global_transparent = {.voidp = NULL};
volatile_int global_volatile = 1234;
const_string_ptr global_const_string = "Constant";
struct packed_struct global_packed = {'P', 777, 999.0};

/* Function using callback type */
static int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int val, void *data) {
    *(int*)data = val * 2;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct types */
    global_struct.x = 100;
    user_struct.y = 200.0f;
    prevent_optimization += global_struct.x;
    
    /* Use union type */
    global_union.i = 50;
    prevent_optimization += global_union.i;
    
    /* Use array types */
    global_array[0] = 999;
    global_matrix[0][0] = 'Z';
    prevent_optimization += global_array[0];
    
    /* Use pointer types */
    *global_int_ptr = 111;
    global_struct_ptr->z = 'M';
    prevent_optimization += *global_int_ptr;
    
    /* Use callback types */
    global_comparator = sample_comparator;
    global_callback = sample_callback;
    
    int callback_data = 0;
    if (global_callback) {
        global_callback(5, &callback_data);
    }
    prevent_optimization += callback_data;
    
    /* Use complex nested type */
    global_complex.inner.x = 333;
    global_complex.numbers[0] = 444;
    prevent_optimization += global_complex.inner.x;
    
    /* Use GCC extension types */
    global_gcc_struct.a = 'G';
    global_transparent.intp = &global_volatile;
    prevent_optimization += global_gcc_struct.a;
    
    /* Use volatile/const types */
    global_volatile = 555;
    prevent_optimization += global_volatile;
    
    /* Use packed struct */
    global_packed.a = 'S';
    prevent_optimization += global_packed.b;
    
    /* Use string type */
    prevent_optimization += greeting[0];
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct {
    int hidden_field;
    struct complex_nested *link;
} file_local_struct = {0, NULL};

/* Function returning pointer to struct */
struct basic_struct* get_struct_ptr(void) {
    return &global_struct;
}

/* Typedef for function pointer returning pointer to struct */
typedef struct basic_struct* (*struct_factory_t)(void);

/* Array of function pointers */
struct_factory_t factories[] = {get_struct_ptr, NULL};
