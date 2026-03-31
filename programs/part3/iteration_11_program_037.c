/* gengtype_test.c - Test program to trigger gengtype.cc type classification logic */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED and TYPE_LANG_STRUCT ==================== */
/* Forward declarations create incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern union undefined_union;             /* TYPE_UNDEFINED */
extern int incomplete_array[];            /* TYPE_UNDEFINED array */

/* GCC language-specific internal struct (TYPE_LANG_STRUCT) */
struct __gcc_internal_lang_struct __attribute__((annotate("gengtype")));

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
_Bool bool_scalar __attribute__((unused)) = 1;
int int_scalar __attribute__((unused)) = 42;
float float_scalar __attribute__((unused)) = 3.14f;
volatile const long volatile_const_scalar __attribute__((unused)) = 100L;

/* ==================== TYPE_STRING ==================== */
/* String literals and character pointers */
const char* string_literal __attribute__((unused)) = "Hello, gengtype!";
char* mutable_string __attribute__((unused)) = "Mutable";
volatile const char* volatile_const_string __attribute__((unused)) = "VolatileConst";

/* ==================== TYPE_STRUCT and TYPE_UNION ==================== */
/* Named struct with gengtype annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int x;
    float y;
    const char* name;
};

/* Named union with nested complexity */
union complex_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_pointer;
    struct {
        char a;
        char b;
    } nested;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef struct creates TYPE_USER_STRUCT */
typedef struct {
    int id;
    double value;
    volatile const int* ref;
} user_struct_t;

/* Another typedef with annotation */
typedef struct __attribute__((annotate("gengtype"))) {
    unsigned flags;
    char data[16];
} annotated_user_struct_t;

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
int* int_pointer __attribute__((unused));
const float* const const_float_pointer __attribute__((unused));
volatile char* volatile volatile_char_pointer __attribute__((unused));
volatile const int* const volatile_const_int_pointer __attribute__((unused)) = &int_scalar;

/* Pointer to incomplete type */
struct undefined_struct* ptr_to_undefined __attribute__((unused));

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
int fixed_array[10] __attribute__((unused)) = {0};
const char* string_array[5] __attribute__((unused)) = {"a", "b", "c"};

/* Multi-dimensional array */
float matrix[3][4] __attribute__((unused));

/* Array with qualifiers */
volatile const int volatile_const_array[8] __attribute__((unused));

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (callbacks) */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union complex_union*)
    __attribute__((annotate("gengtype")));

/* Function pointer variables */
int (*func_ptr)(int, int) __attribute__((unused));
void (*volatile volatile_func_ptr)(void) __attribute__((unused));

/* ==================== COMPLEX NESTED CONSTRUCTS ==================== */
/* Struct containing array of function pointers */
struct nested_container {
    simple_callback_t callbacks[4];
    union complex_union data;
    volatile const int* const volatile_ptr;
} __attribute__((annotate("gengtype")));

/* Union with pointer to struct */
union pointer_union {
    struct annotated_struct* struct_ptr;
    user_struct_t* user_struct_ptr;
    complex_callback_t callback;
};

/* Typedef for complex function signature */
typedef union pointer_union* (*meta_callback_t)(
    struct nested_container*,
    const char*,
    int
) __attribute__((annotate("gengtype")));

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p to trigger type classification */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* These comparisons force GCC to analyze and classify the types */
    int scalar_vs_pointer = __builtin_types_compatible_p(typeof(int_scalar), typeof(int_pointer));
    int struct_vs_union = __builtin_types_compatible_p(
        typeof(struct annotated_struct), 
        typeof(union complex_union)
    );
    int array_vs_pointer = __builtin_types_compatible_p(typeof(fixed_array), typeof(int*));
    int callback_vs_pointer = __builtin_types_compatible_p(
        typeof(simple_callback_t), 
        typeof(void*)
    );
    
    /* Compare qualified vs unqualified types */
    int const_vs_nonconst = __builtin_types_compatible_p(
        typeof(const int*), 
        typeof(int*)
    );
    int volatile_vs_nonvolatile = __builtin_types_compatible_p(
        typeof(volatile int), 
        typeof(int)
    );
    
    /* Prevent dead code elimination */
    (void)scalar_vs_pointer;
    (void)struct_vs_union;
    (void)array_vs_pointer;
    (void)callback_vs_pointer;
    (void)const_vs_nonconst;
    (void)volatile_vs_nonvolatile;
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Declare variables using our complex types */
    struct annotated_struct my_struct = {1, 2.0f, "test"};
    union complex_union my_union;
    user_struct_t my_user_struct = {100, 3.14159, &int_scalar};
    struct nested_container container;
    
    /* Initialize function pointers (NULL for safety) */
    func_ptr = NULL;
    volatile_func_ptr = NULL;
    
    /* Use sizeof with incomplete types (valid in GCC) */
    size_t sz_undefined = sizeof(struct undefined_struct*);  /* Pointer size */
    size_t sz_incomplete_array = sizeof(incomplete_array);   /* Array pointer size */
    
    /* Simple operations to ensure processing */
    my_union.as_int = 42;
    container.volatile_ptr = &int_scalar;
    
    /* Call type comparison function */
    type_comparisons();
    
    /* Use variables to prevent dead code elimination */
    (void)my_struct;
    (void)my_union;
    (void)my_user_struct;
    (void)container;
    (void)sz_undefined;
    (void)sz_incomplete_array;
    
    return 0;
}

/* ==================== EXTERNAL DECLARATIONS ==================== */
/* External declarations without definitions */
extern struct external_struct {
    int magic;
    void* data;
} external_instance __attribute__((annotate("gengtype")));

extern int external_array[100];

/* Incomplete struct used in pointer */
struct forward_declared_struct;
struct forward_declared_struct* global_fwd_ptr = NULL;
