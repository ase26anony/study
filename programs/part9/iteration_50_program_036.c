/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int my_int;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef float my_float;

/* TYPE_STRUCT with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(4))) packed_struct {
    my_int a;
    char b;
    float c;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    int y;
} point_t;

/* TYPE_UNION with volatile member */
union data_union {
    int i;
    float f;
    char str[4];
    volatile long vl;
};

/* TYPE_ARRAY examples */
int int_array[10];
char char_matrix[5][5];
const float const_array[3] = {1.0f, 2.0f, 3.0f};

/* TYPE_POINTER examples */
int *int_ptr;
void *void_ptr;
struct packed_struct *struct_ptr;
const volatile int *cv_int_ptr;

/* TYPE_STRING */
const char *greeting = "Hello, gengtype!";
char filename[] = "test.txt";

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, char *);

/* Complex nested type for deep traversal */
struct nested_container {
    point_t points[5];
    union data_union *union_array[3];
    comparator_t compare_func;
    struct inner_struct {
        int id;
        char name[20];
        struct inner_struct *next;
    } *inner;
};

/* Another struct with GCC attributes */
struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
    char tag;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) trans_union {
    int *intp;
    void *vp;
} trans_union_t;

/* Function pointer returning pointer to struct */
typedef struct packed_struct *(*factory_t)(int);

/* Global variables using all types */
my_int global_int = 42;
color_enum global_color = GREEN;
my_float global_float = 3.14159f;
struct packed_struct global_packed = {1, 'A', 2.5f};
point_t global_point = {10, 20};
union data_union global_union = {.i = 255};
struct nested_container global_container;
struct aligned_struct global_aligned;
trans_union_t global_trans_union;
factory_t global_factory = NULL;

/* Initialize arrays */
int partial_array[7] = {1, 2, 3, [6] = 7};

/* Struct with bitfields (may affect type representation) */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int : 4; /* padding */
    signed int value : 8;
};

/* External declaration (simulating multi-file) */
extern int external_var;

/* Callback function definitions */
int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int val, char *msg) {
    /* Do nothing */
}

/* Main function to ensure all types are referenced */
int main(void) {
    /* Reference scalar types */
    volatile int prevent_optimization = global_int + global_color;
    
    /* Use struct types */
    global_point.x = 100;
    global_packed.b = 'B';
    
    /* Use union */
    global_union.f = 2.71828f;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[2][2] = 'X';
    
    /* Use pointers */
    int_ptr = &global_int;
    struct_ptr = &global_packed;
    
    /* Use string */
    char first_char = greeting[0];
    
    /* Use function pointers */
    comparator_t comp = sample_comparator;
    callback_t cb = sample_callback;
    
    /* Use nested types */
    global_container.points[0].x = 5;
    global_container.compare_func = comp;
    
    /* Use transparent union */
    global_trans_union.intp = &global_int;
    
    /* Use aligned struct */
    global_aligned.tag = 'Z';
    
    /* Use bitfield struct */
    struct bitfield_struct bfs = {0};
    bfs.flag1 = 1;
    bfs.value = 127;
    
    /* Prevent dead code elimination */
    if (prevent_optimization > 1000) {
        cb(0, NULL);
    }
    
    return 0;
}

/* Additional type in file scope */
static struct {
    int hidden;
    char secret[10];
} file_local_struct = {42, "secret"};
