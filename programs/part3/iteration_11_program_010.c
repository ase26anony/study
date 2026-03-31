/* gengtype_test.c - Comprehensive type declarations to exercise gengtype.cc type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED array */
struct forward_declared;         /* Forward declaration creates incomplete type */

/* ========== TYPE_SCALAR declarations ========== */
_Bool __attribute__((unused)) scalar_bool = 0;
int __attribute__((unused)) scalar_int = 42;
float __attribute__((unused)) scalar_float = 3.14f;
volatile const long __attribute__((unused)) scalar_volatile_const = 100L;

/* ========== TYPE_STRING declarations ========== */
char* __attribute__((unused)) string_literal = "Hello, gengtype!";
const char* __attribute__((unused)) const_string = "Constant string";
volatile char* __attribute__((unused)) volatile_string_ptr;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    float field2;
    char* field3;
};

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int x;
    int y;
} point_t;  /* TYPE_USER_STRUCT */

typedef struct __attribute__((annotate("gengtype"))) {
    point_t position;
    float velocity[3];
    void (*update)(void*);
} entity_t;  /* TYPE_USER_STRUCT with annotation */

/* ========== TYPE_POINTER variations ========== */
int* __attribute__((unused)) int_ptr = &scalar_int;
const float* __attribute__((unused)) const_float_ptr = &scalar_float;
volatile const int* const __attribute__((unused)) volatile_const_ptr = &scalar_int;
void* __attribute__((unused)) void_ptr = NULL;
struct annotated_struct* __attribute__((unused)) struct_ptr = NULL;
union annotated_union* __attribute__((unused)) union_ptr = NULL;

/* ========== TYPE_ARRAY variations ========== */
int __attribute__((unused)) fixed_array[10] = {0};
float __attribute__((unused)) multi_dim_array[5][3];
extern int extern_array[];  /* Incomplete array type */
int __attribute__((unused)) variable_len_array(size_t n) {
    int vla[n];  /* Variable-length array */
    return sizeof(vla);
}

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op_t)(int, int);  /* Function pointer typedef */

int __attribute__((unused)) add(int a, int b) { return a + b; }
int __attribute__((unused)) multiply(int a, int b) { return a * b; }

binary_op_t __attribute__((unused)) operation = add;

/* Complex callback with annotation */
typedef void (*__attribute__((annotate("gengtype"))) event_callback_t)
    (void* context, int event_type, const char* message);

/* ========== Complex nested type constructs ========== */
struct __attribute__((annotate("gengtype"))) complex_container {
    /* Struct containing array of function pointers */
    binary_op_t operations[4];
    
    /* Union with pointer to struct */
    union {
        struct annotated_struct* struct_ptr;
        entity_t* entity_ptr;
    } data_union;
    
    /* Pointer to array of pointers */
    int** matrix_ptr;
    
    /* Callback field */
    event_callback_t notify;
};

/* Union containing struct with array of function pointers */
union __attribute__((annotate("gengtype"))) mega_union {
    struct complex_container container;
    struct {
        void (*handlers[3])(void);
        int priorities[5];
    } handler_set;
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be triggered by certain constructs */
struct __attribute__((transparent_union)) transparent_union {
    int* ptr;
};

/* ========== Type comparison expressions ========== */
/* These may trigger internal type classification */
static void check_type_compatibility(void) __attribute__((unused));
static void check_type_compatibility(void) {
    /* Compare scalar vs pointer */
    int scalar_check = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    int struct_union_check = __builtin_types_compatible_p(
        struct annotated_struct, union annotated_union);
    
    /* Compare pointer types with different qualifiers */
    int qualifier_check = __builtin_types_compatible_p(
        const int*, volatile int*);
    
    /* Compare function pointer types */
    int callback_check = __builtin_types_compatible_p(
        binary_op_t, event_callback_t);
    
    /* Compare array types */
    int array_check = __builtin_types_compatible_p(
        int[10], float[10]);
    
    /* Use results to avoid dead code elimination */
    volatile int dummy = scalar_check + struct_union_check + 
                        qualifier_check + callback_check + array_check;
    (void)dummy;
}

/* ========== Main function with usage ========== */
int main(void) {
    /* Declare and initialize complex types */
    struct complex_container container = {
        .operations = {add, multiply, add, multiply},
        .data_union = {.struct_ptr = NULL},
        .matrix_ptr = NULL,
        .notify = NULL
    };
    
    union mega_union munion;
    entity_t entity = {
        .position = {0, 0},
        .velocity = {0.0f, 0.0f, 0.0f},
        .update = NULL
    };
    
    /* Use various types to ensure they're processed */
    int array_size = sizeof(fixed_array);
    int struct_size = sizeof(struct annotated_struct);
    int union_size = sizeof(union annotated_union);
    
    /* Use function pointer */
    int result = operation(5, 3);
    
    /* Use volatile and const qualified pointers */
    volatile const int* const read_only_ptr = &scalar_int;
    int value = *read_only_ptr;
    
    /* Use incomplete types through pointers */
    struct forward_declared* fwd_ptr = NULL;
    (void)fwd_ptr;
    
    /* Trigger type compatibility checks */
    check_type_compatibility();
    
    /* Use all declared variables to avoid elimination */
    (void)scalar_bool;
    (void)scalar_float;
    (void)string_literal;
    (void)const_string;
    (void)int_ptr;
    (void)void_ptr;
    (void)container;
    (void)munion;
    (void)entity;
    (void)array_size;
    (void)struct_size;
    (void)union_size;
    (void)value;
    
    return result - 8;  /* Should return 0 since 5+3-8=0 */
}

/* Additional incomplete type definitions */
struct forward_declared {
    int some_field;
};

/* External array definition */
int undefined_array[5] = {1, 2, 3, 4, 5};
