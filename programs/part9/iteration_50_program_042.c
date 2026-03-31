/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ==================== TYPE_SCALAR ==================== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef char scalar_char_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* Global scalar variables */
scalar_int_t global_int = 42;
scalar_float_t global_float = 3.14159f;
scalar_double_t global_double = 2.71828;
scalar_char_t global_char = 'A';
color_enum global_color = GREEN;

/* ==================== TYPE_STRUCT ==================== */
struct plain_struct {
    int x;
    float y;
    char z;
    double w;
};

/* Struct with attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) packed_struct {
    int a;
    char b;
    short c;
    long d;
};

/* Struct with transparent union attribute */
struct with_transparent_union {
    int tag;
    union __attribute__((transparent_union)) {
        int i;
        float f;
        void *p;
    } value;
};

/* Global struct instances */
struct plain_struct global_plain_struct = {1, 2.0f, 'c', 4.0};
struct packed_struct global_packed_struct = {10, 'x', 20, 30L};
struct with_transparent_union global_transparent = {0, {.p = NULL}};

/* ==================== TYPE_USER_STRUCT ==================== */
typedef struct plain_struct user_struct_t;
typedef struct packed_struct user_packed_t;

/* User struct variables */
user_struct_t global_user_struct = {5, 6.0f, 'd', 7.0};
user_packed_t global_user_packed = {15, 'y', 25, 35L};

/* ==================== TYPE_UNION ==================== */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

union nested_union {
    struct {
        int type;
        union {
            int i;
            float f;
        } data;
    } tagged;
    long long raw;
};

/* Global union instances */
union basic_union global_basic_union = {.as_int = 100};
union nested_union global_nested_union = {.tagged = {1, {.i = 200}}};

/* ==================== TYPE_POINTER ==================== */
int *global_int_ptr = &global_int;
float *global_float_ptr = &global_float;
struct plain_struct *global_struct_ptr = &global_plain_struct;
void *global_void_ptr = NULL;
const int *global_const_ptr = &global_int;
volatile char *global_volatile_ptr = (volatile char *)&global_char;
int *const global_const_int_ptr = &global_int;
volatile int *const global_const_volatile_ptr = &global_int;

/* Function pointer (TYPE_CALLBACK) */
typedef int (*callback_t)(int, float);
typedef void (*simple_callback_t)(void);
typedef struct plain_struct *(*struct_callback_t)(int);

/* Callback variable */
callback_t global_callback = NULL;

/* ==================== TYPE_ARRAY ==================== */
int global_int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float global_float_array[5][5];
char global_char_array[3][4][5];
struct plain_struct global_struct_array[2] = {{1, 1.0f, 'a', 1.0}, {2, 2.0f, 'b', 2.0}};
union basic_union global_union_array[3];

/* String literal (TYPE_STRING) */
const char *global_string = "Hello, gengtype!";
const char global_char_string[] = "Test string";
const char *global_string_array[] = {"first", "second", "third"};

/* ==================== Complex Nesting ==================== */
struct complex_nested {
    int id;
    struct plain_struct *nested_struct_ptr;
    union basic_union data_union;
    int (*operation)(struct complex_nested *);
    char name[32];
    struct complex_nested *next;
};

/* Function returning pointer to struct */
struct complex_nested *create_complex(int id) {
    static struct complex_nested instance;
    instance.id = id;
    return &instance;
}

/* Callback implementation */
int sample_callback(int x, float y) {
    return x + (int)y;
}

/* Function using function pointer */
void use_callback(callback_t cb) {
    if (cb) {
        cb(10, 20.5f);
    }
}

/* ==================== Main Function ==================== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int++;
    global_float += 1.0f;
    global_char = 'B';
    prevent_optimization += global_int;
    
    /* Use struct types */
    global_plain_struct.x = 100;
    global_user_struct.y = 200.0f;
    prevent_optimization += global_plain_struct.x;
    
    /* Use union types */
    global_basic_union.as_float = 3.14f;
    prevent_optimization += (int)global_basic_union.as_float;
    
    /* Use pointer types */
    if (global_int_ptr) {
        *global_int_ptr = 50;
    }
    prevent_optimization += *global_int_ptr;
    
    /* Use array types */
    global_int_array[0] = 999;
    prevent_optimization += global_int_array[0];
    
    /* Use string */
    prevent_optimization += global_string[0];
    
    /* Use callback */
    global_callback = sample_callback;
    if (global_callback) {
        prevent_optimization += global_callback(5, 10.5f);
    }
    
    /* Use complex nested type */
    struct complex_nested complex = {
        .id = 1,
        .nested_struct_ptr = &global_plain_struct,
        .data_union = {.as_int = 42},
        .operation = NULL,
        .name = "test",
        .next = NULL
    };
    prevent_optimization += complex.id;
    
    /* Create and use function pointer chain */
    struct_callback_t struct_cb = create_complex;
    if (struct_cb) {
        struct complex_nested *result = struct_cb(100);
        prevent_optimization += result->id;
    }
    
    /* Ensure all globals are referenced */
    (void)global_double;
    (void)global_color;
    (void)global_packed_struct;
    (void)global_transparent;
    (void)global_user_packed;
    (void)global_nested_union;
    (void)global_float_ptr;
    (void)global_struct_ptr;
    (void)global_void_ptr;
    (void)global_const_ptr;
    (void)global_volatile_ptr;
    (void)global_const_int_ptr;
    (void)global_const_volatile_ptr;
    (void)global_float_array;
    (void)global_char_array;
    (void)global_struct_array;
    (void)global_union_array;
    (void)global_char_string;
    (void)global_string_array;
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* ==================== Additional GCC-specific constructs ==================== */
/* Pragmas */
#pragma pack(push, 1)
struct pragma_packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* GCC attributes on variables */
int global_attributed __attribute__((aligned(32))) = 0;
const char *global_weak_string __attribute__((weak)) = "weak string";

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
v4si global_vector = {1, 2, 3, 4};

/* Transparent union in function argument */
union transparent_arg {
    int *int_ptr;
    float *float_ptr;
};

void transparent_func(union transparent_arg __attribute__((transparent_union)) arg) {
    (void)arg;
}
