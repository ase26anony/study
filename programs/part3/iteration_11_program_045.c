/* gengtype_coverage.c - Program to exercise GCC's gengtype type classification */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_struct;           /* TYPE_UNDEFINED */
extern int undefined_array[];             /* TYPE_UNDEFINED array */
extern union undefined_union;             /* TYPE_UNDEFINED */

/* Forward declarations creating incomplete types */
struct forward_declared_struct;
union forward_declared_union;

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool scalar_bool = 0;                    /* TYPE_SCALAR */
__attribute__((unused)) int scalar_int = 42;                      /* TYPE_SCALAR */
__attribute__((unused)) float scalar_float = 3.14f;               /* TYPE_SCALAR */
__attribute__((unused)) volatile const long scalar_volatile_const = 100L; /* TYPE_SCALAR */

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) char* string_literal = "Hello, gengtype!"; /* TYPE_STRING */
__attribute__((unused)) const char* const const_string = "Constant string"; /* TYPE_STRING */

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    float field2;
    char* field3;
};

/* Nested struct with complex type combinations */
struct __attribute__((annotate("gengtype"))) complex_container {
    struct annotated_struct nested_struct;
    volatile int volatile_member;
    const char* const string_member;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int x;
    int y;
} point_t;  /* TYPE_USER_STRUCT */

typedef struct __attribute__((annotate("gengtype"))) {
    point_t start;
    point_t end;
    double weight;
} line_t;  /* TYPE_USER_STRUCT with annotation */

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    char* as_string;
    void* as_pointer;
};

/* Tagged union with struct */
union __attribute__((annotate("gengtype"))) tagged_union {
    struct {
        int type;
        union data_union data;
    } tagged;
    unsigned char raw[16];
};

/* ========== TYPE_POINTER variations ========== */
__attribute__((unused)) int* int_pointer = &scalar_int;                     /* TYPE_POINTER */
__attribute__((unused)) volatile const int* const volatile_pointer = NULL;  /* TYPE_POINTER */
__attribute__((unused)) struct complex_container* struct_pointer = NULL;    /* TYPE_POINTER */
__attribute__((unused)) union data_union* union_pointer = NULL;             /* TYPE_POINTER */
__attribute__((unused)) void (*function_pointer)(void) = NULL;              /* TYPE_POINTER */
__attribute__((unused)) char** pointer_to_pointer = &string_literal;        /* TYPE_POINTER to TYPE_POINTER */

/* Triple pointer with qualifiers */
__attribute__((unused)) const volatile int * const * volatile triple_pointer = NULL;

/* ========== TYPE_ARRAY variations ========== */
__attribute__((unused)) int fixed_array[10];                                /* TYPE_ARRAY */
__attribute__((unused)) float multi_dim_array[5][3];                        /* TYPE_ARRAY of TYPE_ARRAY */
__attribute__((unused)) struct annotated_struct struct_array[4];            /* TYPE_ARRAY of TYPE_STRUCT */
__attribute__((unused)) union data_union union_array[8];                    /* TYPE_ARRAY of TYPE_UNION */

/* Variable-length array (in function scope) */
__attribute__((unused)) char* string_array[] = {"one", "two", "three"};     /* TYPE_ARRAY of TYPE_STRING */

/* ========== TYPE_CALLBACK (function pointers) ========== */
/* Simple callback */
typedef int (*simple_callback_t)(int, float);  /* TYPE_CALLBACK */

/* Complex callback with struct parameter */
typedef void (*complex_callback_t)(
    struct complex_container*,
    union data_union,
    simple_callback_t
) __attribute__((annotate("gengtype")));  /* TYPE_CALLBACK with annotation */

/* Callback returning pointer to callback */
typedef simple_callback_t (*meta_callback_t)(int);

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Use GCC extension for language-specific struct */
struct __attribute__((transaction_safe)) transaction_struct {
    int value;
    void* data;
};

