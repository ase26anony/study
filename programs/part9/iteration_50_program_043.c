/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int my_int;                     /* Basic scalar typedef */
typedef enum { RED, GREEN, BLUE } color_t;  /* Enum scalar type */
typedef float my_float;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) attributed_struct {
    int id;
    char data[16];
    volatile long counter;
};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;
typedef struct attributed_struct attr_struct_t;

/* ========== TYPE_UNION examples ========== */
union basic_union {
    int as_int;
    float as_float;
    void *as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
int *int_ptr;
const int *const_ptr;
volatile int *volatile_ptr;
struct plain_struct *struct_ptr;
void (*func_ptr)(void);
int *const const_int_ptr = (int*)0x1000;

/* Complex pointer chain */
struct node {
    int value;
    struct node *next;
    struct node *prev;
};

/* ========== TYPE_ARRAY examples ========== */
int simple_array[10];
int multi_dim_array[5][5];
char string_array[3][20];
const int const_array[5] = {1, 2, 3, 4, 5};

/* Array of pointers */
int *ptr_array[8];

/* ========== TYPE_STRING examples ========== */
const char *string_literal = "Hello, gengtype!";
char initialized_string[] = "Initialized array";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_func)(int, void *);

struct with_callback {
    int id;
    callback_func callback;
};

/* Function pointer returning pointer to struct */
typedef struct plain_struct *(*struct_factory_t)(int);

/* ========== Complex nested types ========== */
struct complex_nested {
    union {
        struct {
            int a;
            float b;
        } inner_struct;
        long long big_num;
    } data_union;
    
    struct complex_nested *self_ptr;
    int (*operations[5])(int, int);
    char flexible_array[];
};

/* Packed struct with bitfields */
#pragma pack(push, 1)
struct packed_with_bitfields {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int value : 12;
    unsigned char byte;
};
#pragma pack(pop)

/* ========== Global variable definitions ========== */
/* TYPE_STRUCT instance */
struct plain_struct global_struct = { .x = 42, .y = 3.14f, .z = 'A' };

/* TYPE_USER_STRUCT instance */
user_struct_t user_struct_instance = { .x = 100, .y = 2.718f, .z = 'B' };

/* TYPE_UNION instance */
union basic_union global_union = { .as_int = 0xDEADBEEF };

/* TYPE_ARRAY instances */
int global_array[3] = {10, 20, 30};
struct plain_struct struct_array[2] = {
    {1, 1.1f, 'X'},
    {2, 2.2f, 'Y'}
};

/* TYPE_POINTER instances */
struct node *list_head = NULL;
comparator_t global_comparator = NULL;

/* TYPE_CALLBACK instance */
void sample_callback(int x, void *data) {
    *(int*)data = x * 2;
}

/* External declaration (simulating multi-TU) */
extern int external_variable;

/* ========== Main function to use all types ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use TYPE_SCALAR */
    my_int scalar = 100;
    color_t color = GREEN;
    my_float fval = 3.14159f;
    prevent_optimization += scalar + color + (int)fval;
    
    /* Use TYPE_STRUCT */
    struct plain_struct local_struct = {0};
    local_struct.x = 10;
    local_struct.y = 20.5f;
    prevent_optimization += local_struct.x;
    
    /* Use TYPE_USER_STRUCT */
    user_struct_t local_user = global_struct;
    prevent_optimization += local_user.x;
    
    /* Use TYPE_UNION */
    union basic_union local_union;
    local_union.as_float = 123.456f;
    prevent_optimization += (int)local_union.as_float;
    
    /* Use TYPE_POINTER */
    int *local_ptr = &scalar;
    *local_ptr = 999;
    prevent_optimization += *local_ptr;
    
    struct node node1 = {1, NULL, NULL};
    struct node node2 = {2, NULL, &node1};
    node1.next = &node2;
    prevent_optimization += node1.value + node2.value;
    
    /* Use TYPE_ARRAY */
    simple_array[0] = 100;
    multi_dim_array[2][2] = 50;
    prevent_optimization += simple_array[0] + multi_dim_array[2][2];
    
    /* Use TYPE_STRING */
    const char *local_str = string_literal;
    prevent_optimization += local_str[0];
    
    /* Use TYPE_CALLBACK */
    struct with_callback cb_struct = {0};
    cb_struct.callback = sample_callback;
    
    int callback_data = 0;
    if (cb_struct.callback) {
        cb_struct.callback(21, &callback_data);
        prevent_optimization += callback_data;
    }
    
    /* Use complex nested type */
    struct complex_nested *nested = (struct complex_nested*)malloc(
        sizeof(struct complex_nested) + 10);
    if (nested) {
        nested->data_union.inner_struct.a = 100;
        nested->self_ptr = nested;
        prevent_optimization += nested->data_union.inner_struct.a;
        free(nested);
    }
    
    /* Use packed struct */
    struct packed_with_bitfields packed = {0};
    packed.flag1 = 1;
    packed.value = 2047;
    prevent_optimization += packed.value;
    
    /* Use transparent union */
    int transparent_test = 42;
    transparent_union_t tu;
    tu.int_ptr = &transparent_test;
    prevent_optimization += *tu.int_ptr;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional function using function pointer type */
void register_callback(callback_func func) {
    static callback_func stored = NULL;
    stored = func;
}

/* Force inclusion of all types in object file */
static void __attribute__((used)) force_references(void) {
    /* Reference all global variables */
    (void)global_struct;
    (void)user_struct_instance;
    (void)global_union;
    (void)global_array;
    (void)struct_array;
    (void)list_head;
    (void)global_comparator;
    (void)string_literal;
    (void)initialized_string;
    (void)int_ptr;
    (void)const_ptr;
    (void)volatile_ptr;
    (void)struct_ptr;
    (void)func_ptr;
    (void)const_int_ptr;
    (void)simple_array;
    (void)multi_dim_array;
    (void)string_array;
    (void)const_array;
    (void)ptr_array;
}
