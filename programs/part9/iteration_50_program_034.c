/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct SimpleStruct {
    int a;
    float b;
    char c;
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    long x;
    double y;
} UserStruct;

/* TYPE_UNION: Union with several members */
union DataUnion {
    int i;
    float f;
    char str[16];
    void *ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct SimpleStruct *struct_ptr;
void *void_ptr;
volatile int *volatile_ptr;
const char *const_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct SimpleStruct struct_array[3];
int (*array_of_ptrs)[10];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enum type (also scalar) */
enum Color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, World!";
char message[] = "Test message";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*CallbackFunc)(int, void*);
typedef void (*SimpleCallback)(void);

/* Complex nested structure with function pointer */
struct ComplexStruct {
    int id;
    union DataUnion data;
    struct ComplexStruct *next;
    CallbackFunc callback;
    int values[5];
};

/* GCC-specific attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(8)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} TransparentUnion;

/* #pragma pack directive */
#pragma pack(push, 1)
struct PackedWithPragma {
    char c;
    int i;
    short s;
};
#pragma pack(pop)

/* More complex type chains */
typedef struct Node {
    int value;
    struct Node *left;
    struct Node *right;
    void (*print)(struct Node*);
} TreeNode;

/* Array of function pointers */
CallbackFunc callbacks[3];

/* Pointer to array */
int (*ptr_to_array)[10];

/* Multi-dimensional pointer array */
int *ptr_array_2d[5][5];

/* Const volatile qualified pointer */
const volatile int *const volatile cv_ptr;

/* Nested type example */
struct Outer {
    struct {
        int inner_a;
        float inner_b;
    } nested;
    union {
        int x;
        long y;
    } data_union;
    struct Outer *self_ptr;
};

/* Function returning pointer to struct */
struct SimpleStruct* (*func_returning_struct_ptr)(int);

/* Global variable definitions */
struct SimpleStruct global_struct = {1, 2.5f, 'A'};
UserStruct global_user_struct = {100, 3.14159};
union DataUnion global_union = {.i = 42};
int global_array[5] = {1, 2, 3, 4, 5};
enum Color global_color = BLUE;
struct ComplexStruct complex_instance = {
    .id = 1,
    .data = {.i = 100},
    .next = NULL,
    .callback = NULL,
    .values = {1, 2, 3, 4, 5}
};
struct PackedStruct packed_instance = {'X', 999, 777};

/* Function definitions that use the types */
static int sample_callback(int val, void *data) {
    return val * 2;
}

static void print_node(TreeNode *node) {
    /* Empty for test purposes */
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct types */
    global_struct.a = 10;
    global_user_struct.x = 20;
    
    /* Use union */
    global_union.f = 3.14f;
    
    /* Use pointers */
    int_ptr = &global_struct.a;
    struct_ptr = &global_struct;
    
    /* Use arrays */
    int_array[0] = 100;
    char_matrix[0][0] = 'Z';
    
    /* Use scalars */
    scalar_int = 500;
    scalar_float = 2.718f;
    
    /* Use string */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use callback */
    callbacks[0] = sample_callback;
    if (callbacks[0]) {
        prevent_optimization += callbacks[0](5, NULL);
    }
    
    /* Use complex struct */
    complex_instance.id = 99;
    complex_instance.values[0] = 111;
    
    /* Use packed struct */
    packed_instance.a = 'Y';
    
    /* Use tree node */
    TreeNode node = {
        .value = 42,
        .left = NULL,
        .right = NULL,
        .print = print_node
    };
    
    /* Use transparent union */
    TransparentUnion tu;
    int local_int = 7;
    tu.intp = &local_int;
    
    /* Use const volatile pointer */
    int local_cv = 99;
    cv_ptr = &local_cv;
    
    /* Use outer struct */
    struct Outer outer_instance = {
        .nested = {.inner_a = 1, .inner_b = 2.0f},
        .data_union = {.x = 100},
        .self_ptr = &outer_instance
    };
    
    /* Use pointer to array */
    ptr_to_array = &global_array;
    
    /* Ensure everything is used */
    return prevent_optimization;
}

/* Additional declarations in file scope */
static struct {
    int anonymous_member;
} anonymous_struct;

typedef int (*ComplexCallback)(struct ComplexStruct*, union DataUnion*, int[]);

/* Multi-file simulation with extern */
extern int external_variable;

/* Force generation of TYPE_UNDEFINED? 
   Forward declaration without definition creates undefined type reference */
struct UndefinedStruct;
extern struct UndefinedStruct *undefined_ptr;
