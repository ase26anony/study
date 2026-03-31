/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_SCALAR examples */
typedef int my_int;
typedef float my_float;
typedef enum { RED, GREEN, BLUE } color_t;

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

/* TYPE_UNION with volatile */
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

/* TYPE_STRING usage */
const char *greeting = "Hello, gengtype!";
char initialized[] = "Test string";

/* TYPE_POINTER examples */
int *int_ptr;
struct packed_struct *struct_ptr;
void *void_ptr;
const volatile int *cv_int_ptr;

/* TYPE_CALLBACK (function pointer) */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, char *);

/* Complex nested type for deep traversal */
struct nested_container {
    struct packed_struct inner;
    union data_union data;
    point_t points[3];
    comparator_t cmp;
    struct nested_container *next;  /* Self-referential pointer */
};

/* GCC-specific pragma */
#pragma pack(push, 1)
struct gcc_packed {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} transparent_union_t;

/* Another struct with bitfields (GCC extension) */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_member;
};

/* Function pointer returning pointer to struct */
typedef struct nested_container *(*allocator_t)(size_t);

/* Global variables using all types */
struct packed_struct global_packed = {1, 'A', 3.14f};
point_t global_point = {10, 20};
union data_union global_union = {.i = 42};
struct nested_container global_container = {
    .inner = {2, 'B', 2.71f},
    .data = {.f = 3.14f},
    .points = {{1,1}, {2,2}, {3,3}},
    .cmp = NULL,
    .next = NULL
};
struct gcc_packed global_gcc_packed = {'X', 100, 50};
struct bitfield_struct global_bitfield = {1, 3, 9, -1};

/* Function pointer variables */
comparator_t global_comparator = NULL;
allocator_t global_allocator = NULL;

/* Array of function pointers */
callback_t callbacks[2];

/* Complex pointer chain */
int ***triple_ptr = NULL;

/* External declaration (simulates multi-file) */
extern void external_function(struct nested_container *);

/* Minimal main to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_packed.a = prevent_optimization + 1;
    global_point.x++;
    
    /* Use union */
    global_union.i = 100;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[0][0] = 'A';
    
    /* Use pointers */
    if (int_ptr) *int_ptr = 0;
    if (struct_ptr) struct_ptr->a = 0;
    
    /* Use string */
    if (greeting[0]) prevent_optimization++;
    
    /* Use nested struct */
    global_container.inner.b = 'C';
    global_container.points[0].x = 5;
    
    /* Use GCC-specific structs */
    global_gcc_packed.a = 'Y';
    global_bitfield.flag1 = 0;
    
    /* Use function pointer if safe */
    if (global_comparator) {
        int result = global_comparator(&global_point, &global_container);
        prevent_optimization += result;
    }
    
    /* Use triple pointer */
    if (triple_ptr && *triple_ptr && **triple_ptr) {
        ***triple_ptr = 999;
    }
    
    /* Use transparent union */
    transparent_union_t tu;
    tu.intp = &prevent_optimization;
    
    /* Use callbacks array */
    if (callbacks[0]) {
        callbacks[0](1, initialized);
    }
    
    return prevent_optimization > 0 ? 0 : 0;  /* Always return 0 */
}

/* Function definitions */
static int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int val, char *str) {
    str[0] = (char)val;
}

/* Initialize function pointers */
void init_pointers(void) {
    global_comparator = sample_comparator;
    callbacks[0] = sample_callback;
}
