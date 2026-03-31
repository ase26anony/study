/* gengtype-coverage-test.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRUCT with basic members */
struct plain_struct {
    scalar_int id;
    scalar_float value;
    char name[32];
};

/* TYPE_USER_STRUCT via typedef */
typedef struct plain_struct user_struct_t;

/* GCC-specific attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) lang_struct {
    int data;
    char __attribute__((aligned(16))) aligned_char;
    struct plain_struct* nested;
} __attribute__((transparent_union));

/* TYPE_UNION */
union data_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

/* TYPE_POINTER variations */
typedef int* int_ptr_t;
typedef struct plain_struct* struct_ptr_t;
typedef void (*func_ptr_t)(void);
typedef volatile int* volatile volatile_int_ptr;
typedef const char* const const_string_ptr;

/* TYPE_ARRAY variations */
typedef int int_array_1d[10];
typedef char char_array_2d[5][5];
typedef struct plain_struct struct_array[3];

/* TYPE_STRING context */
#define TEST_STRING "Hello, gengtype!"

/* TYPE_CALLBACK - function pointer type */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, const char*);

/* Complex nested type chains */
struct complex_nested {
    struct plain_struct base;
    union data_union variant;
    int_array_1d numbers;
    struct complex_nested* next;  /* Self-referential pointer */
    comparator_t compare_func;
};

/* Transparent union GCC extension */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int i;
    float f;
    void* p;
} transparent_union_t;

/* Pragmas for alignment */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Global variables with initializers */
struct plain_struct global_struct = { 
    .id = 42, 
    .value = 3.14f, 
    .name = "test" 
};

union data_union global_union = { .as_int = 100 };

int global_array[5] = {1, 2, 3, 4, 5};
char global_string[] = TEST_STRING;

struct complex_nested global_complex = {
    .base = { .id = 1, .value = 2.0f, .name = "nested" },
    .variant = { .as_float = 3.14f },
    .numbers = { [0] = 10, [9] = 20 },
    .next = NULL,
    .compare_func = NULL
};

/* Volatile and const qualified variables */
volatile int volatile_counter = 0;
const int read_only_value = 100;
volatile int* const volatile_const_ptr = &volatile_counter;

/* Function pointer variable */
callback_t global_callback = NULL;

/* Function using callback */
static void sample_callback(int val, const char* msg) {
    /* Prevent unused parameter warnings */
    (void)val;
    (void)msg;
}

/* Function returning pointer to struct */
struct plain_struct* get_struct_ptr(void) {
    return &global_struct;
}

/* Function taking transparent union */
void use_transparent_union(transparent_union_t arg) {
    /* Access through integer representation */
    int val = arg.i;
    (void)val;
}

/* Main function to ensure all types are syntactically used */
int main(void) {
    /* Use scalar types */
    scalar_int x = 10;
    scalar_float y = 20.5f;
    color_enum color = GREEN;
    
    /* Use struct types */
    struct plain_struct local_struct = global_struct;
    user_struct_t user_struct = local_struct;
    
    /* Use union */
    union data_union local_union;
    local_union.as_int = x;
    
    /* Use pointers */
    int_ptr_t ptr = &x;
    struct_ptr_t sptr = &local_struct;
    
    /* Use arrays */
    int_array_1d local_array = {0};
    char_array_2d matrix = {{0}};
    
    /* Use string */
    const char* greeting = TEST_STRING;
    
    /* Use function pointer */
    global_callback = sample_callback;
    if (global_callback) {
        global_callback(1, "test");
    }
    
    /* Use complex nested type */
    struct complex_nested local_complex = global_complex;
    local_complex.compare_func = (comparator_t)global_callback;
    
    /* Use volatile/const */
    volatile_counter++;
    int read_val = read_only_value;
    
    /* Use transparent union */
    transparent_union_t tu = { .i = 42 };
    use_transparent_union(tu);
    
    /* Use packed struct */
    struct packed_struct packed = { .a = 'A', .b = 100, .c = 200 };
    
    /* Use lang_struct */
    struct lang_struct ls = { .data = 99, .aligned_char = 'X', .nested = &local_struct };
    
    /* Prevent dead code elimination */
    volatile int keep_alive = 0;
    keep_alive += x + y + color + local_struct.id + local_union.as_int + *ptr;
    keep_alive += local_array[0] + matrix[0][0] + greeting[0];
    keep_alive += local_complex.base.id + read_val + packed.b + ls.data;
    
    return keep_alive == 0 ? 0 : 0;  /* Always return 0 */
}

/* Additional declarations in file scope for more coverage */
static struct {
    int anonymous_member;
} anonymous_struct = { .anonymous_member = 999 };

typedef int (*complex_callback_t)(struct plain_struct*, union data_union*, int_array_1d);

/* Multi-dimensional pointer array */
int (*func_ptr_array[5])(void) = { NULL, NULL, NULL, NULL, NULL };

/* Const pointer to array of pointers to struct */
struct plain_struct* const * const complex_ptr_chain = NULL;

/* Flexible array member struct */
struct flex_array_struct {
    int count;
    int data[];  /* Flexible array member */
};
