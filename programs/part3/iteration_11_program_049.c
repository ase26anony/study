/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED */
union forward_declared_union;           /* TYPE_UNDEFINED */

/* TYPE_SCALAR declarations with qualifiers */
volatile const int volatile_const_scalar = 42;
static _Bool static_bool_scalar = 1;
float float_scalar __attribute__((unused));
double double_scalar;

/* TYPE_STRING declarations */
const char* string_literal1 = "Hello, gengtype!";
char* mutable_string __attribute__((unused)) = "Mutable";
volatile char* volatile_string_ptr = "Volatile string";

/* Annotated struct (TYPE_STRUCT with annotation) */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int x;
    float y;
    const char* name;
};

/* Regular struct (TYPE_STRUCT) */
struct regular_struct {
    short s;
    long l;
    volatile int vi;
};

/* Annotated union (TYPE_UNION with annotation) */
union annotated_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* Regular union (TYPE_UNION) */
union regular_union {
    double d;
    unsigned long ul;
    const char* str;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int id;
    struct regular_struct nested;
    union annotated_union data;
} user_struct_t;

/* Another typedef struct with annotation */
typedef struct __attribute__((annotate("gengtype"))) {
    user_struct_t items[4];
    int count;
} container_t;

/* TYPE_POINTER declarations with various qualifiers */
const int* const const_int_ptr_const = &volatile_const_scalar;
volatile const int* volatile_const_int_ptr;
int* restrict restricted_ptr __attribute__((unused));
struct regular_struct* struct_ptr;
union regular_union* union_ptr;
void (*volatile volatile_func_ptr)(void);

/* TYPE_ARRAY declarations */
int fixed_array[10] = {0};
int variable_array[] = {1, 2, 3, 4, 5};
const char* string_array[] = {"one", "two", "three"};
struct regular_struct struct_array[3];
union annotated_union union_array[2] __attribute__((unused));

/* Multi-dimensional arrays */
int matrix[3][4];
const float const_matrix[2][2] = {{1.0f, 2.0f}, {3.0f, 4.0f}};

/* TYPE_CALLBACK - Function pointer types */
typedef int (*binary_op_t)(int, int);
typedef void (*callback_t)(const char*, void*);
typedef struct regular_struct* (*struct_factory_t)(int);

/* Complex nested type combining multiple classifications */
struct complex_nested __attribute__((annotate("gengtype"))) {
    /* Array of function pointers (TYPE_ARRAY of TYPE_CALLBACK) */
    binary_op_t operations[5];
    
    /* Pointer to union containing struct pointer */
    union {
        struct regular_struct* rs_ptr;
        user_struct_t* us_ptr;
    } *union_ptr_field;
    
    /* Function pointer returning pointer to array */
    int (*(*complex_callback)(int))[10];
    
    /* Nested anonymous struct with volatile member */
    struct {
        volatile int counter;
        const char* volatile volatile_name;
    } inner;
    
    /* Flexible array member of function pointers */
    callback_t dynamic_callbacks[];
};

/* Global variables using the diverse types */
user_struct_t global_user_struct = {1, {2, 3L, 4}, {.as_int = 42}};
container_t global_container __attribute__((unused));
struct complex_nested* global_complex_ptr = NULL;

