/* test_gengtype_coverage.c - Comprehensive type coverage for gengtype */

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
int *const const_int_ptr;
volatile int *const volatile_const_ptr;

/* TYPE_ARRAY examples */
int int_array[10];
char char_array[5][5];
struct basic_struct struct_array[3];
union data_union union_array[4][2];

/* TYPE_STRING - string literals in initializers */
const char *greeting = "Hello, World!";
char message[] = "Test message";

/* TYPE_CALLBACK - function pointers */
typedef int (*compare_func)(const void *, const void *);
typedef void (*callback_func)(int, char *);
typedef struct basic_struct *(*factory_func)(void);

/* Complex nested types */
struct complex_nested {
    struct basic_struct inner;
    union data_union data;
    int *pointer_array[5];
    compare_func comparator;
    struct complex_nested *next;  /* Self-referential pointer */
};

/* GCC-specific attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(4))) packed_struct {
    char a;
    int b;
    char c;
};

struct __attribute__((aligned(16))) aligned_struct {
    double data[2];
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) trans_union {
    int *int_ptr;
    void *void_ptr;
} trans_union_t;

/* Volatile and const qualified complex type */
volatile const struct complex_nested * volatile const *complex_qual_ptr;

/* Array of function pointers */
callback_func callbacks[3];

/* Nested pointer chain */
struct chain_link {
    int value;
    struct chain_link *next;
};

/* Function pointer returning pointer to struct */
typedef struct basic_struct *(*struct_factory)(int, float);

/* Global variables using all types */
my_int global_int = 42;
my_float global_float = 3.14f;
color_enum global_color = GREEN;
struct basic_struct global_struct = {1, 2.0f, 'A', 3.14};
my_struct_t global_typedef_struct = {2, 4.0f, 'B', 6.28};
union data_union global_union = {.i = 100};
struct complex_nested global_complex = {
    .inner = {3, 5.0f, 'C', 9.42},
    .data = {.f = 3.14f},
    .comparator = NULL,
    .next = NULL
};
struct packed_struct global_packed = {'X', 999, 'Y'};
struct aligned_struct global_aligned = {{1.0, 2.0}};
trans_union_t global_trans_union = {.void_ptr = NULL};
struct chain_link global_chain = {42, NULL};

/* Initialize pointer array */
int *pointer_init[3] = {&global_int, NULL, &global_struct.a};

/* Initialize array with values */
int initialized_array[5] = {1, 2, 3, 4, 5};

/* Function using callback */
static void sample_callback(int x, char *str) {
    /* Empty callback for demonstration */
    (void)x;
    (void)str;
}

/* Function returning struct pointer */
static struct basic_struct *create_struct(int a, float b) {
    static struct basic_struct local;
    local.a = a;
    local.b = b;
    local.c = 'S';
    local.d = 0.0;
    return &local;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int++;
    global_float += 1.0f;
    global_color = BLUE;
    
    /* Use struct types */
    global_struct.a = 10;
    global_typedef_struct.b = 20.0f;
    global_complex.inner.c = 'Z';
    
    /* Use union */
    global_union.f = 3.14159f;
    
    /* Use pointers */
    if (int_ptr) *int_ptr = 5;
    if (struct_ptr) struct_ptr->a = 15;
    
    /* Use arrays */
    int_array[0] = 100;
    char_array[2][2] = 'X';
    struct_array[0].a = 1;
    union_array[1][0].i = 42;
    
    /* Use strings */
    if (greeting[0] != '\0') prevent_optimization++;
    message[0] = 'T';
    
    /* Use function pointers */
    callbacks[0] = sample_callback;
    if (callbacks[0]) callbacks[0](1, "test");
    
    /* Use complex nested types */
    if (global_complex.next) {
        global_complex.next->inner.a = 99;
    }
    
    /* Use packed and aligned structs */
    global_packed.a = 'P';
    global_aligned.data[0] = 3.0;
    
    /* Use transparent union */
    global_trans_union.int_ptr = &global_int;
    
    /* Use qualified pointer */
    if (complex_qual_ptr) {
        /* Access through volatile const pointer */
        const struct complex_nested *temp = *complex_qual_ptr;
        (void)temp;
    }
    
    /* Use chain */
    global_chain.value = 100;
    
    /* Use factory function */
    struct_factory factory = create_struct;
    if (factory) {
        struct basic_struct *new_struct = factory(1, 2.0f);
        (void)new_struct;
    }
    
    /* Use initialized arrays */
    initialized_array[0] = 99;
    pointer_init[0] = &prevent_optimization;
    
    return prevent_optimization > 0 ? 0 : 0;  /* Always return 0 */
}

/* Additional type in different linkage to test cross-file scenarios */
static struct static_only_struct {
    long long big_int;
    unsigned char bytes[8];
} static_instance = {0x123456789ABCDEF0LL, {0,1,2,3,4,5,6,7}};

/* Another callback type variation */
typedef void (*no_args_callback)(void);
static no_args_callback simple_callback = NULL;

/* Multi-dimensional pointer array */
int *multi_ptr_array[2][3];

/* Const array of structs */
const struct basic_struct const_structs[] = {
    {1, 1.0f, 'A', 1.0},
    {2, 2.0f, 'B', 2.0},
    {3, 3.0f, 'C', 3.0}
};

/* Anonymous struct/union */
struct {
    int tag;
    union {
        int i;
        float f;
    } value;
} anonymous_instance = {0, {.i = 42}};
