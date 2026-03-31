/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct incomplete_extern_struct;  /* TYPE_UNDEFINED */
extern int incomplete_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;          /* Forward declaration */

/* TYPE_SCALAR declarations with qualifiers */
__attribute__((unused)) volatile const _Bool volatile_bool = 0;
__attribute__((unused)) const int const_int = 42;
__attribute__((unused)) volatile float volatile_float = 3.14f;

/* TYPE_STRING declarations */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";

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
    char data[64];
} user_struct_t;

/* TYPE_POINTER variations with complex qualifiers */
__attribute__((unused)) volatile const int* const volatile_const_ptr = &const_int;
__attribute__((unused)) int* restrict restricted_ptr;
__attribute__((unused)) const struct annotated_struct* struct_ptr;
__attribute__((unused)) volatile union annotated_union* volatile_union_ptr;

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];                     /* Fixed-size */
__attribute__((unused)) int variable_array[] = {1, 2, 3, 4};     /* Variable-length */
__attribute__((unused)) const char* string_array[] = {"a", "b", "c"};
__attribute__((unused)) struct annotated_struct struct_array[5];

/* TYPE_CALLBACK - Function pointer types */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union annotated_union*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
int (*annotated_func_ptr)(const char*, ...);

/* Nested complex type combining multiple classifications */
struct __attribute__((annotate("gengtype"))) complex_container {
    /* TYPE_STRUCT field */
    struct annotated_struct nested_struct;
    
    /* TYPE_UNION field */
    union annotated_union nested_union;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_CALLBACK */
    simple_callback_t callback_array[4];
    
    /* TYPE_POINTER to TYPE_ARRAY */
    int (*pointer_to_array)[10];
    
    /* TYPE_USER_STRUCT */
    user_struct_t user_data;
    
    /* TYPE_STRING */
    const char* description;
    
    /* TYPE_SCALAR with qualifiers */
    volatile const int volatile_counter;
};

/* Another complex nested type */
union __attribute__((annotate("gengtype"))) mega_union {
    /* TYPE_POINTER to TYPE_STRUCT */
    struct complex_container* container_ptr;
    
    /* TYPE_ARRAY of TYPE_UNION */
    union annotated_union union_array[8];
    
    /* TYPE_CALLBACK */
    complex_callback_t action_callback;
    
    /* TYPE_POINTER to TYPE_POINTER (double pointer) */
    void** void_double_ptr;
};

/* Global instances */
__attribute__((unused)) struct complex_container global_container;
__attribute__((unused)) union mega_union global_mega_union;
__attribute__((unused)) user_struct_t global_user_struct;

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare scalar types */
    int is_int_compatible = __builtin_types_compatible_p(int, const int);
    int is_float_int_incompatible = __builtin_types_compatible_p(float, int);
    
    /* Compare pointer types */
    int is_ptr_compatible = __builtin_types_compatible_p(int*, const int*);
    int is_struct_ptr_compatible = __builtin_types_compatible_p(
        struct annotated_struct*, 
        const struct annotated_struct*
    );
    
    /* Compare struct vs union */
    int is_struct_union_incompatible = __builtin_types_compatible_p(
        struct annotated_struct,
        union annotated_union
    );
    
    /* Compare array types */
    int is_array_compatible = __builtin_types_compatible_p(int[10], int[]);
    
    /* Compare function pointer types */
    int is_callback_compatible = __builtin_types_compatible_p(
        simple_callback_t,
        int (*)(int, float)
    );
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = is_int_compatible + is_float_int_incompatible +
                        is_ptr_compatible + is_struct_ptr_compatible +
                        is_struct_union_incompatible + is_array_compatible +
                        is_callback_compatible;
    (void)dummy;
}

/* Function using incomplete types */
static void use_incomplete_types(void) {
    /* Pointer to incomplete type is valid */
    struct forward_declared_struct* incomplete_ptr = 0;
    extern struct incomplete_extern_struct* extern_incomplete_ptr;
    
    /* sizeof with incomplete type (in pointer context) */
    size_t ptr_size = sizeof(struct forward_declared_struct*);
    size_t extern_ptr_size = sizeof(struct incomplete_extern_struct*);
    
    (void)incomplete_ptr;
    (void)extern_incomplete_ptr;
    (void)ptr_size;
    (void)extern_ptr_size;
}

/* Callback function implementations */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void complex_callback_impl(struct annotated_struct* s, union annotated_union* u) {
    if (s) s->x = 42;
    if (u) u->as_int = 100;
}

int main(void) {
    /* Initialize some variables to prevent dead code elimination */
    global_container.nested_struct.x = 1;
    global_container.nested_struct.y = 2.0f;
    global_container.nested_struct.name = "Nested";
    global_container.description = "Complex container";
    global_container.volatile_counter = 99;
    
    global_user_struct.id = 123;
    
    /* Use function pointers */
    simple_callback_t my_callback = sample_callback;
    complex_callback_t my_complex_callback = complex_callback_impl;
    
    /* Call through function pointer */
    int result = my_callback(10, 20.5f);
    
    /* Use the complex callback */
    my_complex_callback(&global_container.nested_struct, 
                       &global_container.nested_union);
    
    /* Use arrays */
    for (int i = 0; i < 4; i++) {
        global_container.callback_array[i] = sample_callback;
    }
    
    /* Use pointer to array */
    int local_array[10];
    global_container.pointer_to_array = &local_array;
    
    /* Perform type comparisons */
    compare_types();
    
    /* Use incomplete types */
    use_incomplete_types();
    
    /* Use sizeof with various types */
    size_t sizes[] = {
        sizeof(_Bool),
        sizeof(int),
        sizeof(float),
        sizeof(char*),
        sizeof(struct annotated_struct),
        sizeof(union annotated_union),
        sizeof(user_struct_t),
        sizeof(int*),
        sizeof(int[10]),
        sizeof(simple_callback_t),
        sizeof(struct complex_container),
        sizeof(union mega_union)
    };
    
    /* Use volatile pointer */
    *volatile_const_ptr = 0;  /* Through const pointer */
    
    return result > 0 ? 0 : 1;
}

/* Define previously forward-declared struct */
struct forward_declared_struct {
    int defined_now;
    void* some_pointer;
};
