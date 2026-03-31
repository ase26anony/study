/* gengtype-coverage-test.c
 * Comprehensive test to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* ==================== TYPE_SCALAR ==================== */
typedef enum color { RED, GREEN, BLUE } color_t;
typedef int my_int;
typedef float my_float;
typedef double my_double;

/* ==================== TYPE_STRUCT ==================== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* ==================== TYPE_USER_STRUCT ==================== */
typedef struct plain_struct my_struct_t;

/* ==================== TYPE_UNION ==================== */
union data_union {
    int i;
    float f;
    char c;
    void *p;
};

/* ==================== TYPE_POINTER ==================== */
typedef int* int_ptr_t;
typedef struct plain_struct* struct_ptr_t;
typedef void (*void_func_ptr)(void);

/* ==================== TYPE_ARRAY ==================== */
typedef int int_array_10[10];
typedef char char_matrix[5][5];
typedef struct plain_struct struct_array[3];

/* ==================== TYPE_STRING ==================== */
const char* greeting = "Hello, gengtype!";

/* ==================== TYPE_CALLBACK ==================== */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, float, void*);

/* ==================== Complex Nesting ==================== */
struct nested_struct {
    struct plain_struct base;
    union data_union data;
    int_array_10 numbers;
    char_matrix matrix;
    struct nested_struct* next;  /* Self-referential pointer */
    comparator_t compare_func;
};

union complex_union {
    struct nested_struct ns;
    struct plain_struct ps;
    int_ptr_t ip;
    callback_t cb;
};

/* ==================== GCC Attributes ==================== */
struct __attribute__((packed, aligned(4))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union_t {
    int i;
    long l;
};

struct __attribute__((aligned(32))) aligned_struct {
    double data[4];
    volatile int counter;
};

/* ==================== TYPE_LANG_STRUCT (GCC extensions) ==================== */
#ifdef __GNUC__
struct gcc_ext_struct {
    int field __attribute__((aligned(16)));
    char flexible_array[];
} __attribute__((packed));
#endif

/* ==================== Volatile and Const Qualifiers ==================== */
volatile int global_counter = 0;
const float PI = 3.14159f;
volatile int* volatile volatile_ptr = &global_counter;
const struct plain_struct* const const_struct_ptr = NULL;

/* ==================== Global Variables ==================== */
struct plain_struct g_struct = {1, 2.0f, 'A'};
union data_union g_union = {.i = 42};
my_struct_t g_user_struct = {2, 3.0f, 'B'};
int_array_10 g_array = {0,1,2,3,4,5,6,7,8,9};
char_matrix g_matrix = {"abcd","efgh","ijkl","mnop","qrst"};
struct nested_struct g_nested = {
    .base = {3, 4.0f, 'C'},
    .data = {.f = 3.14f},
    .numbers = {10,11,12,13,14,15,16,17,18,19},
    .matrix = {{'x','y'},{'z','w'}},
    .next = NULL,
    .compare_func = NULL
};
struct packed_struct g_packed = {'X', 999, 77};
aligned_struct g_aligned = {{1.0, 2.0, 3.0, 4.0}, 100};

/* ==================== Function Pointers ==================== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(int x, float y, void* data) {
    *(int*)data = x + (int)y;
}

/* ==================== Main Function ==================== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    g_struct.x = prevent_optimization + 1;
    g_struct.y = 5.0f;
    
    /* Use union */
    g_union.i = 100;
    
    /* Use user struct */
    g_user_struct.z = 'D';
    
    /* Use array */
    g_array[0] = prevent_optimization;
    
    /* Use matrix */
    g_matrix[0][0] = 'Z';
    
    /* Use nested struct */
    g_nested.base.x = 99;
    g_nested.data.i = 123;
    
    /* Use packed struct */
    g_packed.a = 'Y';
    
    /* Use aligned struct */
    g_aligned.counter++;
    
    /* Use pointers */
    int local_var = 42;
    int_ptr_t ptr = &local_var;
    *ptr = 43;
    
    struct_ptr_t sptr = &g_struct;
    sptr->x = 44;
    
    /* Use volatile pointer */
    *volatile_ptr = 1;
    
    /* Use function pointer */
    comparator_t cmp = compare_ints;
    int a = 5, b = 10;
    int result = cmp(&a, &b);
    
    callback_t cb = sample_callback;
    int cb_data = 0;
    cb(10, 20.5f, &cb_data);
    
    /* Use string */
    const char* local_greeting = greeting;
    prevent_optimization += local_greeting[0];
    
    /* Use enum */
    color_t col = RED;
    prevent_optimization += col;
    
    /* Use typedef scalar */
    my_int mi = 1000;
    my_float mf = 3.14f;
    prevent_optimization += mi + (int)mf;
    
    return prevent_optimization == 0 ? 0 : 0;  /* Always return 0 */
}

/* ==================== Additional Translation Unit Simulation ==================== */
#ifdef MULTI_TU
/* In a real multi-TU test, this would be in a separate file */
extern struct plain_struct g_struct;
extern int_array_10 g_array;

void external_function(void) {
    g_struct.x++;
    g_array[1] = 999;
}
#endif
