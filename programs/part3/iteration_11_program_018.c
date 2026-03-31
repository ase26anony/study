/* gengtype_coverage.c
 * 
 * This program is specifically designed to trigger coverage of the
 * type classification switch cases in gengtype.cc (lines 182-213).
 * It uses diverse type declarations, GCC attributes, nested constructs,
 * and builtin type comparisons to exercise the compiler's internal
 * type processing during compilation.
 */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */

/* Forward declarations create incomplete/undefined types */
extern struct undefined_struct;           /* TYPE_UNDEFINED candidate */
extern union undefined_union;             /* TYPE_UNDEFINED candidate */
extern int undefined_array[];             /* TYPE_UNDEFINED (incomplete array) */

/* Forward-declared struct that might become TYPE_LANG_STRUCT */
struct lang_struct_forward;

/* ==================== TYPE_SCALAR ==================== */

/* Basic scalar types */
static _Bool __attribute__((unused)) scalar_bool = 0;
static int __attribute__((unused)) scalar_int = 42;
static float __attribute__((unused)) scalar_float = 3.14f;
static volatile const long __attribute__((unused)) scalar_volatile_const = 100L;

/* ==================== TYPE_STRING ==================== */

/* String literals and pointers */
static const char* __attribute__((unused)) string_literal = "Hello, gengtype!";
static char* __attribute__((unused)) string_array[] = {"test1", "test2"};
static volatile const char* const __attribute__((unused)) volatile_const_string = "readonly";

/* ==================== TYPE_STRUCT & TYPE_UNION ==================== */

/* Annotated struct to trigger metadata generation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    const char* name;
};

/* Annotated union */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Regular struct without annotation */
struct regular_struct {
    short a;
    double b;
};

/* Regular union without annotation */
union regular_union {
    int i;
    char c[4];
};

/* ==================== TYPE_USER_STRUCT ==================== */

/* Typedef creates TYPE_USER_STRUCT */
typedef struct regular_struct user_struct_t;
typedef union regular_union user_union_t;

/* Complex typedef with attributes */
typedef struct __attribute__((packed)) {
    unsigned char byte;
    unsigned int word;
} __attribute__((aligned(8))) packed_struct_t;

/* ==================== TYPE_POINTER ==================== */

/* Pointers to various types */
static int* __attribute__((unused)) pointer_to_int;
static const float* __attribute__((unused)) pointer_to_const_float;
static volatile char* volatile __attribute__((unused)) volatile_pointer_to_char;
static struct annotated_struct* __attribute__((unused)) pointer_to_struct;
static union annotated_union* __attribute__((unused)) pointer_to_union;
static void (* volatile __attribute__((unused)) volatile_function_pointer)(void);

/* Triple pointer for complexity */
static int*** __attribute__((unused)) triple_pointer;

/* ==================== TYPE_ARRAY ==================== */

/* Fixed-size arrays */
static int __attribute__((unused)) fixed_array[10];
static struct annotated_struct __attribute__((unused)) struct_array[5];
static const char* const __attribute__((unused)) string_ptr_array[3] = {"a", "b", "c"};

/* Multi-dimensional array */
static float __attribute__((unused)) matrix[3][3];

/* Array with volatile elements */
static volatile int __attribute__((unused)) volatile_array[8];

/* ==================== TYPE_CALLBACK ==================== */

/* Function pointer types (callbacks) */
typedef int (*binary_op_t)(int, int);
typedef void (*event_callback_t)(void* context, int event_id);

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) 
        void (*annotated_callback_t)(struct annotated_struct*, union annotated_union*);

/* Complex callback with multiple parameters */
typedef int (*complex_callback_t)(int, float, const char*, void*);

/* ==================== NESTED & COMPLEX TYPES ==================== */

/* Struct containing array of function pointers */
struct processor {
    const char* name;
    complex_callback_t handlers[4];
    volatile int state;
};

