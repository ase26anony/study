/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_STRUCT ========== */
struct PlainStruct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x, y;
    double z;
} UserStruct;

/* Struct with GCC attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* ========== TYPE_UNION ========== */
union DataUnion {
    int int_val;
    float float_val;
    char* str_val;
    void* ptr_val;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* void_ptr;
} TransparentUnion;

/* ========== TYPE_POINTER ========== */
int* int_ptr;
volatile int* volatile volatile_int_ptr;
const char* const const_string_ptr = "constant";
void* void_ptr;
struct PlainStruct* struct_ptr;
int** double_ptr;

/* ========== TYPE_ARRAY ========== */
int int_array[10];
char char_matrix[5][5];
float three_dim[3][3][3];
struct PlainStruct struct_array[4];
int* pointer_array[8];

/* ========== TYPE_SCALAR ========== */
int scalar_int;
float scalar_float;
double scalar_double;
enum Color { RED, GREEN, BLUE } scalar_enum;
_Bool scalar_bool;

/* ========== TYPE_STRING ========== */
const char* string_literal = "Hello, gengtype!";
char string_array[] = "Initialized array";

/* ========== TYPE_CALLBACK ========== */
typedef int (*CallbackFunc)(int, void*);
typedef void (*VoidCallback)(struct PlainStruct*);
typedef int* (*PointerReturnFunc)(char**, size_t);

/* ========== Complex Nesting ========== */
struct ComplexNest {
    struct PlainStruct base;
    union DataUnion data;
    int* ptr_member;
    int array_member[7];
    CallbackFunc callback;
    struct ComplexNest* next;  /* Linked list */
};

/* Union with nested struct */
union NestedUnion {
    struct {
        int type;
        union {
            int int_val;
            float float_val;
        } value;
    } tagged;
    char raw_data[16];
};

/* ========== Global Variables ========== */
struct PlainStruct global_struct = {1, 3.14f, "test"};
UserStruct global_user_struct = {10, 20, 30.5};
union DataUnion global_union = {.int_val = 42};
struct ComplexNest global_nest = {
    .base = {2, 2.718f, "nested"},
    .data = {.float_val = 1.414f},
    .ptr_member = &scalar_int,
    .array_member = {1,2,3,4,5,6,7},
    .callback = NULL,
    .next = NULL
};

int* global_ptr_array[] = {&scalar_int, NULL, &scalar_int};
CallbackFunc global_callbacks[3];

/* ========== Function using types ========== */
static int sample_callback(int val, void* context) {
    return val * 2;
}

static void* process_struct(struct PlainStruct* ps, int count) {
    static int counter = 0;
    counter++;
    return (void*)(intptr_t)counter;
}

/* Function pointer variable */
CallbackFunc active_callback = sample_callback;

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 100;
    prevent_optimization += global_struct.id;
    
    /* Use union */
    global_union.float_val = 3.14159f;
    prevent_optimization += (int)global_union.float_val;
    
    /* Use pointer */
    int local_int = 42;
    int_ptr = &local_int;
    prevent_optimization += *int_ptr;
    
    /* Use array */
    int_array[0] = 1;
    prevent_optimization += int_array[0];
    
    /* Use scalar */
    scalar_int = 99;
    prevent_optimization += scalar_int;
    
    /* Use string */
    prevent_optimization += string_literal[0];
    
    /* Use callback */
    if (active_callback) {
        prevent_optimization += active_callback(5, NULL);
    }
    
    /* Use nested struct */
    global_nest.base.value = 2.5f;
    prevent_optimization += (int)global_nest.base.value;
    
    /* Use typedef struct */
    global_user_struct.x = 50;
    prevent_optimization += global_user_struct.x;
    
    /* Use double pointer */
    int** pp = &int_ptr;
    prevent_optimization += **pp;
    
    /* Use matrix */
    char_matrix[2][2] = 'X';
    prevent_optimization += char_matrix[2][2];
    
    /* Use enum */
    scalar_enum = GREEN;
    prevent_optimization += scalar_enum;
    
    return prevent_optimization > 0 ? 0 : 1;
}
