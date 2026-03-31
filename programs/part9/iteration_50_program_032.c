/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRING example ========== */
const char* greeting = "Hello, gengtype!";

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int id;
    float value;
    char name[32];
};

/* Struct with GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) attributed_struct {
    volatile int counter;
    const double pi;
    char data[16];
};

/* ========== TYPE_USER_STRUCT example ========== */
typedef struct plain_struct user_struct_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
    int as_int;
    float as_float;
    char as_bytes[4];
    void* as_pointer;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    void* void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int* int_ptr;
volatile int* volatile volatile_int_ptr;
const char* const const_string_ptr = "constant";
struct plain_struct* struct_ptr;
void (*function_ptr)(void);
void* void_ptr_array[10];

/* ========== TYPE_ARRAY examples ========== */
int int_array[10];
float float_matrix[5][5];
char* string_array[3];
struct plain_struct struct_array[4];
union data_union union_array[8];

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, void*);
typedef struct plain_struct* (*factory_t)(void);

/* Complex nested callback */
typedef void (*nested_callback_t)(comparator_t, factory_t);

/* ========== Complex nested type chains ========== */
struct nested_container {
    /* Pointer to struct */
    struct attributed_struct* attr_ptr;
    
    /* Array of pointers to unions */
    union data_union* union_ptrs[5];
    
    /* Pointer to array */
    int (*matrix_ptr)[5][5];
    
    /* Callback member */
    callback_t notify;
    
    /* Nested struct */
    struct {
        int depth;
        struct nested_container* next;
    } inner;
    
    /* Flexible array member */
    char flexible_array[];
};

/* ========== TYPE_LANG_STRUCT candidates ========== */
/* Use #pragma pack to potentially trigger language-specific handling */
#pragma pack(push, 1)
struct packed_lang_struct {
    unsigned char type;
    unsigned int size;
    void* data;
};
#pragma pack(pop)

/* Another GCC extension */
struct __attribute__((aligned(32), may_alias)) aligned_struct {
    long double big_value;
    char padding[16];
};

/* ========== Global variables with initializers ========== */
struct plain_struct global_struct = { 
    .id = 42, 
    .value = 3.14f, 
    .name = "test" 
};

union data_union global_union = { .as_int = 0xDEADBEEF };

int global_array[3] = {1, 2, 3};

color_enum global_color = GREEN;

/* Function pointer with implementation */
static void sample_callback(int value, void* context) {
    volatile int* ptr = (volatile int*)context;
    *ptr = value;
}

comparator_t global_comparator = NULL;
factory_t global_factory = NULL;

/* ========== Main function to ensure usage ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    scalar_int_t x = 10;
    scalar_float_t y = 2.5f;
    prevent_optimization += x + (int)y;
    
    /* Use struct types */
    struct plain_struct local_struct = global_struct;
    local_struct.id++;
    prevent_optimization += local_struct.id;
    
    /* Use union type */
    union data_union local_union;
    local_union.as_int = 100;
    prevent_optimization += local_union.as_int;
    
    /* Use pointer types */
    int_ptr = &x;
    prevent_optimization += *int_ptr;
    
    struct_ptr = &global_struct;
    prevent_optimization += struct_ptr->id;
    
    /* Use array types */
    prevent_optimization += int_array[0];
    prevent_optimization += float_matrix[0][0];
    
    /* Use callback type */
    callback_t cb = sample_callback;
    if (cb) {
        cb(42, (void*)&prevent_optimization);
    }
    
    /* Use string */
    const char* local_greeting = greeting;
    while (*local_greeting) {
        prevent_optimization += *local_greeting++;
    }
    
    /* Complex nested access */
    struct nested_container container;
    container.notify = cb;
    if (container.notify) {
        container.notify(99, (void*)&prevent_optimization);
    }
    
    /* Use transparent union */
    transparent_union_t tu;
    int local_int = 5;
    tu.int_ptr = &local_int;
    prevent_optimization += *tu.int_ptr;
    
    /* Ensure all types are referenced to avoid dead code elimination */
    (void)global_union;
    (void)global_array;
    (void)global_color;
    (void)global_comparator;
    (void)global_factory;
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional declarations in different linkage */
static struct packed_lang_struct static_lang_struct;
extern struct aligned_struct external_aligned_struct;

/* Function returning complex pointer type */
struct nested_container** get_container_chain(void) {
    static struct nested_container* chain[3];
    return chain;
}

/* Typedef chain for deep type traversal */
typedef struct nested_container*** container_ref_chain_t;
typedef container_ref_chain_t (*chain_getter_t)(void);
