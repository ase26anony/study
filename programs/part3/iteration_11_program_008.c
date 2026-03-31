/* gengtype_test.c - Comprehensive type declarations to exercise gengtype.cc */

/* Forward declarations for incomplete types */
extern struct undefined_extern_struct;
extern union undefined_extern_union;
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_UNDEFINED triggers */
extern int undefined_extern_array[];
extern struct undefined_extern_struct* undefined_ptr;

/* TYPE_SCALAR declarations */
__attribute__((unused)) volatile const _Bool scalar_bool = 0;
__attribute__((unused)) volatile int scalar_int = 42;
__attribute__((unused)) const float scalar_float = 3.14f;
__attribute__((unused)) volatile const double scalar_double = 2.71828;

/* TYPE_STRING declarations */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) volatile char* volatile string_ptr = "Test";
__attribute__((unused)) const char* const const_string_ptr = "Constant";

/* TYPE_STRUCT with annotation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    volatile int field1;
    const char* field2;
    float field3;
};

/* TYPE_USER_STRUCT (typedef struct) */
typedef struct __attribute__((annotate("gengtype"))) {
    long data;
    void* ptr;
} user_struct_t;

/* TYPE_UNION with annotation */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_POINTER variations */
__attribute__((unused)) volatile int* volatile volatile_int_ptr;
__attribute__((unused)) const struct annotated_struct* const_struct_ptr;
__attribute__((unused)) user_struct_t* user_struct_ptr;
__attribute__((unused)) volatile const int* const complex_ptr = &scalar_int;
__attribute__((unused)) void (*volatile volatile_func_ptr)(void);

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) volatile float volatile_array[5];
__attribute__((unused)) const char* string_array[] = {"one", "two", "three"};
__attribute__((unused)) struct annotated_struct struct_array[3];
__attribute__((unused)) int variable_len_array[scalar_int > 0 ? 5 : 10]; /* VLA */

/* TYPE_CALLBACK - function pointer types */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, user_struct_t*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
void (*annotated_callback)(int, const char*);

/* Complex nested type constructs */

/* Struct containing array of function pointers */
struct nested_struct {
    simple_callback_t callbacks[5];
    union annotated_union data_union;
    volatile int* ptr_array[3];
};

/* Union with pointer to struct */
union pointer_union {
    struct nested_struct* nested_ptr;
    complex_callback_t callback_ptr;
    int (*func_ptr_array[2])(void);
};

/* Typedef for complex function callback signature */
typedef union pointer_union* (*meta_callback_t)(
    struct nested_struct**, 
    const volatile void*
);

/* TYPE_LANG_STRUCT simulation via GCC extension */
#ifdef __GNUC__
struct __attribute__((transparent_union)) transparent_union {
    int i;
    float f;
};

/* Another GCC extension that might create lang-specific structs */
struct __attribute__((packed, aligned(2))) packed_struct {
    char c;
    int i;
    double d;
};
#endif

/* Global variables using complex types */
__attribute__((unused)) struct nested_struct global_nested;
__attribute__((unused)) union pointer_union global_pointer_union;
__attribute__((unused)) meta_callback_t global_meta_callback;

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare scalar types */
    int scalar_check = __builtin_types_compatible_p(int, float);
    scalar_check += __builtin_types_compatible_p(_Bool, int);
    
    /* Compare pointer types */
    int ptr_check = __builtin_types_compatible_p(int*, float*);
    ptr_check += __builtin_types_compatible_p(void*, char*);
    
    /* Compare struct vs union */
    int struct_union_check = __builtin_types_compatible_p(
        struct annotated_struct, 
        union annotated_union
    );
    
    /* Compare array types */
    int array_check = __builtin_types_compatible_p(int[10], int[5]);
    array_check += __builtin_types_compatible_p(char*, char[]);
    
    /* Compare function pointers */
    int callback_check = __builtin_types_compatible_p(
        simple_callback_t,
        complex_callback_t
    );
    
    /* Use results to avoid dead code elimination */
    volatile int dummy = scalar_check + ptr_check + struct_union_check + 
                        array_check + callback_check;
    (void)dummy;
}

/* Function to exercise function pointers */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void annotated_callback_impl(int x, const char* msg) {
    volatile int dummy = x;
    (void)msg;
    (void)dummy;
}

int main(void) {
    /* Initialize some variables to ensure they're not optimized away */
    user_struct_t user_struct = {.data = 100, .ptr = (void*)&scalar_int};
    struct nested_struct nested = {0};
    union pointer_union pu = {0};
    
    /* Use incomplete types in sizeof (valid in some contexts) */
    volatile size_t incomplete_size = sizeof(struct forward_declared_struct*);
    incomplete_size += sizeof(undefined_extern_array);
    
    /* Assign function pointers */
    simple_callback_t cb = sample_callback;
    annotated_callback = annotated_callback_impl;
    
    /* Use the function pointers */
    if (cb) {
        volatile int result = cb(10, 20.5f);
        (void)result;
    }
    
    if (annotated_callback) {
        annotated_callback(42, "test");
    }
    
    /* Use complex pointer */
    const_struct_ptr = (const struct annotated_struct*)&nested;
    
    /* Perform type comparisons */
    compare_types();
    
    /* Use volatile and const qualified pointers */
    volatile_int_ptr = (volatile int*)&scalar_int;
    *volatile_int_ptr = 99;
    
    /* Access array elements */
    fixed_array[0] = 1;
    volatile_array[1] = 2.0f;
    
    /* Use union */
    pu.nested_ptr = &nested;
    
    /* Return statement ensures main has defined behavior */
    return 0;
}

/* Additional incomplete type declarations */
struct forward_declared_struct {
    int some_field;
};

union forward_declared_union {
    int i;
    char c;
};
