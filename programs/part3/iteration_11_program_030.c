/* gengtype_test.c - Comprehensive type declarations to exercise GCC's type classification */
#include <stddef.h>

/* ========== 1. DIVERSE TYPE DECLARATIONS ========== */

/* TYPE_SCALAR: Basic scalar types */
__attribute__((unused)) _Bool bool_var;
__attribute__((unused)) int int_var;
__attribute__((unused)) float float_var;
__attribute__((unused)) double double_var;
__attribute__((unused)) long long long_long_var;

/* TYPE_STRING: String literals and char pointers */
__attribute__((unused)) char* string_literal = "Hello, gengtype!";
__attribute__((unused)) const char* const_string = "Constant string";

/* TYPE_STRUCT: Named struct types */
__attribute__((annotate("gengtype")))
struct SimpleStruct {
    int x;
    float y;
};

/* TYPE_UNION: Named union types */
__attribute__((annotate("gengtype")))
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    char name[32];
    int id;
} __attribute__((annotate("gengtype"))) UserStruct;

/* TYPE_POINTER: Pointers to various types */
__attribute__((unused)) int* int_ptr;
__attribute__((unused)) float* float_ptr;
__attribute__((unused)) struct SimpleStruct* struct_ptr;
__attribute__((unused)) union SimpleUnion* union_ptr;
__attribute__((unused)) void (*func_ptr)(void);

/* TYPE_ARRAY: Fixed-size and variable-length arrays */
__attribute__((unused)) int fixed_array[10];
__attribute__((unused)) char char_array[] = {'a', 'b', 'c'};
__attribute__((unused)) struct SimpleStruct struct_array[5];

/* TYPE_CALLBACK: Function pointers with different signatures */
typedef int (*BinaryOp)(int, int);
typedef void (*Callback)(void* data, int result);
typedef const char* (*StringProcessor)(const char*);

/* ========== 2. COMPLEX NESTED TYPE CONSTRUCTS ========== */

/* Struct containing array of function pointers */
__attribute__((annotate("gengtype")))
struct ComplexStruct {
    BinaryOp operations[4];
    Callback completion;
    int (*matrix_calc[2][3])(float, float);
};

/* Union with pointer to struct */
__attribute__((annotate("gengtype")))
union ComplexUnion {
    struct ComplexStruct* nested_struct;
    BinaryOp (*op_table)[4];
    void* generic_data;
};

/* Typedef for complex function callback */
typedef union ComplexUnion* (*FactoryFunc)(
    int count, 
    __attribute__((annotate("gengtype"))) struct SimpleStruct config
);

/* Nested declaration using the complex types */
__attribute__((unused)) 
struct Container {
    struct ComplexStruct processor;
    union ComplexUnion data_holder;
    FactoryFunc factory;
    volatile const int* const read_only_ptr;
};

/* ========== 3. VOLATILE AND CONST QUALIFIERS ========== */

/* Complex qualified pointers */
__attribute__((unused)) volatile const int* const volatile_qualified;
__attribute__((unused)) const struct SimpleStruct* const const_struct_ptr;
__attribute__((unused)) volatile union SimpleUnion* volatile volatile_union_ptr;

/* Qualified function pointer */
__attribute__((unused)) 
int (* const volatile volatile_func_ptr)(volatile int*, const float) = 0;

/* ========== 4. EXTERN AND INCOMPLETE TYPES ========== */

/* Forward declarations (incomplete types) */
struct ForwardDeclared;
union IncompleteUnion;

/* Extern declarations without definitions */
extern struct UndefinedStruct undefined_struct;
extern int external_array[];

/* Pointer to incomplete type */
__attribute__((unused)) struct ForwardDeclared* forward_ptr;
__attribute__((unused)) union IncompleteUnion* incomplete_union_ptr;

/* ========== 5. TYPE COMPARISONS ========== */

/* Use __builtin_types_compatible_p for type comparisons */
#define CHECK_COMPATIBLE(t1, t2) \
    __builtin_types_compatible_p(t1, t2)

/* Force type comparisons that may trigger classification */
static inline void force_type_checks(void) {
    /* These comparisons should trigger type classification */
    int check1 = CHECK_COMPATIBLE(typeof(int_var), typeof(float_var));
    int check2 = CHECK_COMPATIBLE(typeof(struct_ptr), typeof(union_ptr));
    int check3 = CHECK_COMPATIBLE(typeof(func_ptr), typeof(BinaryOp));
    int check4 = CHECK_COMPATIBLE(typeof(fixed_array), typeof(char_array));
    
    /* Prevent dead code elimination */
    __attribute__((unused)) volatile int dummy = check1 + check2 + check3 + check4;
}

/* ========== 6. FUNCTION DECLARATIONS ========== */

/* Function using various types */
static int process_data(
    __attribute__((annotate("gengtype"))) struct ComplexStruct* cs,
    const UserStruct* user,
    Callback cb
) {
    if (cs && user && cb) {
        /* Use the types to prevent optimization */
        return cs->operations[0](user->id, 42);
    }
    return 0;
}

/* Function pointer variable */
__attribute__((unused)) 
int (*processor)(struct ComplexStruct*, const UserStruct*, Callback) = process_data;

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Declare and initialize variables */
    struct SimpleStruct ss = {.x = 1, .y = 3.14f};
    union SimpleUnion su = {.as_int = 42};
    UserStruct us = {.name = "Test", .id = 100};
    struct ComplexStruct cs = {0};
    
    /* Use volatile/const qualified variables */
    volatile const int local_const = 100;
    const struct SimpleStruct* local_struct_ptr = &ss;
    
    /* Use incomplete type pointers */
    struct ForwardDeclared* local_forward = (struct ForwardDeclared*)&local_const;
    
    /* Use arrays */
    fixed_array[0] = 1;
    struct_array[0].x = 10;
    
    /* Use function pointers */
    BinaryOp add = 0;  /* Would normally point to actual function */
    if (add) {
        int result = add(1, 2);
        __attribute__((unused)) volatile int unused = result;
    }
    
    /* Force type comparisons */
    force_type_checks();
    
    /* Use sizeof with various types (including incomplete) */
    __attribute__((unused)) size_t sizes[] = {
        sizeof(_Bool),
        sizeof(struct SimpleStruct),
        sizeof(union SimpleUnion),
        sizeof(UserStruct),
        sizeof(int*),
        sizeof(fixed_array),
        sizeof(BinaryOp),
        sizeof(struct ForwardDeclared*)  /* Pointer to incomplete type */
    };
    
    /* Call function via pointer */
    if (processor) {
        int ret = processor(&cs, &us, 0);
        __attribute__((unused)) volatile int store = ret;
    }
    
    /* Return statement ensures all paths return */
    return 0;
}

/* ========== ADDITIONAL DECLARATIONS ========== */

/* Define previously forward-declared types */
struct ForwardDeclared {
    int placeholder;
};

union IncompleteUnion {
    long long_data;
    double double_data;
};

/* Global variable using TYPE_LANG_STRUCT-like pattern */
__attribute__((unused))
static struct {
    int lang_specific;
    void* lang_data;
} lang_struct_instance = {0};

/* Array of function pointers */
static int (*func_array[])(int, int) = {0, 0, 0};

/* Complex const-volatile combination */
__attribute__((unused))
const volatile int* volatile const* complex_ptr_chain = 0;
