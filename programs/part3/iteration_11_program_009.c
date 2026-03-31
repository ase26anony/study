/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct undefined_extern;  /* TYPE_UNDEFINED candidate */
extern int extern_array[];       /* Incomplete array type */

/* Forward declared struct/union for TYPE_LANG_STRUCT */
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_SCALAR declarations with qualifiers */
volatile const int volatile_const_scalar = 42;
static _Bool static_bool_scalar = 1;
float float_scalar __attribute__((unused)) = 3.14f;
long double long_double_scalar;

/* TYPE_STRING declarations */
char* string_literal = "Hello, gengtype!";
const char* const const_string_literal = "Constant string";
volatile char* volatile_string_ptr;

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int x;
    float y;
    char* str;
};

/* TYPE_UNION with nested complexity */
union complex_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_pointer;
    struct {
        char a;
        char b;
    } nested_struct;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int id;
    char name[32];
    union complex_union data;
} user_struct_t;

/* TYPE_POINTER variations */
int* int_pointer;
const float* const const_float_pointer;
volatile char* volatile_char_pointer;
void* void_pointer;
struct annotated_struct* struct_pointer;
user_struct_t* user_struct_pointer;
void (*function_pointer)(void);  /* Also TYPE_CALLBACK */

/* TYPE_ARRAY variations */
int fixed_array[10];
int variable_array[] = {1, 2, 3, 4, 5};
char* pointer_array[5];
const int const_array[3] = {10, 20, 30};
volatile float volatile_array[2];

/* Complex nested array types */
struct annotated_struct* struct_pointer_array[8];
union complex_union union_array[4];

/* TYPE_CALLBACK - function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* context, int result);
typedef struct annotated_struct* (*struct_factory_t)(int);

/* Complex nested struct with multiple type classifications */
struct master_container __attribute__((annotate("gengtype"))) {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_POINTER */
    void* payload;
    
    /* TYPE_ARRAY */
    binary_op_t operations[5];  /* Array of function pointers */
    
    /* TYPE_UNION */
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    } variant;
    
    /* TYPE_STRUCT nested */
    struct {
        char tag;
        union complex_union data;
    } tagged_data;
    
    /* TYPE_CALLBACK */
    callback_t completion_callback;
    
    /* TYPE_POINTER to TYPE_ARRAY */
    int* dynamic_array;
    
    /* TYPE_USER_STRUCT */
    user_struct_t user_data;
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void perform_type_comparisons(void) {
    /* Compare various type pairs to trigger classification */
    int is_same1 = __builtin_types_compatible_p(int, float);  /* SCALAR vs SCALAR */
    int is_same2 = __builtin_types_compatible_p(int*, float*); /* POINTER vs POINTER */
    int is_same3 = __builtin_types_compatible_p(struct annotated_struct*, 
                                               union complex_union*); /* STRUCT vs UNION */
    int is_same4 = __builtin_types_compatible_p(int[10], int*); /* ARRAY vs POINTER */
    int is_same5 = __builtin_types_compatible_p(binary_op_t, void*); /* CALLBACK vs POINTER */
    
    /* Use results to prevent dead code elimination */
    volatile int result = is_same1 + is_same2 + is_same3 + is_same4 + is_same5;
    (void)result;
}

/* Function pointer callback implementations */
static int add(int a, int b) { return a + b; }
static int multiply(int a, int b) { return a * b; }
static void dummy_callback(void* context, int result) { (void)context; (void)result; }

int main(void) {
    /* Declare and initialize variables with diverse types */
    
    /* TYPE_SCALAR */
    volatile int local_scalar = 100;
    const _Bool local_bool = 0;
    
    /* TYPE_STRING */
    char local_string[] = "Local string";
    const char* local_string_ptr = "Pointer to string";
    
    /* TYPE_STRUCT */
    struct annotated_struct local_struct = {1, 2.0f, "struct string"};
    
    /* TYPE_UNION */
    union complex_union local_union;
    local_union.as_int = 255;
    
    /* TYPE_USER_STRUCT */
    user_struct_t local_user_struct = {
        .id = 1,
        .name = "Test",
        .data = {.as_float = 3.14f}
    };
    
    /* TYPE_POINTER with complex qualifiers */
    volatile const int* const volatile_qualified_ptr = &volatile_const_scalar;
    const struct annotated_struct* const const_struct_ptr = &local_struct;
    
    /* TYPE_ARRAY */
    int local_array[5] = {1, 2, 3, 4, 5};
    binary_op_t local_func_array[2] = {add, multiply};
    
    /* TYPE_CALLBACK */
    callback_t local_callback = dummy_callback;
    struct_factory_t factory_ptr = NULL;
    
    /* Complex nested type */
    struct master_container container = {
        .counter = 0,
        .payload = &local_user_struct,
        .operations = {add, multiply, add, multiply, add},
        .variant = {.int_val = 42},
        .tagged_data = {'A', {.as_int = 100}},
        .completion_callback = dummy_callback,
        .dynamic_array = local_array,
        .user_data = local_user_struct
    };
    
    /* Use incomplete types in valid contexts */
    size_t forward_size = sizeof(struct forward_declared_struct*);  /* Pointer to incomplete */
    size_t extern_size = sizeof(extern_array[0]);  /* Element of incomplete array */
    
    /* Trigger type comparisons */
    perform_type_comparisons();
    
    /* Use variables to prevent dead code elimination */
    local_scalar += container.counter;
    local_callback(&container, local_scalar);
    
    /* Use function pointers */
    int sum = local_func_array[0](10, 20);
    int product = local_func_array[1](10, 20);
    
    /* Use __builtin_types_compatible_p with local types */
    int is_compatible = __builtin_types_compatible_p(
        typeof(local_struct), 
        typeof(container.user_data)
    );
    
    /* Return computed value to ensure all code paths matter */
    return (sum + product + is_compatible + forward_size + extern_size) == 0 ? 0 : 0;
}

/* Additional external declarations for incomplete types */
struct undefined_extern {
    int dummy;  /* Definition elsewhere */
};

/* GCC-specific extension for TYPE_LANG_STRUCT simulation */
struct gcc_internal_struct __attribute__((annotate("gengtype"))) {
    int gcc_specific_field;
};

/* Complex function pointer type definition */
typedef int (*(*complex_callback_t)(int))(void*, float);
