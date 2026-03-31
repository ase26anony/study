/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_STRUCT ========== */
struct PlainStruct {
    int id;
    float value;
    char name[32];
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct PlainStruct UserStructType;

/* ========== TYPE_UNION ========== */
union DataUnion {
    int int_val;
    float float_val;
    char char_val;
    void *ptr_val;
};

/* ========== TYPE_POINTER ========== */
typedef int* IntPtr;
typedef struct PlainStruct* StructPtr;
typedef void (*FuncPtr)(void);

/* ========== TYPE_ARRAY ========== */
typedef int IntArray[10];
typedef char CharMatrix[5][5];
typedef struct PlainStruct StructArray[3];

/* ========== TYPE_SCALAR ========== */
typedef enum Color { RED, GREEN, BLUE } ColorEnum;
typedef volatile int VolatileInt;
typedef const float ConstFloat;

/* ========== TYPE_STRING ========== */
const char* greeting = "Hello, gengtype!";

/* ========== TYPE_CALLBACK ========== */
typedef int (*Comparator)(const void*, const void*);
typedef void (*StateCallback)(struct PlainStruct*, union DataUnion*);

/* ========== TYPE_LANG_STRUCT (GCC extensions) ========== */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    char c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
};

#pragma pack(push, 1)
struct PragmaPacked {
    double d;
    char c;
    int i;
};
#pragma pack(pop)

/* ========== Complex nesting and chains ========== */
struct ComplexNode {
    int data;
    struct ComplexNode* next;
    struct ComplexNode* prev;
    union DataUnion payload;
    Comparator compare_func;
};

struct Container {
    struct PlainStruct base;
    union DataUnion variant;
    IntArray numbers;
    CharMatrix matrix;
    struct ComplexNode* node_list;
    VolatileInt counter;
    StateCallback on_update;
};

/* ========== Global variables with initializers ========== */
struct PlainStruct global_struct = {1, 3.14f, "test"};
union DataUnion global_union = {.int_val = 42};
IntArray global_array = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
CharMatrix global_matrix = {{'a','b','c','d','e'},
                            {'f','g','h','i','j'},
                            {'k','l','m','n','o'},
                            {'p','q','r','s','t'},
                            {'u','v','w','x','y'}};
struct PackedStruct global_packed = {'X', 999, 'Y'};
struct PragmaPacked global_pragma_packed = {3.14159, 'Z', 1000};

/* Pointer variables with qualifiers */
volatile int* volatile volatile_ptr;
const struct PlainStruct* const const_struct_ptr = &global_struct;
volatile IntPtr const volatile_intptr_const;

/* Function pointer variables */
Comparator global_comparator = NULL;
StateCallback global_callback = NULL;

/* Array of function pointers */
FuncPtr func_array[5];

/* ========== Callback function definitions ========== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void update_handler(struct PlainStruct* s, union DataUnion* u) {
    if (s && u) {
        s->id++;
        u->int_val = s->id;
    }
}

/* ========== Main function to ensure all types are used ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    struct PlainStruct local_struct = global_struct;
    local_struct.id++;
    prevent_optimization += local_struct.id;
    
    /* Use union */
    union DataUnion local_union;
    local_union.float_val = 2.718f;
    prevent_optimization += (int)local_union.float_val;
    
    /* Use user-defined struct type */
    UserStructType user_struct = {2, 1.618f, "phi"};
    prevent_optimization += user_struct.id;
    
    /* Use pointers */
    int x = 10;
    IntPtr int_ptr = &x;
    *int_ptr = 20;
    prevent_optimization += *int_ptr;
    
    StructPtr struct_ptr = &global_struct;
    prevent_optimization += struct_ptr->id;
    
    /* Use arrays */
    for (int i = 0; i < 10; i++) {
        prevent_optimization += global_array[i];
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            prevent_optimization += global_matrix[i][j];
        }
    }
    
    /* Use scalar types */
    ColorEnum color = GREEN;
    prevent_optimization += color;
    
    VolatileInt vi = 100;
    prevent_optimization += vi;
    
    ConstFloat cf = 2.5f;
    prevent_optimization += (int)cf;
    
    /* Use string */
    const char* local_greeting = greeting;
    prevent_optimization += local_greeting[0];
    
    /* Use callbacks */
    global_comparator = compare_ints;
    global_callback = update_handler;
    
    int a = 5, b = 10;
    if (global_comparator) {
        prevent_optimization += global_comparator(&a, &b);
    }
    
    if (global_callback) {
        global_callback(&local_struct, &local_union);
    }
    
    /* Use GCC extension types */
    struct PackedStruct packed = global_packed;
    prevent_optimization += packed.a + packed.b + packed.c;
    
    struct PragmaPacked pragma_packed = global_pragma_packed;
    prevent_optimization += (int)pragma_packed.d + pragma_packed.c + pragma_packed.i;
    
    /* Use transparent union */
    union TransparentUnion tu;
    tu.int_ptr = &x;
    prevent_optimization += *tu.int_ptr;
    
    /* Use complex nested structure */
    struct ComplexNode node1 = {100, NULL, NULL, {.int_val = 200}, compare_ints};
    struct ComplexNode node2 = {200, &node1, NULL, {.float_val = 300.0f}, NULL};
    node1.next = &node2;
    
    prevent_optimization += node1.data + node2.payload.int_val;
    
    /* Use container */
    struct Container container = {
        .base = {3, 1.414f, "root2"},
        .variant = {.char_val = 'A'},
        .numbers = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
        .node_list = &node1,
        .counter = 999,
        .on_update = update_handler
    };
    
    prevent_optimization += container.base.id + container.counter;
    
    /* Use function pointer array */
    func_array[0] = (FuncPtr)compare_ints;
    func_array[1] = (FuncPtr)update_handler;
    
    /* Use qualified pointers */
    volatile_ptr = &prevent_optimization;
    prevent_optimization += *volatile_ptr;
    
    return prevent_optimization > 0 ? 0 : 1;
}
