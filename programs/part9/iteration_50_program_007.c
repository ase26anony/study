/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* TYPE_STRUCT with attributes */
struct __attribute__((packed, aligned(8))) base_struct {
    int id;
    float value;
    char tag;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct base_struct my_struct_t;

/* TYPE_UNION with volatile */
union data_union {
    int int_val;
    float float_val;
    char *string_val;
    volatile long volatile_val;
};

/* TYPE_ARRAY examples */
int global_array[10];
char multi_dim[5][5];
static const float const_array[3] = {1.0, 2.0, 3.0};

/* TYPE_POINTER examples */
int *int_ptr;
struct base_struct *struct_ptr;
void *void_ptr;
const volatile int *cv_ptr;
int **double_ptr;

/* TYPE_STRING */
const char *greeting = "Hello, gengtype!";
char filename[] = "test.txt";

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, char *);

/* Complex nested type for deep traversal */
struct complex_nested {
    struct base_struct header;
    union data_union data;
    int *ptr_array[5];
    struct complex_nested *next;
    callback_t handler;
};

/* TYPE_LANG_STRUCT via GCC extensions */
struct __attribute__((transparent_union)) transparent_union {
    int int_member;
    float float_member;
};

/* Another struct with bitfields (GCC extension) */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 5;
    int regular;
} __attribute__((packed));

/* Array of unions */
union data_union union_array[4];

/* Struct containing array of function pointers */
struct callback_container {
    const char *name;
    comparator_t comparators[3];
    void (*actions[2])(void);
};

/* Global variables using all types */
my_struct_t global_struct = {1, 3.14, 'A'};
union data_union global_union = {.int_val = 42};
struct complex_nested *global_nested = NULL;
comparator_t global_comparator = NULL;
color_t global_color = GREEN;

/* Function using callback */
static int compare_ints(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

/* Function with pointer to struct */
static void init_struct(struct base_struct *s) {
    if (s) {
        s->id = 100;
        s->value = 2.718;
        s->tag = 'X';
    }
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 10;
    prevent_optimization += global_struct.id;
    
    /* Use union */
    global_union.float_val = 3.14159;
    prevent_optimization += (int)global_union.float_val;
    
    /* Use array */
    global_array[0] = 1;
    prevent_optimization += global_array[0];
    
    /* Use pointers */
    int local_var = 42;
    int_ptr = &local_var;
    prevent_optimization += *int_ptr;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use callback */
    global_comparator = compare_ints;
    int nums[2] = {5, 3};
    if (global_comparator) {
        prevent_optimization += global_comparator(&nums[0], &nums[1]);
    }
    
    /* Use nested struct */
    struct complex_nested nested;
    nested.header.id = 99;
    nested.handler = NULL;
    prevent_optimization += nested.header.id;
    
    /* Use enum */
    global_color = BLUE;
    prevent_optimization += global_color;
    
    /* Use multi-dim array */
    multi_dim[0][0] = 'Z';
    prevent_optimization += multi_dim[0][0];
    
    /* Use const array */
    prevent_optimization += (int)const_array[0];
    
    /* Use double pointer */
    int **pp = &int_ptr;
    prevent_optimization += **pp;
    
    /* Use bitfield struct */
    struct bitfield_struct bfs = {1, 2, 10, -1};
    prevent_optimization += bfs.flag3;
    
    /* Use transparent union */
    struct transparent_union tu;
    tu.int_member = 999;
    prevent_optimization += tu.int_member;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in file scope for TYPE_UNDEFINED testing */
extern struct undefined_struct *external_undefined;

/* Array of pointers to different types */
void *type_pointers[] = {
    &global_struct,
    &global_union,
    global_array,
    int_ptr,
    greeting,
    &global_comparator
};

/* Static variable with initializer containing nested designators */
static struct complex_nested static_nested = {
    .header = { .id = 255, .value = 1.618, .tag = 'Z' },
    .data = { .int_val = 4096 },
    .ptr_array = { NULL, NULL, NULL, NULL, NULL },
    .next = NULL,
    .handler = NULL
};
