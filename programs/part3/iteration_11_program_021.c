/* Complex type declarations to exercise GCC's gengtype type classification */
#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED for incomplete array */
struct forward_declared;         /* Forward declaration creates undefined type */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

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

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int id;
    double value;
    struct annotated_struct* nested;
} user_struct_t;

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_ptr = &int_scalar;
__attribute__((unused)) volatile const int* const volatile_const_ptr = &int_scalar;
__attribute__((unused)) void* void_ptr = NULL;
__attribute__((unused)) struct annotated_struct* struct_ptr = NULL;
__attribute__((unused)) user_struct_t* user_struct_ptr = NULL;

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float multi_dim_array[5][3];
__attribute__((unused)) volatile char volatile_array[8];
__attribute__((unused)) const int const_array[] = {1, 2, 3, 4, 5};  /* VLA-like */

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, user_struct_t*);

__attribute__((annotate("gengtype"))) 
int (*annotated_func_ptr)(int, char**);

/* Complex callback signature */
typedef union annotated_union* (*callback_returning_union_ptr_t)(
    int, 
    const char*, 
    void (*nested_callback)(void)
);

/* ========== Nested type constructs ========== */
/* Struct containing array of function pointers */
struct nested_container {
    simple_callback_t callbacks[5];
    union annotated_union data;
    volatile const int* volatile pointers[3];
};

/* Union with pointer to struct */
union pointer_union {
    struct nested_container* container_ptr;
    callback_returning_union_ptr_t func_ptr;
    int (*array_of_funcs[2])(void);
};

/* Typedef for complex function callback signature */
typedef int (*(*complex_func_factory_t)(int))(
    struct nested_container*, 
    union pointer_union
);

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Using GCC extension for transparent union */
typedef union __attribute__((transparent_union)) transparent_union_t {
    int i;
    float f;
    void* p;
} transparent_union_t;

/* ========== Builtin type comparisons ========== */
/* These expressions force GCC to classify types internally */
#define CHECK_TYPE_COMPAT(a, b) \
    __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

/* ========== Main function with type usage ========== */
int main(int argc, char** argv) {
    /* Force processing of all types */
    
    /* Use scalar types */
    int_scalar = float_scalar + 1;
    
    /* Use string type */
    if (string_literal[0]) {
        int_scalar++;
    }
    
    /* Declare and use struct */
    struct annotated_struct my_struct = {1, 2.0f, "test"};
    struct_ptr = &my_struct;
    
    /* Declare and use union */
    union annotated_union my_union;
    my_union.as_int = 42;
    
    /* Use user struct */
    user_struct_t user_struct = {100, 3.14159, &my_struct};
    user_struct_ptr = &user_struct;
    
    /* Use arrays */
    fixed_array[0] = int_scalar;
    volatile_array[0] = 'A';
    
    /* Use function pointer */
    __attribute__((unused)) int (*local_func_ptr)(int) = NULL;
    
    /* Use nested container */
    struct nested_container container = {{0}, {0}, {NULL}};
    
    /* Use pointer union */
    union pointer_union pu;
    pu.container_ptr = &container;
    
    /* Perform type comparisons to trigger classification */
    __attribute__((unused)) int type_checks[] = {
        CHECK_TYPE_COMPAT(int_scalar, float_scalar),
        CHECK_TYPE_COMPAT(int_ptr, void_ptr),
        CHECK_TYPE_COMPAT(my_struct, my_union),
        CHECK_TYPE_COMPAT(fixed_array, int_ptr),
        CHECK_TYPE_COMPAT(annotated_func_ptr, local_func_ptr)
    };
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    __attribute__((unused)) size_t incomplete_size = sizeof(struct forward_declared*);
    __attribute__((unused)) size_t extern_size = sizeof(undefined_array[0]);
    
    /* Use volatile and const qualified pointers */
    volatile const int* const complex_ptr = &int_scalar;
    __attribute__((unused)) int deref = *complex_ptr;
    
    /* Call via function pointer if non-NULL */
    if (annotated_func_ptr) {
        annotated_func_ptr(argc, argv);
    }
    
    /* Ensure no dead code elimination */
    return int_scalar > 0 ? 0 : 1;
}

/* Additional external declarations for incomplete types */
extern struct undefined_struct* get_undefined(void);
extern void use_callback(callback_returning_union_ptr_t cb);

/* Function definitions to satisfy references */
__attribute__((weak)) 
int default_callback(int x, float y) { return x + (int)y; }

/* Array of mixed function pointers */
__attribute__((unused)) 
static void* mixed_ptr_array[] = {
    &bool_scalar,
    string_literal,
    &my_struct,
    &my_union,
    fixed_array,
    default_callback
};

/* Complex nested declaration */
__attribute__((unused)) 
volatile const union pointer_union* const volatile nested_volatile_ptr = NULL;
