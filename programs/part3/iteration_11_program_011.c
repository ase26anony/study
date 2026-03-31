/* gengtype_coverage.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct undefined_extern;  /* TYPE_UNDEFINED */
extern int extern_array[];       /* TYPE_UNDEFINED for incomplete array */
struct forward_declared;         /* Forward declaration */

/* TYPE_SCALAR declarations with qualifiers */
volatile const _Bool volatile_bool __attribute__((unused));
const int const_int __attribute__((unused)) = 42;
volatile float volatile_float __attribute__((unused));

/* TYPE_STRING declarations */
char* string_literal __attribute__((unused)) = "Hello, gengtype!";
const char* const const_string_ptr __attribute__((unused)) = "Constant";

/* TYPE_STRUCT with annotation */
struct annotated_struct __attribute__((annotate("gengtype"))) {
    int x;
    float y;
    char* str;
};

/* TYPE_UNION with nested complexity */
union complex_union __attribute__((annotate("gengtype"))) {
    int as_int;
    float as_float;
    void* as_pointer;
    struct {
        char a;
        char b;
    } nested;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    long id;
    double value;
    union complex_union data;
} user_struct_t;

/* TYPE_POINTER variations */
volatile const int* const volatile_const_ptr __attribute__((unused));
int* plain_ptr __attribute__((unused));
void* void_ptr __attribute__((unused));
struct annotated_struct* struct_ptr __attribute__((unused));
user_struct_t* user_struct_ptr __attribute__((unused));

/* TYPE_ARRAY variations */
int fixed_array[10] __attribute__((unused));
int variable_array[] __attribute__((unused)) = {1, 2, 3, 4, 5};
struct annotated_struct struct_array[5] __attribute__((unused));
int* pointer_array[8] __attribute__((unused));

/* TYPE_CALLBACK - function pointers */
typedef int (*simple_callback_t)(int, float);
typedef void (*complex_callback_t)(struct annotated_struct*, user_struct_t*);

/* Annotated function pointer */
int (*annotated_callback)(char*, int) __attribute__((annotate("gengtype")));

/* Nested type combining multiple classifications */
struct container_struct __attribute__((annotate("gengtype"))) {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_STRING */
    char* name;
    
    /* TYPE_UNION */
    union {
        int num;
        void* ptr;
    } choice;
    
    /* TYPE_ARRAY of TYPE_CALLBACK */
    simple_callback_t callbacks[4];
    
    /* TYPE_POINTER to TYPE_ARRAY */
    int (*matrix_ptr)[3][3];
    
    /* TYPE_USER_STRUCT */
    user_struct_t user_data;
};

/* Complex callback signature */
typedef union complex_union* (*nested_callback_t)(
    struct container_struct*,
    complex_callback_t
);

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) __attribute__((unused));
static void compare_types(void) {
    /* Trigger type classification through compatibility checks */
    int check1 = __builtin_types_compatible_p(int, float);  /* scalar vs scalar */
    int check2 = __builtin_types_compatible_p(int*, float*); /* pointer vs pointer */
    int check3 = __builtin_types_compatible_p(
        struct annotated_struct, 
        union complex_union
    ); /* struct vs union */
    int check4 = __builtin_types_compatible_p(
        int[10], 
        int*
    ); /* array vs pointer */
    int check5 = __builtin_types_compatible_p(
        simple_callback_t,
        complex_callback_t
    ); /* callback vs callback */
    
    /* Use results to avoid dead code elimination */
    volatile int dummy = check1 + check2 + check3 + check4 + check5;
    (void)dummy;
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of our complex types */
    struct container_struct container __attribute__((unused));
    user_struct_t user_data __attribute__((unused));
    union complex_union data_union __attribute__((unused));
    
    /* Initialize some values to avoid dead code */
    container.counter = 100;
    container.name = "Container";
    user_data.id = 12345;
    user_data.value = 3.14159;
    
    /* Use sizeof with various types (works with incomplete types for pointers) */
    size_t sizes[] = {
        sizeof(struct undefined_extern*),  /* Pointer to incomplete type */
        sizeof(extern_array),              /* Incomplete array */
        sizeof(struct container_struct),
        sizeof(user_struct_t),
        sizeof(simple_callback_t),
        sizeof(int volatile const*)
    };
    
    /* Use function pointer */
    simple_callback_t my_callback = 0;
    if (my_callback) {
        my_callback(1, 2.0f);  /* Not executed, but keeps reference */
    }
    
    /* Call type comparison function */
    compare_types();
    
    /* Use volatile to prevent optimization */
    volatile int result = (int)sizes[0] + container.counter;
    
    return result > 0 ? 0 : 1;
}

/* Additional incomplete type definitions (after use) */
struct forward_declared {
    int x;
    int y;
};

/* Function definitions for callbacks (if needed) */
int sample_callback(int a, float b) {
    return a + (int)b;
}

void complex_callback(struct annotated_struct* s, user_struct_t* u) {
    if (s && u) {
        s->x = (int)u->value;
    }
}
