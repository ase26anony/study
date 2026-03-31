/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover all TYPE_* cases in gengtype-state.cc
 */

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

/* TYPE_USER_STRUCT via typedef */
typedef struct plain_struct user_struct_t;

/* Struct with GCC attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) attributed_struct {
    int a;
    double b;
    char c;
} __attribute__((aligned(16)));

/* ========== TYPE_UNION examples ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int *int_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;
void *void_ptr;
struct plain_struct *struct_ptr;
union basic_union *union_ptr;
int *const const_ptr = &(int){0};
volatile int *restrict volatile_restrict_ptr;

/* Function pointer (TYPE_CALLBACK) */
typedef int (*callback_func)(int, float);
typedef void (*void_callback)(void);

/* Complex callback returning pointer to struct */
typedef struct plain_struct *(*struct_callback)(int);

/* ========== TYPE_ARRAY examples ========== */
int int_array[10];
char char_array[5][5];
float multi_dim[2][3][4];
struct plain_struct struct_array[3];
union basic_union union_array[5];

/* Array with initialization */
int initialized_array[5] = {1, 2, 3, 4, 5};

/* ========== TYPE_STRING examples ========== */
const char *string_literal = "Hello, gengtype!";
char string_array[] = "Initialized string";

/* ========== Complex nested types ========== */
struct nested_struct {
    struct plain_struct inner_struct;
    struct nested_struct *self_ptr;  /* Recursive pointer */
    int *int_ptr_array[5];
    union {
        int x;
        float y;
    } anonymous_union;
    callback_func func_ptr;
    struct attributed_struct attr_struct;
};

/* Union containing array of structs */
union complex_union {
    struct nested_struct nested;
    struct plain_struct struct_array[3];
    callback_func func_ptrs[2];
};

/* ========== Global variables ========== */
struct plain_struct global_struct = {1, 2.0f, 'A'};
union basic_union global_union = {.as_int = 42};
user_struct_t global_user_struct = {2, 3.0f, 'B'};
struct attributed_struct global_attributed_struct = {10, 20.5, 'C'};
transparent_union_t global_transparent_union = {.void_ptr = NULL};

int *global_int_ptr = &(int){100};
callback_func global_callback = NULL;
struct nested_struct global_nested = {
    .inner_struct = {3, 4.0f, 'D'},
    .self_ptr = NULL,
    .func_ptr = NULL,
    .attr_struct = {5, 6.0, 'E'}
};

/* ========== Function using types ========== */
int sample_callback(int a, float b) {
    return a + (int)b;
}

struct plain_struct *struct_returning_callback(int x) {
    static struct plain_struct result = {0};
    result.x = x;
    return &result;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    scalar_int si = 10;
    scalar_float sf = 3.14f;
    color_enum ce = GREEN;
    prevent_optimization += si + (int)sf + ce;
    
    /* Use struct types */
    struct plain_struct local_struct = {5, 6.0f, 'F'};
    user_struct_t local_user = {6, 7.0f, 'G'};
    prevent_optimization += local_struct.x + local_user.x;
    
    /* Use union types */
    union basic_union local_union;
    local_union.as_int = 100;
    prevent_optimization += local_union.as_int;
    
    /* Use pointers */
    int local_int = 50;
    int_ptr = &local_int;
    prevent_optimization += *int_ptr;
    
    /* Use arrays */
    prevent_optimization += int_array[0];
    prevent_optimization += char_array[0][0];
    
    /* Use strings */
    prevent_optimization += string_literal[0];
    prevent_optimization += string_array[0];
    
    /* Use callbacks */
    callback_func local_callback = sample_callback;
    if (local_callback) {
        prevent_optimization += local_callback(1, 2.0f);
    }
    
    /* Use nested struct */
    global_nested.self_ptr = &global_nested;
    prevent_optimization += global_nested.inner_struct.x;
    
    /* Use attributed struct */
    prevent_optimization += global_attributed_struct.a;
    
    /* Use transparent union */
    int transparent_int = 200;
    transparent_union_t tu = {.int_ptr = &transparent_int};
    prevent_optimization += *tu.int_ptr;
    
    /* Complex chain */
    struct_callback scb = struct_returning_callback;
    if (scb) {
        struct plain_struct *sptr = scb(42);
        prevent_optimization += sptr->x;
    }
    
    /* Use all global variables */
    prevent_optimization += global_struct.x;
    prevent_optimization += global_union.as_int;
    prevent_optimization += global_user_struct.x;
    
    return prevent_optimization == 0 ? 0 : 0;  /* Always return 0 */
}

/* ========== Additional declarations ========== */
/* Forward declarations to test TYPE_UNDEFINED? */
struct forward_declared;
typedef struct forward_declared forward_t;

/* Multiple translation unit simulation */
extern int external_variable;
static int static_variable = 42;

/* Pragmas for GCC */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    char c;
};
#pragma pack(pop)

/* Inline assembly might affect type analysis */
static int asm_global __asm__("custom_asm_symbol") = 100;

/* Alias via asm */
extern int aliased_int __asm__("global_alias");

/* Weak symbol */
int weak_symbol __attribute__((weak)) = 0;

/* Section attribute */
int __attribute__((section(".custom_section"))) section_var = 999;
