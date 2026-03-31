/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* Forward declarations for incomplete types */
extern struct undefined_struct;          /* TYPE_UNDEFINED */
extern union undefined_union;            /* TYPE_UNDEFINED */
extern int undefined_array[];            /* TYPE_UNDEFINED (incomplete array) */

/* TYPE_SCALAR declarations with various qualifiers */
__attribute__((unused)) volatile const _Bool scalar_bool = 0;
__attribute__((unused)) volatile int scalar_int = 42;
__attribute__((unused)) const float scalar_float = 3.14f;
__attribute__((unused)) volatile const double scalar_double = 2.71828;

/* TYPE_STRING declarations */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) volatile char* volatile string_pointer = "Test";
__attribute__((unused)) const char* const const_string_ptr = "Constant";

/* TYPE_STRUCT with annotation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* TYPE_UNION with annotation */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int id;
    char data[32];
} user_struct_t;

/* TYPE_POINTER variations */
__attribute__((unused)) volatile int* volatile volatile_int_ptr;
__attribute__((unused)) const struct annotated_struct* struct_ptr;
__attribute__((unused)) union annotated_union* union_ptr;
__attribute__((unused)) user_struct_t* user_struct_ptr;
__attribute__((unused)) void (*volatile volatile_func_ptr)(void);

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];                     /* Fixed-size */
__attribute__((unused)) volatile int volatile_array[5];          /* Volatile elements */
__attribute__((unused)) const float const_array[3] = {1.0, 2.0, 3.0};
__attribute__((unused)) struct annotated_struct struct_array[2];
__attribute__((unused)) user_struct_t user_struct_array[4];

/* TYPE_CALLBACK - function pointer types */
typedef int (*binary_op_t)(int, int);                           /* Simple callback */
typedef void (*complex_callback_t)(struct annotated_struct*,    /* Complex callback */
                                   union annotated_union*,
                                   user_struct_t*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
int (*annotated_callback)(const char*, int);

/* Nested type constructs */
struct nested_container {
    /* Struct containing array of function pointers */
    binary_op_t operations[4];                  /* TYPE_ARRAY of TYPE_CALLBACK */
    
    /* Union with pointer to struct */
    union {
        struct annotated_struct* sptr;
        user_struct_t* uptr;
    } data_union;                               /* TYPE_UNION with TYPE_POINTER fields */
    
    /* Pointer to array */
    volatile int (*matrix_ptr)[3][3];           /* TYPE_POINTER to TYPE_ARRAY */
    
    /* Callback field */
    complex_callback_t processor;               /* TYPE_CALLBACK */
};

/* Another complex type */
union type_kitchen_sink {
    struct nested_container container;          /* TYPE_STRUCT */
    binary_op_t func_array[2];                  /* TYPE_ARRAY of TYPE_CALLBACK */
    void* generic_pointer;                      /* TYPE_POINTER */
    char string_storage[256];                   /* TYPE_ARRAY (as string buffer) */
};

/* Global instances */
__attribute__((unused)) struct nested_container global_container;
__attribute__((unused)) union type_kitchen_sink global_sink;
__attribute__((unused)) volatile const struct undefined_struct* extern_ptr;

/* Function using __builtin_types_compatible_p for type comparisons */
static void perform_type_comparisons(void) {
    /* Compare scalar types */
    int is_int_compatible = __builtin_types_compatible_p(int, volatile int);
    int is_float_int_compatible = __builtin_types_compatible_p(float, int);
    
    /* Compare pointer types */
    int is_ptr_compatible = __builtin_types_compatible_p(int*, const int*);
    int is_struct_ptr_compatible = __builtin_types_compatible_p(
        struct annotated_struct*, 
        const struct annotated_struct*
    );
    
    /* Compare struct vs union */
    int is_struct_union_compatible = __builtin_types_compatible_p(
        struct annotated_struct,
        union annotated_union
    );
    
    /* Compare array types */
    int is_array_compatible = __builtin_types_compatible_p(int[10], int[5]);
    int is_ptr_to_array_compatible = __builtin_types_compatible_p(
        int (*)[10],
        int*
    );
    
    /* Compare callback types */
    int is_callback_compatible = __builtin_types_compatible_p(
        binary_op_t,
        int (*)(int, int)
    );
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = 0;
    dummy += is_int_compatible + is_float_int_compatible + 
             is_ptr_compatible + is_struct_ptr_compatible +
             is_struct_union_compatible + is_array_compatible +
             is_ptr_to_array_compatible + is_callback_compatible;
}

/* Function to exercise function pointers */
static int add(int a, int b) { return a + b; }
static int multiply(int a, int b) { return a * b; }

static void exercise_callbacks(void) {
    binary_op_t ops[] = {add, multiply};
    volatile int result = 0;
    
    if (ops[0]) result = ops[0](5, 3);    /* 5 + 3 = 8 */
    if (ops[1]) result = ops[1](5, 3);    /* 5 * 3 = 15 */
    
    /* Use annotated callback if initialized */
    if (annotated_callback) {
        result = annotated_callback("test", result);
    }
}

/* Main function with diverse type usage */
int main(void) {
    /* Initialize some structures */
    global_container.operations[0] = add;
    global_container.operations[1] = multiply;
    
    /* Use incomplete types in sizeof (valid for pointers) */
    size_t undefined_size = sizeof(struct undefined_struct*);
    size_t extern_array_size = sizeof(undefined_array);
    
    /* Exercise type comparisons */
    perform_type_comparisons();
    
    /* Exercise callbacks */
    exercise_callbacks();
    
    /* Use various types to prevent elimination */
    volatile int checksum = 0;
    checksum += scalar_int;
    checksum += (int)scalar_float;
    checksum += fixed_array[0];  /* May be uninitialized, but access is valid */
    checksum += (int)undefined_size;
    checksum += (int)extern_array_size;
    
    /* Access through pointers */
    if (struct_ptr) checksum += 1;
    if (union_ptr) checksum += 2;
    
    /* Use string literal */
    if (string_literal[0]) checksum += 4;
    
    return checksum == 0 ? 0 : 1;
}

/* Additional incomplete/forward declarations */
struct lang_struct;  /* TYPE_LANG_STRUCT (when processed by language-specific code) */

/* Array of pointers to different types */
void* type_pointers[] = {
    (void*)&scalar_int,
    (void*)string_literal,
    (void*)&global_container,
    (void*)&global_sink,
    (void*)add,
    (void*)undefined_array
};

/* Const volatile qualified function pointer */
int (* const volatile volatile_const_callback)(int, int) = add;
