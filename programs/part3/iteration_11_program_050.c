/* Complex type declarations to exercise GCC's gengtype type classification */
#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED */
struct forward_declared;         /* Forward declaration */

/* ========== TYPE_SCALAR ========== */
__attribute__((unused)) _Bool bool_scalar;
__attribute__((unused)) int int_scalar;
__attribute__((unused)) float float_scalar;
__attribute__((unused)) volatile const long volatile_const_scalar;

/* ========== TYPE_STRING ========== */
__attribute__((unused)) char* string_literal = "test";
__attribute__((unused)) const char* const const_string = "constant";
__attribute__((unused)) volatile char* volatile volatile_string;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    char* field2;
    volatile const float field3;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct __attribute__((annotate("gengtype"))) {
    double x;
    double y;
} point_t;

/* Nested struct for complex type */
struct container {
    point_t point;
    struct annotated_struct nested;
};

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    char* as_string;
    void* as_pointer;
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_ptr;
__attribute__((unused)) const int* const_ptr_to_const;
__attribute__((unused)) volatile int* volatile_ptr;
__attribute__((unused)) volatile const int* const volatile_ptr;
__attribute__((unused)) struct annotated_struct* struct_ptr;
__attribute__((unused)) union data_union* union_ptr;
__attribute__((unused)) point_t* user_struct_ptr;
__attribute__((unused)) void (*func_ptr)(void);  /* Also TYPE_CALLBACK */

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) char* string_array[5];
__attribute__((unused)) volatile const float volatile_const_array[3];
__attribute__((unused)) struct annotated_struct struct_array[2];
__attribute__((unused)) point_t user_struct_array[4];
__attribute__((unused)) int multi_dim_array[3][4][5];

/* Variable length array (in function scope) */
void use_vla(int n) {
    int vla[n];  /* TYPE_ARRAY */
    __attribute__((unused)) volatile int* vla_ptr = vla;
}

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*binary_op_t)(int, int);  /* TYPE_USER_STRUCT -> TYPE_CALLBACK */

/* Complex callback signature */
typedef void (*event_callback_t)(
    struct container*,
    union data_union*,
    binary_op_t
) __attribute__((annotate("gengtype")));

/* Function pointer variables */
__attribute__((unused)) binary_op_t op_func;
__attribute__((unused)) event_callback_t event_handler;
__attribute__((unused)) void (*simple_callback)(void);

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed via attributes or builtins */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
};

/* ========== Complex nested type ========== */
struct __attribute__((annotate("gengtype"))) complex_type {
    /* Array of function pointers */
    binary_op_t operations[4];
    
    /* Pointer to union containing struct pointer */
    union data_union* data;
    
    /* Nested struct with array */
    struct {
        point_t points[10];
        int count;
    } geometry;
    
    /* Callback field */
    event_callback_t notify;
    
    /* Pointer to incomplete type */
    struct forward_declared* incomplete;
};

/* ========== Type comparison expressions ========== */
/* These force GCC to classify types for compatibility checks */
static void type_comparisons(void) {
    /* Compare scalar vs pointer */
    __attribute__((unused)) int scalar_vs_ptr = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    __attribute__((unused)) int struct_vs_union = __builtin_types_compatible_p(
        struct annotated_struct, 
        union data_union
    );
    
    /* Compare pointer types with different qualifiers */
    __attribute__((unused)) int qual_comparison = __builtin_types_compatible_p(
        const int*, 
        volatile int*
    );
    
    /* Compare array types */
    __attribute__((unused)) int array_comparison = __builtin_types_compatible_p(
        int[10], 
        int[5]
    );
    
    /* Compare function pointer types */
    __attribute__((unused)) int callback_comparison = __builtin_types_compatible_p(
        binary_op_t,
        void (*)(void)
    );
    
    /* Compare user-defined structs */
    __attribute__((unused)) int user_struct_comparison = __builtin_types_compatible_p(
        point_t,
        struct container
    );
}

/* ========== Function using the complex types ========== */
static int sample_binary_op(int a, int b) {
    return a + b;
}

static void sample_event(
    struct container* c,
    union data_union* d,
    binary_op_t op
) {
    if (c && d && op) {
        d->as_int = op(c->point.x, c->point.y);
    }
}

/* ========== Main function ========== */
int main(void) {
    /* Declare variables of our complex types */
    struct complex_type complex_var;
    struct container container_var = {{1.0, 2.0}, {42, "nested", 3.14f}};
    union data_union union_var = {.as_int = 100};
    point_t points[3] = {{0.0, 0.0}, {1.0, 1.0}, {2.0, 2.0}};
    
    /* Initialize function pointers */
    op_func = sample_binary_op;
    event_handler = sample_event;
    
    /* Use variable length array */
    use_vla(20);
    
    /* Force type comparisons */
    type_comparisons();
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    __attribute__((unused)) size_t incomplete_size = sizeof(struct forward_declared*);
    __attribute__((unused)) size_t extern_size = sizeof(undefined_array);
    
    /* Create complex nested usage */
    complex_var.operations[0] = sample_binary_op;
    complex_var.data = &union_var;
    complex_var.geometry.count = 3;
    complex_var.notify = sample_event;
    
    /* Use the types in non-dead code */
    if (op_func) {
        union_var.as_int = op_func(container_var.point.x, container_var.point.y);
    }
    
    if (event_handler) {
        event_handler(&container_var, &union_var, op_func);
    }
    
    /* Array operations */
    for (int i = 0; i < 3; i++) {
        points[i].x += 1.0;
        points[i].y += 1.0;
    }
    
    /* Pointer arithmetic with qualified pointers */
    const int* const const_array = (const int[]){1, 2, 3, 4, 5};
    __attribute__((unused)) const int* const_array_ptr = const_array + 2;
    
    return 0;
}

/* ========== Additional incomplete type definitions ========== */
/* Define previously forward-declared struct */
struct forward_declared {
    int magic;
    struct complex_type* link;
};

/* Define previously extern declared struct */
struct undefined_struct {
    int defined_now;
};
