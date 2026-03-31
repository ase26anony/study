/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ===== TYPE_SCALAR ===== */
typedef int scalar_t;
typedef enum { RED, GREEN, BLUE } color_t;
typedef float float_t;

/* ===== TYPE_STRING ===== */
const char* string_literal = "Hello, gengtype!";
static const char* const static_string = "Static string";

/* ===== TYPE_ARRAY ===== */
int int_array[10];
float float_2d_array[5][5];
const char* string_array[] = {"one", "two", "three"};

/* ===== TYPE_POINTER ===== */
int* int_ptr;
void* void_ptr;
const volatile int* volatile const volatile_ptr;

/* ===== TYPE_STRUCT ===== */
struct plain_struct {
    int x;
    scalar_t y;
    float_t z;
    char name[32];
};

/* ===== TYPE_USER_STRUCT ===== */
typedef struct plain_struct user_struct_t;

/* ===== TYPE_UNION ===== */
union data_union {
    int i;
    float f;
    double d;
    char str[16];
};

/* ===== TYPE_CALLBACK ===== */
typedef int (*callback_t)(int, const char*);
typedef void (*void_callback_t)(void);
typedef struct plain_struct* (*struct_factory_t)(void);

/* ===== TYPE_LANG_STRUCT (GCC extensions) ===== */
struct __attribute__((packed, aligned(8))) packed_struct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) transparent_union {
    int* int_ptr;
    void* void_ptr;
};

struct __attribute__((may_alias)) aliasing_struct {
    long long data;
};

/* ===== Complex nested types ===== */
typedef struct node {
    int value;
    struct node* next;  /* TYPE_POINTER to TYPE_STRUCT */
    struct node* prev;
    union data_union data;  /* TYPE_UNION */
    callback_t handler;  /* TYPE_CALLBACK */
} node_t;

typedef struct container {
    node_t* nodes[10];  /* TYPE_ARRAY of TYPE_POINTER to TYPE_USER_STRUCT */
    int (*comparator)(const void*, const void*);  /* TYPE_CALLBACK */
    union {
        int count;
        float ratio;
    } metrics __attribute__((packed));
    
    struct __attribute__((packed)) {
        unsigned char flags[4];
        volatile int status;
    } internal;
} container_t;

/* ===== More complex chains ===== */
typedef container_t* (*container_allocator_t)(size_t);
typedef void (*container_deleter_t)(container_t**);

struct master_container {
    container_t** containers;  /* TYPE_POINTER to TYPE_POINTER to TYPE_STRUCT */
    container_allocator_t alloc;
    container_deleter_t free;
    volatile int ref_count;
    
    struct {
        int max_items;
        int current_items;
    } __attribute__((packed)) stats;
};

/* ===== Global variables using all types ===== */
scalar_t global_scalar = 42;
color_t global_color = GREEN;
float_t global_float = 3.14159f;

struct plain_struct global_struct = {1, 2, 3.0f, "test"};
user_struct_t global_user_struct = {4, 5, 6.0f, "user"};

union data_union global_union = {.i = 100};
int* global_int_ptr = &global_scalar;
void* global_void_ptr = NULL;

int global_int_array[3] = {1, 2, 3};
float global_float_array[2][2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};

callback_t global_callback = NULL;
void_callback_t global_void_callback = NULL;

struct packed_struct global_packed = {'A', 123, 45.67};
union transparent_union global_transparent;

node_t global_node = {
    .value = 999,
    .next = NULL,
    .prev = NULL,
    .data = {.i = 42},
    .handler = NULL
};

container_t global_container = {
    .nodes = {NULL},
    .comparator = NULL,
    .metrics = {.count = 0},
    .internal = {{0}, 0}
};

struct master_container global_master = {
    .containers = NULL,
    .alloc = NULL,
    .free = NULL,
    .ref_count = 1,
    .stats = {100, 0}
};

/* ===== Function definitions ===== */
static int sample_callback(int x, const char* str) {
    volatile int result = x;  /* Prevent optimization */
    if (str) result += 1;
    return result;
}

static struct plain_struct* create_struct(void) {
    static struct plain_struct s = {0, 0, 0.0f, ""};
    return &s;
}

static container_t* create_container(size_t size) {
    (void)size;  /* Unused parameter */
    return &global_container;
}

static void delete_container(container_t** c) {
    if (c) *c = NULL;
}

/* ===== Main function to use all types ===== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_scalar++;
    global_color = BLUE;
    global_float *= 2.0f;
    
    /* Use struct types */
    global_struct.x = 10;
    global_user_struct.y = 20;
    prevent_optimization += global_struct.x + global_user_struct.y;
    
    /* Use union */
    global_union.f = 3.14f;
    prevent_optimization += (int)global_union.f;
    
    /* Use pointers */
    if (global_int_ptr) {
        *global_int_ptr = 100;
    }
    
    /* Use arrays */
    global_int_array[0] = 99;
    global_float_array[1][1] = 88.0f;
    prevent_optimization += global_int_array[0] + (int)global_float_array[1][1];
    
    /* Use string */
    if (string_literal[0] != '\0') {
        prevent_optimization++;
    }
    
    /* Use callbacks */
    global_callback = sample_callback;
    if (global_callback) {
        prevent_optimization += global_callback(5, "test");
    }
    
    global_void_callback = (void_callback_t)0;
    
    /* Use GCC extension types */
    global_packed.a = 'B';
    global_packed.b = 456;
    prevent_optimization += global_packed.b;
    
    /* Use complex nested types */
    global_node.value = 111;
    global_node.handler = sample_callback;
    
    global_container.nodes[0] = &global_node;
    global_container.comparator = (int (*)(const void*, const void*))sample_callback;
    global_container.metrics.count = 50;
    
    global_master.alloc = create_container;
    global_master.free = delete_container;
    
    struct_factory_t factory = create_struct;
    if (factory) {
        struct plain_struct* s = factory();
        prevent_optimization += s->x;
    }
    
    /* Use transparent union */
    global_transparent.int_ptr = &global_scalar;
    if (global_transparent.void_ptr) {
        prevent_optimization++;
    }
    
    /* Ensure all variables are marked as used */
    (void)global_void_ptr;
    (void)global_void_callback;
    (void)global_transparent;
    (void)static_string;
    (void)string_array;
    (void)void_ptr;
    (void)volatile_ptr;
    
    return prevent_optimization > 0 ? 0 : 1;
}
