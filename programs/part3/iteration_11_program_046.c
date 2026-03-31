/* gengtype_coverage.c
 * This program declares diverse types to trigger GCC's internal type
 * classification in gengtype.cc, specifically targeting the switch cases
 * for TYPE_UNDEFINED through TYPE_LANG_STRUCT.
 */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */
/* Forward declarations create incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern union undefined_union;             /* TYPE_UNDEFINED */
extern int undefined_array[];             /* TYPE_UNDEFINED (incomplete array) */

/* GCC language-specific struct (TYPE_LANG_STRUCT) */
struct __attribute__((annotate("gengtype"))) lang_struct {
    int lang_field;
};

/* ==================== TYPE_SCALAR ==================== */
static __attribute__((unused)) _Bool scalar_bool = 0;      /* TYPE_SCALAR */
static __attribute__((unused)) int scalar_int = 42;        /* TYPE_SCALAR */
static __attribute__((unused)) float scalar_float = 3.14f; /* TYPE_SCALAR */
static __attribute__((unused)) volatile const int volatile_const_scalar = 100;

/* ==================== TYPE_STRING ==================== */
static __attribute__((unused)) const char* string_literal = "Hello, gengtype!"; /* TYPE_STRING */
static __attribute__((unused)) char* mutable_string = "Mutable";               /* TYPE_STRING */
static __attribute__((unused)) volatile const char* volatile_const_string = "VolatileConst";

/* ==================== TYPE_STRUCT & TYPE_USER_STRUCT ==================== */
/* Basic struct (TYPE_STRUCT) */
struct __attribute__((annotate("gengtype"))) basic_struct {
    int x;
    float y;
    const char* name;
};

/* Typedef creates TYPE_USER_STRUCT */
typedef struct basic_struct user_struct_t;  /* TYPE_USER_STRUCT */

/* Complex nested struct with multiple type classifications */
struct __attribute__((annotate("gengtype"))) complex_nested {
    /* TYPE_SCALAR fields */
    _Bool flag;
    int counter;
    
    /* TYPE_POINTER field */
    volatile const int* volatile_ptr;
    
    /* TYPE_ARRAY field */
    float data[10];
    
    /* TYPE_UNION field nested inside */
    union {
        int as_int;
        float as_float;
    } value_union;
    
    /* TYPE_CALLBACK field */
    void (*callback)(int, float);
    
    /* TYPE_POINTER to incomplete type */
    struct undefined_struct* undefined_ptr;
};

/* ==================== TYPE_UNION ==================== */
union __attribute__((annotate("gengtype"))) basic_union {
    int int_val;
    float float_val;
    double double_val;
    void* ptr_val;
};

/* Union containing struct pointer */
union nested_union {
    struct basic_struct* struct_ptr;
    union basic_union* union_ptr;
    int (*func_ptr)(void);
};

/* ==================== TYPE_POINTER ==================== */
static __attribute__((unused)) int* int_ptr = &scalar_int;                    /* TYPE_POINTER */
static __attribute__((unused)) const float* const_float_ptr = &scalar_float;  /* TYPE_POINTER */
static __attribute__((unused)) volatile const int* const volatile_const_ptr = &scalar_int;
static __attribute__((unused)) struct basic_struct* struct_ptr = 0;           /* TYPE_POINTER */
static __attribute__((unused)) union basic_union* union_ptr = 0;              /* TYPE_POINTER */
static __attribute__((unused)) void (*func_ptr)(void) = 0;                    /* TYPE_POINTER (to function) */

/* Complex pointer declaration with multiple qualifiers */
static __attribute__((unused)) volatile const int* const restrict volatile_complex_ptr = &scalar_int;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size array */
static __attribute__((unused)) int fixed_array[20];                           /* TYPE_ARRAY */

/* Variable-length array (in function scope) */
static __attribute__((unused)) int* vla_ptr = 0;

/* Array of pointers */
static __attribute__((unused)) void* pointer_array[5];                        /* TYPE_ARRAY of TYPE_POINTER */

