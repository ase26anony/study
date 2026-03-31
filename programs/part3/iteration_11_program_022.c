/* 
 * Complex type declarations to exercise GCC's gengtype type classification
 * Targeting uncovered switch cases in gengtype.cc lines 182-213
 */

#include <stddef.h>

/* ========== TYPE_UNDEFINED and incomplete types ========== */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED array */
struct forward_declared_struct;         /* Forward declaration */

/* ========== TYPE_SCALAR declarations ========== */
__attribute__((unused)) _Bool scalar_bool = 0;
__attribute__((unused)) int scalar_int = 42;
__attribute__((unused)) float scalar_float = 3.14f;
__attribute__((unused)) volatile const long scalar_volatile_const = 100L;

/* ========== TYPE_STRING declarations ========== */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";
__attribute__((unused)) char* mutable_string = "Mutable";
__attribute__((unused)) volatile const char* volatile_const_string = "VolatileConst";

/* ========== TYPE_STRUCT with annotation ========== */
struct __attribute__((annotate("gengtype"))) annotated_struct {
    int field1;
    float field2;
    const char* field3;
};

/* ========== TYPE_USER_STRUCT (typedef struct) ========== */
typedef struct {
    int x;
    int y;
} point_t;

typedef struct __attribute__((annotate("gengtype"))) {
    point_t position;
    float velocity[3];
    const char* name;
} entity_t;

/* ========== TYPE_UNION with annotation ========== */
union __attribute__((annotate("gengtype"))) data_union {
    int as_int;
    float as_float;
    void* as_pointer;
    char as_bytes[8];
};

/* ========== TYPE_POINTER with various qualifiers ========== */
__attribute__((unused)) int* pointer_to_int;
__attribute__((unused)) const int* pointer_to_const_int;
__attribute__((unused)) volatile int* volatile pointer_to_volatile_int;
__attribute__((unused)) volatile const int* const volatile_const_pointer = &scalar_int;
__attribute__((unused)) struct annotated_struct* struct_pointer;
__attribute__((unused)) union data_union* union_pointer;
__attribute__((unused)) point_t* user_struct_pointer;

/* ========== TYPE_ARRAY declarations ========== */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) float volatile_float_array[5];
__attribute__((unused)) const char* string_array[] = {"one", "two", "three"};
__attribute__((unused)) struct annotated_struct struct_array[3];
__attribute__((unused)) int multi_dim_array[2][3][4];

/* Variable Length Array (VLA) - TYPE_ARRAY */
void use_vla(int size) {
    __attribute__((unused)) int vla[size];
    /* Use VLA to prevent optimization */
    for (int i = 0; i < size; i++) {
        vla[i] = i * i;
    }
}

/* ========== TYPE_CALLBACK (function pointers) ========== */
typedef int (*simple_callback_t)(int, int);
typedef void (*complex_callback_t)(struct annotated_struct*, union data_union*);

/* Annotated function pointer type */
typedef __attribute__((annotate("gengtype"))) 
        void (*annotated_callback_t)(const char*, int);

/* Function pointer declarations */
__attribute__((unused)) simple_callback_t callback_func;
__attribute__((unused)) complex_callback_t complex_callback;
__attribute__((unused)) annotated_callback_t annotated_callback;

/* ========== Complex nested type constructs ========== */

/* Struct containing array of function pointers */
struct processor {
    int id;
    /* Array of 5 function pointers */
    simple_callback_t handlers[5];
    /* Pointer to union */
    union data_union* data;
};

/* Union with pointer to struct */
union container {
    struct processor* proc;
    entity_t* entity;
    annotated_callback_t callback;
};

/* Typedef for complex function callback signature */
typedef int (*nested_callback_t)(struct processor*, union container, point_t*);

