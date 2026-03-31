/* test_gengtype_coverage.c
 * Comprehensive test file to cover all TYPE_* cases in gengtype-state.cc
 */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct PlainStruct {
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

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
float three_d_array[3][3][3];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct PlainStruct *struct_ptr;
void *void_ptr;
const char *const_string_ptr;
volatile int *volatile_int_ptr;

/* TYPE_SCALAR: Scalar types including enum */
enum Color { RED, GREEN, BLUE };
typedef unsigned long ulong;

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, World!";
char filename[] = "/tmp/test.txt";

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*Logger)(const char *message);

/* Complex nesting and chains */
struct ComplexNode {
    int data;
    struct ComplexNode *next;
    union DataUnion value;
    Comparator compare;
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

/* Volatile and const qualifiers in complex combinations */
volatile int * const volatile_ptr_const = 0;
const volatile float cv_float = 3.14f;

/* Nested type example */
typedef struct Outer {
    struct {
        int inner_id;
        char inner_name[20];
    } nested;
    UserStruct user;
    int (*operation)(int, int);
} OuterStruct;

/* Array of function pointers */
int (*func_array[5])(int, int);

/* Union with struct member */
union ComplexUnion {
    struct {
        int type;
        union DataUnion data;
    } tagged;
    long raw;
};

/* Global variables using all declared types */
struct PlainStruct global_struct = {1, 3.14f, "test"};
UserStruct global_user_struct = {100, 2.71828};
union DataUnion global_union = {.as_int = 42};
enum Color global_color = GREEN;
struct ComplexNode *global_node = 0;
struct PackedStruct global_packed = {'X', 999, 7};
TransparentUnion global_transparent;
OuterStruct global_outer = {{2, "inner"}, {50, 1.618}, 0};
union ComplexUnion global_complex_union;

/* Function definitions for callbacks */
int compare_ints(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

void log_message(const char *message) {
    /* Empty for test purposes */
}

int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 10;
    prevent_optimization += global_struct.id;
    
    /* Use user struct */
    global_user_struct.counter++;
    prevent_optimization += (int)global_user_struct.counter;
    
    /* Use union */
    global_union.as_float = 1.5f;
    prevent_optimization += (int)global_union.as_int;
    
    /* Use arrays */
    int_array[0] = 100;
    char_matrix[0][0] = 'A';
    prevent_optimization += int_array[0] + char_matrix[0][0];
    
    /* Use pointers */
    int_ptr = &int_array[0];
    prevent_optimization += *int_ptr;
    
    /* Use string */
    prevent_optimization += greeting[0];
    
    /* Use enum */
    global_color = BLUE;
    prevent_optimization += global_color;
    
    /* Use function pointers */
    Comparator cmp = compare_ints;
    int x = 5, y = 10;
    prevent_optimization += cmp(&x, &y);
    
    Logger logger = log_message;
    logger("Test");
    
    /* Use complex nested types */
    global_outer.nested.inner_id = 30;
    prevent_optimization += global_outer.nested.inner_id;
    
    /* Use array of function pointers */
    func_array[0] = add;
    func_array[1] = subtract;
    prevent_optimization += func_array[0](3, 4);
    
    /* Use GCC extended types */
    global_packed.a = 'Y';
    prevent_optimization += global_packed.a;
    
    /* Use transparent union */
    global_transparent.int_ptr = &x;
    prevent_optimization += *global_transparent.int_ptr;
    
    /* Use complex union */
    global_complex_union.tagged.type = 1;
    global_complex_union.tagged.data.as_int = 99;
    prevent_optimization += global_complex_union.tagged.data.as_int;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional declarations in different scopes */
static struct {
    int private_data;
    char private_buffer[64];
} file_scope_struct = {123, "private"};

/* Multi-dimensional pointer array */
char *string_array[] = {"one", "two", "three"};

/* Const pointer to volatile array */
volatile int volatile_array[5];
const volatile int * const volatile_array_ptr = volatile_array;

/* Function returning pointer to struct */
struct PlainStruct *get_struct_ptr(void) {
    return &global_struct;
}

/* Struct with bitfields (another GCC extension) */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 5;
    signed int value : 8;
};

/* Anonymous struct/union (C11/GCC extension) */
struct AnonymousContainer {
    struct {
        int x;
        int y;
    };
    union {
        int coord_id;
        float coord_val;
    };
};
