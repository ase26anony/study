/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int integer;
typedef float real;

/* TYPE_STRUCT with basic types */
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

/* TYPE_POINTER examples */
int *int_ptr;
struct basic_struct *struct_ptr;
void *void_ptr;
volatile int *volatile_ptr;
const char *const_ptr;
volatile int *const volatile_const_ptr;

/* TYPE_ARRAY examples */
int int_array[10];
char char_array[5][5];
struct basic_struct struct_array[3];
union data_union union_array[4][2];

/* String literal (TYPE_STRING context) */
const char *greeting = "Hello, gengtype!";
char message[] = "Test string";

/* TYPE_CALLBACK (function pointer) */
typedef int (*callback_t)(int, float);
typedef void (*simple_cb)(void);
typedef struct basic_struct* (*struct_factory)(void);

/* Complex nested struct with pointers and arrays */
struct complex_nested {
    struct basic_struct inner;
    union data_union data;
    int *int_pointer;
    struct complex_nested *next;  /* Self-referential pointer */
    callback_t handler;
    char buffer[256];
};

/* GCC-specific attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union {
    int i;
    long l;
};

/* Using #pragma pack */
#pragma pack(push, 1)
struct packed_with_pragma {
    char c;
    int i;
    short s;
};
#pragma pack(pop)

/* More complex type chains */
typedef union data_union* union_ptr_t;
typedef int (*array_of_callbacks[5])(void);

/* Global variables using all types */
struct basic_struct global_struct = {1, 2.5, 'A', 3.14};
union data_union global_union = {.i = 42};
my_struct_t user_struct_var = {2, 3.14, 'B', 2.718};
color_t global_color = GREEN;
integer global_int = 100;
real global_float = 3.14159;

int* global_int_ptr = &global_int;
struct basic_struct* global_struct_ptr = &global_struct;

int global_int_array[5] = {1, 2, 3, 4, 5};
char global_char_matrix[3][3] = {{'a','b','c'},{'d','e','f'},{'g','h','i'}};

struct complex_nested global_complex = {
    .inner = {10, 20.5, 'X', 30.7},
    .data = {.f = 3.14},
    .int_pointer = &global_int,
    .next = NULL,
    .handler = NULL,
    .buffer = "Initial buffer"
};

struct packed_struct global_packed = {'Z', 999, 77};
struct packed_with_pragma global_pragma_packed = {'Y', 888, 66};

/* Function pointer variables */
callback_t global_callback = NULL;
simple_cb global_simple_cb = NULL;
array_of_callbacks global_callback_array = {NULL, NULL, NULL, NULL, NULL};

/* Function using callback */
int sample_callback(int a, float b) {
    return a + (int)b;
}

struct basic_struct* create_struct(void) {
    static struct basic_struct s = {0, 0.0, '\0', 0.0};
    return &s;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.x = 100;
    prevent_optimization += global_struct.x;
    
    /* Use union */
    global_union.i = 200;
    prevent_optimization += global_union.i;
    
    /* Use user struct */
    user_struct_var.y = 300.5;
    prevent_optimization += (int)user_struct_var.y;
    
    /* Use pointers */
    if (int_ptr) prevent_optimization += *int_ptr;
    if (struct_ptr) prevent_optimization += struct_ptr->x;
    
    /* Use arrays */
    prevent_optimization += int_array[0];
    prevent_optimization += char_array[0][0];
    
    /* Use string */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use complex nested */
    global_complex.inner.x = 400;
    prevent_optimization += global_complex.inner.x;
    
    /* Use packed structs */
    prevent_optimization += global_packed.a;
    prevent_optimization += global_pragma_packed.c;
    
    /* Use function pointer */
    global_callback = sample_callback;
    if (global_callback) {
        prevent_optimization += global_callback(5, 3.5);
    }
    
    /* Use typedef function pointer */
    global_simple_cb = (simple_cb)0;
    
    /* Use array of callbacks */
    global_callback_array[0] = (int (*)(void))sample_callback;
    
    /* Use all scalar types */
    prevent_optimization += global_int;
    prevent_optimization += (int)global_float;
    prevent_optimization += global_color;
    
    /* Ensure everything is referenced */
    (void)global_struct_ptr;
    (void)global_int_ptr;
    (void)void_ptr;
    (void)volatile_ptr;
    (void)const_ptr;
    (void)volatile_const_ptr;
    (void)struct_array;
    (void)union_array;
    (void)global_char_matrix;
    (void)global_union;
    (void)create_struct;
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct static_only_struct {
    int hidden;
    float secret;
} static_var = {123, 456.789};

/* Extern declaration simulating multi-file scenario */
extern void external_function(void);

/* Inline function with types */
static inline void inline_helper(struct basic_struct *s) {
    if (s) s->x++;
}
