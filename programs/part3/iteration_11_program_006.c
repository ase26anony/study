/* gengtype_test.c - Comprehensive type declarations to exercise GCC's gengtype type classifier */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct incomplete_struct;
extern union incomplete_union;
extern int incomplete_array[];

/* TYPE_UNDEFINED via extern without definition */
extern struct undefined_struct undefined_var;

/* TYPE_SCALAR declarations with qualifiers */
volatile const _Bool volatile_bool __attribute__((unused));
const int const_int __attribute__((unused)) = 42;
volatile float volatile_float __attribute__((unused));

/* TYPE_STRING declarations */
char* string_literal __attribute__((unused)) = "test string";
const char* const const_string __attribute__((unused)) = "const string";

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int field1;
    float field2;
    char* field3;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    long data;
    void* ptr;
} user_struct_t;

/* TYPE_UNION with complex nesting */
union complex_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    struct {
        char byte1;
        char byte2;
    } as_bytes;
    void* as_ptr;
};

/* TYPE_POINTER variations */
int* int_ptr __attribute__((unused));
const volatile char* volatile_const_char_ptr __attribute__((unused));
struct annotated_struct* struct_ptr __attribute__((unused));
user_struct_t* user_struct_ptr __attribute__((unused));
union complex_union* union_ptr __attribute__((unused));

/* TYPE_ARRAY declarations */
int fixed_array[10] __attribute__((unused));
float multi_dim_array[5][3] __attribute__((unused));
char* pointer_array[8] __attribute__((unused));

/* Variable-length array (C99) */
void use_vla(int size) {
    int vla[size] __attribute__((unused));
    /* Use to avoid dead code elimination */
    for (int i = 0; i < size; i++) vla[i] = i;
}

/* TYPE_CALLBACK - function pointers */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union complex_union*);

/* Annotated function pointer */
int (*annotated_callback)(int, char**) __attribute__((annotate("gengtype")));

/* Complex nested type combining multiple classifications */
struct container_struct __attribute__((annotate("gengtype"))) {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_STRING */
    char* name;
    
    /* TYPE_UNION */
    union {
        int option1;
        float option2;
    } choice;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_CALLBACK */
    simple_callback_t callbacks[5];
    
    /* TYPE_POINTER to TYPE_USER_STRUCT */
    user_struct_t* user_data;
    
    /* TYPE_ARRAY within union */
    union {
        int ints[4];
        float floats[4];
    } data_union;
};

/* Another complex type: union containing struct with array of function pointers */
union super_union {
    struct {
        int type;
        /* Array of complex callbacks */
        complex_callback_t handlers[3];
    } handler_struct;
    
    struct {
        void* data;
        size_t size;
    } buffer_info;
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare scalar types */
    int is_int_compatible = __builtin_types_compatible_p(int, const int);
    int is_float_int_compatible = __builtin_types_compatible_p(float, int);
    
    /* Compare pointer types */
    int is_ptr_compatible = __builtin_types_compatible_p(int*, const int*);
    int is_struct_ptr_compatible = __builtin_types_compatible_p(
        struct annotated_struct*, 
        struct incomplete_struct*
    );
    
    /* Compare struct vs union */
    int is_struct_union_compatible = __builtin_types_compatible_p(
        struct container_struct,
        union super_union
    );
    
    /* Compare array types */
    int is_array_compatible = __builtin_types_compatible_p(int[10], int[]);
    int is_multi_array_compatible = __builtin_types_compatible_p(
        float[5][3],
        float[][3]
    );
    
    /* Compare function pointers */
    int is_callback_compatible = __builtin_types_compatible_p(
        simple_callback_t,
        complex_callback_t
    );
    
    /* Use results to avoid dead code elimination */
    volatile int results[] = {
        is_int_compatible,
        is_float_int_compatible,
        is_ptr_compatible,
        is_struct_ptr_compatible,
        is_struct_union_compatible,
        is_array_compatible,
        is_multi_array_compatible,
        is_callback_compatible
    };
    (void)results; /* Suppress unused warning */
}

/* Function pointer usage */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void complex_handler(struct annotated_struct* s, union complex_union* u) {
    if (s) s->field1 = 42;
    if (u) u->as_int = 100;
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of our complex types */
    struct container_struct container __attribute__((unused));
    union super_union super __attribute__((unused));
    user_struct_t user_data __attribute__((unused));
    
    /* Initialize function pointers */
    simple_callback_t cb = sample_callback;
    complex_callback_t complex_cb = complex_handler;
    
    /* Use variable-length array */
    use_vla(20);
    
    /* Perform type comparisons */
    compare_types();
    
    /* Use some variables to avoid dead code elimination */
    container.counter = 100;
    container.name = "Container";
    container.choice.option1 = 42;
    container.callbacks[0] = cb;
    
    super.handler_struct.type = 1;
    super.handler_struct.handlers[0] = complex_cb;
    
    user_data.data = 999;
    user_data.ptr = &container;
    
    /* Use sizeof with incomplete types (valid for pointers) */
    size_t incomplete_size = sizeof(struct incomplete_struct*);
    size_t undefined_size = sizeof(&undefined_var);
    
    /* Call through function pointer */
    int result = cb(10, 3.14f);
    
    /* Use volatile to prevent optimization */
    volatile int vol_result = result;
    volatile size_t vol_size1 = incomplete_size;
    volatile size_t vol_size2 = undefined_size;
    
    (void)vol_result;
    (void)vol_size1;
    (void)vol_size2;
    
    return 0;
}

/* Additional incomplete type declarations */
struct forward_declared_struct;

/* Pointer to incomplete type */
struct forward_declared_struct* forward_ptr __attribute__((unused));

/* Array of pointers to incomplete types */
struct incomplete_struct* incomplete_ptrs[5] __attribute__((unused));

/* Const pointer to volatile incomplete union */
const union incomplete_union* volatile const_vol_incomplete_ptr __attribute__((unused));
