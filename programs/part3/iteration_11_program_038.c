/* gengtype_trigger.c
 * A program designed to exercise GCC's internal type classification system
 * to trigger coverage of specific switch cases in gengtype.cc
 */

/* ========== TYPE_UNDEFINED and TYPE_LANG_STRUCT ========== */
/* Forward declarations and incomplete types */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED */
union forward_declared_union;           /* TYPE_UNDEFINED */

/* GCC language-specific struct (TYPE_LANG_STRUCT) */
struct __attribute__((annotate("gengtype"))) lang_struct {
    int lang_field;
};

/* ========== TYPE_SCALAR ========== */
/* Basic scalar types */
__attribute__((unused)) _Bool bool_scalar;               /* TYPE_SCALAR */
__attribute__((unused)) int int_scalar;                  /* TYPE_SCALAR */
__attribute__((unused)) float float_scalar;              /* TYPE_SCALAR */
__attribute__((unused)) volatile const long volatile_const_scalar;  /* TYPE_SCALAR */

/* ========== TYPE_STRING ========== */
/* String literals and pointers */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";  /* TYPE_STRING */
__attribute__((unused)) char* mutable_string;            /* TYPE_STRING */
__attribute__((unused)) volatile const char* volatile_const_string; /* TYPE_STRING */

/* ========== TYPE_STRUCT ========== */
/* Named struct types */
struct __attribute__((annotate("gengtype"))) named_struct {
    int field1;
    float field2;
    const char* field3;
};  /* TYPE_STRUCT */

/* ========== TYPE_USER_STRUCT ========== */
/* Typedef struct (TYPE_USER_STRUCT) */
typedef struct __attribute__((annotate("gengtype"))) {
    int data;
    void* ptr;
} user_struct_t;  /* TYPE_USER_STRUCT */

/* Another typedef struct with qualifiers */
typedef volatile const struct {
    int counter;
    float value;
} volatile_user_struct_t;  /* TYPE_USER_STRUCT */

/* ========== TYPE_UNION ========== */
/* Named union type */
union __attribute__((annotate("gengtype"))) named_union {
    int as_int;
    float as_float;
    void* as_ptr;
};  /* TYPE_UNION */

/* Typedef union */
typedef union {
    long long_data;
    double double_data;
} user_union_t;  /* TYPE_UNION */

/* ========== TYPE_POINTER ========== */
/* Pointers to various types */
__attribute__((unused)) int* int_ptr;                    /* TYPE_POINTER */
__attribute__((unused)) float* float_ptr;                /* TYPE_POINTER */
__attribute__((unused)) struct named_struct* struct_ptr; /* TYPE_POINTER */
__attribute__((unused)) union named_union* union_ptr;    /* TYPE_POINTER */
__attribute__((unused)) user_struct_t* user_struct_ptr;  /* TYPE_POINTER */

/* Complex pointer with qualifiers */
__attribute__((unused)) volatile const int* const volatile_const_ptr;  /* TYPE_POINTER */

/* Pointer to pointer */
__attribute__((unused)) int** ptr_to_ptr;                /* TYPE_POINTER */

/* Pointer to incomplete type */
__attribute__((unused)) struct forward_declared_struct* incomplete_ptr;  /* TYPE_POINTER */

/* ========== TYPE_ARRAY ========== */
/* Fixed-size arrays */
__attribute__((unused)) int fixed_array[10];             /* TYPE_ARRAY */
__attribute__((unused)) float float_array[5][3];         /* TYPE_ARRAY (multi-dimensional) */
__attribute__((unused)) struct named_struct struct_array[4];  /* TYPE_ARRAY */

/* Array with qualifiers */
__attribute__((unused)) const volatile int qualified_array[8];  /* TYPE_ARRAY */

/* Variable-length array (in function scope) */
__attribute__((unused)) void use_vla(int size) {
    int vla[size];  /* TYPE_ARRAY */
    (void)vla;
}

/* ========== TYPE_CALLBACK ========== */
/* Function pointer types */
typedef int (*simple_callback_t)(int, float);  /* TYPE_CALLBACK */

/* Complex function pointer with attributes */
typedef void (*__attribute__((annotate("gengtype"))) complex_callback_t)(
    struct named_struct*, 
    user_union_t, 
    const char*
);  /* TYPE_CALLBACK */

/* Function pointer returning pointer */
typedef volatile_user_struct_t* (*ptr_returning_callback_t)(int);  /* TYPE_CALLBACK */