/* Struct with nested union field */
struct nested_example {
    int tag;
    union {
        int int_value;
        float float_value;
        struct processor* proc_ptr;
        nested_callback_t callback;
    } data;
    /* Array of pointers to various types */
    void* ptr_array[4];
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* GCC internal types might be exposed through extensions */
#ifdef __GNUC__
struct __attribute__((annotate("gengtype"))) lang_specific {
    __extension__ __int128_t wide_int;
    vector int simd_vector __attribute__((vector_size(16)));
};
#endif

/* ========== Helper functions ========== */
int add(int a, int b) {
    return a + b;
}

void process_entity(struct annotated_struct* s, union data_union* u) {
    if (s && u) {
        /* Do nothing meaningful, just use parameters */
        (void)s->field1;
        (void)u->as_int;
    }
}

void log_message(const char* msg, int severity) {
    (void)msg;
    (void)severity;
}

/* ========== Type comparison using __builtin_types_compatible_p ========== */
static void perform_type_comparisons(void) {
    /* Compare scalar vs pointer */
    __attribute__((unused)) _Bool cmp1 = __builtin_types_compatible_p(int, int*);
    __attribute__((unused)) _Bool cmp2 = __builtin_types_compatible_p(float, float*);
    
    /* Compare struct vs union */
    __attribute__((unused)) _Bool cmp3 = __builtin_types_compatible_p(
        struct annotated_struct, union data_union);
    
    /* Compare pointer types with different qualifiers */
    __attribute__((unused)) _Bool cmp4 = __builtin_types_compatible_p(
        int*, const int*);
    __attribute__((unused)) _Bool cmp5 = __builtin_types_compatible_p(
        volatile int*, const volatile int*);
    
    /* Compare array types */
    __attribute__((unused)) _Bool cmp6 = __builtin_types_compatible_p(
        int[10], int[5]);
    __attribute__((unused)) _Bool cmp7 = __builtin_types_compatible_p(
        int*, int[]);
    
    /* Compare function pointer types */
    __attribute__((unused)) _Bool cmp8 = __builtin_types_compatible_p(
        simple_callback_t, complex_callback_t);
    
    /* Compare user-defined struct types */
    __attribute__((unused)) _Bool cmp9 = __builtin_types_compatible_p(
        point_t, entity_t);
    
    /* Compare with incomplete types */
    __attribute__((unused)) _Bool cmp10 = __builtin_types_compatible_p(
        struct forward_declared_struct*, 
        struct undefined_extern_struct*);
}

/* ========== Main function ========== */
int main(void) {
    /* Declare and initialize variables of various types */
    struct annotated_struct my_struct = {1, 2.0f, "test"};
    union data_union my_union;
    point_t my_point = {10, 20};
    entity_t my_entity = {{0, 0}, {1.0f, 2.0f, 3.0f}, "Entity1"};
    struct processor my_processor = {0};
    struct nested_example nested = {0};
    
    /* Initialize function pointers */
    callback_func = add;
    complex_callback = process_entity;
    annotated_callback = log_message;
    
    /* Use variables to prevent dead code elimination */
    my_union.as_int = 42;
    
    /* Call function with VLA */
    use_vla(5);
    
    /* Use pointers */
    struct_pointer = &my_struct;
    union_pointer = &my_union;
    user_struct_pointer = &my_point;
    
    /* Use arrays */
    fixed_array[0] = scalar_int;
    volatile_float_array[1] = scalar_float;
    
    /* Call via function pointer */
    if (callback_func) {
        int result = callback_func(10, 20);
        fixed_array[1] = result;
    }
    
    /* Use sizeof with various types (including incomplete) */
    __attribute__((unused)) size_t sz1 = sizeof(struct annotated_struct);
    __attribute__((unused)) size_t sz2 = sizeof(union data_union);
    __attribute__((unused)) size_t sz3 = sizeof(point_t);
    __attribute__((unused)) size_t sz4 = sizeof(fixed_array);
    __attribute__((unused)) size_t sz5 = sizeof(callback_func);
    __attribute__((unused)) size_t sz6 = sizeof(struct forward_declared_struct*);
    
    /* Perform type comparisons */
    perform_type_comparisons();
    
    /* Use nested structure */
    nested.tag = 1;
    nested.data.int_value = 100;
    nested.ptr_array[0] = &my_struct;
    nested.ptr_array[1] = &my_union;
    nested.ptr_array[2] = callback_func;
    
    /* Return success */
    return 0;
}

/* Additional incomplete type usage */
struct forward_declared_struct* forward_ptr = NULL;
int* extern_array_ptr = undefined_extern_array;
