/* gengtype_trigger.c - Program to exercise GCC's internal type classification */

/* Forward declarations for incomplete types (TYPE_UNDEFINED/TYPE_LANG_STRUCT) */
extern struct undefined_extern;  /* TYPE_UNDEFINED */
extern int incomplete_array[];   /* TYPE_UNDEFINED array */
struct forward_declared;         /* Forward declaration */

/* TYPE_SCALAR declarations with qualifiers */
__attribute__((unused)) volatile const _Bool volatile_bool = 0;
__attribute__((unused)) const int const_int = 42;
__attribute__((unused)) volatile float volatile_float = 3.14f;

/* TYPE_STRING declarations */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";

/* TYPE_STRUCT with annotation */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int x;
    float y;
    char* name;
};

/* TYPE_UNION with annotation */
union __attribute__((annotate("gengtype"))) annotated_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT via typedef */
typedef struct {
    int id;
    char data[64];
} user_struct_t;

/* TYPE_POINTER variations with qualifiers */
__attribute__((unused)) volatile const int* const volatile_ptr_to_const = &const_int;
__attribute__((unused)) int* restrict restricted_ptr;
__attribute__((unused)) const struct annotated_struct* ptr_to_struct;
__attribute__((unused)) volatile union annotated_union* volatile ptr_to_union;

/* TYPE_ARRAY variations */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) const char* string_array[] = {"one", "two", "three"};
__attribute__((unused)) int variable_len_array(int n) {
    int vla[n];  /* TYPE_ARRAY with variable length */
    return sizeof(vla);
}

/* TYPE_CALLBACK - function pointer types */
typedef int (*simple_callback)(int, float);  /* TYPE_CALLBACK in typedef */
typedef void (*complex_callback)(struct annotated_struct*, union annotated_union*);

/* Annotated function pointer */
__attribute__((annotate("gengtype"))) 
int (*annotated_func_ptr)(char*, ...);

/* Complex nested type combining multiple classifications */
struct __attribute__((annotate("gengtype"))) complex_container {
    /* TYPE_STRUCT containing: */
    int scalar_field;                     /* TYPE_SCALAR */
    char* string_field;                   /* TYPE_STRING */
    struct annotated_struct nested_struct; /* TYPE_STRUCT */
    union annotated_union data_union;     /* TYPE_UNION */
    int* pointer_array[5];                /* TYPE_ARRAY of TYPE_POINTER */
    simple_callback callbacks[3];         /* TYPE_ARRAY of TYPE_CALLBACK */
    volatile const void* const volatile volatile_const_ptr; /* TYPE_POINTER with qualifiers */
};

/* Union with nested function pointer array */
union nested_union {
    int value;
    struct {
        float (*func_ptrs[2])(int);  /* TYPE_ARRAY of TYPE_CALLBACK */
        char* names[2];              /* TYPE_ARRAY of TYPE_STRING */
    } nested;
};

/* Function using __builtin_types_compatible_p for type comparisons */
static void compare_types(void) {
    /* Compare various type combinations to trigger classification */
    int is_same;
    
    /* TYPE_SCALAR vs TYPE_POINTER */
    is_same = __builtin_types_compatible_p(int, int*);
    
    /* TYPE_STRUCT vs TYPE_UNION */
    is_same = __builtin_types_compatible_p(struct annotated_struct, union annotated_union);
    
    /* TYPE_ARRAY vs TYPE_POINTER (array decay) */
    is_same = __builtin_types_compatible_p(int[10], int*);
    
    /* TYPE_USER_STRUCT vs underlying struct */
    is_same = __builtin_types_compatible_p(user_struct_t, struct { int id; char data[64]; });
    
    /* TYPE_CALLBACK comparisons */
    is_same = __builtin_types_compatible_p(simple_callback, complex_callback);
    
    /* TYPE_STRING (char*) vs TYPE_POINTER to char */
    is_same = __builtin_types_compatible_p(char*, const char*);
    
    /* Volatile/const qualified vs unqualified */
    is_same = __builtin_types_compatible_p(volatile int, int);
    is_same = __builtin_types_compatible_p(const int*, int*);
    
    /* Prevent unused variable warning */
    (void)is_same;
}

/* Function to exercise function pointers (TYPE_CALLBACK) */
static int sample_callback(int a, float b) {
    return a + (int)b;
}

static void process_callback(simple_callback cb) {
    if (cb) {
        cb(10, 20.5f);
    }
}

/* Main function with diverse type usage */
int main(void) {
    /* Declare variables of our complex types */
    struct complex_container container = {
        .scalar_field = 100,
        .string_field = "container",
        .volatile_const_ptr = (void*)0x1000
    };
    
    union nested_union nu = { .value = 42 };
    
    user_struct_t user_data = { .id = 1, .data = "test" };
    
    /* Initialize function pointer array */
    int (*func_ptr_array[2])(int) = { NULL, NULL };
    
    /* Use incomplete types in valid contexts */
    struct forward_declared* ptr_to_incomplete = 0;
    size_t incomplete_size = sizeof(ptr_to_incomplete);  /* Valid: pointer size */
    
    /* Exercise arrays */
    for (int i = 0; i < 10; i++) {
        fixed_array[i] = i * 2;
    }
    
    /* Use function pointers */
    simple_callback cb = sample_callback;
    process_callback(cb);
    
    /* Use annotated function pointer */
    annotated_func_ptr = (int (*)(char*, ...))printf;
    
    /* Trigger type comparisons */
    compare_types();
    
    /* Use variable length array */
    int vla_size = variable_len_array(20);
    
    /* Use volatile/const pointers */
    const int* read_ptr = &const_int;
    int value = *read_ptr;
    
    /* Use the container's nested types */
    container.callbacks[0] = sample_callback;
    container.pointer_array[0] = &container.scalar_field;
    
    /* Prevent unused variable warnings */
    (void)value;
    (void)vla_size;
    (void)incomplete_size;
    (void)user_data;
    (void)nu;
    (void)func_ptr_array;
    
    return 0;
}

/* Additional incomplete type definitions (if needed for linking) */
struct forward_declared {
    int dummy;
};