/* Function using __builtin_types_compatible_p for type comparisons */
static void perform_type_comparisons(void) {
    /* Compare scalar types */
    int scalar_check1 = __builtin_types_compatible_p(int, float);
    int scalar_check2 = __builtin_types_compatible_p(_Bool, char);
    
    /* Compare pointer types */
    int ptr_check1 = __builtin_types_compatible_p(int*, const int*);
    int ptr_check2 = __builtin_types_compatible_p(void*, char*);
    
    /* Compare struct/union types */
    int struct_check = __builtin_types_compatible_p(
        struct regular_struct, 
        struct annotated_struct
    );
    int union_check = __builtin_types_compatible_p(
        union regular_union,
        union annotated_union
    );
    
    /* Compare array types */
    int array_check1 = __builtin_types_compatible_p(int[10], int[]);
    int array_check2 = __builtin_types_compatible_p(char*[], const char*[]);
    
    /* Compare function pointer types */
    typedef void (*func1_t)(void);
    typedef void (*func2_t)(int);
    int callback_check = __builtin_types_compatible_p(func1_t, func2_t);
    
    /* Compare with incomplete types */
    int incomplete_check = __builtin_types_compatible_p(
        struct forward_declared_struct*,
        struct undefined_extern_struct*
    );
    
    /* Suppress unused variable warnings */
    (void)scalar_check1; (void)scalar_check2;
    (void)ptr_check1; (void)ptr_check2;
    (void)struct_check; (void)union_check;
    (void)array_check1; (void)array_check2;
    (void)callback_check; (void)incomplete_check;
}

/* Function pointers and callbacks */
static int add(int a, int b) { return a + b; }
static int multiply(int a, int b) { return a * b; }
static void log_message(const char* msg, void* data) {
    (void)msg; (void)data; /* Suppress unused parameter warnings */
}

/* Main function with diverse type usage */
int main(void) {
    /* Local variables using all type classifications */
    volatile const short local_scalar = 255;
    const char* local_string = "Local string";
    struct regular_struct local_struct = {1, 2L, 3};
    union annotated_union local_union = {.as_float = 3.14f};
    user_struct_t local_user_struct = {2, {3, 4L, 5}, {.as_ptr = NULL}};
    int* local_ptr = fixed_array;
    int local_array[5] = {10, 20, 30, 40, 50};
    binary_op_t local_callback = add;
    
    /* Use incomplete types */
    struct forward_declared_struct* incomplete_ptr = NULL;
    extern struct undefined_extern_struct* extern_ptr;
    (void)extern_ptr;
    
    /* Perform type comparisons */
    perform_type_comparisons();
    
    /* Use sizeof with various types (including incomplete in pointers) */
    size_t sizes[] = {
        sizeof(local_scalar),
        sizeof(local_string),
        sizeof(local_struct),
        sizeof(local_union),
        sizeof(local_user_struct),
        sizeof(local_ptr),
        sizeof(local_array),
        sizeof(local_callback),
        sizeof(incomplete_ptr),  /* Valid even for incomplete types */
        sizeof(struct forward_declared_struct*)  /* Pointer to incomplete */
    };
    
    /* Use the types to prevent dead code elimination */
    local_struct.vi = (int)sizes[0];
    local_union.as_int = (int)local_struct.vi;
    local_ptr = &local_array[local_union.as_int % 5];
    
    /* Call via function pointer */
    int result = local_callback(local_array[0], local_array[1]);
    local_user_struct.nested.s = (short)result;
    
    /* Use volatile to prevent optimization */
    volatile int vol_result = result;
    (void)vol_result;
    
    /* Array of function pointers */
    binary_op_t ops[] = {add, multiply};
    result = ops[1](result, 2);
    
    /* Complex nested type usage */
    struct complex_nested complex_instance = {
        .operations = {add, multiply, add, multiply, add},
        .union_ptr_field = NULL,
        .complex_callback = NULL,
        .inner = {.counter = 0, .volatile_name = "test"}
    };
    
    /* Use the complex type */
    complex_instance.operations[0](1, 2);
    complex_instance.inner.counter++;
    
    return 0;
}

/* Additional external declarations for incomplete types */
extern int external_incomplete_array[];
extern struct external_incomplete_struct;

/* GCC-specific attributes on function to potentially trigger more processing */
void __attribute__((annotate("gengtype"), noinline)) 
annotated_function(void* param __attribute__((unused))) {
    /* Empty but annotated */
}

/* Variadic function pointer type */
typedef int (*variadic_func_t)(int, ...);
variadic_func_t variadic_ptr __attribute__((unused));
