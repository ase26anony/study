/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;
typedef _Bool scalar_bool_t;

/* Global scalar variables */
scalar_int_t global_int = 42;
volatile scalar_float_t global_volatile_float = 3.14f;
const scalar_double_t global_const_double = 2.71828;
color_enum global_color = GREEN;
const volatile scalar_bool_t global_cv_bool = 1;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with attributes (may trigger TYPE_LANG_STRUCT) */
struct __attribute__((packed, aligned(8))) attributed_struct {
    long long a;
    int b __attribute__((aligned(16)));
    char c;
};

/* Struct with bitfields */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int value : 24;
};

/* Global struct instances */
struct plain_struct global_plain_struct = {1, 2.0f, 'A'};
struct attributed_struct global_attributed_struct;
struct bitfield_struct global_bitfield_struct = {1, 7, 15, 1000};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;
typedef struct attributed_struct user_attributed_struct_t;

/* User struct variables */
user_struct_t global_user_struct = {10, 20.5f, 'Z'};
user_attributed_struct_t global_user_attributed_struct;

/* ========== TYPE_UNION examples ========== */
union basic_union {
    int as_int;
    float as_float;
    char as_char;
    void* as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int* int_ptr;
    void* void_ptr;
} transparent_union_t;

/* Union with attributes */
union __attribute__((aligned(32))) aligned_union {
    double d;
    long long ll;
    char buffer[16];
};

/* Global union instances */
union basic_union global_basic_union = {.as_int = 255};
union aligned_union global_aligned_union;
transparent_union_t global_transparent_union;

/* ========== TYPE_POINTER examples ========== */
int* global_int_ptr = &global_int;
const float* global_const_float_ptr = &global_volatile_float;
volatile char* global_volatile_char_ptr;
void* global_void_ptr;
struct plain_struct* global_struct_ptr = &global_plain_struct;
user_struct_t* global_user_struct_ptr = &global_user_struct;
union basic_union* global_union_ptr = &global_basic_union;

/* Complex pointer types */
int** global_double_ptr = &global_int_ptr;
const volatile int* volatile global_cv_complex_ptr;
int* const global_const_ptr = &global_int;
volatile int* restrict global_restrict_ptr;

/* ========== TYPE_ARRAY examples ========== */
int global_int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float global_float_array[5][5];
char global_char_array[3][4][5];
const int global_const_array[3] = {100, 200, 300};
volatile short global_volatile_array[8];

/* Array of structs */
struct plain_struct global_struct_array[4] = {
    {1, 1.0f, 'a'},
    {2, 2.0f, 'b'},
    {3, 3.0f, 'c'},
    {4, 4.0f, 'd'}
};

/* Array of pointers */
int* global_ptr_array[5];

/* ========== TYPE_STRING examples ========== */
const char* global_string_literal = "Hello, gengtype!";
char global_char_string[] = "Test string";
const char* global_string_array[] = {"first", "second", "third"};
volatile const char* global_volatile_string = "Volatile string";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct plain_struct*, union basic_union*);
typedef int* (*returning_pointer_callback_t)(void);
typedef void (*const_callback_t)(void) const;

/* Callback variables */
simple_callback_t global_simple_callback = NULL;
complex_callback_t global_complex_callback;
returning_pointer_callback_t global_returning_callback;

/* ========== Complex nested types ========== */
/* Struct containing pointer to another struct */
struct nested_struct_a {
    int id;
    struct nested_struct_b* next;
};

struct nested_struct_b {
    int id;
    struct nested_struct_a* prev;
    union basic_union data;
};

/* Union containing array of structs */
union container_union {
    struct {
        int count;
        struct plain_struct items[10];
    } struct_array;
    struct {
        float values[5][5];
    } float_matrix;
};

/* Typedef for function pointer returning pointer to struct */
typedef struct nested_struct_a* (*struct_factory_t)(int);

/* Struct with all type kinds */
struct kitchen_sink {
    /* scalar */
    int scalar_member;
    
    /* struct */
    struct plain_struct struct_member;
    
    /* union */
    union basic_union union_member;
    
    /* pointer */
    void* pointer_member;
    
    /* array */
    int array_member[5];
    
    /* string */
    const char* string_member;
    
    /* callback */
    simple_callback_t callback_member;
    
    /* nested pointer to self */
    struct kitchen_sink* self_pointer;
};

/* Global instance of complex type */
struct kitchen_sink global_kitchen_sink = {
    .scalar_member = 999,
    .struct_member = {50, 60.5f, 'X'},
    .union_member = {.as_float = 3.14159f},
    .pointer_member = &global_int,
    .array_member = {10, 20, 30, 40, 50},
    .string_member = "Kitchen sink string",
    .callback_member = NULL,
    .self_pointer = NULL
};

/* ========== GCC pragmas and attributes ========== */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    short c;
};
#pragma pack(pop)

struct __attribute__((may_alias)) aliasing_struct {
    int x;
    float y;
};

/* ========== Function definitions ========== */
/* Simple callback implementation */
int sample_callback(int a, float b) {
    return a + (int)b;
}

/* Function using transparent union */
void use_transparent_union(transparent_union_t u) {
    *(int*)u.void_ptr = 42;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use scalar types */
    global_int++;
    global_volatile_float = global_volatile_float * 2.0f;
    prevent_optimization += global_int;
    
    /* Use struct types */
    global_plain_struct.x = 100;
    global_user_struct.y = 200.5f;
    global_kitchen_sink.struct_member.z = 'M';
    
    /* Use union types */
    global_basic_union.as_float = 1.234f;
    global_aligned_union.d = 3.1415926535;
    
    /* Use pointer types */
    *global_int_ptr = 999;
    global_struct_ptr = &global_kitchen_sink.struct_member;
    
    /* Use array types */
    global_int_array[0] = 111;
    global_struct_array[1].x = 222;
    
    /* Use string types */
    prevent_optimization += global_string_literal[0];
    global_char_string[0] = 'T';
    
    /* Use callback types */
    global_simple_callback = sample_callback;
    if (global_simple_callback) {
        prevent_optimization += global_simple_callback(10, 20.0f);
    }
    
    /* Initialize nested structures */
    static struct nested_struct_a node_a = {1, NULL};
    static struct nested_struct_b node_b = {2, &node_a, {.as_int = 100}};
    node_a.next = &node_b;
    
    /* Use complex nested types */
    global_kitchen_sink.self_pointer = &global_kitchen_sink;
    
    /* Use transparent union */
    int transparent_target = 0;
    transparent_union_t tu = {.int_ptr = &transparent_target};
    use_transparent_union(tu);
    
    return prevent_optimization == 0 ? 0 : 1;
}
