/* test_gengtype_coverage.c
 * This file contains declarations to exercise all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int my_int;
typedef float my_float;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRUCT with basic types */
struct basic_struct {
    int a;
    float b;
    char c;
    double d;
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
const char *const_string_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY examples */
int int_array[10];
char char_array[5][5];
struct basic_struct struct_array[3];
union data_union union_array[4];

/* TYPE_STRING - string literals in initializers */
const char *greeting = "Hello, World!";
char message[] = "Test message";

/* TYPE_CALLBACK - function pointers */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, void *);

/* Complex nested type for deep traversal */
struct complex_nested {
    struct basic_struct inner;
    union data_union data;
    int *pointer_array[5];
    struct complex_nested *next;  /* Self-referential pointer */
};

/* TYPE_LANG_STRUCT - GCC-specific attributes */
struct __attribute__((packed, aligned(8))) gcc_struct {
    char a;
    int b __attribute__((aligned(16)));
    double c;
} __attribute__((aligned(32)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) trans_union {
    int *intp;
    void *voidp;
} trans_union_t;

/* More complex callback with struct return */
struct basic_struct* (*struct_factory)(int, const char*);
void (*cleanup_callback)(struct basic_struct*);

/* Volatile and const qualified types */
volatile const int vci = 42;
const volatile float cvf = 3.14f;
volatile int* const volatile_ptr = (volatile int*)&vci;

/* Array of function pointers */
callback_t callbacks[3];

/* Nested array of pointers to structs */
struct basic_struct *struct_ptr_array[2][3];

/* Anonymous struct/union */
struct {
    int x;
    union {
        int a;
        float b;
    } data;
} anonymous_var = {0};

/* Global variables using all types */
my_int global_int = 100;
my_float global_float = 2.718f;
color_enum global_color = GREEN;
struct basic_struct global_struct = {1, 2.0f, 'A', 3.14};
my_struct_t global_typedef_struct = {2, 3.0f, 'B', 2.718};
union data_union global_union = {.i = 42};
struct complex_nested global_complex = {
    .inner = {3, 4.0f, 'C', 1.618},
    .data = {.f = 3.14f},
    .next = NULL
};
struct gcc_struct global_gcc_struct = {'X', 99, 2.718};

/* Function using callback */
static int compare_ints(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

/* Function that returns pointer to struct */
struct basic_struct* get_struct_ptr(void) {
    return &global_struct;
}

/* Minimal main to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int++;
    global_float *= 2.0f;
    
    /* Use struct types */
    global_struct.a = 10;
    global_typedef_struct.b = 20.0f;
    
    /* Use union */
    global_union.f = 3.14f;
    
    /* Use pointers */
    *int_ptr = global_int;
    struct_ptr = &global_struct;
    
    /* Use arrays */
    int_array[0] = 1;
    char_array[0][0] = 'A';
    struct_array[0].a = 5;
    
    /* Use string */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use function pointers */
    comparator_t cmp = compare_ints;
    int x = 1, y = 2;
    prevent_optimization += cmp(&x, &y);
    
    /* Use complex nested */
    global_complex.inner.a = 100;
    if (global_complex.next) {
        global_complex.next->inner.b = 200.0f;
    }
    
    /* Use GCC struct */
    global_gcc_struct.a = 'Z';
    
    /* Use transparent union */
    trans_union_t tu;
    tu.intp = &global_int;
    
    /* Use volatile/const */
    prevent_optimization += vci;
    prevent_optimization += (int)cvf;
    
    /* Use array of function pointers */
    if (callbacks[0]) {
        callbacks[0](1, NULL);
    }
    
    /* Use struct pointer array */
    struct_ptr_array[0][0] = &global_struct;
    
    /* Use anonymous struct */
    anonymous_var.x = 42;
    anonymous_var.data.a = 100;
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional declarations to ensure TYPE_UNDEFINED might be triggered */
extern void undefined_function(void);
extern struct undefined_struct *undefined_ptr;

/* Multiple translation unit simulation */
#ifdef MULTI_TU
/* In a real multi-file test, this would be in a separate file */
struct cross_file_struct {
    int cross_member;
    struct basic_struct *linked;
};
#else
/* Forward declaration to simulate cross-file reference */
struct cross_file_struct;
extern struct cross_file_struct *external_ref;
#endif
