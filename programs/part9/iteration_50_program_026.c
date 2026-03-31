/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ===== TYPE_SCALAR ===== */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* ===== TYPE_STRUCT ===== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct plain_struct my_struct_t;

/* ===== TYPE_UNION ===== */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

/* ===== TYPE_POINTER ===== */
typedef int* int_ptr_t;
typedef void (*void_func_t)(void);

/* ===== TYPE_ARRAY ===== */
typedef int int_array_10[10];
typedef char char_matrix[5][5];

/* ===== TYPE_STRING ===== */
const char* greeting = "Hello, gengtype!";

/* ===== TYPE_CALLBACK ===== */
typedef int (*comparator_t)(const void*, const void*);

/* ===== TYPE_LANG_STRUCT (GCC extensions) ===== */
#ifdef __GNUC__
struct __attribute__((packed, aligned(8))) gcc_struct {
    int a;
    char b;
    double c __attribute__((aligned(16)));
};

union __attribute__((transparent_union)) transparent_union {
    int* int_ptr;
    void* void_ptr;
};
#endif

/* ===== Complex nested types ===== */
struct nested_container {
    struct plain_struct inner_struct;
    union data_union data;
    int_array_10 numbers;
    char_matrix matrix;
    struct nested_container* next;  /* Pointer to self */
    comparator_t compare_func;
};

/* ===== Volatile and const qualifiers ===== */
volatile int* volatile volatile_ptr;
const volatile int* const volatile const_volatile_ptr;

/* ===== Global variable definitions ===== */
struct plain_struct global_struct = {1, 2.5f, 'A'};
union data_union global_union = {.i = 42};
my_struct_t user_struct_var = {2, 3.14f, 'B'};
int_array_10 global_array = {0,1,2,3,4,5,6,7,8,9};
char_matrix global_matrix = {"abcd","efgh","ijkl","mnop","qrst"};
color_t global_color = BLUE;
int_ptr_t global_int_ptr = NULL;
comparator_t global_comparator = NULL;

#ifdef __GNUC__
struct gcc_struct global_gcc_struct = {10, 'X', 3.14159};
union transparent_union global_transparent;
#endif

struct nested_container global_container = {
    .inner_struct = {3, 4.5f, 'C'},
    .data = {.f = 9.99f},
    .numbers = {10,11,12,13,14,15,16,17,18,19},
    .matrix = {"12345","67890","abcde","fghij","klmno"},
    .next = NULL,
    .compare_func = NULL
};

/* ===== Function pointer usage ===== */
static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* ===== Main function to ensure all types are referenced ===== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.x = prevent_optimization + 1;
    
    /* Use union */
    global_union.i = 100;
    
    /* Use user struct */
    user_struct_var.y = 2.718f;
    
    /* Use array */
    global_array[0] = 999;
    
    /* Use matrix */
    global_matrix[0][0] = 'Z';
    
    /* Use enum */
    global_color = RED;
    
    /* Use pointer */
    global_int_ptr = &global_array[0];
    
    /* Use callback */
    global_comparator = sample_comparator;
    
    /* Use nested container */
    global_container.inner_struct.z = 'D';
    global_container.next = &global_container;
    
    /* Use volatile/const pointers */
    volatile_ptr = &prevent_optimization;
    
    /* Use GCC-specific types */
    #ifdef __GNUC__
    global_gcc_struct.a = 20;
    global_transparent.int_ptr = global_int_ptr;
    #endif
    
    /* Use string */
    if (greeting[0] != '\0') {
        prevent_optimization++;
    }
    
    return prevent_optimization;
}

/* ===== Additional cross-file type declarations (simulated) ===== */
#ifndef HEADER_GUARD
#define HEADER_GUARD

extern struct externally_defined_struct {
    long external_member;
    struct nested_container* link;
} external_var;

typedef struct externally_defined_struct* external_ptr_t;

#endif /* HEADER_GUARD */
