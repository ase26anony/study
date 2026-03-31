/* 
 * Comprehensive type declarations to exercise GCC's gengtype type classification
 * Target: Trigger coverage of TYPE_* enumeration cases in gengtype.cc
 */

#include <stddef.h>

/* ==================== TYPE_UNDEFINED & TYPE_LANG_STRUCT ==================== */
/* Forward declarations creating incomplete/undefined types */
extern struct undefined_extern_struct;  /* TYPE_UNDEFINED */
extern int undefined_extern_array[];    /* TYPE_UNDEFINED */
struct forward_declared_struct;         /* TYPE_UNDEFINED until defined */
union forward_declared_union;           /* TYPE_UNDEFINED until defined */

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
__attribute__((unused)) _Bool scalar_bool = 0;                /* TYPE_SCALAR */
__attribute__((unused)) int scalar_int = 42;                  /* TYPE_SCALAR */
__attribute__((unused)) float scalar_float = 3.14f;           /* TYPE_SCALAR */
__attribute__((unused)) double scalar_double = 2.71828;       /* TYPE_SCALAR */
__attribute__((unused)) char scalar_char = 'A';               /* TYPE_SCALAR */
__attribute__((unused)) long scalar_long = 100L;              /* TYPE_SCALAR */
__attribute__((unused)) short scalar_short = 10;              /* TYPE_SCALAR */

/* ==================== TYPE_STRING ==================== */
/* String literals and character arrays */
__attribute__((unused)) const char* string_literal = "Hello, gengtype!";  /* TYPE_STRING */
__attribute__((unused)) char string_array[] = "Test string";              /* TYPE_STRING */
__attribute__((unused)) const char* const const_string_ptr = "Constant";  /* TYPE_STRING */

/* ==================== TYPE_STRUCT ==================== */
/* Named struct types with __attribute__((annotate("gengtype"))) */
struct __attribute__((annotate("gengtype"))) SimpleStruct {
    int x;
    float y;
    char z;
};

struct __attribute__((annotate("gengtype"))) NestedStruct {
    struct SimpleStruct inner;
    double extra;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* Typedef structs (TYPE_USER_STRUCT) */
typedef struct __attribute__((annotate("gengtype"))) {
    int id;
    char name[32];
} UserStruct;

typedef struct SimpleStruct SimpleStructTypedef;  /* Another TYPE_USER_STRUCT */

/* ==================== TYPE_UNION ==================== */
/* Named union types */
union __attribute__((annotate("gengtype"))) DataUnion {
    int as_int;
    float as_float;
    char* as_string;
    void* as_pointer;
};

union __attribute__((annotate("gengtype"))) TaggedUnion {
    int type_tag;
    struct {
        int tag;
        union DataUnion data;
    } tagged;
};

/* ==================== TYPE_POINTER ==================== */
/* Pointers to various types with qualifiers */
__attribute__((unused)) volatile const int* const volatile_pointer = &scalar_int;  /* TYPE_POINTER */
__attribute__((unused)) float* float_ptr = &scalar_float;                          /* TYPE_POINTER */
__attribute__((unused)) struct SimpleStruct* struct_ptr = 0;                       /* TYPE_POINTER */
__attribute__((unused)) union DataUnion* union_ptr = 0;                            /* TYPE_POINTER */
__attribute__((unused)) void (*func_ptr)(void) = 0;                                /* TYPE_POINTER */
__attribute__((unused)) char** double_ptr = 0;                                     /* TYPE_POINTER to TYPE_POINTER */

/* Pointer with multiple indirections */
__attribute__((unused)) int*** triple_int_ptr = 0;                                 /* TYPE_POINTER */

/* ==================== TYPE_ARRAY ==================== */
/* Fixed-size arrays */
__attribute__((unused)) int fixed_array[10];                                       /* TYPE_ARRAY */
__attribute__((unused)) struct SimpleStruct struct_array[5];                       /* TYPE_ARRAY */
__attribute__((unused)) union DataUnion union_array[3];                            /* TYPE_ARRAY */

/* Multi-dimensional arrays */
__attribute__((unused)) int matrix[3][4];                                          /* TYPE_ARRAY */
__attribute__((unused)) char* string_array_array[2][3];                            /* TYPE_ARRAY */

/* Variable-length array (in function scope) */
__attribute__((unused)) void use_vla(int size) {
    int vla[size];  /* TYPE_ARRAY */
    (void)vla;
}

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types (TYPE_CALLBACK) */
typedef int (*BinaryCallback)(int, int);                                          /* TYPE_CALLBACK */
typedef void (*VoidCallback)(struct SimpleStruct*, union DataUnion);              /* TYPE_CALLBACK */

/* Complex callback signature */
typedef int (*ComplexCallback)(
    const char*,
    struct SimpleStruct*,
    union DataUnion,
    BinaryCallback
) __attribute__((annotate("gengtype")));                                          /* TYPE_CALLBACK */

/* ==================== COMPLEX NESTED TYPE CONSTRUCTS ==================== */
/* Struct containing array of function pointers */
struct __attribute__((annotate("gengtype"))) Processor {
    BinaryCallback callbacks[5];
    ComplexCallback complex_cb;
    int state;
};

/* Union with pointer to struct */
union __attribute__((annotate("gengtype"))) Container {
    struct Processor* proc;
    UserStruct* user;
    void* data;
};

/* Typedef for complex nested type */
typedef struct __attribute__((annotate("gengtype"))) {
    union Container container;
    int (*processor)(struct Processor*);
    volatile const char* name;
} SystemType;

/* ==================== QUALIFIED TYPE DECLARATIONS ==================== */
/* Complex qualified types */
__attribute__((unused)) volatile const int* const restrict qualified_ptr = &scalar_int;
__attribute__((unused)) const struct SimpleStruct* const const_struct_ptr = 0;
__attribute__((unused)) volatile union DataUnion volatile_union_var = {0};

/* ==================== TYPE COMPARISONS ==================== */
/* Use __builtin_types_compatible_p for type comparisons */
#define CHECK_TYPE_COMPAT(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Variable declarations using our diverse types */
    struct SimpleStruct my_struct = {1, 2.0f, 'c'};
    union DataUnion my_union = {.as_int = 42};
    UserStruct my_user_struct = {100, "Test"};
    SystemType system = {0};
    