/* ========== Nested type constructs ========== */
/* Struct containing array of function pointers */
struct callback_container {
    complex_callback_t callbacks[5];
    meta_callback_t meta_callback;
};

/* Union with pointer to struct */
union pointer_union {
    struct complex_container* struct_ptr;
    struct callback_container* callback_ptr;
    void** void_ptr_ptr;
};

/* Typedef for complex function callback signature */
typedef union pointer_union (*ultra_callback_t)(
    struct callback_container**,
    int (*)(int, int),
    volatile const char* []
);

/* ========== Type comparison expressions ========== */
/* These may trigger __builtin_types_compatible_p internal processing */
#define CHECK_TYPE_COMPAT(type1, type2) \
    __builtin_types_compatible_p(type1, type2)

/* ========== Main function with type usage ========== */
int main(void) {
    /* Declare variables using our complex types */
    struct complex_container container = {
        .nested_struct = {1, 2.0f, "nested"},
        .volatile_member = 99,
        .string_member = "main string"
    };
    
    union data_union data = {.as_int = 42};
    line_t line = {{0, 0}, {10, 10}, 1.5};
    struct callback_container callbacks = {{0}};
    
    /* Use incomplete types through pointers */
    struct forward_declared_struct* forward_ptr = NULL;
    union forward_declared_union* forward_union_ptr = NULL;
    
    /* Use extern incomplete types */
    extern struct undefined_struct* get_undefined(void);
    struct undefined_struct* undefined_ptr = NULL;
    
    /* Array usage */
    fixed_array[0] = scalar_int;
    struct_array[0].field1 = 100;
    
    /* Function pointer usage */
    simple_callback_t callback = NULL;
    if (callback) {
        /* Prevent dead code elimination */
        volatile int dummy = callback(1, 2.0f);
        (void)dummy;
    }
    
    /* Type compatibility checks (compile-time) */
    int compat1 = CHECK_TYPE_COMPAT(typeof(scalar_int), int);
    int compat2 = CHECK_TYPE_COMPAT(typeof(int_pointer), int*);
    int compat3 = CHECK_TYPE_COMPAT(typeof(struct complex_container), 
                                   struct annotated_struct);
    int compat4 = CHECK_TYPE_COMPAT(typeof(data_union), union tagged_union);
    
    /* Use sizeof with various types (including incomplete in some compilers) */
    size_t sizes[] = {
        sizeof(scalar_int),
        sizeof(struct complex_container),
        sizeof(union data_union),
        sizeof(fixed_array),
        sizeof(simple_callback_t),
        sizeof(struct forward_declared_struct*),  /* Pointer to incomplete OK */
        sizeof(line_t)
    };
    
    /* Use volatile/const qualified pointers */
    volatile const int* const volatile_ptr = &scalar_int;
    const char* const const_ptr = "test";
    
    /* Prevent optimization of unused variables */
    (void)container;
    (void)data;
    (void)line;
    (void)callbacks;
    (void)forward_ptr;
    (void)forward_union_ptr;
    (void)undefined_ptr;
    (void)compat1;
    (void)compat2;
    (void)compat3;
    (void)compat4;
    (void)sizes;
    (void)volatile_ptr;
    (void)const_ptr;
    
    return 0;
}

/* ========== Additional external declarations ========== */
/* Create more TYPE_UNDEFINED references */
extern struct external_undefined;
extern int external_undefined_function(struct external_undefined* param);

/* Function using many of our types */
static void use_types(
    struct complex_container* c,
    union data_union u,
    simple_callback_t cb,
    volatile const char* strings[]
) __attribute__((unused));

static void use_types(
    struct complex_container* c,
    union data_union u,
    simple_callback_t cb,
    volatile const char* strings[]
) {
    /* Use parameters to prevent dead code elimination */
    if (c && cb) {
        volatile int result = cb(c->nested_struct.field1, 0.0f);
        (void)result;
    }
    if (strings) {
        volatile const char* first = strings[0];
        (void)first;
    }
    (void)u;
}
