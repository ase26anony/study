/* gengtype_test.c - Comprehensive type declarations to exercise gengtype.cc classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;          /* TYPE_UNDEFINED */
extern int undefined_array[];            /* TYPE_UNDEFINED array */
struct forward_declared;                 /* Forward declaration */

/* ========== TYPE_SCALAR ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";

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

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_ptr = &int_scalar;
__attribute__((unused)) volatile const int* const volatile_const_ptr = &int_scalar;
__attribute__((unused)) struct annotated_struct* struct_ptr = 0;
__attribute__((unused)) void (*func_ptr)(void) = 0;

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float multi_dim[3][4];
__attribute__((unused)) char* pointer_array[5];
__attribute__((unused)) volatile const short volatile_array[] = {1, 2, 3};

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, user_struct_t*);

__attribute__((annotate("gengtype"))) 
int (*annotated_callback)(char*, ...);  /* Variadic function pointer */

/* ========== Complex nested type constructs ========== */

/* Struct containing array of function pointers */
struct nested_container {
    simple_callback_t callbacks[5];
    union annotated_union data;
    volatile const int* ref_count;
};

/* Union with pointer to struct */
union pointer_union {
    struct nested_container* container;
    complex_callback_t callback;
    long long large_int;
};

/* Typedef for complex function signature */
typedef union pointer_union* (*factory_callback_t)(
    int, 
    struct forward_declared**
);

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed through attributes */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
};

/* ========== Type comparison expressions ========== */
/* These may trigger internal type classification */
static void type_comparisons(void) {
    /* Use __builtin_types_compatible_p to compare types */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    int struct_vs_union = __builtin_types_compatible_p(
        struct annotated_struct, 
        union annotated_union
    );
    int array_vs_pointer = __builtin_types_compatible_p(
        int[10], 
        int*
    );
    int callback_vs_ptr = __builtin_types_compatible_p(
        simple_callback_t,
        void*
    );
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t undefined_size = sizeof(struct undefined_struct*);
    size_t forward_size = sizeof(struct forward_declared*);
    
    /* Suppress unused warnings */
    (void)scalar_vs_pointer;
    (void)struct_vs_union;
    (void)array_vs_pointer;
    (void)callback_vs_ptr;
    (void)undefined_size;
    (void)forward_size;
}

/* ========== Main function with usage ========== */
int main(void) {
    /* Declare variables of our complex types */
    struct nested_container container __attribute__((unused));
    union pointer_union pu __attribute__((unused));
    user_struct_t user_struct __attribute__((unused));
    struct transaction_struct ts __attribute__((unused));
    
    /* Initialize some values to prevent dead code elimination */
    container.ref_count = &int_scalar;
    pu.container = &container;
    user_struct.id = 1;
    user_struct.value = 2.71828;
    ts.data = 99;
    
    /* Use function pointers */
    if (func_ptr) func_ptr();
    
    /* Perform type comparisons */
    type_comparisons();
    
    /* Use sizeof with various types */
    size_t sizes[] = {
        sizeof(bool_scalar),
        sizeof(string_literal),
        sizeof(struct annotated_struct),
        sizeof(union annotated_union),
        sizeof(fixed_array),
        sizeof(simple_callback_t),
        sizeof(volatile_const_ptr)
    };
    
    /* Trivial use to prevent optimization */
    return (int)sizes[0] - (int)sizes[0];  /* Always returns 0 */
}

/* ========== Additional external declarations ========== */
/* Create more TYPE_UNDEFINED references */
extern struct undefined_struct* get_undefined(void);
extern void use_callback(complex_callback_t cb);

/* Forward-declared struct definition (if needed elsewhere) */
struct forward_declared {
    int placeholder;
};