/* Array of structs */
static __attribute__((unused)) struct basic_struct struct_array[3];           /* TYPE_ARRAY of TYPE_STRUCT */

/* Multidimensional array */
static __attribute__((unused)) int matrix[4][4];                              /* TYPE_ARRAY of TYPE_ARRAY */

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer typedef (TYPE_CALLBACK) */
typedef int (*binary_func_t)(int, int);  /* TYPE_CALLBACK */

/* Complex callback signature */
typedef void (*complex_callback_t)(
    struct basic_struct*, 
    union basic_union*, 
    int (*nested_callback)(float)
);

/* Struct with callback field */
struct callback_container {
    binary_func_t binary_op;            /* TYPE_CALLBACK */
    complex_callback_t complex_op;      /* TYPE_CALLBACK */
    void (*simple_op)(void);            /* TYPE_CALLBACK */
};

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p to force type comparisons */
#define CHECK_TYPE_COMPAT(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* ==================== FUNCTION DECLARATIONS ==================== */
static int add(int a, int b) { return a + b; }
static void simple_callback(void) { }
static void complex_handler(struct basic_struct* s, union basic_union* u, 
                           int (*cb)(float)) { }

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Force processing of variable-length array type */
    int vla_size = 10;
    int variable_length_array[vla_size];  /* TYPE_ARRAY */
    vla_ptr = variable_length_array;
    
    /* Initialize some variables to prevent dead code elimination */
    scalar_int = 100;
    scalar_float = 2.718f;
    
    /* Use incomplete types in valid contexts */
    size_t undefined_size = sizeof(struct undefined_struct*);  /* Pointer to incomplete type */
    size_t array_size = sizeof(undefined_array);               /* Incomplete array type */
    
    /* Initialize struct with nested union */
    struct complex_nested nested = {
        .flag = 1,
        .counter = 42,
        .volatile_ptr = &scalar_int,
        .data = {1.0f, 2.0f, 3.0f},
        .value_union = {.as_int = 100},
        .callback = 0,
        .undefined_ptr = 0
    };
    
    /* Initialize union */
    union basic_union u = {.int_val = 255};
    
    /* Set up callback functions */
    struct callback_container callbacks = {
        .binary_op = add,
        .complex_op = complex_handler,
        .simple_op = simple_callback
    };
    
    /* Use __builtin_types_compatible_p for type comparisons */
    int compat_results[10];
    
    /* Compare various type combinations to trigger classification */
    compat_results[0] = CHECK_TYPE_COMPAT(int, float);              /* Scalar vs Scalar */
    compat_results[1] = CHECK_TYPE_COMPAT(int*, float*);            /* Pointer vs Pointer */
    compat_results[2] = CHECK_TYPE_COMPAT(struct basic_struct*, 
                                         union basic_union*);       /* Struct ptr vs Union ptr */
    compat_results[3] = CHECK_TYPE_COMPAT(int[10], float[10]);      /* Array vs Array */
    compat_results[4] = CHECK_TYPE_COMPAT(binary_func_t, 
                                         void (*)(void));           /* Callback vs Callback */
    compat_results[5] = CHECK_TYPE_COMPAT(volatile const int*, 
                                         const volatile int*);      /* Qualified pointers */
    compat_results[6] = CHECK_TYPE_COMPAT(struct undefined_struct*,
                                         void*);                    /* Incomplete type ptr */
    compat_results[7] = CHECK_TYPE_COMPAT(user_struct_t,
                                         struct basic_struct);      /* User struct vs Struct */
    
    /* Use the callbacks to ensure they're processed */
    if (callbacks.binary_op) {
        int result = callbacks.binary_op(10, 20);
        scalar_int = result;
    }
    
    if (callbacks.simple_op) {
        callbacks.simple_op();
    }
    
    /* Use the nested struct */
    nested.counter++;
    if (nested.volatile_ptr) {
        scalar_int = *nested.volatile_ptr;
    }
    
    /* Use the union */
    u.float_val = 3.14159f;
    
    /* Return a value based on type compatibility checks */
    return compat_results[0] + compat_results[1] + compat_results[2];
}
