/* Complex type declarations to exercise GCC's gengtype type classification */
#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;  /* TYPE_UNDEFINED */
extern int undefined_array[];    /* TYPE_UNDEFINED array */
struct forward_declared;         /* Forward declaration */

/* ========== TYPE_SCALAR ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING ========== */
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
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int id;
    char data[32];
} user_struct_t;

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_ptr = &int_scalar;
__attribute__((unused)) float* float_ptr = &float_scalar;
__attribute__((unused)) char** string_ptr_ptr = &string_literal;
__attribute__((unused)) volatile const int* const volatile_const_ptr = &int_scalar;
__attribute__((unused)) struct annotated_struct* struct_ptr;
__attribute__((unused)) union annotated_union* union_ptr;
__attribute__((unused)) user_struct_t* user_struct_ptr;

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float matrix[3][3];
__attribute__((unused)) volatile char volatile_array[5];
__attribute__((unused)) const int const_array[] = {1, 2, 3, 4, 5};

/* Variable Length Array (VLA) - TYPE_ARRAY */
void use_vla(size_t n) {
    __attribute__((unused)) int vla[n];
    (void)vla; /* Suppress unused warning */
}

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union annotated_union*);

__attribute__((annotate("gengtype"))) 
int (*annotated_func_ptr)(int, char**);

/* Callback with qualifiers */
typedef const char* (*const_string_callback_t)(void) __attribute__((const));

/* ========== Complex nested type constructs ========== */
struct __attribute__((annotate("gengtype"))) complex_container {
    /* Struct containing array of function pointers */
    simple_callback_t callbacks[5];
    
    /* Union with pointer to struct */
    union {
        struct annotated_struct* struct_ptr;
        user_struct_t* user_ptr;
    } data_union;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Nested struct */
    struct {
        int depth;
        float factor;
    } nested;
};

/* Union containing struct with function pointer */
union __attribute__((annotate("gengtype"))) mega_union {
    struct {
        int type;
        void (*handler)(void);
    } tagged;
    double value;
    void* generic_ptr;
};

/* ========== Type comparison expressions ========== */
/* These may trigger type classification during compilation */
static void type_comparisons(void) {
    /* Use __builtin_types_compatible_p for various type comparisons */
    __attribute__((unused)) int scalar_vs_pointer = 
        __builtin_types_compatible_p(typeof(int_scalar), typeof(int_ptr));
    
    __attribute__((unused)) int struct_vs_union = 
        __builtin_types_compatible_p(struct annotated_struct, union annotated_union);
    
    __attribute__((unused)) int array_vs_pointer = 
        __builtin_types_compatible_p(typeof(fixed_array), typeof(int_ptr));
    
    __attribute__((unused)) int callback_vs_pointer = 
        __builtin_types_compatible_p(simple_callback_t, void*);
    
    __attribute__((unused)) int const_vs_nonconst = 
        __builtin_types_compatible_p(const int, int);
    
    __attribute__((unused)) int volatile_vs_nonvolatile = 
        __builtin_types_compatible_p(volatile int, int);
}

/* ========== Function using function pointer ========== */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static const char* get_const_string(void) {
    return "Test";
}

/* ========== Main function ========== */
int main(void) {
    /* Declare and initialize complex types */
    struct complex_container container = {
        .callbacks = {NULL, NULL, NULL, NULL, NULL},
        .data_union = {.struct_ptr = NULL},
        .array_ptr = &fixed_array,
        .nested = {.depth = 1, .factor = 2.0f}
    };
    
    union mega_union munion = {.tagged = {.type = 1, .handler = NULL}};
    
    user_struct_t user_struct = {.id = 100, .data = "User data"};
    
    /* Initialize function pointers */
    simple_callback_t my_callback = sample_callback;
    const_string_callback_t string_callback = get_const_string;
    
    /* Use VLAs */
    use_vla(20);
    
    /* Perform type comparisons (compile-time) */
    type_comparisons();
    
    /* Use some variables to prevent dead code elimination */
    int result = my_callback(10, 3.5f);
    const char* str = string_callback();
    
    /* Use sizeof with incomplete types through pointers */
    __attribute__((unused)) size_t ptr_size = sizeof(struct forward_declared*);
    __attribute__((unused)) size_t extern_size = sizeof(struct undefined_struct*);
    
    /* Array operations */
    fixed_array[0] = result;
    matrix[1][1] = float_scalar;
    
    /* Pointer operations */
    struct_ptr = &(struct annotated_struct){.x = 1, .y = 2.0f, .name = "Test"};
    int_ptr = &fixed_array[0];
    
    /* Access complex nested types */
    container.nested.depth = result;
    munion.value = 3.14159;
    
    /* String operations */
    string_literal = (char*)str;
    
    return 0;
}

/* ========== Additional incomplete/forward declarations ========== */
struct forward_declared {
    /* Defined later or never */
    int some_field;
};

/* Potential TYPE_LANG_STRUCT trigger through GCC extensions */
#ifdef __GNUC__
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
};
#endif
