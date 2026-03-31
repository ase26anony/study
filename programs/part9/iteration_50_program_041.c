/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* ========== TYPE_SCALAR examples ========== */
typedef int scalar_int_t;
typedef float scalar_float_t;
typedef double scalar_double_t;
typedef enum { RED, GREEN, BLUE } color_enum;

/* ========== TYPE_STRUCT examples ========== */
struct plain_struct {
    int x;
    float y;
    char z;
};

/* Struct with GCC attributes for TYPE_LANG_STRUCT */
struct __attribute__((packed, aligned(8))) attributed_struct {
    int id;
    char name[20];
    volatile long counter;
};

/* ========== TYPE_USER_STRUCT examples ========== */
typedef struct plain_struct user_struct_t;
typedef struct attributed_struct attributed_user_t;

/* ========== TYPE_UNION examples ========== */
union data_union {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
} transparent_union_t;

/* ========== TYPE_POINTER examples ========== */
typedef int* int_ptr_t;
typedef struct plain_struct* struct_ptr_t;
typedef void (*void_func_ptr)(void);
typedef const volatile int* const volatile_qualified_ptr;

/* ========== TYPE_ARRAY examples ========== */
typedef int int_array_1d[10];
typedef char char_array_2d[5][5];
typedef struct plain_struct struct_array[3];

/* ========== TYPE_STRING examples ========== */
const char* string_literal = "Hello, gengtype!";
static const char static_string[] = "Static string";

/* ========== TYPE_CALLBACK examples ========== */
typedef int (*comparator_t)(const void*, const void*);
typedef void (*callback_t)(int, const char*);
typedef struct plain_struct* (*struct_factory_t)(void);

/* ========== Complex nested types ========== */
struct nested_container {
    /* TYPE_STRUCT containing various types */
    struct inner_struct {
        int id;
        union data_union data;  /* TYPE_UNION member */
        int_array_1d numbers;   /* TYPE_ARRAY member */
    } inner;
    
    /* TYPE_POINTER to union */
    union data_union *union_ptr;
    
    /* TYPE_ARRAY of pointers */
    struct plain_struct* struct_ptrs[5];
    
    /* TYPE_CALLBACK */
    comparator_t compare_func;
    
    /* TYPE_STRING */
    const char *description;
};

/* ========== Global variables for each type ========== */
/* TYPE_SCALAR */
scalar_int_t global_int = 42;
color_enum global_color = GREEN;

/* TYPE_STRUCT */
struct plain_struct global_struct = {1, 3.14f, 'A'};
struct attributed_struct global_attributed_struct = {100, "test", 999};

/* TYPE_USER_STRUCT */
user_struct_t global_user_struct = {2, 2.718f, 'B'};

/* TYPE_UNION */
union data_union global_union = {.as_int = 255};

/* TYPE_POINTER */
int_ptr_t global_int_ptr = &global_int;
struct_ptr_t global_struct_ptr = &global_struct;
void_func_ptr global_func_ptr = NULL;
volatile_qualified_ptr global_qualified_ptr = NULL;

/* TYPE_ARRAY */
int_array_1d global_int_array = {0,1,2,3,4,5,6,7,8,9};
char_array_2d global_char_array = {{'a','b','c','d','e'},
                                   {'f','g','h','i','j'},
                                   {'k','l','m','n','o'},
                                   {'p','q','r','s','t'},
                                   {'u','v','w','x','y'}};
struct_array global_struct_array = {{10, 1.1f, 'X'},
                                   {20, 2.2f, 'Y'},
                                   {30, 3.3f, 'Z'}};

/* TYPE_STRING */
const char* global_string_ptr = string_literal;

/* TYPE_CALLBACK */
comparator_t global_comparator = NULL;

/* Complex nested type */
struct nested_container global_container = {
    .inner = {1000, {.as_float = 3.14159f}, {100, 200, 300}},
    .union_ptr = &global_union,
    .struct_ptrs = {&global_struct, &global_struct, &global_struct},
    .compare_func = NULL,
    .description = "Nested container"
};

/* ========== Function using callback ========== */
static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_callback(int val, const char* msg) {
    /* Prevent unused parameter warnings */
    (void)val;
    (void)msg;
}

/* ========== Main function ========== */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use TYPE_SCALAR */
    global_int++;
    global_color = BLUE;
    
    /* Use TYPE_STRUCT */
    global_struct.x = 100;
    global_attributed_struct.counter++;
    
    /* Use TYPE_USER_STRUCT */
    global_user_struct.z = 'C';
    
    /* Use TYPE_UNION */
    global_union.as_float = 2.5f;
    
    /* Use TYPE_POINTER */
    if (global_int_ptr) {
        *global_int_ptr = 50;
    }
    
    /* Use TYPE_ARRAY */
    global_int_array[0] = 999;
    global_char_array[0][0] = 'Z';
    global_struct_array[1].y = 4.0f;
    
    /* Use TYPE_STRING */
    prevent_optimization += (global_string_ptr[0] == 'H');
    
    /* Use TYPE_CALLBACK */
    global_comparator = sample_comparator;
    callback_t local_callback = sample_callback;
    local_callback(1, "test");
    
    /* Use complex nested type */
    global_container.inner.id = 2000;
    if (global_container.union_ptr) {
        global_container.union_ptr->as_int = 500;
    }
    
    /* Use function pointer */
    if (global_func_ptr) {
        global_func_ptr();
    }
    
    return prevent_optimization;
}

/* ========== Additional GCC-specific constructs ========== */
#pragma pack(push, 1)
struct packed_struct {
    char a;
    int b;
    char c;
};
#pragma pack(pop)

/* Struct with vector attribute (GCC extension) */
typedef int v4si __attribute__ ((vector_size (16)));

/* Struct with cleanup attribute */
typedef struct {
    int fd;
} file_handle __attribute__((cleanup));

/* Forward declaration to test TYPE_UNDEFINED? */
struct forward_declared_struct;

/* Array of function pointers */
typedef void (*func_ptr_array_t[5])(void);

/* Const pointer to array of const pointers to const int */
const int *const (*complex_ptr)[10];

/* Anonymous struct/union */
struct {
    union {
        int x;
        float y;
    };
    struct {
        char a;
        char b;
    };
} anonymous_compound;
