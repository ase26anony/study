/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct BasicStruct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    long counter;
    double data;
} UserStruct;

/* TYPE_UNION: Union with several members */
union DataUnion {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct BasicStruct *struct_ptr;
void *void_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct BasicStruct struct_array[3];
int *pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enumeration type (also scalar) */
enum Color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, gengtype!";
char message[] = "Test string";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* Complex nested types */
struct ComplexType {
    struct BasicStruct base;
    union DataUnion data;
    int (*compare)(struct ComplexType *, struct ComplexType *);
    void *extra_data;
    char buffer[64];
};

/* TYPE_LANG_STRUCT: GCC-specific attributes and extensions */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Volatile and const qualified types */
volatile int volatile_counter;
const double pi = 3.141592653589793;
volatile int *const volatile_const_ptr = NULL;

/* Function returning pointer to struct */
struct BasicStruct *get_struct_ptr(void) {
    static struct BasicStruct instance = {1, 3.14f, "test"};
    return &instance;
}

/* Callback function implementation */
int compare_ints(const void *a, const void *b) {
    return *(const int *)a - *(const int *)b;
}

void sample_callback(int value, void *context) {
    /* Do nothing for test */
    (void)value;
    (void)context;
}

/* Global variables using all declared types */
struct BasicStruct global_struct = {42, 2.718f, "global"};
UserStruct global_user_struct = {1000, 99.99};
union DataUnion global_union = {.as_int = 42};
enum Color global_color = BLUE;
Comparator global_comparator = compare_ints;
CallbackFunc global_callback = sample_callback;
struct ComplexType global_complex;
struct PackedStruct global_packed = {'X', 1234, 5678};

/* Array initializations */
int initialized_array[5] = {1, 2, 3, 4, 5};
char *string_array[] = {"one", "two", "three"};

/* Nested structure with pointer chain */
struct Node {
    int value;
    struct Node *next;
    struct Node *prev;
};

struct Container {
    struct Node *head;
    struct Node *tail;
    int count;
    void (*processor)(struct Container *);
};

/* Function pointer in struct */
struct Operations {
    int (*add)(int, int);
    void (*log)(const char *);
};

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct types */
    global_struct.id = 100;
    global_user_struct.counter++;
    
    /* Use union */
    global_union.as_float = 3.14f;
    
    /* Use pointers */
    int_ptr = &scalar_int;
    *int_ptr = 42;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[2][2] = 'X';
    
    /* Use scalar types */
    scalar_int = 10;
    scalar_float = 20.5f;
    global_color = RED;
    
    /* Use string */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use callback */
    if (global_comparator) {
        int a = 1, b = 2;
        prevent_optimization += global_comparator(&a, &b);
    }
    
    if (global_callback) {
        global_callback(42, NULL);
    }
    
    /* Use complex nested types */
    global_complex.base.id = 999;
    global_complex.data.as_int = 123;
    
    /* Use GCC-specific struct */
    global_packed.a = 'Y';
    global_packed.b = 4321;
    
    /* Use volatile/const */
    volatile_counter = 100;
    prevent_optimization += (int)pi;
    
    /* Use function returning struct pointer */
    struct_ptr = get_struct_ptr();
    if (struct_ptr) {
        prevent_optimization += struct_ptr->id;
    }
    
    /* Initialize and use linked structure */
    struct Node node1 = {1, NULL, NULL};
    struct Node node2 = {2, NULL, &node1};
    node1.next = &node2;
    
    struct Container container = {&node1, &node2, 2, NULL};
    prevent_optimization += container.count;
    
    /* Use initialized arrays */
    prevent_optimization += initialized_array[0];
    prevent_optimization += string_array[0][0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in different linkage to test extern */
extern struct ExternalType {
    int external_id;
    char external_name[64];
} external_instance;

/* Static variables at file scope */
static int static_counter = 0;
static struct {
    int hidden;
    float secret;
} static_struct = {0, 0.0f};
