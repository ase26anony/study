/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED */
union forward_declared_union;           /* TYPE_UNDEFINED */

/* TYPE_SCALAR declarations with qualifiers */
volatile const int volatile_const_int = 42;
static _Bool static_bool = 1;
float global_float __attribute__((unused));
double volatile global_volatile_double;

/* TYPE_STRING declarations */
const char* string_literal = "Hello, gengtype!";
char* mutable_string __attribute__((unused)) = "Mutable";
volatile const char* volatile_const_string = "Volatile const string";

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int x;
    float y;
    const char* name;
};

/* TYPE_UNION with annotation */
union annotated_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_pointer;
};

/* TYPE_USER_STRUCT (typedef struct) */
typedef struct {
    int id;
    char tag;
    double value;
} user_struct_t;

/* TYPE_POINTER variations */
int* int_pointer;
const float* const const_float_pointer_const;
volatile char* volatile volatile_char_pointer_volatile;
const volatile int* const volatile complex_qualified_pointer;

/* TYPE_ARRAY variations */
int fixed_array[10];
float variable_length_array[];
extern int extern_array[];
const char* const string_array[] = {"one", "two", "three"};
volatile int volatile_array[5][3];  /* Multi-dimensional */

/* TYPE_CALLBACK - Function pointer types */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, union annotated_union*);

/* Complex nested type combining multiple classifications */
struct complex_nested_struct __attribute__((annotate("gengtype"))) {
    /* TYPE_STRUCT field */
    struct annotated_struct inner_struct;
    
    /* TYPE_UNION field */
    union annotated_union data_union;
    
    /* TYPE_ARRAY of TYPE_POINTER to TYPE_CALLBACK */
    simple_callback_t callback_array[5];
    
    /* TYPE_POINTER to TYPE_ARRAY */
    int (*pointer_to_array)[10];
    
    /* TYPE_USER_STRUCT */
    user_struct_t user_data;
    
    /* TYPE_STRING */
    const char* description;
};

/* Another complex type: union with pointer to struct */
union pointer_union {
    struct complex_nested_struct* struct_ptr;
    void (*func_ptr)(void);
    const char** string_ptr_ptr;
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) __attribute__((unused));
static void compare_types(void) {
    /* Compare scalar types */
    int scalar_check = __builtin_types_compatible_p(int, float);
    scalar_check += __builtin_types_compatible_p(_Bool, char);
    
    /* Compare pointer types */
    int ptr_check = __builtin_types_compatible_p(int*, float*);
    ptr_check += __builtin_types_compatible_p(char**, const char**);
    
    /* Compare struct vs union */
    int struct_union_check = __builtin_types_compatible_p(
        struct annotated_struct, 
        union annotated_union
    );
    
    /* Compare array types */
    int array_check = __builtin_types_compatible_p(int[10], int[5]);
    array_check += __builtin_types_compatible_p(float[], double[]);
    
    /* Compare function pointers */
    int callback_check = __builtin_types_compatible_p(
        simple_callback_t,
        complex_callback_t
    );
    
    /* Compare with incomplete types */
    int incomplete_check = __builtin_types_compatible_p(
        struct forward_declared_struct*,
        struct undefined_extern_struct*
    );
    
    /* Use results to prevent dead code elimination */
    volatile int prevent_optimization = scalar_check + ptr_check + 
                                       struct_union_check + array_check + 
                                       callback_check + incomplete_check;
    (void)prevent_optimization;
}

/* Function using sizeof with incomplete types */
static size_t measure_incomplete_types(void) __attribute__((unused));
static size_t measure_incomplete_types(void) {
    /* These work because we're taking sizeof pointers, not the incomplete types */
    size_t s1 = sizeof(struct forward_declared_struct*);
    size_t s2 = sizeof(union forward_declared_union*);
    size_t s3 = sizeof(int (*)(void));  /* Function pointer */
    size_t s4 = sizeof(undefined_extern_array);  /* Incomplete array type */
    
    return s1 + s2 + s3 + s4;
}

/* Global variable with complex type */
struct complex_nested_struct global_complex_var __attribute__((unused));

/* Function pointer variable */
complex_callback_t global_callback __attribute__((unused));

/* Main function - minimal runtime logic, but ensures types are used */
int main(void) {
    /* Declare and initialize variables of various types */
    user_struct_t local_user_struct = {1, 'A', 3.14};
    
    /* Use function pointers */
    int (*local_func_ptr)(int, int) = 0;
    
    /* Use arrays */
    int local_array[3] = {1, 2, 3};
    float local_vla[local_array[0] + 2];  /* VLA */
    
    /* Use pointers */
    const int* local_ptr = &volatile_const_int;
    
    /* Use struct */
    struct annotated_struct local_struct = {10, 20.5, "local"};
    
    /* Use union */
    union annotated_union local_union;
    local_union.as_int = 100;
    
    /* Use nested complex type */
    struct complex_nested_struct local_complex;
    local_complex.inner_struct = local_struct;
    local_complex.data_union = local_union;
    local_complex.description = "Nested example";
    
    /* Call type comparison function */
    compare_types();
    
    /* Use sizeof on various types */
    size_t total_size = 
        sizeof(local_user_struct) +
        sizeof(local_array) +
        sizeof(local_ptr) +
        sizeof(local_struct) +
        sizeof(local_union) +
        sizeof(local_complex) +
        measure_incomplete_types();
    
    /* Trivial operation to use variables and prevent dead code elimination */
    volatile int result = (int)total_size + local_array[0] + local_union.as_int;
    
    /* Use function pointer (even if NULL) */
    if (local_func_ptr) {
        result = local_func_ptr(result, 2);
    }
    
    return result > 0 ? 0 : 1;
}

/* Additional incomplete type definitions (after use) */
struct forward_declared_struct {
    int x;
    struct forward_declared_struct* next;  /* Self-referential */
};

union forward_declared_union {
    int i;
    struct forward_declared_struct* s;
};

/* Array definition to complete incomplete declaration */
float variable_length_array[8];  /* Now complete */

/* Function definitions for callbacks */
static int sample_callback(int a, float b) __attribute__((unused));
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void another_callback(struct annotated_struct* s, union annotated_union* u) 
    __attribute__((unused));
static void another_callback(struct annotated_struct* s, union annotated_union* u) {
    if (s) s->x++;
    if (u) u->as_float = 0.0f;
}
