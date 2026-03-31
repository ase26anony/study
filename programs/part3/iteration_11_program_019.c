/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct incomplete_struct;
extern union incomplete_union;
extern int incomplete_array[];

/* TYPE_SCALAR declarations with various qualifiers */
__attribute__((unused)) volatile const _Bool volatile_bool = 0;
__attribute__((unused)) const int const_int = 42;
__attribute__((unused)) volatile float volatile_float = 3.14f;

/* TYPE_STRING declarations */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";
__attribute__((unused)) const char* const const_string_ptr = "Constant pointer";

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
    struct annotated_struct nested;
    union annotated_union variant;
} user_struct_t;

/* TYPE_POINTER variations */
__attribute__((unused)) volatile const int* volatile const volatile_const_ptr;
__attribute__((unused)) user_struct_t* struct_ptr;
__attribute__((unused)) void (*function_ptr)(void);
__attribute__((unused)) char** pointer_to_pointer;

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float matrix[3][3];
__attribute__((unused)) char* string_array[] = {"one", "two", "three"};
__attribute__((unused)) struct annotated_struct struct_array[5];

/* TYPE_CALLBACK - function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* context, int result);
typedef char* (*string_transform_t)(const char*);

/* Complex nested type combining multiple classifications */
struct __attribute__((annotate("gengtype"))) complex_container {
    /* TYPE_STRUCT containing TYPE_ARRAY of TYPE_CALLBACK */
    binary_op_t operations[4];
    
    /* TYPE_UNION field */
    union {
        int tag;
        void* data;
    } discriminator;
    
    /* TYPE_POINTER to TYPE_USER_STRUCT */
    user_struct_t* user_data;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRING */
    const char* messages[8];
};

/* Function using TYPE_CALLBACK */
static int add(int a, int b) { return a + b; }
static int multiply(int a, int b) { return a * b; }

/* Volatile function pointer */
__attribute__((unused)) static volatile binary_op_t volatile_op = add;

/* Const-qualified complex pointer */
__attribute__((unused)) const struct complex_container* const const_complex_ptr = 0;

/* Use __builtin_types_compatible_p for type comparisons */
static void perform_type_comparisons(void) {
    /* Compare various type combinations */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    int struct_vs_union = __builtin_types_compatible_p(struct annotated_struct, 
                                                      union annotated_union);
    int ptr_vs_array = __builtin_types_compatible_p(int*, int[]);
    int callback_vs_ptr = __builtin_types_compatible_p(binary_op_t, void*);
    int const_vs_nonconst = __builtin_types_compatible_p(const int, int);
    int volatile_vs_nonvolatile = __builtin_types_compatible_p(volatile int, int);
    
    /* Use results to prevent dead code elimination */
    __attribute__((unused)) int comparisons = scalar_vs_pointer + struct_vs_union +
                                            ptr_vs_array + callback_vs_ptr +
                                            const_vs_nonconst + volatile_vs_nonvolatile;
}

/* Main function with diverse type usage */
int main(void) {
    /* TYPE_USER_STRUCT variable */
    user_struct_t user_var = {
        .id = 1,
        .nested = { .x = 10, .y = 2.5f, .name = "nested" },
        .variant = { .as_int = 100 }
    };
    
    /* TYPE_STRUCT variable */
    struct annotated_struct annotated_var = { .x = 5, .y = 1.0f, .name = "test" };
    
    /* TYPE_UNION variable */
    union annotated_union union_var;
    union_var.as_float = 3.14159f;
    
    /* TYPE_ARRAY initialization */
    int local_array[] = {1, 2, 3, 4, 5};
    
    /* TYPE_POINTER operations */
    int* int_ptr = local_array;
    struct complex_container* container_ptr = 0;
    
    /* TYPE_CALLBACK usage */
    binary_op_t ops[] = {add, multiply};
    int result = ops[0](2, 3);  /* Should be 5 */
    
    /* Use incomplete types through pointers (TYPE_UNDEFINED) */
    struct incomplete_struct* inc_struct_ptr = 0;
    union incomplete_union* inc_union_ptr = 0;
    
    /* sizeof with various types */
    __attribute__((unused)) size_t sizes[] = {
        sizeof(_Bool),
        sizeof(user_struct_t),
        sizeof(struct annotated_struct),
        sizeof(union annotated_union),
        sizeof(binary_op_t),
        sizeof(local_array),
        sizeof(int_ptr),
        /* sizeof(incomplete_array) would be incomplete type */
    };
    
    /* Trigger type comparisons */
    perform_type_comparisons();
    
    /* Use volatile/const qualified types */
    volatile const int read_only = result;
    volatile int* volatile volatile_int_ptr = &user_var.id;
    
    /* Complex nested type usage */
    struct complex_container container = {
        .operations = {add, multiply, add, multiply},
        .discriminator = { .tag = 1 },
        .user_data = &user_var,
        .messages = {"msg1", "msg2", "msg3"}
    };
    
    /* Use function pointer from struct */
    if (container.operations[0]) {
        result = container.operations[0](result, 2);
    }
    
    /* Ensure all variables are used to prevent elimination */
    __attribute__((unused)) int dummy = 
        user_var.id + annotated_var.x + (int)union_var.as_float + 
        *int_ptr + (int)(inc_struct_ptr != 0) + (int)(inc_union_ptr != 0) +
        read_only + *volatile_int_ptr + container.discriminator.tag;
    
    return 0;
}

/* Additional external declarations for incomplete types */
extern struct incomplete_struct {
    /* Never defined - remains incomplete */
};

extern union incomplete_union {
    /* Never defined - remains incomplete */
};

/* Function using callback type */
static void example_callback(void* context, int result) {
    __attribute__((unused)) char** ctx = (char**)context;
    __attribute__((unused)) int res = result;
}

/* Array of callbacks */
__attribute__((unused)) static callback_t callbacks[] = {
    example_callback,
    0
};