/* ========== COMPLEX NESTED TYPES ========== */
/* Struct containing array of function pointers */
struct __attribute__((annotate("gengtype"))) struct_with_callbacks {
    simple_callback_t callbacks[5];
    complex_callback_t complex_cb;
    int data;
};  /* TYPE_STRUCT with nested TYPE_ARRAY of TYPE_CALLBACK */

/* Union with pointer to struct */
union __attribute__((annotate("gengtype"))) union_with_pointer {
    struct named_struct* struct_ptr;
    user_struct_t* user_struct_ptr;
    int* int_ptr;
};  /* TYPE_UNION with TYPE_POINTER fields */

/* Typedef for complex nested type */
typedef struct {
    union union_with_pointer u;
    struct struct_with_callbacks s;
    ptr_returning_callback_t getter;
} nested_complex_t;  /* TYPE_USER_STRUCT */

/* ========== TYPE COMPARISONS ========== */
/* Use __builtin_types_compatible_p for type comparisons */
__attribute__((unused)) static void type_comparisons(void) {
    /* Compare scalar vs pointer */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    int struct_vs_union = __builtin_types_compatible_p(struct named_struct, union named_union);
    
    /* Compare pointer types */
    int ptr_vs_ptr = __builtin_types_compatible_p(int*, float*);
    
    /* Compare array types */
    int array_vs_array = __builtin_types_compatible_p(int[10], int[5]);
    
    /* Compare function pointers */
    int callback_vs_callback = __builtin_types_compatible_p(
        simple_callback_t, 
        complex_callback_t
    );
    
    /* Compare with qualifiers */
    int qualified_vs_unqualified = __builtin_types_compatible_p(
        const int, 
        volatile int
    );
    
    /* Compare typedef vs original */
    int typedef_vs_struct = __builtin_types_compatible_p(
        user_struct_t, 
        struct { int data; void* ptr; }
    );
    
    (void)scalar_vs_pointer;
    (void)struct_vs_union;
    (void)ptr_vs_ptr;
    (void)array_vs_array;
    (void)callback_vs_callback;
    (void)qualified_vs_unqualified;
    (void)typedef_vs_struct;
}

/* ========== VARIABLE DECLARATIONS ========== */
/* Declare variables of complex types */
__attribute__((unused)) struct struct_with_callbacks callback_struct;
__attribute__((unused)) union union_with_pointer pointer_union;
__attribute__((unused)) nested_complex_t nested_var;
__attribute__((unused)) user_struct_t user_var;
__attribute__((unused)) user_union_t union_var;

/* Function pointer variables */
__attribute__((unused)) simple_callback_t simple_cb;
__attribute__((unused)) complex_callback_t complex_cb;
__attribute__((unused)) ptr_returning_callback_t ptr_cb;

/* Arrays of complex types */
__attribute__((unused)) nested_complex_t nested_array[3];
__attribute__((unused)) simple_callback_t callback_array[4];

/* ========== HELPER FUNCTIONS ========== */
/* Functions to use function pointers */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static volatile_user_struct_t* sample_ptr_callback(int x) {
    static volatile_user_struct_t instance = {0};
    instance.counter = x;
    return &instance;
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Initialize some variables to avoid dead code elimination */
    int_scalar = 42;
    float_scalar = 3.14159f;
    
    /* Use incomplete types in sizeof (valid for pointers) */
    size_t incomplete_size = sizeof(struct forward_declared_struct*);
    size_t extern_size = sizeof(undefined_extern_array);
    (void)incomplete_size;
    (void)extern_size;
    
    /* Initialize struct */
    callback_struct.data = 100;
    callback_struct.complex_cb = 0;  /* NULL */
    
    /* Initialize union */
    pointer_union.struct_ptr = &callback_struct;
    
    /* Use function pointers */
    simple_cb = sample_callback;
    if (simple_cb) {
        int result = simple_cb(10, 2.5f);
        (void)result;
    }
    
    ptr_cb = sample_ptr_callback;
    if (ptr_cb) {
        volatile_user_struct_t* instance = ptr_cb(99);
        (void)instance;
    }
    
    /* Use type comparisons */
    type_comparisons();
    
    /* Use variable-length array */
    use_vla(7);
    
    /* Access array elements */
    if (sizeof(nested_array) > 0) {
        nested_array[0].getter = ptr_cb;
    }
    
    /* Use string */
    if (string_literal[0] != '\0') {
        mutable_string = (char*)string_literal;
    }
    
    /* Return success */
    return 0;
}
