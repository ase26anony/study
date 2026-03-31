/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED */
struct forward_declared;         /* Forward declaration creates incomplete type */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const const_string = "Constant string";
__attribute__((unused)) volatile char* volatile volatile_string_ptr;

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int id;
    double value;
    char description[32];
} user_struct_t;

/* ========== Complex nested struct with union ========== */
struct complex_container {
    struct annotated_struct nested_struct;
    union annotated_union data_union;
    user_struct_t user_type;
    
    /* TYPE_ARRAY inside struct */
    int int_array[10];
    
    /* TYPE_POINTER inside struct */
    struct complex_container* next;
};

/* ========== TYPE_POINTER declarations ========== */
__attribute__((unused)) int* int_ptr = &int_scalar;
__attribute__((unused)) float* float_ptr = &float_scalar;
__attribute__((unused)) char** string_ptr_ptr = &string_literal;
__attribute__((unused)) volatile const int* const volatile_const_ptr = &int_scalar;
__attribute__((unused)) struct complex_container* struct_ptr = NULL;
__attribute__((unused)) void* void_ptr = NULL;

/* ========== TYPE_ARRAY declarations ========== */
__attribute__((unused)) int fixed_array[5] = {1, 2, 3, 4, 5};
__attribute__((unused)) float matrix[3][3];  /* Multi-dimensional array */
__attribute__((unused)) char variable_len_array[sizeof(int) * 2];  /* VLA-like */
__attribute__((unused)) struct annotated_struct struct_array[2];

/* ========== TYPE_CALLBACK declarations ========== */
typedef int (*simple_callback_t)(int, float);  /* Function pointer typedef */

/* Complex callback signature */
typedef void (*complex_callback_t)(
    struct complex_container*,
    user_struct_t*,
    simple_callback_t
) __attribute__((annotate("gengtype")));

/* Callback variable */
__attribute__((unused)) complex_callback_t registered_callback = NULL;

/* Function to be used as callback */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed through attributes */
struct __attribute__((transaction_safe)) transaction_struct {
    int data;
    void* ptr;
};

/* ========== Type comparison expressions ========== */
/* These force GCC to analyze type compatibility */
#define CHECK_TYPE_COMPAT(type1, type2) \
    __builtin_types_compatible_p(type1, type2)

/* ========== Main function with type usage ========== */
int main(void) {
    /* Declare and initialize variables to ensure they're processed */
    struct complex_container container = {
        .nested_struct = {1, 2.0f, "nested"},
        .data_union = {.as_int = 42},
        .user_type = {100, 3.14159, "user struct"},
        .next = NULL
    };
    
    user_struct_t user_var = {999, 2.71828, "euler"};
    
    /* Use __builtin_types_compatible_p to force type analysis */
    int compat_results[] = {
        CHECK_TYPE_COMPAT(int, float),                    /* scalar vs scalar */
        CHECK_TYPE_COMPAT(int*, float*),                  /* pointer vs pointer */
        CHECK_TYPE_COMPAT(struct complex_container, 
                         struct annotated_struct),        /* struct vs struct */
        CHECK_TYPE_COMPAT(union annotated_union,
                         struct annotated_struct),        /* union vs struct */
        CHECK_TYPE_COMPAT(int[5], int*),                  /* array vs pointer */
        CHECK_TYPE_COMPAT(simple_callback_t, void*),      /* callback vs pointer */
        CHECK_TYPE_COMPAT(user_struct_t*, 
                         struct complex_container*),      /* user struct vs struct */
        CHECK_TYPE_COMPAT(volatile const int*, 
                         const volatile int*),            /* qualified pointers */
        CHECK_TYPE_COMPAT(typeof(undefined_array),
                         int[]),                          /* undefined array */
        CHECK_TYPE_COMPAT(struct forward_declared*,
                         void*)                           /* incomplete type */
    };
    
    /* Use sizeof with various types (works with incomplete types for pointers) */
    size_t sizes[] = {
        sizeof(int),
        sizeof(int*),
        sizeof(int[5]),
        sizeof(struct complex_container),
        sizeof(union annotated_union),
        sizeof(user_struct_t),
        sizeof(simple_callback_t),
        sizeof(struct forward_declared*),  /* Pointer to incomplete type OK */
        sizeof(undefined_array)            /* Incomplete array - may be 0 or error */
    };
    
    /* Use function pointer */
    simple_callback_t cb = sample_callback;
    int result = cb(10, 20.5f);
    
    /* Use volatile/const qualified pointers */
    volatile const int* const restrict_ptr = &int_scalar;
    int read_value = *restrict_ptr;
    
    /* Use the container */
    container.int_array[0] = result;
    container.next = &container;  /* Self-reference */
    
    /* Ensure variables aren't optimized away */
    asm volatile("" : : "r"(compat_results), "r"(sizes), "r"(read_value));
    
    return 0;
}

/* ========== Additional external declarations ========== */
/* More incomplete types for TYPE_UNDEFINED */
extern union unknown_union;
extern enum unknown_enum;

/* Complex function pointer array */
static simple_callback_t callback_array[] = {
    sample_callback,
    NULL,
    (simple_callback_t)sample_callback
};

/* Struct with bitfields (creates interesting internal types) */
struct bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    signed int value : 24;
};

/* Anonymous union inside struct */
struct with_anonymous_union {
    int type;
    union {
        int int_val;
        float float_val;
        void* ptr_val;
    } data;
};

/* Type definition for pointer to function returning pointer to array */
typedef int (*(*complex_func_ptr_t)(void))[5];

/* Final declaration with all qualifiers */
const volatile static struct __attribute__((packed)) fully_qualified {
    char c;
    int i;
    double d;
} fully_qualified_var = {'A', 42, 3.14};
