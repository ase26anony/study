/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */
/* Forward declarations creating incomplete/undefined types */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern union undefined_extern_union;    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED */
union forward_declared_union;           /* TYPE_UNDEFINED */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
__attribute__((unused)) _Bool bool_scalar;
__attribute__((unused)) int int_scalar;
__attribute__((unused)) float float_scalar;
__attribute__((unused)) double double_scalar;
__attribute__((unused)) long long long_long_scalar;

/* Qualified scalars */
__attribute__((unused)) volatile const int volatile_const_scalar;
__attribute__((unused)) const float const_float_scalar;
__attribute__((unused)) volatile _Bool volatile_bool_scalar;

/* ==================== TYPE_STRING ==================== */
/* String literals and pointers */
__attribute__((unused)) char* string_ptr = "Hello, gengtype!";
__attribute__((unused)) const char* const_string_ptr = "Constant string";
__attribute__((unused)) volatile char* volatile_string_ptr = "Volatile string";

/* ==================== TYPE_STRUCT & TYPE_UNION ==================== */
/* Annotated struct and union to trigger metadata generation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Complex nested struct */
struct complex_nested {
    struct inner_struct {
        int a;
        double b;
    } inner;
    
    union inner_union {
        char c[4];
        int i;
    } data;
    
    struct complex_nested* self_ptr;  /* Recursive pointer */
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef structs create TYPE_USER_STRUCT */
typedef struct {
    int id;
    char name[32];
} user_struct_t;

typedef struct complex_nested complex_nested_t;

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
__attribute__((unused)) int* int_ptr;
__attribute__((unused)) float* float_ptr;
__attribute__((unused)) char** char_double_ptr;
__attribute__((unused)) const int* const_int_ptr;
__attribute__((unused)) volatile const float* volatile_const_float_ptr;
__attribute__((unused)) struct annotated_struct* struct_ptr;
__attribute__((unused)) union annotated_union* union_ptr;
__attribute__((unused)) user_struct_t* user_struct_ptr;

/* Complex pointer declaration */
__attribute__((unused)) volatile const int* const restrict volatile_complex_ptr;

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float matrix[3][3];
__attribute__((unused)) char string_array[5][20];

/* Array of pointers */
__attribute__((unused)) int* pointer_array[8];

/* Array of structs */
__attribute__((unused)) struct annotated_struct struct_array[4];

/* Variable-length array (in function context) */
void use_vla(int size) {
    __attribute__((unused)) int vla[size];
    /* Use VLA to prevent optimization */
    for (int i = 0; i < size; i++) {
        vla[i] = i;
    }
}

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct annotated_struct*, union annotated_union*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
int (*annotated_func_ptr)(float, double);

/* Struct with function pointer field */
struct with_callback {
    int data;
    simple_callback_t callback;
    complex_callback_t complex_cb;
};

/* Array of function pointers */
__attribute__((unused)) simple_callback_t callback_array[3];

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare scalar types */
    _Static_assert(__builtin_types_compatible_p(int, int), "int should match int");
    _Static_assert(!__builtin_types_compatible_p(int, float), "int should not match float");
    
    /* Compare pointer types */
    _Static_assert(__builtin_types_compatible_p(int*, int*), "int* should match int*");
    _Static_assert(!__builtin_types_compatible_p(int*, float*), "int* should not match float*");
    
    /* Compare struct vs union */
    _Static_assert(!__builtin_types_compatible_p(struct annotated_struct, 
                                                 union annotated_union),
                   "struct should not match union");
    
    /* Compare qualified vs unqualified */
    _Static_assert(!__builtin_types_compatible_p(const int, int),
                   "const int should not match int");
    
    /* Compare array types */
    int arr1[5];
    int arr2[5];
    int arr3[10];
    _Static_assert(__builtin_types_compatible_p(typeof(arr1), typeof(arr2)),
                   "Same size arrays should match");
    _Static_assert(!__builtin_types_compatible_p(typeof(arr1), typeof(arr3)),
                   "Different size arrays should not match");
}

/* ==================== COMPLEX NESTED TYPES ==================== */
/* Ultimate complex type combining everything */
typedef union __attribute__((annotate("gengtype"))) ultimate_type {
    /* Scalar */
    long double scalar;
    
    /* Pointer */
    void* pointer;
    
    /* Struct */
    struct {
        int tag;
        union {
            /* Array */
            int values[8];
            
            /* String */
            char* str;
            
            /* Callback */
            int (*func)(void);
        } data;
    } tagged;
    
    /* Array of structs with function pointers */
    struct with_callback callbacks[2];
} ultimate_type_t;

/* Function using the ultimate type */
static void use_ultimate_type(ultimate_type_t* ut) {
    /* Access various fields to ensure they're used */
    ut->scalar = 3.14;
    ut->tagged.tag = 42;
    ut->tagged.data.str = "Nested string";
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Declare variables of various types */
    struct annotated_struct my_struct = {1, 2.5, "Test"};
    union annotated_union my_union;
    user_struct_t my_user_struct = {100, "User"};
    complex_nested_t my_nested = {{{1, 2.0}, {{'a', 'b', 'c', 'd'}}, NULL}};
    
    /* Use function pointers */
    int add(int a, int b) { return a + b; }
    simple_callback_t my_callback = add;
    
    /* Use arrays */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * 2;
    }
    
    /* Use variable-length array */
    use_vla(5);
    
    /* Use pointers */
    int_ptr = &int_scalar;
    struct_ptr = &my_struct;
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    __attribute__((unused)) size_t forward_size = sizeof(struct forward_declared_struct*);
    __attribute__((unused)) size_t extern_size = sizeof(union undefined_extern_union*);
    
    /* Call function via pointer */
    __attribute__((unused)) int result = my_callback(3, 4);
    
    /* Use ultimate type */
    ultimate_type_t ultimate;
    use_ultimate_type(&ultimate);
    
    /* Return success */
    return 0;
}

/* Additional incomplete type usage */
struct forward_declared_struct* get_forward_ptr(void) {
    return NULL;
}

union forward_declared_union* get_forward_union_ptr(void) {
    return NULL;
}
