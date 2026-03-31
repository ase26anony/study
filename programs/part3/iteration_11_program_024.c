/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED / TYPE_LANG_STRUCT ========== */
/* Forward declarations and incomplete types */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED */
union forward_declared_union;           /* TYPE_UNDEFINED */

/* ========== TYPE_SCALAR ========== */
__attribute__((unused)) _Bool bool_scalar = 0;
__attribute__((unused)) int int_scalar = 42;
__attribute__((unused)) float float_scalar = 3.14f;
__attribute__((unused)) volatile const long volatile_const_scalar = 100L;

/* ========== TYPE_STRING ========== */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";
__attribute__((unused)) volatile const char* volatile_const_string = "VolatileConst";

/* ========== TYPE_STRUCT ========== */
struct simple_struct {
    int x;
    float y;
    char z;
} __attribute__((annotate("gengtype")));

struct nested_struct {
    struct simple_struct inner;
    double extra;
} __attribute__((annotate("gengtype")));

/* ========== TYPE_USER_STRUCT ========== */
typedef struct {
    int id;
    char name[32];
} user_struct_t __attribute__((annotate("gengtype")));

typedef struct simple_struct renamed_struct_t;

/* ========== TYPE_UNION ========== */
union data_union {
    int as_int;
    float as_float;
    void* as_ptr;
    char as_bytes[8];
} __attribute__((annotate("gengtype")));

union nested_union {
    struct simple_struct as_struct;
    union data_union as_union;
    long as_long;
};

/* ========== TYPE_POINTER ========== */
__attribute__((unused)) int* int_pointer = &int_scalar;
__attribute__((unused)) float* float_pointer = &float_scalar;
__attribute__((unused)) const char* const const_string_pointer = "Constant";
__attribute__((unused)) volatile const int* const volatile_const_pointer = &int_scalar;
__attribute__((unused)) struct simple_struct* struct_pointer = 0;
__attribute__((unused)) union data_union* union_pointer = 0;
__attribute__((unused)) void* void_pointer = 0;

/* Complex pointer types */
__attribute__((unused)) int** pointer_to_pointer = &int_pointer;
__attribute__((unused)) const volatile int* volatile* complex_pointer = (const volatile int* volatile*)&int_pointer;

/* ========== TYPE_ARRAY ========== */
__attribute__((unused)) int fixed_array[10] = {0};
__attribute__((unused)) float multi_dim_array[5][3];
__attribute__((unused)) char variable_length_array[sizeof(int) * 2];
__attribute__((unused)) struct simple_struct struct_array[4];
__attribute__((unused)) union data_union union_array[8];
__attribute__((unused)) const int const_array[] = {1, 2, 3, 4, 5};

/* ========== TYPE_CALLBACK ========== */
typedef int (*simple_callback_t)(int, float) __attribute__((annotate("gengtype")));
typedef void (*complex_callback_t)(struct simple_struct*, union data_union**, size_t);

/* Function pointer variables */
__attribute__((unused)) int (*func_ptr)(int, int);
__attribute__((unused)) void (*void_func_ptr)(void);
__attribute__((unused)) simple_callback_t typed_callback = 0;
__attribute__((unused)) complex_callback_t complex_callback = 0;

/* Array of function pointers */
__attribute__((unused)) int (*callback_array[5])(void);

/* ========== COMPLEX NESTED TYPES ========== */
/* Struct containing array of function pointers */
struct struct_with_callbacks {
    int id;
    int (*handlers[4])(int, void*);
    void (*cleanup)(void);
} __attribute__((annotate("gengtype")));

/* Union with pointer to struct */
union union_with_struct_ptr {
    struct simple_struct* ptr;
    int (*comparator)(const void*, const void*);
    char tag;
};

/* Typedef for complex function callback signature */
typedef union data_union* (*allocator_t)(size_t, void* (*)(size_t));

/* Struct with nested union and function pointer */
struct master_struct {
    struct struct_with_callbacks callbacks;
    union {
        struct simple_struct nested_struct;
        union data_union nested_union;
        allocator_t allocator;
    } variant;
    volatile const int* volatile const_ptr;
    int flexible_array[];
} __attribute__((annotate("gengtype")));

/* ========== TYPE COMPARISONS ========== */
/* Use __builtin_types_compatible_p to trigger type classification */
#define CHECK_TYPE_COMPAT(a, b) \
    __builtin_types_compatible_p(__typeof__(a), __typeof__(b))

/* ========== FUNCTION DECLARATIONS ========== */
static int sample_callback(int a, float b) __attribute__((unused));
static void process_struct(struct simple_struct* s) __attribute__((unused));
static union data_union* custom_allocator(size_t sz, void* (*fallback)(size_t)) __attribute__((unused));

static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void process_struct(struct simple_struct* s) {
    s->x = 42;
    s->y = 3.14f;
    s->z = 'A';
}

static union data_union* custom_allocator(size_t sz, void* (*fallback)(size_t)) {
    return (union data_union*)fallback(sz);
}

/* ========== MAIN FUNCTION ========== */
int main(void) {
    /* Variable declarations using our diverse types */
    struct simple_struct local_struct = {1, 2.0f, 'X'};
    union data_union local_union;
    user_struct_t local_user_struct = {123, "Test"};
    struct master_struct* master_ptr = 0;
    
    /* Initialize some variables to prevent dead code elimination */
    local_union.as_int = 42;
    int_pointer = &int_scalar;
    float_pointer = &float_scalar;
    
    /* Use function pointers */
    typed_callback = sample_callback;
    if (typed_callback) {
        int result = typed_callback(10, 20.5f);
        int_scalar = result;  /* Use result to prevent optimization */
    }
    
    /* Use struct pointer */
    struct_pointer = &local_struct;
    process_struct(struct_pointer);
    
    /* Use union */
    union_pointer = &local_union;
    union_pointer->as_float = 3.14159f;
    
    /* Perform type comparisons to trigger classification */
    int type_checks = 0;
    
    /* Scalar vs Pointer */
    type_checks += CHECK_TYPE_COMPAT(int_scalar, int_pointer) ? 1 : 0;
    
    /* Struct vs Union */
    type_checks += CHECK_TYPE_COMPAT(local_struct, local_union) ? 1 : 0;
    
    /* Array vs Pointer */
    type_checks += CHECK_TYPE_COMPAT(fixed_array, int_pointer) ? 1 : 0;
    
    /* Function pointer vs regular pointer */
    type_checks += CHECK_TYPE_COMPAT(func_ptr, void_pointer) ? 1 : 0;
    
    /* Const vs non-const */
    type_checks += CHECK_TYPE_COMPAT(string_literal, mutable_string) ? 1 : 0;
    
    /* Use sizeof with incomplete types (valid in some contexts) */
    size_t forward_size = sizeof(struct forward_declared_struct*);
    size_t extern_size = sizeof(undefined_extern_array);
    
    /* Use volatile/const qualified types */
    volatile const int* const volatile_ptr = &int_scalar;
    int deref = *volatile_ptr;  /* Read from volatile pointer */
    
    /* Use array types */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * type_checks;
    }
    
    /* Use complex nested type */
    struct struct_with_callbacks complex_var = {
        .id = 99,
        .cleanup = 0
    };
    
    /* Ensure all variables are used to prevent optimization */
    (void)type_checks;
    (void)forward_size;
    (void)extern_size;
    (void)deref;
    (void)complex_var;
    (void)local_user_struct;
    (void)master_ptr;
    
    return 0;
}
