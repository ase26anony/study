/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct incomplete_extern_struct;  /* TYPE_UNDEFINED */
extern int incomplete_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;          /* TYPE_UNDEFINED */
union forward_declared_union;            /* TYPE_UNDEFINED */

/* TYPE_SCALAR declarations with various qualifiers */
volatile const _Bool volatile_scalar_bool __attribute__((unused));
const int const_scalar_int __attribute__((unused)) = 42;
volatile float volatile_scalar_float __attribute__((unused));

/* TYPE_STRING declarations */
char* string_literal1 __attribute__((unused)) = "Hello";
const char* const_string_literal __attribute__((unused)) = "World";
volatile char* volatile_string_ptr __attribute__((unused));

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int field1;
    float field2;
    char* field3;
};

/* TYPE_UNION with annotation */
union annotated_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    long data1;
    double data2;
} user_struct_t;

/* Complex nested struct combining multiple type classifications */
struct complex_container __attribute__((annotate("gengtype"))) {
    /* TYPE_ARRAY inside struct */
    int int_array[10];
    
    /* TYPE_POINTER inside struct */
    void* data_ptr;
    
    /* TYPE_UNION inside struct */
    union {
        int option_a;
        float option_b;
    } choice;
    
    /* TYPE_CALLBACK inside struct */
    int (*callback_func)(int, char*);
    
    /* TYPE_ARRAY of TYPE_POINTER */
    struct annotated_struct* struct_ptr_array[5];
    
    /* TYPE_POINTER to incomplete type */
    struct forward_declared_struct* fwd_ptr;
};

/* TYPE_POINTER declarations with complex qualifiers */
volatile const int* const volatile_qualified_ptr __attribute__((unused));
int* const const_pointer __attribute__((unused));
const volatile void* complex_qual_ptr __attribute__((unused));

/* TYPE_ARRAY declarations */
int fixed_array[100] __attribute__((unused));
int variable_len_array __attribute__((unused)) [sizeof(int) * 10];
struct annotated_struct struct_array[3] __attribute__((unused));

/* TYPE_CALLBACK declarations with annotation */
typedef int (*complex_callback_t)(struct complex_container*, user_struct_t) 
    __attribute__((annotate("gengtype")));

complex_callback_t annotated_callback __attribute__((unused));

/* Another function pointer type */
void (*simple_callback)(void) __attribute__((unused));

/* Function using __builtin_types_compatible_p for type comparisons */
static void perform_type_comparisons(void) __attribute__((unused));
static void perform_type_comparisons(void) {
    /* Compare various type combinations to trigger classification */
    int is_same1 = __builtin_types_compatible_p(int, float);  /* scalar vs scalar */
    int is_same2 = __builtin_types_compatible_p(int*, float*); /* pointer vs pointer */
    int is_same3 = __builtin_types_compatible_p(struct annotated_struct*, 
                                               union annotated_union*);
    int is_same4 = __builtin_types_compatible_p(int[10], int*);
    int is_same5 = __builtin_types_compatible_p(user_struct_t, 
                                               struct complex_container);
    int is_same6 = __builtin_types_compatible_p(complex_callback_t, 
                                               void(*)(void));
    
    /* Use results to avoid dead code elimination */
    volatile int dummy __attribute__((unused)) = 
        is_same1 + is_same2 + is_same3 + is_same4 + is_same5 + is_same6;
}

/* Function to initialize and use function pointers */
static int sample_callback(struct complex_container* c, user_struct_t u) 
    __attribute__((unused));
static int sample_callback(struct complex_container* c, user_struct_t u) {
    return c->int_array[0] + (int)u.data1;
}

static void dummy_function(void) __attribute__((unused));
static void dummy_function(void) {
    /* Empty but referenced to prevent elimination */
}

int main(void) {
    /* Declare variables of our complex types */
    struct complex_container container __attribute__((unused));
    union annotated_union my_union __attribute__((unused));
    user_struct_t user_struct __attribute__((unused)) = {0};
    
    /* Initialize function pointers */
    annotated_callback = sample_callback;
    simple_callback = dummy_function;
    
    /* Use incomplete types in valid contexts */
    size_t incomplete_size = sizeof(struct forward_declared_struct*);
    struct forward_declared_struct** fwd_ptr_ptr = 0;
    
    /* Perform type comparisons */
    perform_type_comparisons();
    
    /* Use some variables to prevent dead code elimination */
    container.callback_func = 0;
    my_union.as_int = 42;
    user_struct.data1 = 100;
    
    /* Call through function pointer */
    if (simple_callback) {
        simple_callback();
    }
    
    /* Array and pointer operations */
    fixed_array[0] = 1;
    volatile_qualified_ptr = fixed_array;
    
    /* Use sizeof with various types */
    volatile size_t sizes __attribute__((unused)) = 
        sizeof(struct annotated_struct) + 
        sizeof(union annotated_union) +
        sizeof(user_struct_t) +
        sizeof(int*) +
        sizeof(int[10]) +
        sizeof(complex_callback_t);
    
    return 0;
}

/* Additional incomplete type definitions (after use) */
struct forward_declared_struct {
    int some_field;
};

union forward_declared_union {
    long data;
    void* ptr;
};
