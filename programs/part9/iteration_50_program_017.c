/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct basic_struct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    double x, y;
} point_t;

/* TYPE_UNION: Union with several members */
union data_union {
    int int_val;
    float float_val;
    char char_val;
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct basic_struct *struct_ptr;
void *void_ptr;
const char *const_string_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct basic_struct struct_array[3];
point_t point_array[4][4];

/* TYPE_SCALAR: Scalar types */
enum color { RED, GREEN, BLUE };
typedef enum color color_t;
unsigned long long big_scalar;
_Bool boolean_scalar;

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, gengtype!";
char default_name[] = "default";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*comparator_t)(const void *, const void *);
typedef void (*callback_t)(int, void *);

/* TYPE_LANG_STRUCT: GCC-specific constructs */
#ifdef __GNUC__
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union {
    int *int_ptr;
    void *void_ptr;
};

#pragma pack(push, 1)
struct pragma_packed {
    char c;
    int i;
    double d;
};
#pragma pack(pop)
#endif

/* Complex nesting and chains */
struct nested_container {
    struct basic_struct inner;
    union data_union data;
    struct nested_container *next;  /* Pointer to self */
    comparator_t compare_func;
    char buffer[256];
};

/* More complex type chains */
typedef struct node {
    int value;
    struct node *left;
    struct node *right;
    void (*visit)(struct node *);
} node_t;

/* Function pointer returning pointer to struct */
typedef struct basic_struct *(*factory_t)(int, const char *);

/* Volatile and const qualifiers in complex combinations */
const volatile int *const volatile_qualified;
int *const const_pointer;
const struct basic_struct *const_struct_ptr;

/* Global variables using all declared types */
struct basic_struct global_struct = {1, 3.14f, "test"};
point_t global_point = {2.5, 3.5};
union data_union global_union = {.int_val = 42};
node_t *global_tree = NULL;
comparator_t global_comparator = NULL;
factory_t global_factory = NULL;

#ifdef __GNUC__
struct packed_struct global_packed = {'a', 123, 456};
#endif

/* Function using callback */
static void sample_callback(int event, void *data) {
    /* Empty callback for demonstration */
    (void)event;
    (void)data;
}

/* Function with complex return type */
static struct basic_struct *create_struct(int id, const char *name) {
    static struct basic_struct local;
    local.id = id;
    if (name) {
        for (int i = 0; i < 31 && name[i]; i++) {
            local.name[i] = name[i];
        }
    }
    return &local;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 100;
    prevent_optimization += global_struct.id;
    
    /* Use union */
    global_union.float_val = 3.14159f;
    prevent_optimization += (int)global_union.float_val;
    
    /* Use pointers */
    if (int_ptr) prevent_optimization++;
    if (struct_ptr) prevent_optimization++;
    
    /* Use arrays */
    int_array[0] = 1;
    prevent_optimization += int_array[0];
    
    char_matrix[0][0] = 'A';
    prevent_optimization += char_matrix[0][0];
    
    /* Use scalar types */
    color_t col = RED;
    prevent_optimization += col;
    
    big_scalar = 0xFFFFFFFFFFFFFFFFULL;
    prevent_optimization += (big_scalar > 0);
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use callback */
    global_comparator = (comparator_t)NULL;
    if (global_comparator) prevent_optimization++;
    
    /* Use function pointer */
    global_factory = create_struct;
    if (global_factory) {
        struct basic_struct *s = global_factory(1, "test");
        prevent_optimization += s->id;
    }
    
    /* Use nested types */
    struct nested_container container = {
        .inner = {2, 2.718f, "nested"},
        .data = {.int_val = 99},
        .next = NULL,
        .compare_func = NULL,
        .buffer = "test buffer"
    };
    prevent_optimization += container.inner.id;
    
    /* Use volatile/const qualified */
    if (volatile_qualified) prevent_optimization++;
    if (const_pointer) prevent_optimization++;
    
#ifdef __GNUC__
    /* Use GCC-specific types */
    prevent_optimization += global_packed.a;
    prevent_optimization += global_packed.b;
#endif
    
    /* Call callback */
    sample_callback(1, &prevent_optimization);
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct {
    int hidden_field;
    point_t hidden_point;
} static_struct = {999, {1.0, 2.0}};
