/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int scalar_int;
typedef float scalar_float;
typedef enum { RED, GREEN, BLUE } color_enum;

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct plain_struct {
    int x;
    float y;
    char z;
} __attribute__((packed));

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int id;
    char name[32];
} user_struct_t;

/* TYPE_UNION with GCC attributes */
union data_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
} __attribute__((aligned(16)));

/* TYPE_POINTER examples */
typedef int* int_ptr;
typedef struct plain_struct* struct_ptr;
typedef void (*func_ptr)(void);

/* TYPE_ARRAY examples */
typedef int int_array_1d[10];
typedef char char_array_2d[5][5];
typedef struct plain_struct struct_array[3];

/* TYPE_STRING context */
const char* greeting = "Hello, gengtype!";

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void*, const void*);

/* Complex nested type for deep traversal */
struct nested_container {
    struct plain_struct base;
    union data_union data;
    int_array_1d numbers;
    struct nested_container* next;  /* Pointer to self */
    comparator_t compare;
} __attribute__((packed, aligned(8)));

/* TYPE_LANG_STRUCT candidate with GCC extensions */
struct __attribute__((transparent_union)) transparent_union {
    int i;
    float f;
};

/* Volatile and const qualified types */
volatile int volatile_counter;
const int* const const_int_ptr;
volatile struct plain_struct* volatile_struct_ptr;

/* Packed struct with #pragma */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Global variable definitions (ensure gengtype sees instances) */
struct plain_struct global_struct = {1, 2.5f, 'A'};
user_struct_t global_user_struct = {100, "Test Name"};
union data_union global_union = {.as_int = 42};
int_array_1d global_array = {0,1,2,3,4,5,6,7,8,9};
struct nested_container global_container = {
    .base = {2, 3.14f, 'B'},
    .data = {.as_float = 2.718f},
    .numbers = {10,20,30},
    .next = NULL,
    .compare = NULL
};
struct packed_struct global_packed = {'X', 999, 123};

/* Function using callback type */
int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use each type to prevent elimination */
    global_struct.x = prevent_optimization;
    global_user_struct.id++;
    global_union.as_int = 100;
    global_array[0] = prevent_optimization;
    
    /* Pointer operations */
    int_ptr ptr = &global_array[0];
    *ptr = 50;
    
    struct_ptr sptr = &global_struct;
    sptr->z = 'C';
    
    /* Array operations */
    char_array_2d matrix;
    matrix[0][0] = 'X';
    
    /* Callback usage */
    comparator_t cmp = sample_comparator;
    int a = 5, b = 10;
    int result = cmp(&a, &b);
    
    /* Nested structure */
    global_container.next = &global_container;
    global_container.compare = cmp;
    
    /* String usage */
    const char* local_greeting = greeting;
    prevent_optimization += local_greeting[0];
    
    /* Packed struct */
    global_packed.a = 'Y';
    
    return result + prevent_optimization;
}

/* Additional cross-file type (simulating multiple translation units) */
extern struct external_type {
    long long big_value;
    struct nested_container* link;
} external_var;
