/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classifier */

/* Forward declarations for incomplete types */
extern struct undefined_extern;
extern union undefined_extern_union;
struct forward_declared_struct;
union forward_declared_union;

/* TYPE_UNDEFINED: Incomplete/extern types */
extern struct undefined_extern *p_undefined;
extern int incomplete_array[];

/* TYPE_SCALAR: Basic scalar types with qualifiers */
volatile const _Bool volatile_scalar_bool __attribute__((unused));
const int const_scalar_int __attribute__((unused)) = 42;
volatile float volatile_scalar_float __attribute__((unused));

/* TYPE_STRING: String literals and pointers */
const char* string_literal __attribute__((unused)) = "Hello, gengtype!";
char* mutable_string __attribute__((unused));
volatile const char* volatile_const_string __attribute__((unused));

/* TYPE_STRUCT: Named struct types with attributes */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int field1;
    float field2;
    const char* field3;
};

struct complex_nested_struct {
    /* TYPE_ARRAY inside struct */
    int fixed_array[10];
    /* TYPE_POINTER inside struct */
    struct annotated_struct* nested_ptr;
    /* TYPE_UNION inside struct */
    union internal_union {
        int as_int;
        float as_float;
        void* as_ptr;
    } data;
    /* TYPE_CALLBACK inside struct */
    void (*callback)(int, float);
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    long id;
    char name[32];
    struct complex_nested_struct* link;
} user_struct_t __attribute__((annotate("gengtype")));

/* TYPE_UNION: Named union types */
union annotated_union __attribute__((annotate("gengtype"))) {
    int integer;
    double floating;
    void* pointer;
    struct annotated_struct nested_struct;
};

/* TYPE_POINTER: Various pointer types with qualifiers */
volatile const int* const volatile_const_pointer __attribute__((unused));
struct annotated_struct* struct_pointer __attribute__((unused));
union annotated_union* union_pointer __attribute__((unused));
user_struct_t* user_struct_pointer __attribute__((unused));
void (* volatile volatile_func_ptr)(void) __attribute__((unused));

/* TYPE_ARRAY: Different array types */
int fixed_size_array[5][3] __attribute__((unused));
volatile char volatile_array[10] __attribute__((unused));
struct annotated_struct struct_array[4] __attribute__((unused));
union annotated_union union_array[2] __attribute__((unused));

/* Variable-length array (C99 feature) */
void use_vla(int size) {
    int vla[size] __attribute__((unused));
    /* Use in sizeof to prevent optimization */
    (void)sizeof(vla);
}

/* TYPE_CALLBACK: Function pointer types */
typedef int (*binary_op_t)(int, int) __attribute__((annotate("gengtype")));
typedef void (*complex_callback_t)(struct complex_nested_struct*, 
                                   union annotated_union*,
                                   binary_op_t);

/* Complex callback variable */
complex_callback_t complex_callback __attribute__((unused));

/* TYPE_LANG_STRUCT: GCC extension types */
/* Using __attribute__((transparent_union)) creates a special union type */
typedef union __attribute__((transparent_union)) {
    int i;
    float f;
} transparent_union_t;

/* Another GCC extension: vector types */
typedef int v4si __attribute__((vector_size(16)));

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) __attribute__((unused));
static void compare_types(void) {
    /* Compare various type combinations */
    int is_scalar_compatible = __builtin_types_compatible_p(int, float);
    int is_ptr_compatible = __builtin_types_compatible_p(int*, float*);
    int is_struct_compatible = __builtin_types_compatible_p(
        struct annotated_struct, 
        struct complex_nested_struct);
    int is_union_compatible = __builtin_types_compatible_p(
        union annotated_union,
        union internal_union);
    
    /* Compare with qualifiers */
    int is_qualified_compatible = __builtin_types_compatible_p(
        const int, 
        volatile int);
    int is_ptr_qualified_compatible = __builtin_types_compatible_p(
        int* const, 
        const int*);
    
    /* Use results to prevent dead code elimination */
    volatile int dummy __attribute__((unused)) = 
        is_scalar_compatible + is_ptr_compatible + 
        is_struct_compatible + is_union_compatible +
        is_qualified_compatible + is_ptr_qualified_compatible;
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of various types */
    struct annotated_struct my_struct __attribute__((unused)) = {
        .field1 = 1,
        .field2 = 3.14f,
        .field3 = "test"
    };
    
    union annotated_union my_union __attribute__((unused));
    my_union.integer = 42;
    
    user_struct_t my_user_struct __attribute__((unused));
    
    /* Array initialization */
    int matrix[2][2] __attribute__((unused)) = {{1, 2}, {3, 4}};
    
    /* Function pointer assignment */
    binary_op_t add_func = NULL;
    complex_callback = NULL;
    
    /* Use variable-length array */
    use_vla(10);
    
    /* Perform type comparisons */
    compare_types();
    
    /* Use sizeof with various types (including incomplete) */
    size_t sizes[] __attribute__((unused)) = {
        sizeof(_Bool),
        sizeof(int),
        sizeof(float),
        sizeof(char*),
        sizeof(struct annotated_struct),
        sizeof(union annotated_union),
        sizeof(user_struct_t),
        sizeof(int*),
        sizeof(int[5]),
        sizeof(binary_op_t),
        sizeof(transparent_union_t),
        sizeof(v4si)
    };
    
    /* Use pointers to incomplete types */
    struct forward_declared_struct* p_fwd_struct = NULL;
    union forward_declared_union* p_fwd_union = NULL;
    
    /* Prevent optimization of unused variables */
    (void)p_fwd_struct;
    (void)p_fwd_union;
    (void)add_func;
    
    return 0;
}

/* Additional incomplete type definitions (after use) */
struct forward_declared_struct {
    int complete_now;
};

union forward_declared_union {
    int complete_now;
};

/* Callback function implementations */
int sample_binary_op(int a, int b) {
    return a + b;
}

void sample_complex_callback(struct complex_nested_struct* s,
                            union annotated_union* u,
                            binary_op_t op) {
    if (s && u && op) {
        /* Do nothing meaningful, just use parameters */
        (void)s->fixed_array[0];
        (void)u->integer;
        (void)op(1, 2);
    }
}

/* Initialize function pointers */
void init_pointers(void) __attribute__((constructor));
void init_pointers(void) {
    complex_callback = sample_complex_callback;
}
