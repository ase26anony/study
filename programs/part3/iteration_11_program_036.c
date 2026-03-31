/* Complex type declarations to exercise GCC's gengtype type classification */
#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED array */
struct forward_declared;         /* Forward declaration creates TYPE_UNDEFINED */

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
    int x;
    float y;
    char* name;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct __attribute__((annotate("gengtype"))) {
    int id;
    struct annotated_struct* nested;
} user_struct_t;

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[16];
};

/* ========== TYPE_POINTER variations ========== */
int* __attribute__((unused)) int_ptr = &scalar_int;
float* __attribute__((unused)) float_ptr = &scalar_float;
const volatile int* __attribute__((unused)) const_volatile_int_ptr;
int* const __attribute__((unused)) const_ptr = &scalar_int;
int** __attribute__((unused)) ptr_to_ptr = &int_ptr;
struct annotated_struct* __attribute__((unused)) struct_ptr;
user_struct_t* __attribute__((unused)) user_struct_ptr;

/* ========== TYPE_ARRAY variations ========== */
int __attribute__((unused)) fixed_array[10] = {0};
float __attribute__((unused)) multi_dim_array[5][3];
char __attribute__((unused)) variable_len_array[scalar_int];  /* VLA */
const int __attribute__((unused)) const_array[] = {1, 2, 3, 4, 5};

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, user_struct_t*);

int __attribute__((unused)) sample_function(int a, float b) {
    return a + (int)b;
}

void __attribute__((unused)) complex_function(struct annotated_struct* s, user_struct_t* u) {
    if (s) s->x = 42;
    if (u) u->id = 99;
}

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) int (*annotated_callback_t)(void);

/* ========== Complex nested type constructs ========== */
struct __attribute__((annotate("gengtype"))) container {
    /* Nested union */
    union {
        int tag;
        float value;
    } data;
    
    /* Array of function pointers */
    simple_callback_t __attribute__((unused)) callbacks[5];
    
    /* Pointer to union */
    union data_union* union_ptr;
    
    /* Flexible array member */
    int flexible_array[];
};

/* Another complex nested type */
typedef struct {
    struct container* containers[3];
    complex_callback_t handler;
    volatile const int* volatile const tricky_ptr;
} super_container_t;

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed through extensions */
#ifdef __GNUC__
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void* data;
};
#endif

/* ========== Main function with type comparisons ========== */
int main() {
    /* Variable declarations using our complex types */
    struct annotated_struct my_struct = {1, 2.0f, "test"};
    user_struct_t my_user_struct = {100, &my_struct};
    union data_union my_union = {.as_int = 42};
    struct container my_container = {
        .data = {.tag = 1},
        .union_ptr = &my_union
    };
    
    /* Initialize function pointers */
    simple_callback_t my_callback = sample_function;
    complex_callback_t my_complex_callback = complex_function;
    
    /* Use __builtin_types_compatible_p for type comparisons */
    int __attribute__((unused)) type_comparisons = 0;
    
    /* Compare scalar types */
    type_comparisons += __builtin_types_compatible_p(typeof(scalar_int), int);
    type_comparisons += __builtin_types_compatible_p(typeof(scalar_float), float);
    
    /* Compare pointer types */
    type_comparisons += __builtin_types_compatible_p(typeof(int_ptr), int*);
    type_comparisons += __builtin_types_compatible_p(typeof(struct_ptr), struct annotated_struct*);
    
    /* Compare struct vs union */
    type_comparisons += __builtin_types_compatible_p(typeof(my_struct), typeof(my_union)) ? 1 : 0;
    
    /* Compare array types */
    type_comparisons += __builtin_types_compatible_p(typeof(fixed_array), int[10]);
    type_comparisons += __builtin_types_compatible_p(typeof(const_array), const int[5]);
    
    /* Compare function pointer types */
    type_comparisons += __builtin_types_compatible_p(typeof(my_callback), simple_callback_t);
    
    /* Use incomplete types in sizeof (valid in some contexts) */
    size_t __attribute__((unused)) incomplete_size = sizeof(struct forward_declared*);
    incomplete_size += sizeof(extern struct undefined_struct*);
    
    /* Use volatile and const qualified pointers */
    volatile const int* const volatile_ptr = &scalar_int;
    const volatile int* volatile const_ptr = (const volatile int*)&scalar_int;
    
    /* Call function via pointer to ensure it's not dead code */
    if (my_callback) {
        int __attribute__((unused)) result = my_callback(10, 20.5f);
    }
    
    /* Use the complex callback */
    if (my_complex_callback) {
        my_complex_callback(&my_struct, &my_user_struct);
    }
    
    /* Access nested types */
    my_container.data.tag = 2;
    if (my_container.union_ptr) {
        my_container.union_ptr->as_int = 100;
    }
    
    /* Use string literal */
    const char* __attribute__((unused)) local_string = string_literal;
    
    /* Return the sum of type comparisons (prevents dead code elimination) */
    return type_comparisons > 0 ? 0 : 1;
}

/* Additional external declarations for incomplete types */
extern struct external_undefined {
    int x;
    int y[];
} external_var;

/* Function with parameter of incomplete type */
void process_incomplete(struct forward_declared* param) {
    /* Can only use pointer operations */
    if (param) {
        /* Do nothing - just using the parameter */
    }
}
