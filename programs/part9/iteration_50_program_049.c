/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(4))) base_struct {
    my_int id;
    my_float value;
    char tag;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct base_struct my_struct_t;

/* TYPE_UNION with volatile */
union data_union {
    my_int as_int;
    my_float as_float;
    volatile char as_char;
    void *as_ptr;
};

/* TYPE_ARRAY examples */
typedef int matrix[3][3];
typedef char string_array[5][32];

/* TYPE_POINTER examples with qualifiers */
typedef const int *const_int_ptr;
typedef volatile struct base_struct *volatile_struct_ptr;
typedef void (*void_func_ptr)(void);

/* TYPE_CALLBACK - function pointer types */
typedef int (*comparator)(const void *, const void *);
typedef void (*callback_func)(int, union data_union *);

/* TYPE_STRING usage */
const char *greeting = "Hello, gengtype!";
static const char *messages[] = {"msg1", "msg2", "msg3"};

/* Complex nested type for deep traversal */
struct complex_nested {
    struct base_struct base;
    union data_union data;
    matrix transform;
    const_int_ptr const_ptr;
    struct complex_nested *next;  /* Self-referential pointer */
    callback_func callback;
};

/* GCC-specific pragma */
#pragma pack(push, 1)
struct packed_lang_struct {
    unsigned char flags;
    unsigned int data;
    void *extended;
} __attribute__((aligned(8)));
#pragma pack(pop)

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* Global variables of each type */
struct base_struct global_struct = {1, 3.14f, 'A'};
union data_union global_union = {.as_int = 42};
my_struct_t user_struct_var = {2, 2.718f, 'B'};
matrix global_matrix = {{1,2,3},{4,5,6},{7,8,9}};
const_int_ptr global_const_ptr = (const int[]){10, 20, 30};
volatile_struct_ptr global_volatile_ptr = (volatile struct base_struct *)&global_struct;
comparator global_comparator = NULL;
struct complex_nested global_nested = {
    .base = {3, 1.414f, 'C'},
    .data = {.as_float = 3.14159f},
    .transform = {{0}},
    .const_ptr = global_const_ptr,
    .next = NULL,
    .callback = NULL
};
struct packed_lang_struct global_packed = {0xFF, 0xDEADBEEF, NULL};
transparent_union_t global_transparent = {.void_ptr = NULL};

/* Array of pointers to different types */
void *type_pointer_array[] = {
    &global_struct,
    &global_union,
    &user_struct_var,
    &global_matrix,
    (void *)global_const_ptr,
    (void *)global_volatile_ptr,
    global_comparator,
    &global_nested,
    &global_packed,
    global_transparent.void_ptr,
    (void *)greeting,
    (void *)messages
};

/* Function using callback type */
static int sample_comparator(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

static void sample_callback(int val, union data_union *data) {
    data->as_int = val * 2;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = prevent_optimization + 1;
    
    /* Use union */
    global_union.as_int = 100;
    
    /* Use user struct */
    user_struct_tag = 'X';
    
    /* Use array */
    global_matrix[0][0] = prevent_optimization;
    
    /* Use pointers */
    if (global_const_ptr) {
        prevent_optimization += global_const_ptr[0];
    }
    
    /* Use function pointer */
    global_comparator = sample_comparator;
    
    /* Use nested struct */
    global_nested.next = &global_nested;
    global_nested.callback = sample_callback;
    
    /* Use packed struct */
    global_packed.flags = 0xAA;
    
    /* Use transparent union */
    int local_int = 5;
    global_transparent.int_ptr = &local_int;
    
    /* Use string */
    if (greeting[0] != '\0') {
        prevent_optimization++;
    }
    
    /* Call through callback */
    if (global_nested.callback) {
        union data_union temp;
        global_nested.callback(42, &temp);
    }
    
    /* Use pointer array */
    for (size_t i = 0; i < sizeof(type_pointer_array)/sizeof(type_pointer_array[0]); i++) {
        if (type_pointer_array[i]) {
            prevent_optimization++;
        }
    }
    
    return prevent_optimization > 0 ? 0 : 1;
}
