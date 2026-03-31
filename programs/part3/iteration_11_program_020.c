/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_extern;  /* TYPE_UNDEFINED */
extern int extern_array[];       /* TYPE_UNDEFINED for incomplete array */
struct forward_declared;         /* Forward declaration creates TYPE_UNDEFINED */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";
__attribute__((unused)) volatile char* volatile volatile_string_ptr = 0;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* ========== TYPE_USER_STRUCT via typedef ========== */
typedef struct {
    int id;
    double value;
    struct forward_declared* next;  /* Pointer to incomplete type */
} user_struct_t;

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_ptr = &int_scalar;
__attribute__((unused)) const float* const const_float_ptr = &float_scalar;
__attribute__((unused)) volatile const int* const volatile_const_ptr = 0;
__attribute__((unused)) void* void_ptr = 0;
__attribute__((unused)) struct annotated_struct* struct_ptr = 0;
__attribute__((unused)) union data_union* union_ptr = 0;
__attribute__((unused)) user_struct_t* user_struct_ptr = 0;

/* ========== TYPE_ARRAY declarations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float const const_array[5] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f};
__attribute__((unused)) volatile int volatile_array[3];
__attribute__((unused)) char* pointer_array[4];
__attribute__((unused)) struct annotated_struct struct_array[2];

/* Variable-length array (in function scope) */
void use_vla(int size) {
    int vla[size];  /* TYPE_ARRAY with variable length */
    __attribute__((unused)) volatile int* vla_ptr = vla;
}

/* ========== TYPE_CALLBACK function pointers ========== */
typedef int (*simple_callback)(int, float);  /* TYPE_CALLBACK in typedef */

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) void (*annotated_callback)(
    struct annotated_struct*, 
    union data_union*
);

/* Complex callback with multiple qualifiers */
typedef const char* (*complex_callback)(
    volatile const int*, 
    void* __attribute__((unused))
);

/* ========== Nested type constructs ========== */

/* Struct containing array of function pointers */
struct nested_container {
    int count;
    simple_callback callbacks[5];  /* TYPE_ARRAY of TYPE_CALLBACK */
    union data_union data;
    struct {
        int x;
        int y;
    } embedded;  /* Anonymous struct */
};

/* Union with pointer to struct */
union pointer_union {
    struct nested_container* container_ptr;
    annotated_callback callback_ptr;
    int (*func_ptr_array[3])(void);  /* Array of function pointers */
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed via attributes or builtins */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
    void* ptr;
};

/* ========== Main function with type usage ========== */
int main() {
    /* Declare variables of our complex types */
    struct annotated_struct my_struct = {1, 2.0f, "test"};
    union data_union my_union;
    user_struct_t my_user_struct = {42, 3.14159, 0};
    struct nested_container container = {0};
    union pointer_union ptr_union;
    
    /* Initialize function pointers */
    __attribute__((unused)) simple_callback my_callback = 0;
    __attribute__((unused)) annotated_callback annotated_cb = 0;
    __attribute__((unused)) complex_callback complex_cb = 0;
    
    /* Use variable-length array */
    use_vla(10);
    
    /* Type comparisons using __builtin_types_compatible_p */
    __attribute__((unused)) int type_check_1 = 
        __builtin_types_compatible_p(typeof(int_scalar), typeof(float_scalar));
    
    __attribute__((unused)) int type_check_2 = 
        __builtin_types_compatible_p(typeof(int_ptr), typeof(void_ptr));
    
    __attribute__((unused)) int type_check_3 = 
        __builtin_types_compatible_p(typeof(my_struct), typeof(my_user_struct));
    
    __attribute__((unused)) int type_check_4 = 
        __builtin_types_compatible_p(typeof(my_union), typeof(ptr_union));
    
    __attribute__((unused)) int type_check_5 = 
        __builtin_types_compatible_p(typeof(my_callback), typeof(complex_cb));
    
    /* Use incomplete types in sizeof (allowed for pointers) */
    __attribute__((unused)) size_t size1 = sizeof(struct forward_declared*);
    __attribute__((unused)) size_t size2 = sizeof(extern_array);
    
    /* Use volatile/const qualified pointers */
    volatile_const_ptr = &int_scalar;
    volatile_string_ptr = (char*)string_literal;
    
    /* Simple operations to prevent dead code elimination */
    my_union.as_int = int_scalar;
    container.count = 1;
    my_user_struct.value += float_scalar;
    
    /* Call via function pointer if non-null */
    if (my_callback) {
        my_callback(1, 2.0f);
    }
    
    /* Use the annotated callback */
    if (annotated_cb) {
        annotated_cb(&my_struct, &my_union);
    }
    
    /* Array operations */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i;
    }
    
    /* Pointer arithmetic with different pointer types */
    int_ptr++;
    struct_ptr++;
    void_ptr = (void*)((char*)void_ptr + 1);
    
    return 0;
}

/* ========== Additional external declarations ========== */
/* More incomplete types for TYPE_UNDEFINED */
extern union undefined_union;
extern struct undefined_extern* extern_ptr_array[];

/* Complex typedef chain */
typedef user_struct_t* user_ptr_t;
typedef user_ptr_t (*get_user_ptr_fn)(int);
typedef get_user_ptr_fn fn_array_t[3];

/* Anonymous union in struct */
struct with_anonymous_union {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    } data;
};

/* Const-volatile qualified function pointer */
typedef int (__attribute__((const)) *const_volatile_fp)(volatile int*);
