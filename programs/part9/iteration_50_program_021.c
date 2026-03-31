/* test_gengtype_coverage.c
 * Comprehensive type declarations to cover gengtype-state.cc switch cases
 */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct BaseStruct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef creating user-defined struct type */
typedef struct BaseStruct UserStruct;

/* TYPE_UNION: Union with several members */
union DataUnion {
    int int_val;
    float float_val;
    char* str_val;
    void* ptr_val;
};

/* TYPE_POINTER: Various pointer types */
int* int_ptr;
struct BaseStruct* struct_ptr;
void* void_ptr;
volatile int* volatile volatile_int_ptr;
const char* const const_string_ptr = "constant";

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct BaseStruct struct_array[3];
int* pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;
enum Color { RED, GREEN, BLUE } color_enum;

/* TYPE_STRING: String literal in initializer */
const char* greeting = "Hello, gengtype!";
char initialized_string[] = "Initialized array";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, const char*);

/* Complex nesting and chains */
struct ComplexStruct {
    struct BaseStruct* nested_struct;
    union DataUnion data;
    int (*compare)(struct BaseStruct*, struct BaseStruct*);
    CallbackFunc callback;
    volatile int* volatile_ptr;
};

/* TYPE_LANG_STRUCT: GCC-specific attributes and extensions */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
} TransparentUnion;

/* Struct with bitfields */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
};

/* Nested type chain for deep traversal */
struct Level1 {
    struct Level2* next;
    int data;
};

struct Level2 {
    struct Level3* next;
    union DataUnion value;
};

struct Level3 {
    struct Level1* parent;
    int array[4];
    Comparator comparator;
};

/* Function pointer returning pointer to struct */
typedef struct BaseStruct* (*StructFactory)(int, const char*);

/* Volatile and const qualified types */
volatile int volatile_counter;
const double pi = 3.141592653589793;
volatile int* const volatile_int_const_ptr = &volatile_counter;
const volatile int cv_int = 42;

/* Global variables using all declared types */
struct BaseStruct global_struct = {1, 3.14f, "test"};
union DataUnion global_union = {.int_val = 100};
struct ComplexStruct complex_instance;
struct PackedStruct packed_instance = {'A', 42, 7};
struct Level1 level1_instance;
struct BitfieldStruct bitfield_instance = {1, 3, 5, -10};

/* Array of function pointers */
CallbackFunc callbacks[3];

/* Function using transparent union */
void process_transparent(TransparentUnion tu) {
    /* Use the union */
    void* ptr = tu.void_ptr;
    (void)ptr;
}

/* Example comparator function */
int compare_structs(const void* a, const void* b) {
    const struct BaseStruct* sa = (const struct BaseStruct*)a;
    const struct BaseStruct* sb = (const struct BaseStruct*)b;
    return sa->id - sb->id;
}

/* Example callback function */
void sample_callback(int id, const char* msg) {
    /* Do nothing for test */
    (void)id;
    (void)msg;
}

/* Struct factory function */
struct BaseStruct* create_struct(int id, const char* name) {
    static struct BaseStruct instance;
    instance.id = id;
    if (name) {
        for (int i = 0; i < 31 && name[i]; i++) {
            instance.name[i] = name[i];
        }
    }
    return &instance;
}

/* Main function to ensure all types are syntactically used */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 10;
    prevent_optimization += global_struct.id;
    
    /* Use union */
    global_union.float_val = 2.718f;
    prevent_optimization += (int)global_union.float_val;
    
    /* Use pointers */
    int_ptr = &prevent_optimization;
    prevent_optimization += *int_ptr;
    
    /* Use arrays */
    int_array[0] = 1;
    prevent_optimization += int_array[0];
    
    /* Use scalar types */
    scalar_int = 100;
    prevent_optimization += scalar_int;
    
    color_enum = GREEN;
    prevent_optimization += color_enum;
    
    /* Use strings */
    prevent_optimization += greeting[0];
    
    /* Use function pointers */
    Comparator cmp = compare_structs;
    callbacks[0] = sample_callback;
    
    /* Use complex struct */
    complex_instance.nested_struct = &global_struct;
    complex_instance.callback = sample_callback;
    prevent_optimization += complex_instance.nested_struct->id;
    
    /* Use packed struct */
    packed_instance.a = 'B';
    prevent_optimization += packed_instance.a;
    
    /* Use bitfield struct */
    bitfield_instance.flag1 = 1;
    prevent_optimization += bitfield_instance.flag1;
    
    /* Use nested chain */
    level1_instance.data = 99;
    prevent_optimization += level1_instance.data;
    
    /* Use volatile/const */
    volatile_counter = 50;
    prevent_optimization += volatile_counter;
    prevent_optimization += (int)pi;
    prevent_optimization += cv_int;
    
    /* Use transparent union */
    TransparentUnion tu = {.int_ptr = &prevent_optimization};
    process_transparent(tu);
    
    /* Use struct factory */
    StructFactory factory = create_struct;
    struct BaseStruct* new_struct = factory(5, "test");
    prevent_optimization += new_struct->id;
    
    /* Call callback */
    if (callbacks[0]) {
        callbacks[0](1, "test");
    }
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional declarations in file scope for more coverage */
static struct {
    int anonymous_member;
} anonymous_struct;

typedef int (*ComplexCallback)(struct ComplexStruct*, union DataUnion, TransparentUnion);

/* Multi-dimensional pointer array */
void* complex_ptr_array[2][3];

/* Const array */
const int const_array[] = {1, 2, 3, 4, 5};

/* Volatile struct */
volatile struct BaseStruct volatile_struct;
