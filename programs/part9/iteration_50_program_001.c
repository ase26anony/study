/* test_gengtype_coverage.c
 * A comprehensive test file to exercise all TYPE_* cases in gengtype-state.cc
 */

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
    char* str_val;
    void* ptr_val;
};

/* ========== TYPE_POINTER ========== */
typedef int* IntPtr;
typedef struct PlainStruct* StructPtr;
typedef void (*VoidFuncPtr)(void);

/* ========== TYPE_ARRAY ========== */
typedef int IntArray[10];
typedef char CharMatrix[5][5];
typedef struct PlainStruct StructArray[3];

/* ========== TYPE_SCALAR ========== */
typedef enum Color { RED, GREEN, BLUE } ColorEnum;
typedef volatile int VolatileInt;
typedef const double ConstDouble;

/* ========== TYPE_STRING ========== */
const char* global_string = "Hello, gengtype!";

/* ========== TYPE_CALLBACK ========== */
typedef int (*Comparator)(const void*, const void*);
typedef void (*EventHandler)(int event_id, void* user_data);

/* ========== TYPE_LANG_STRUCT (GCC extensions) ========== */
#ifdef __GNUC__
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
};

union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
};

struct __attribute__((aligned(32))) AlignedStruct {
    double data[4];
    long long timestamp;
};
#endif

/* ========== Complex nesting and chains ========== */
struct ComplexNode {
    int data;
    struct ComplexNode* next;
    union DataUnion value;
    IntArray scores;
    Comparator compare_func;
};

typedef struct ComplexNode* (*NodeAllocator)(size_t size);
typedef void (*NodeProcessor)(struct ComplexNode*, void* context);

struct Container {
    struct ComplexNode* head;
    struct ComplexNode** node_ptrs;
    union {
        int count;
        float ratio;
    } metrics;
    EventHandler on_update;
    char name[64];
    volatile int status;
    const char* const* string_table;
};

/* ========== Global variable definitions ========== */
struct PlainStruct global_struct = {1, 3.14f, "test"};
UserStructType user_struct_instance = {2, 2.718f, "user"};
union DataUnion global_union = {.int_val = 42};
IntPtr global_int_ptr = NULL;
StructPtr global_struct_ptr = &global_struct;
IntArray global_array = {0,1,2,3,4,5,6,7,8,9};
CharMatrix global_matrix = {"abcd","efgh","ijkl","mnop","qrst"};
ColorEnum global_color = BLUE;
VolatileInt global_volatile = 100;
ConstDouble global_const_double = 3.141592653589793;

Comparator global_comparator = NULL;
EventHandler global_handler = NULL;

#ifdef __GNUC__
struct PackedStruct global_packed = {'X', 12345, 67};
struct AlignedStruct global_aligned = {{1.0, 2.0, 3.0, 4.0}, 9876543210LL};
#endif

struct ComplexNode node1 = {
    .data = 10,
    .next = NULL,
    .value = {.int_val = 99},
    .scores = {5,4,3,2,1},
    .compare_func = NULL
};

struct ComplexNode node2 = {
    .data = 20,
    .next = &node1,
    .value = {.float_val = 1.5f},
    .scores = {1,2,3,4,5},
    .compare_func = NULL
};

struct Container global_container = {
    .head = &node2,
    .node_ptrs = (struct ComplexNode*[]){&node1, &node2, NULL},
    .metrics = {.count = 2},
    .on_update = NULL,
    .name = "TestContainer",
    .status = 0,
    .string_table = (const char* const[]){"one", "two", "three", NULL}
};

/* ========== Function pointer usage ========== */
int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

void sample_handler(int event_id, void* user_data) {
    *(int*)user_data = event_id;
}

NodeAllocator global_allocator = NULL;

/* ========== Main function to ensure usage ========== */
int main(void) {
    /* Use struct */
    global_struct.id++;
    global_struct.value *= 2.0f;
    
    /* Use union */
    global_union.float_val = 1.234f;
    
    /* Use pointers */
    int local_int = 5;
    global_int_ptr = &local_int;
    *global_int_ptr = 10;
    
    /* Use arrays */
    global_array[0] = 100;
    global_matrix[0][0] = 'Z';
    
    /* Use scalar types */
    global_color = RED;
    global_volatile++;
    
    /* Use string */
    const char* local_str = global_string;
    
    /* Use callbacks */
    global_comparator = compare_ints;
    int nums[] = {5, 2, 8, 1};
    if (global_comparator) {
        /* Simulate comparison */
        int result = compare_ints(&nums[0], &nums[1]);
    }
    
    global_handler = sample_handler;
    int event_data = 0;
    if (global_handler) {
        sample_handler(42, &event_data);
    }
    
    /* Use nested structures */
    global_container.head->data = 30;
    global_container.metrics.count++;
    global_container.status = 1;
    
    /* Use function pointer typedef */
    NodeProcessor processor = NULL;
    
    #ifdef __GNUC__
    /* Use GCC-specific structs */
    global_packed.a = 'Y';
    global_packed.b = 54321;
    global_aligned.data[0] = 5.0;
    #endif
    
    return 0;
}

/* ========== Additional complex type patterns ========== */
/* Multi-level pointer */
typedef struct Container**** QuadContainerPtr;

/* Array of function pointers */
typedef void (*FuncPtrArray[5])(int);

/* Struct with bitfields (another GCC extension) */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 8;
};

/* Opaque forward declaration */
struct OpaqueStruct;
typedef struct OpaqueStruct* OpaqueHandle;

/* Const pointer to volatile array */
volatile int* const volatile const_volatile_ptr = NULL;

/* Anonymous struct/union */
struct AnonymousExample {
    struct {
        int x;
        int y;
    } point;
    union {
        int id;
        char tag;
    } identifier;
};

/* ========== Multiple translation unit simulation ========== */
/* This would normally be in a header */
extern struct PlainStruct external_struct;
extern void external_function(struct ComplexNode* node);

/* Static variables for file scope */
static struct PlainStruct static_struct = {3, 1.414f, "static"};
static union DataUnion static_union = {.str_val = "static string"};
