/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

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
    char *string_val;
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct BaseStruct *struct_ptr;
void *void_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct BaseStruct struct_array[3];
int *pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enum is also scalar */
enum Color { RED, GREEN, BLUE };
enum Color color_enum;

/* TYPE_STRING: String literal in initializer */
const char *greeting = "Hello, gengtype!";
char message[] = "Test string";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, char*);

/* Complex callback returning pointer to struct */
typedef struct BaseStruct* (*StructFactory)(int id);

/* TYPE_LANG_STRUCT: GCC-specific attributes and pragmas */
#pragma pack(push, 1)
struct PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((packed));
#pragma pack(pop)

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Aligned struct with attribute */
struct AlignedStruct {
    long long data1;
    char data2;
} __attribute__((aligned(64)));

/* Complex nested type definitions */

/* Struct containing pointer to another struct */
struct Container {
    struct BaseStruct *item;
    int count;
    union DataUnion data;
    Comparator compare;
};

/* Union containing array of structs */
union ComplexUnion {
    struct BaseStruct items[4];
    struct Container *container_ptr;
    CallbackFunc callback;
};

/* Typedef for function pointer returning pointer to struct */
typedef struct Container* (*ContainerAllocator)(size_t size);

/* Global variables using all these types */
struct BaseStruct global_struct = {1, 3.14f, "test"};
UserStruct user_struct_instance;
union DataUnion global_union = {.int_val = 42};
struct PackedStruct packed_instance = {'x', 100, 5};
struct AlignedStruct aligned_instance = {123456789LL, 'A'};
struct Container global_container = {&global_struct, 1, {.int_val = 99}, NULL};
union ComplexUnion complex_union_instance;

/* Function pointer variables */
Comparator compare_func = NULL;
CallbackFunc callback_func = NULL;
StructFactory factory_func = NULL;
ContainerAllocator allocator_func = NULL;

/* Volatile and const qualified pointers */
volatile int *const volatile_ptr_const = (volatile int*)0x1000;
const struct BaseStruct *const const_struct_ptr = &global_struct;

/* Array with volatile elements */
volatile int volatile_array[5] = {1, 2, 3, 4, 5};

/* Struct with volatile member */
struct VolatileStruct {
    volatile int counter;
    const char *name;
};

struct VolatileStruct volatile_struct = {0, "counter"};

/* Function using callback */
static int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

static struct BaseStruct* sample_factory(int id) {
    static struct BaseStruct instance;
    instance.id = id;
    instance.value = id * 1.5f;
    return &instance;
}

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 10;
    user_struct_instance.value = 2.718f;
    
    /* Use union */
    global_union.float_val = 3.14159f;
    
    /* Use pointers */
    if (int_ptr) *int_ptr = 5;
    struct_ptr = &global_struct;
    
    /* Use arrays */
    int_array[0] = 100;
    char_matrix[2][2] = 'X';
    
    /* Use scalars */
    scalar_int = 42;
    color_enum = GREEN;
    
    /* Use strings */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use callbacks */
    compare_func = sample_comparator;
    factory_func = sample_factory;
    
    /* Use GCC-specific types */
    packed_instance.a = 'y';
    aligned_instance.data2 = 'B';
    
    /* Use nested types */
    global_container.item = &global_struct;
    complex_union_instance.callback = NULL;
    
    /* Use volatile/const types */
    if (volatile_ptr_const) {
        prevent_optimization += 1;
    }
    prevent_optimization += const_struct_ptr->id;
    
    /* Use volatile array */
    volatile_array[0] = 10;
    
    /* Use volatile struct */
    volatile_struct.counter++;
    
    /* Call through function pointer if set */
    if (compare_func) {
        int a = 1, b = 2;
        compare_func(&a, &b);
    }
    
    if (factory_func) {
        struct BaseStruct *p = factory_func(5);
        prevent_optimization += p->id;
    }
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct StaticOnlyStruct {
    int hidden;
    union DataUnion secret;
} static_instance = {0, {.int_val = 999}};
