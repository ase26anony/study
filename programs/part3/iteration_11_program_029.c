/* gengtype_test.c - Comprehensive type declarations to exercise GCC's gengtype.cc type classifier */

/* Forward declarations for incomplete types */
extern struct undefined_extern_struct;      /* TYPE_UNDEFINED */
extern union undefined_extern_union;        /* TYPE_UNDEFINED */
extern int undefined_extern_array[];        /* TYPE_UNDEFINED array */

/* Forward declared struct for TYPE_LANG_STRUCT testing */
struct forward_declared_struct;

/* TYPE_SCALAR declarations with qualifiers */
volatile const _Bool volatile_scalar_bool __attribute__((unused));
const int const_scalar_int __attribute__((unused)) = 42;
volatile float volatile_scalar_float __attribute__((unused));

/* TYPE_STRING declarations */
char* string_literal1 __attribute__((unused)) = "Hello, gengtype!";
const char* const_string_literal __attribute__((unused)) = "Constant string";
volatile char* volatile_string_ptr __attribute__((unused));

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int field1;
    float field2;
    char* field3;
};

/* TYPE_UNION with annotation */
union annotated_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    long data1;
    double data2;
    struct annotated_struct nested_struct;
} user_defined_struct_t;

/* Complex nested struct combining multiple type classifications */
struct complex_nested_struct __attribute__((annotate("gengtype"))) {
    /* TYPE_ARRAY of function pointers (TYPE_CALLBACK) */
    void (*callbacks[5])(int, float);
    
    /* TYPE_POINTER to union */
    union annotated_union* union_ptr;
    
    /* TYPE_ARRAY of structs */
    struct annotated_struct struct_array[3];
    
    /* Nested union */
    union {
        int option1;
        struct annotated_struct option2;
    } nested_choice;
};

/* TYPE_POINTER declarations with various qualifiers */
volatile const int* const volatile_pointer_to_const __attribute__((unused));
int* restrict restricted_pointer __attribute__((unused));
const struct annotated_struct* const const_struct_pointer __attribute__((unused));
void* volatile volatile_void_pointer __attribute__((unused));

/* TYPE_ARRAY declarations */
int fixed_array[10] __attribute__((unused));
float multi_dim_array[3][4][5] __attribute__((unused));
struct annotated_struct struct_array_var[2] __attribute__((unused));
extern int extern_incomplete_array[];  /* TYPE_UNDEFINED */

/* Variable Length Array (VLA) - TYPE_ARRAY */
void use_vla(int size) {
    int vla[size] __attribute__((unused));
    /* Use VLA to prevent optimization */
    for (int i = 0; i < size; i++) {
        vla[i] = i * 2;
    }
}

/* TYPE_CALLBACK - Function pointer types */
typedef int (*simple_callback_t)(void);
typedef void (*complex_callback_t)(struct annotated_struct*, union annotated_union**, int[][3]);

/* Annotated function pointer */
int (*annotated_func_ptr)(int, float) __attribute__((annotate("gengtype")));

/* Callback in struct */
struct callback_container {
    simple_callback_t simple_cb;
    complex_callback_t complex_cb;
    void (*inline_cb)(double);
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) __attribute__((unused));
static void compare_types(void) {
    /* Compare various type combinations to trigger classification */
    int scalar_check = __builtin_types_compatible_p(int, float);  /* TYPE_SCALAR vs TYPE_SCALAR */
    int ptr_check = __builtin_types_compatible_p(int*, float*);   /* TYPE_POINTER vs TYPE_POINTER */
    int struct_check = __builtin_types_compatible_p(struct annotated_struct, 
                                                    struct complex_nested_struct); /* TYPE_STRUCT vs TYPE_STRUCT */
    int scalar_ptr_check = __builtin_types_compatible_p(int, int*); /* TYPE_SCALAR vs TYPE_POINTER */
    
    /* Compare with incomplete types */
    int undefined_check = __builtin_types_compatible_p(struct undefined_extern_struct*,
                                                       struct forward_declared_struct*);
    
    /* Compare function pointers */
    int callback_check = __builtin_types_compatible_p(simple_callback_t,
                                                      complex_callback_t);
    
    /* Force use of results to prevent dead code elimination */
    volatile int dummy __attribute__((unused)) = scalar_check + ptr_check + struct_check + 
                                                 scalar_ptr_check + undefined_check + callback_check;
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of our diverse types */
    struct annotated_struct my_struct __attribute__((unused)) = {1, 3.14f, "test"};
    union annotated_union my_union __attribute__((unused));
    user_defined_struct_t user_struct __attribute__((unused));
    struct complex_nested_struct complex_struct __attribute__((unused));
    
    /* Initialize function pointers */
    simple_callback_t my_callback __attribute__((unused)) = 0;
    annotated_func_ptr __attribute__((unused)) = 0;
    
    /* Use VLAs */
    use_vla(5);
    
    /* Perform type comparisons */
    compare_types();
    
    /* Use sizeof with various types (including incomplete) */
    size_t sizes[10] __attribute__((unused));
    sizes[0] = sizeof(_Bool);                     /* TYPE_SCALAR */
    sizes[1] = sizeof(char*);                     /* TYPE_POINTER */
    sizes[2] = sizeof(struct annotated_struct);   /* TYPE_STRUCT */
    sizes[3] = sizeof(union annotated_union);     /* TYPE_UNION */
    sizes[4] = sizeof(fixed_array);               /* TYPE_ARRAY */
    sizes[5] = sizeof(simple_callback_t);         /* TYPE_CALLBACK */
    /* sizeof on incomplete type pointer is valid */
    sizes[6] = sizeof(struct undefined_extern_struct*);
    
    /* Use the types in non-trivial ways */
    my_union.as_int = 42;
    complex_struct.union_ptr = &my_union;
    
    /* Call via function pointer if non-null */
    if (my_callback) {
        int result = my_callback();
        (void)result;
    }
    
    /* Array operations */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * i;
    }
    
    /* Pointer arithmetic */
    int* ptr = fixed_array;
    ptr += 5;
    *ptr = 99;
    
    return 0;
}

/* Additional incomplete type declarations */
struct forward_declared_struct {
    /* This completes the forward declaration, creating TYPE_LANG_STRUCT potential */
    int data;
};

/* Global variable using forward-declared struct */
struct forward_declared_struct global_lang_struct __attribute__((unused));

/* Complex typedef combining multiple type classifications */
typedef union {
    struct annotated_struct as_struct;
    struct complex_nested_struct* as_complex_ptr;
    void (*as_callback)(int);
} mega_union_t __attribute__((annotate("gengtype")));

/* Static variable with complex type */
static mega_union_t static_mega_union __attribute__((unused));

/* Const volatile qualified function pointer */
int (* const volatile volatile_const_func_ptr)(void) __attribute__((unused));
