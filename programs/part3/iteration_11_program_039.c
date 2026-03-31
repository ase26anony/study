/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */

/* Forward declarations for incomplete types */
struct incomplete_struct;
union incomplete_union;
extern struct incomplete_struct extern_incomplete;

/* TYPE_UNDEFINED triggers */
extern int undefined_extern_array[];
extern struct undefined_struct* undefined_ptr;

/* TYPE_SCALAR declarations */
__attribute__((unused, annotate("gengtype"))) volatile const _Bool scalar_bool = 0;
__attribute__((unused)) volatile int scalar_int = 42;
__attribute__((unused)) const float scalar_float = 3.14f;
__attribute__((unused)) double scalar_double = 2.71828;

/* TYPE_STRING declarations */
__attribute__((unused, annotate("gengtype"))) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) volatile char* volatile_string = "Volatile string";
__attribute__((unused)) char* const const_string_ptr = "Const pointer";

/* TYPE_STRUCT definitions */
struct simple_struct {
    int x;
    float y;
    char z;
};

__attribute__((annotate("gengtype")))
struct complex_struct {
    volatile int counter;
    const char* name;
    struct simple_struct nested;
    void (*callback)(int);  /* Function pointer field */
};

/* TYPE_USER_STRUCT via typedef */
typedef struct simple_struct user_struct_t;
typedef struct complex_struct complex_user_t;

__attribute__((annotate("gengtype")))
typedef struct {
    int id;
    char data[256];
} anonymous_struct_t;

/* TYPE_UNION definitions */
union simple_union {
    int as_int;
    float as_float;
    char* as_string;
};

__attribute__((annotate("gengtype")))
union complex_union {
    struct simple_struct as_struct;
    union simple_union as_union;
    void* as_pointer;
};

/* TYPE_POINTER declarations */
__attribute__((unused)) volatile const int* volatile const volatile_const_ptr = &scalar_int;
__attribute__((unused)) struct complex_struct* struct_ptr = 0;
__attribute__((unused)) union complex_union* union_ptr = 0;
__attribute__((unused)) user_struct_t* user_struct_ptr = 0;
__attribute__((unused)) const char* const* pointer_to_string_pointer = &string_literal;

/* Multi-level pointer */
__attribute__((unused)) int*** triple_pointer = 0;

/* TYPE_ARRAY declarations */
__attribute__((unused, annotate("gengtype"))) int fixed_array[10];
__attribute__((unused)) volatile float volatile_array[5][5];  /* 2D array */
__attribute__((unused)) const char* string_array[] = {"one", "two", "three"};

/* Variable Length Array (VLA) */
__attribute__((unused)) void use_vla(int n) {
    int vla_array[n];
    vla_array[0] = n;
}

/* Array of complex types */
__attribute__((unused)) struct simple_struct struct_array[3];
__attribute__((unused)) union simple_union union_array[2];

/* TYPE_CALLBACK - Function pointer types */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct complex_struct*, union complex_union*);

__attribute__((annotate("gengtype")))
typedef const char* (*string_callback_t)(int, ...);  /* Variadic function pointer */

/* Function pointer variables */
__attribute__((unused)) simple_callback_t simple_cb = 0;
__attribute__((unused)) volatile complex_callback_t volatile_cb = 0;
__attribute__((unused)) string_callback_t string_cb = 0;

/* Nested type: struct containing array of function pointers */
struct has_callback_array {
    int count;
    simple_callback_t callbacks[5];
};

/* TYPE_LANG_STRUCT simulation via attribute */
struct __attribute__((annotate("lang_struct"))) lang_like_struct {
    int magic;
    void* data;
};

/* Complex nested type combining multiple classifications */
typedef struct {
    union {
        struct simple_struct s;
        struct complex_struct* ps;
    } data;
    
    volatile int flags;
    
    struct {
        int size;
        int (*compare)(const void*, const void*);
    } operations;
    
    char flexible_array[];  /* Flexible array member */
} mega_struct_t;

/* Function using __builtin_types_compatible_p for type comparisons */
__attribute__((unused))
static void type_comparisons(void) {
    /* Compare various type combinations */
    int is_same1 = __builtin_types_compatible_p(int, float);  /* Scalar vs scalar */
    int is_same2 = __builtin_types_compatible_p(int*, float*);  /* Pointer vs pointer */
    int is_same3 = __builtin_types_compatible_p(struct simple_struct, 
                                               union simple_union);  /* Struct vs union */
    int is_same4 = __builtin_types_compatible_p(user_struct_t, 
                                               struct simple_struct);  /* Typedef vs struct */
    int is_same5 = __builtin_types_compatible_p(int[10], int*);  /* Array vs pointer */
    int is_same6 = __builtin_types_compatible_p(simple_callback_t,
                                               complex_callback_t);  /* Callback vs callback */
    
    /* Compare qualified types */
    int is_same7 = __builtin_types_compatible_p(const int, volatile int);
    int is_same8 = __builtin_types_compatible_p(const int*, int* const);
    
    /* Use the results to prevent dead code elimination */
    volatile int result = is_same1 + is_same2 + is_same3 + is_same4 + 
                         is_same5 + is_same6 + is_same7 + is_same8;
    (void)result;
}

/* Helper functions for callbacks */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static const char* variadic_callback(int count, ...) {
    return "variadic";
}

/* Main function with diverse type usage */
int main(void) {
    /* Variable declarations using our types */
    struct complex_struct cs = {
        .counter = 100,
        .name = "test",
        .nested = {1, 2.0f, 'A'},
        .callback = 0
    };
    
    union complex_union cu;
    cu.as_pointer = &cs;
    
    anonymous_struct_t anon = {.id = 42, .data = "anonymous"};
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * 2;
    }
    
    struct_array[0].x = 1;
    union_array[0].as_int = 42;
    
    /* Use function pointers */
    simple_cb = sample_callback;
    string_cb = variadic_callback;
    
    /* Call through function pointer */
    if (simple_cb) {
        int result = simple_cb(10, 3.5f);
        (void)result;
    }
    
    /* Use sizeof with various types (including incomplete) */
    size_t sizes[] = {
        sizeof(struct simple_struct),
        sizeof(union complex_union),
        sizeof(user_struct_t),
        sizeof(fixed_array),
        sizeof(simple_callback_t),
        sizeof(struct incomplete_struct*)  /* Pointer to incomplete type */
    };
    
    /* Trigger type comparisons */
    type_comparisons();
    
    /* Use volatile and const qualified pointers */
    const int* read_ptr = fixed_array;
    volatile int* write_ptr = fixed_array;
    
    /* Complex pointer dereference chain */
    int value = ***triple_pointer;
    (void)value;
    
    /* Use the lang-like struct */
    struct lang_like_struct ls = {.magic = 0xDEADBEEF, .data = &cs};
    
    /* Ensure everything is used to prevent optimization */
    return (cs.counter > 0) ? 0 : 1;
}

/* External definitions for incomplete types */
struct incomplete_struct {
    int placeholder;
};

union incomplete_union {
    long a;
    double b;
};
