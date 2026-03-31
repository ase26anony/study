/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* Forward declarations for incomplete types */
struct incomplete_struct;
union incomplete_union;
extern struct incomplete_struct extern_incomplete;
extern int extern_array[];

/* TYPE_UNDEFINED triggers */
struct undefined_type *undef_ptr;
extern struct undefined_type extern_undef;

/* TYPE_SCALAR declarations */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* TYPE_STRING declarations */
__attribute__((unused)) char *string_literal = "Hello, gengtype!";
__attribute__((unused)) const char *const const_string = "Constant string";
__attribute__((unused)) volatile char *volatile volatile_string_ptr;

/* TYPE_STRUCT with annotation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char *name;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct __attribute__((annotate("gengtype"))) {
    double data;
    void *metadata;
} user_struct_t;

/* TYPE_UNION with complex nesting */
union __attribute__((annotate("gengtype"))) complex_union {
    int as_int;
    float as_float;
    struct {
        char byte1;
        char byte2;
        char byte3;
        char byte4;
    } as_bytes;
    void *as_pointer;
};

/* TYPE_POINTER variations */
__attribute__((unused)) int *int_ptr = &int_scalar;
__attribute__((unused)) float *float_ptr = &float_scalar;
__attribute__((unused)) volatile const int *const volatile_const_ptr = &int_scalar;
__attribute__((unused)) struct annotated_struct *struct_ptr;
__attribute__((unused)) union complex_union *union_ptr;
__attribute__((unused)) user_struct_t *user_struct_ptr;
__attribute__((unused)) void **void_ptr_ptr;

/* TYPE_ARRAY declarations */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float multi_dim[5][3];
__attribute__((unused)) volatile const char volatile_const_array[8];
__attribute__((unused)) struct annotated_struct struct_array[4];
__attribute__((unused)) union complex_union union_array[2];

/* Variable Length Array (VLA) */
__attribute__((unused)) void use_vla(int n) {
    int vla_array[n];
    vla_array[0] = 1;
}

/* TYPE_CALLBACK - function pointers */
typedef int (*simple_callback)(int, float);
typedef void (*complex_callback)(struct annotated_struct*, union complex_union*);

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) 
    int (*annotated_callback)(const char*, ...);

/* Nested type: struct containing array of function pointers */
struct __attribute__((annotate("gengtype"))) container {
    int id;
    simple_callback callbacks[5];
    complex_callback complex_handler;
    annotated_callback varargs_handler;
};

/* Union with pointer to struct */
union pointer_union {
    struct container *container_ptr;
    user_struct_t *user_ptr;
    annotated_callback callback_ptr;
};

/* Complex typedef for nested function pointer */
typedef union pointer_union* (*factory_function)(int, const char*);

/* TYPE_LANG_STRUCT simulation via GCC extension */
#ifdef __GNUC__
struct __attribute__((transparent_union)) transparent_union {
    int *int_ptr;
    float *float_ptr;
};
#endif

/* Use __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* These comparisons force GCC to classify types internally */
    int is_scalar_ptr_compatible = __builtin_types_compatible_p(
        typeof(int_scalar), typeof(int_ptr));
    
    int is_struct_union_compatible = __builtin_types_compatible_p(
        typeof(struct container), typeof(union complex_union));
    
    int is_array_pointer_compatible = __builtin_types_compatible_p(
        typeof(fixed_array), typeof(int_ptr));
    
    int is_callback_compatible = __builtin_types_compatible_p(
        typeof(simple_callback), typeof(complex_callback));
    
    /* Use results to avoid dead code elimination */
    if (is_scalar_ptr_compatible) {
        int_scalar = 1;
    }
}

/* Function using many of the declared types */
static void process_types(void) {
    /* Initialize struct */
    struct container my_container = {
        .id = 1,
        .complex_handler = 0,
        .varargs_handler = 0
    };
    
    /* Use union */
    union complex_union my_union;
    my_union.as_int = 42;
    
    /* Use pointers */
    struct_ptr = &(struct annotated_struct){ .x = 1, .y = 2.0f, .name = "test" };
    user_struct_ptr = &(user_struct_t){ .data = 3.14159, .metadata = 0 };
    
    /* Use arrays */
    fixed_array[0] = int_scalar;
    struct_array[0].x = 10;
    
    /* Use function pointer */
    simple_callback my_callback = 0;
    
    /* Sizeof operations with various types */
    size_t sizes[] = {
        sizeof(struct container),
        sizeof(union complex_union),
        sizeof(user_struct_t),
        sizeof(fixed_array),
        sizeof(int_ptr),
        sizeof(my_callback),
        sizeof(struct incomplete_struct*),  /* Incomplete type pointer */
        sizeof(extern_array)                /* Incomplete array */
    };
    
    /* Prevent optimization */
    asm volatile("" : : "r"(sizes) : "memory");
}

int main(void) {
    /* Ensure all types are processed */
    compare_types();
    process_types();
    
    /* Use VLA */
    use_vla(5);
    
    /* Return statement using some declared variables */
    return int_scalar > 0 ? 0 : 1;
}

/* Additional incomplete type declarations */
struct incomplete_struct {
    /* Left incomplete in this translation unit */
};

/* External array definition */
int extern_array[10];
