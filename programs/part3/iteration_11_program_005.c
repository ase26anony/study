/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* Forward declarations for incomplete types */
struct incomplete_struct;
union incomplete_union;
extern struct incomplete_struct extern_struct;
extern int extern_array[];

/* TYPE_UNDEFINED triggers */
struct undefined_type* undefined_ptr;
extern struct undefined_type extern_undefined;

/* TYPE_SCALAR declarations */
__attribute__((unused)) _Bool bool_scalar;
__attribute__((unused)) int int_scalar;
__attribute__((unused)) float float_scalar;
__attribute__((unused)) volatile const long volatile_const_scalar;

/* TYPE_STRING declarations */
__attribute__((unused)) char* string_literal = "test_string";
__attribute__((unused)) const char* const_string = "const_string";
__attribute__((unused)) volatile char* volatile_string;

/* TYPE_STRUCT with annotation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    float field2;
    char* field3;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int x;
    double y;
} user_struct_t;

/* Another TYPE_USER_STRUCT with complex nesting */
typedef struct nested_container {
    union inner_union {
        int i;
        float f;
        void* p;
    } data;
    int tag;
} nested_container_t;

/* TYPE_UNION with annotation */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
    struct annotated_struct as_struct;
};

/* TYPE_POINTER variations */
__attribute__((unused)) int* int_pointer;
__attribute__((unused)) volatile const int* const volatile_int_ptr;
__attribute__((unused)) struct annotated_struct* struct_pointer;
__attribute__((unused)) union annotated_union* union_pointer;
__attribute__((unused)) void (*function_pointer)(void);
__attribute__((unused)) char** pointer_to_pointer;

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float multi_dim_array[5][3];
__attribute__((unused)) volatile char volatile_array[8];
__attribute__((unused)) struct annotated_struct struct_array[4];
__attribute__((unused)) int variable_length_array(int n) {
    int vla[n];
    return sizeof(vla);
}

/* TYPE_CALLBACK - function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(void* context, int result);
typedef struct annotated_struct* (*factory_t)(int id);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
void (*annotated_callback)(int, float, char*);

/* Complex nested type combining multiple classifications */
struct __attribute__((annotate("gengtype"))) complex_type {
    /* TYPE_ARRAY of TYPE_CALLBACK */
    binary_op_t operations[5];
    
    /* TYPE_UNION */
    union {
        int int_val;
        /* TYPE_POINTER to TYPE_STRUCT */
        struct annotated_struct* struct_ptr;
        /* TYPE_POINTER to TYPE_ARRAY */
        int (*array_ptr)[10];
    } data_union;
    
    /* TYPE_POINTER to TYPE_UNION */
    union annotated_union* union_ptr;
    
    /* TYPE_STRING */
    char* name;
    
    /* TYPE_SCALAR */
    volatile const int flags;
    
    /* TYPE_ARRAY of TYPE_POINTER */
    void* pointer_array[8];
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare various type combinations */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    int struct_vs_union = __builtin_types_compatible_p(struct annotated_struct, 
                                                      union annotated_union);
    int ptr_vs_array = __builtin_types_compatible_p(int*, int[10]);
    int callback_vs_ptr = __builtin_types_compatible_p(binary_op_t, void*);
    int const_vs_nonconst = __builtin_types_compatible_p(const int, int);
    int volatile_vs_nonvolatile = __builtin_types_compatible_p(volatile int, int);
    
    /* Use results to avoid dead code elimination */
    if (scalar_vs_pointer || struct_vs_union || ptr_vs_array || 
        callback_vs_ptr || const_vs_nonconst || volatile_vs_nonvolatile) {
        /* This should never happen with proper types, but keeps the code alive */
        asm volatile("" : : "r"(scalar_vs_pointer));
    }
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of our complex types */
    struct complex_type ct = {0};
    user_struct_t us = {0};
    nested_container_t nc = {0};
    
    /* Initialize some values to avoid dead code */
    ct.name = "complex_type_instance";
    ct.flags = 42;
    ct.operations[0] = 0; /* NULL function pointer */
    
    us.x = 10;
    us.y = 3.14159;
    
    nc.data.i = 100;
    nc.tag = 1;
    
    /* Use function pointers */
    binary_op_t op_ptr = 0;
    callback_t cb_ptr = 0;
    
    /* Use arrays */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i;
        sum += fixed_array[i];
    }
    
    /* Use pointers */
    int* dynamic_ptr = fixed_array;
    struct annotated_struct* dyn_struct = &ct.data_union.struct_ptr ? ct.data_union.struct_ptr : 0;
    
    /* Use sizeof with various types (including incomplete) */
    size_t sizes[] = {
        sizeof(bool_scalar),
        sizeof(string_literal),
        sizeof(struct annotated_struct),
        sizeof(union annotated_union),
        sizeof(int_pointer),
        sizeof(fixed_array),
        sizeof(binary_op_t),
        sizeof(struct incomplete_struct*) /* Pointer to incomplete type */
    };
    
    /* Call type comparison function */
    compare_types();
    
    /* Use variable length array */
    int vla_size = variable_length_array(20);
    
    /* Return something based on our operations */
    return (sum + us.x + nc.data.i + vla_size) % 256;
}

/* Additional incomplete type declarations */
struct incomplete_struct {
    /* Never fully defined in this translation unit */
};

/* TYPE_LANG_STRUCT simulation - GCC internal types might appear through attributes */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
};

/* More volatile/const qualified pointers */
volatile const int* const volatile_const_ptr = 0;
const volatile float* volatile const_volatile_ptr = 0;

/* Array of function pointers */
static int add(int a, int b) { return a + b; }
static int subtract(int a, int b) { return a - b; }
binary_op_t operation_table[] = {add, subtract, 0};

/* Complex typedef with multiple indirections */
typedef int (*(*complex_callback_t)(void))[10];