    /* Function pointer usage */
    BinaryCallback add_func = 0;
    ComplexCallback complex_func = 0;
    
    /* Array usage */
    int local_array[5] = {1, 2, 3, 4, 5};
    struct SimpleStruct local_struct_array[2] = {{1, 1.0f, 'a'}, {2, 2.0f, 'b'}};
    
    /* Pointer operations */
    int* int_ptr = &scalar_int;
    struct SimpleStruct* another_struct_ptr = &my_struct;
    
    /* Use incomplete types through pointers */
    struct forward_declared_struct* forward_ptr = 0;
    union forward_declared_union* forward_union_ptr = 0;
    
    /* Type compatibility checks (compile-time evaluations) */
    int compat1 = CHECK_TYPE_COMPAT(int, float);                /* Scalar vs scalar */
    int compat2 = CHECK_TYPE_COMPAT(int*, float*);              /* Pointer vs pointer */
    int compat3 = CHECK_TYPE_COMPAT(struct SimpleStruct*, union DataUnion*); /* Struct vs union pointer */
    int compat4 = CHECK_TYPE_COMPAT(int[5], int[10]);           /* Array vs array */
    int compat5 = CHECK_TYPE_COMPAT(BinaryCallback, void(*)(void)); /* Callback vs callback */
    
    /* Use sizeof with various types (including incomplete through pointers) */
    size_t sizes[] = {
        sizeof(scalar_int),
        sizeof(my_struct),
        sizeof(my_union),
        sizeof(int_ptr),
        sizeof(local_array),
        sizeof(forward_ptr),    /* Valid even with incomplete type */
        sizeof(BinaryCallback)
    };
    
    /* Ensure variables are used to prevent dead code elimination */
    (void)compat1; (void)compat2; (void)compat3; 
    (void)compat4; (void)compat5;
    (void)sizes;
    (void)system;
    (void)add_func;
    (void)complex_func;
    (void)another_struct_ptr;
    (void)forward_union_ptr;
    
    /* Call function with VLA */
    use_vla(10);
    
    /* Simple assignment to ensure processing */
    my_struct.x = compat1;
    my_union.as_int = 100;
    
    return 0;
}

/* ==================== COMPLETE THE FORWARD DECLARATIONS ==================== */
/* Define previously forward-declared types */
struct forward_declared_struct {
    int complete_now;
    float data;
};

union forward_declared_union {
    int as_int;
    char as_char[4];
};

/* External declarations (remain undefined) */
/* These keep TYPE_UNDEFINED status */
extern struct undefined_extern_struct;
extern int undefined_extern_array[];
