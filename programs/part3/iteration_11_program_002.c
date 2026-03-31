/* Complex type declarations to exercise GCC's gengtype type classification */
#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED array */
struct forward_declared;         /* Forward declaration creates undefined type */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 99L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";
__attribute__((unused)) volatile char* volatile volatile_string_ptr;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int id;
    double value;
} user_struct_t;

/* ========== TYPE_UNION with complex qualifiers ========== */
union __attribute__((annotate("gengtype"))) complex_union {
    int as_int;
    float as_float;
    void* as_pointer;
    const char* as_string;
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_pointer = &int_scalar;
__attribute__((unused)) volatile const int* const volatile_const_pointer = &int_scalar;
__attribute__((unused)) struct annotated_struct* struct_pointer;
__attribute__((unused)) union complex_union* union_pointer;
__attribute__((unused)) void (*function_pointer)(void);
__attribute__((unused)) int (*array_pointer)[5];

/* ========== TYPE_ARRAY declarations ========== */
__attribute__((unused)) int fixed_array[10];                     /* Fixed-size */
__attribute__((unused)) volatile const float volatile_const_array[3] = {1.0f, 2.0f, 3.0f};
__attribute__((unused)) struct annotated_struct struct_array[2];
__attribute__((unused)) union complex_union union_array[4];

/* Variable-length array (in function scope) */
void use_vla(int size) {
    int vla[size];  /* TYPE_ARRAY with variable size */
    __attribute__((unused)) int sum = 0;
    for (int i = 0; i < size; i++) {
        vla[i] = i;
        sum += vla[i];
    }
}

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op_t)(int, int);  /* Callback typedef */

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) void (*annotated_callback_t)(int, const char*);

/* Complex callback signature */
typedef union complex_union* (*complex_callback_t)(
    struct annotated_struct*, 
    binary_op_t, 
    int[]
);

/* ========== Nested type constructs ========== */
/* Struct containing array of function pointers */
struct nested_container {
    binary_op_t operations[4];           /* Array of callbacks */
    union complex_union data;            /* Union field */
    struct annotated_struct* next;       /* Pointer to struct */
    volatile const int* volatile const volatile_ptr; /* Complex pointer */
};

/* Union with pointer to struct */
union pointer_union {
    struct nested_container* container_ptr;
    annotated_callback_t callback_ptr;
    int (*array_ptr)[10];
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed through attributes */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
    void* ptr;
};

/* ========== Type comparison expressions ========== */
/* These may trigger type classification during compilation */
#define CHECK_TYPE_COMPAT(a, b) \
    __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

/* ========== Main function with type usage ========== */
int main() {
    /* Declare and initialize complex types */
    struct annotated_struct my_struct = {1, 2.5f, "test"};
    union complex_union my_union;
    my_union.as_int = 100;
    
    user_struct_t user_struct = {42, 3.14159};
    struct nested_container container = {
        .operations = {NULL, NULL, NULL, NULL},
        .data = my_union,
        .next = &my_struct,
        .volatile_ptr = &int_scalar
    };
    
    /* Use variable-length array */
    use_vla(5);
    
    /* Function pointer usage */
    binary_op_t add_func = NULL;
    annotated_callback_t ann_func = NULL;
    
    /* Array usage */
    fixed_array[0] = int_scalar;
    struct_array[0] = my_struct;
    
    /* Pointer operations */
    int_pointer = &fixed_array[0];
    struct_pointer = &my_struct;
    
    /* Type compatibility checks (compile-time) */
    __attribute__((unused)) int compat1 = 
        CHECK_TYPE_COMPAT(int_scalar, float_scalar);  /* Scalar vs scalar */
    __attribute__((unused)) int compat2 = 
        CHECK_TYPE_COMPAT(int_pointer, &float_scalar); /* Pointer vs pointer */
    __attribute__((unused)) int compat3 = 
        CHECK_TYPE_COMPAT(my_struct, user_struct);     /* Struct vs user_struct */
    __attribute__((unused)) int compat4 = 
        CHECK_TYPE_COMPAT(my_union, container.data);   /* Union vs union */
    
    /* Use incomplete/extern types through pointers */
    struct forward_declared* forward_ptr = NULL;
    struct undefined_struct* undefined_ptr = NULL;
    
    /* sizeof operations with various types */
    __attribute__((unused)) size_t sizes[] = {
        sizeof(bool_scalar),
        sizeof(string_literal),
        sizeof(my_struct),
        sizeof(my_union),
        sizeof(int_pointer),
        sizeof(fixed_array),
        sizeof(binary_op_t),
        sizeof(struct transaction_struct)
    };
    
    /* Ensure no dead code elimination */
    if (int_scalar > 0) {
        float_scalar *= 2.0f;
    }
    
    return 0;
}

/* Additional external declarations for TYPE_UNDEFINED */
extern struct external_undefined;
extern int external_undefined_func(void);

/* Complex function pointer variable */
__attribute__((unused)) complex_callback_t global_callback = NULL;

/* Const-qualified array of pointers to const-qualified structs */
__attribute__((unused)) const struct annotated_struct* const const_struct_array[3] = {NULL, NULL, NULL};

/* Volatile-qualified pointer to array of function pointers */
__attribute__((unused)) volatile binary_op_t (*volatile volatile_callback_array)[5] = NULL;