/* Union containing pointer to struct */
union data_container {
    struct annotated_struct* struct_ptr;
    user_struct_t* user_struct_ptr;
    void* generic_ptr;
};

/* Typedef for nested function pointer */
typedef union data_container* (*allocator_t)(size_t);

/* Struct with nested union and function pointer */
struct nested_example {
    int type;
    union {
        int int_value;
        float float_value;
        void* ptr_value;
        binary_op_t op_func;
    } data;
    event_callback_t callback;
};

/* ==================== TYPE COMPARISONS ==================== */

/* Use __builtin_types_compatible_p to trigger type classification */
static void check_type_compatibility(void) __attribute__((unused));
static void check_type_compatibility(void) {
    /* These comparisons force GCC to examine type relationships */
    int scalar_vs_pointer = __builtin_types_compatible_p(int, int*);
    int struct_vs_union = __builtin_types_compatible_p(struct annotated_struct, 
                                                       union annotated_union);
    int callback_vs_pointer = __builtin_types_compatible_p(binary_op_t, void*);
    int array_vs_pointer = __builtin_types_compatible_p(int[10], int*);
    int const_vs_nonconst = __builtin_types_compatible_p(const int, int);
    int volatile_check = __builtin_types_compatible_p(volatile int, int);
    
    /* Use results to avoid dead code elimination */
    if (scalar_vs_pointer || struct_vs_union || callback_vs_pointer || 
        array_vs_pointer || const_vs_nonconst || volatile_check) {
        /* This should never happen with these type pairs */
        return;
    }
}

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    /* Declare variables using our complex types */
    struct annotated_struct annotated_var = {1, 2.0f, "test"};
    union annotated_union annotated_union_var;
    user_struct_t user_var = {5, 3.14159};
    packed_struct_t packed_var = {0xFF, 0xABCD};
    struct processor proc = {"test_proc", {0}, 0};
    union data_container container;
    struct nested_example nested = {0};
    
    /* Initialize function pointers */
    binary_op_t add_func = 0;
    annotated_callback_t annotated_func = 0;
    allocator_t malloc_like = 0;
    
    /* Use incomplete/extern types through pointers */
    struct undefined_struct* undefined_ptr = 0;
    struct lang_struct_forward* lang_ptr = 0;
    
    /* Array operations */
    int array_size = sizeof(fixed_array) / sizeof(fixed_array[0]);
    float matrix_trace = matrix[0][0] + matrix[1][1] + matrix[2][2];
    
    /* Pointer operations */
    pointer_to_int = &scalar_int;
    pointer_to_struct = &annotated_var;
    
    /* Callback invocation (if non-null) */
    if (add_func) {
        int result = add_func(10, 20);
        (void)result;
    }
    
    /* Use volatile variables */
    volatile_array[0] = scalar_int;
    
    /* Check type compatibility */
    check_type_compatibility();
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t ptr_size = sizeof(struct undefined_struct*);
    size_t lang_ptr_size = sizeof(struct lang_struct_forward*);
    
    (void)array_size;
    (void)matrix_trace;
    (void)undefined_ptr;
    (void)lang_ptr;
    (void)ptr_size;
    (void)lang_ptr_size;
    (void)annotated_union_var;
    (void)user_var;
    (void)packed_var;
    (void)proc;
    (void)container;
    (void)nested;
    (void)add_func;
    (void)annotated_func;
    (void)malloc_like;
    
    return 0;
}

/* ==================== DEFINITIONS FOR FORWARD-DECLARED TYPES ==================== */

/* Define previously forward-declared types */
struct undefined_struct {
    int dummy;
};

union undefined_union {
    long dummy;
};

/* This might be treated as TYPE_LANG_STRUCT by GCC */
struct lang_struct_forward {
    int lang_specific_field;
    void (*lang_method)(void);
};

/* Complete the incomplete array */
int undefined_array[5] = {1, 2, 3, 4, 5};
