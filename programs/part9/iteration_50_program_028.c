/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int;
typedef float scalar_float;
typedef double scalar_double;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with GCC attributes (may trigger TYPE_LANG_STRUCT) */
struct attributed_struct __attribute__((packed, aligned(8))) {
    long a;
    short b;
    char c;
};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int *int_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;
void *void_ptr;
struct plain_struct *struct_ptr;
union data_union *union_ptr;

/* Pointer to pointer */
int **int_ptr_ptr;

/* ========== TYPE_ARRAY examples ========== */
int int_array[10];
char char_array[5][5];
float float_3d_array[3][3][3];
struct plain_struct struct_array[5];
union data_union union_array[8];

/* ========== TYPE_STRING examples ========== */
const char *string_literal = "Hello, gengtype!";
char string_array[] = "Test string";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*simple_callback)(void);
typedef void (*complex_callback)(int, char*, struct plain_struct*);
typedef int (*callback_returning_ptr)(float**, union data_union***);

/* ========== Complex nested types ========== */
struct nested_struct {
    struct plain_struct inner_struct;
    union data_union data;
    int *pointer_member;
    int array_member[7];
    simple_callback callback_member;
};

/* Chain of pointers */
struct chain_element {
    int value;
    struct chain_element *next;
};

/* Self-referential structure */
struct tree_node {
    int data;
    struct tree_node *left;
    struct tree_node *right;
};

/* ========== Global variables with initializers ========== */
struct plain_struct global_struct = { 42, 3.14f, 'A' };
union data_union global_union = { .i = 100 };
int global_array[5] = {1, 2, 3, 4, 5};
const char *global_strings[] = {"first", "second", "third"};

/* Array of function pointers */
int (*func_ptr_array[3])(int, int);

/* ========== Volatile and const qualifiers ========== */
volatile int volatile_counter = 0;
const int read_only_value = 255;
volatile const int volatile_const_value = 99;
int *const const_pointer = &volatile_counter;
volatile int *volatile volatile_pointer_to_volatile = &volatile_counter;

/* ========== Pragmas for alignment ========== */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* ========== Function declarations ========== */
int add(int a, int b) { return a + b; }
void process_struct(struct plain_struct *s) { s->x++; }
int callback_impl(void) { return 42; }

/* ========== Main function to use all types ========== */
int main(void) {
    /* Use scalar types */
    scalar_int si = 10;
    scalar_float sf = 2.5f;
    color_enum ce = GREEN;
    
    /* Use struct types */
    struct plain_struct local_struct = {1, 2.0f, 'X'};
    user_struct_t user_struct = local_struct;
    struct attributed_struct attr_struct = {100L, 2, 'Z'};
    
    /* Use union */
    union data_union local_union;
    local_union.i = 42;
    
    /* Use pointers */
    int_ptr = &si;
    struct_ptr = &local_struct;
    union_ptr = &local_union;
    
    /* Use arrays */
    int_array[0] = 100;
    char_array[2][2] = 'C';
    
    /* Use strings */
    const char *local_string = string_array;
    
    /* Use callbacks */
    simple_callback cb = callback_impl;
    int result = cb();
    
    /* Use nested types */
    struct nested_struct nested = {
        .inner_struct = local_struct,
        .data = local_union,
        .pointer_member = &si,
        .callback_member = cb
    };
    
    /* Use chain */
    struct chain_element elem1 = {10, NULL};
    struct chain_element elem2 = {20, &elem1};
    
    /* Use tree */
    struct tree_node left = {1, NULL, NULL};
    struct tree_node right = {2, NULL, NULL};
    struct tree_node root = {0, &left, &right};
    
    /* Use volatile/const */
    volatile_counter++;
    int read_value = read_only_value;
    
    /* Use packed struct */
    struct packed_struct packed = {'P', 123, 456};
    
    /* Use function pointer array */
    func_ptr_array[0] = add;
    if (func_ptr_array[0]) {
        result = func_ptr_array[0](5, 3);
    }
    
    /* Prevent dead code elimination */
    volatile int anti_optimize = 0;
    anti_optimize += si + sf + ce + local_struct.x + local_union.i + 
                     *int_ptr + int_array[0] + char_array[0][0] +
                     nested.inner_struct.y + elem2.value + root.data +
                     packed.b + result;
    
    return anti_optimize == 0 ? 0 : 0;  /* Always return 0 */
}

/* ========== External declarations (simulating multi-file) ========== */
extern int external_variable;
extern struct plain_struct external_struct;

/* Function with complex return type */
struct nested_struct* complex_function(transparent_union_t arg) {
    static struct nested_struct static_nested;
    return &static_nested;
}
