/* 
 * This program is designed to trigger coverage of specific type classification
 * logic in gengtype.cc by presenting a rich variety of type declarations and
 * constructs to GCC's internal type system during compilation.
 */

#include <stddef.h>

/* ========== 1. DIVERSE TYPE DECLARATIONS FOR CLASSIFICATION ========== */

/* TYPE_SCALAR: Basic scalar types */
_Bool __attribute__((unused)) scalar_bool = 0;
int __attribute__((unused)) scalar_int = 42;
float __attribute__((unused)) scalar_float = 3.14f;

/* TYPE_STRING: String literals */
char* __attribute__((unused)) string_literal = "Hello, gengtype!";
const char* __attribute__((unused)) const_string = "Constant string";

/* TYPE_STRUCT: Named struct types */
struct SimpleStruct {
    int x;
    float y;
} __attribute__((annotate("gengtype")));

/* TYPE_UNION: Named union types */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
} __attribute__((annotate("gengtype")));

/* TYPE_USER_STRUCT: Typedef struct */
typedef struct {
    char name[32];
    int id;
} UserStruct;

/* TYPE_POINTER: Pointers to various types */
int* __attribute__((unused)) int_ptr = &scalar_int;
float* __attribute__((unused)) float_ptr = &scalar_float;
char** __attribute__((unused)) string_ptr_ptr = &string_literal;
struct SimpleStruct* __attribute__((unused)) struct_ptr = 0;
union SimpleUnion* __attribute__((unused)) union_ptr = 0;
UserStruct* __attribute__((unused)) user_struct_ptr = 0;

/* TYPE_ARRAY: Fixed-size and conceptually variable-length arrays */
int __attribute__((unused)) fixed_array[10] = {0};
float __attribute__((unused)) multi_dim_array[5][3];
/* Variable-length array (VLA) type - will be used in sizeof */
typedef int VLAType[];

/* TYPE_CALLBACK: Function pointers */
typedef int (*SimpleCallback)(int, float);
typedef void (*ComplexCallback)(struct SimpleStruct*, union SimpleUnion*);

/* ========== 2. COMPLEX, NESTED TYPE CONSTRUCTS ========== */

/* Struct containing an array of function pointers (TYPE_STRUCT + TYPE_ARRAY + TYPE_CALLBACK) */
struct NestedStruct {
    SimpleCallback callbacks[5];
    ComplexCallback complex_cb;
    int count;
} __attribute__((annotate("gengtype")));

/* Union with pointer to struct (TYPE_UNION + TYPE_POINTER + TYPE_STRUCT) */
union NestedUnion {
    struct SimpleStruct* struct_ptr;
    UserStruct* user_struct_ptr;
    void* generic_ptr;
} __attribute__((annotate("gengtype")));

/* Complex typedef for function callback signature */
typedef union NestedUnion* (*UltraCallback)(
    struct NestedStruct*, 
    int (*)(int, int), 
    volatile const float
);

/* ========== 3. VOLATILE AND CONST QUALIFIERS ========== */

volatile const int __attribute__((unused)) volatile_const_int = 100;
volatile const int* const __attribute__((unused)) complex_ptr = &volatile_const_int;
const struct SimpleStruct* __attribute__((unused)) const_struct_ptr = 0;
volatile union SimpleUnion* __attribute__((unused)) volatile_union_ptr = 0;

/* ========== 4. EXTERN DECLARATIONS AND INCOMPLETE TYPES ========== */

/* Forward declarations (incomplete types) */
struct ForwardDeclaredStruct;
union ForwardDeclaredUnion;

/* Extern declarations without definitions */
extern struct ForwardDeclaredStruct* __attribute__((unused)) extern_struct_ptr;
extern int __attribute__((unused)) extern_array[];

/* Use incomplete types in pointer declarations */
struct ForwardDeclaredStruct** __attribute__((unused)) ptr_to_incomplete = 0;
union ForwardDeclaredUnion* __attribute__((unused)) incomplete_union_ptr = 0;

/* ========== 5. TYPE COMPARISONS USING __builtin_types_compatible_p ========== */

/* These comparisons may trigger internal type classification */
static void type_comparisons(void) __attribute__((unused));
static void type_comparisons(void) {
    /* Compare scalar vs pointer */
    int __attribute__((unused)) cmp1 = __builtin_types_compatible_p(int, int*);
    
    /* Compare struct vs union */
    int __attribute__((unused)) cmp2 = __builtin_types_compatible_p(
        struct SimpleStruct, 
        union SimpleUnion
    );
    
    /* Compare pointer types with different qualifiers */
    int __attribute__((unused)) cmp3 = __builtin_types_compatible_p(
        const int*, 
        volatile int*
    );
    
    /* Compare function pointer types */
    int __attribute__((unused)) cmp4 = __builtin_types_compatible_p(
        SimpleCallback, 
        ComplexCallback
    );
    
    /* Compare array types */
    int __attribute__((unused)) cmp5 = __builtin_types_compatible_p(
        int[10], 
        int[5]
    );
}

/* ========== 6. LANG_STRUCT AND UNDEFINED TYPE SCENARIOS ========== */

/* Using __typeof__ to create type expressions */
typedef __typeof__(scalar_int + scalar_float) __attribute__((unused)) ArithmeticType;

/* Anonymous struct/union in typedef */
typedef struct {
    int tag;
    union {
        int int_val;
        float float_val;
    } value;
} TaggedUnion;

/* ========== MAIN FUNCTION WITH NON-DEAD CODE ========== */

/* Simple callback function for demonstration */
static int example_callback(int a, float b) {
    return a + (int)b;
}

/* Another callback using complex types */
static void complex_example_callback(struct SimpleStruct* s, union SimpleUnion* u) {
    if (s && u) {
        s->x = u->as_int;
    }
}

int main(void) {
    /* Declare and initialize variables to ensure they're processed */
    struct SimpleStruct my_struct = {.x = 1, .y = 2.0f};
    union SimpleUnion my_union;
    UserStruct my_user_struct = {"test", 123};
    struct NestedStruct nested = {0};
    union NestedUnion nested_union = {0};
    
    /* Initialize function pointers */
    SimpleCallback cb = example_callback;
    ComplexCallback ccb = complex_example_callback;
    
    /* Use variables in non-dead code */
    my_union.as_int = 42;
    
    /* Call via function pointer */
    int __attribute__((unused)) result = cb(10, 20.5f);
    
    /* Use sizeof with various types (including incomplete via extern) */
    size_t __attribute__((unused)) sizes[] = {
        sizeof(my_struct),
        sizeof(my_union),
        sizeof(my_user_struct),
        sizeof(fixed_array),
        sizeof(int*),
        /* sizeof(extern_array) would be incomplete, but we can use the type */
        sizeof(struct SimpleStruct*),
        sizeof(SimpleCallback)
    };
    
    /* Use nested types */
    nested.callbacks[0] = example_callback;
    nested.complex_cb = complex_example_callback;
    nested_union.struct_ptr = &my_struct;
    
    /* Perform type comparisons */
    type_comparisons();
    
    /* Use volatile/const qualified variables */
    volatile const int __attribute__((unused)) read_volatile = volatile_const_int;
    
    /* Ensure string literal is referenced */
    const char __attribute__((unused)) first_char = string_literal[0];
    
    return 0;
}

/* Additional incomplete type declarations for linkage */
struct ForwardDeclaredStruct {
    int dummy;
};

union ForwardDeclaredUnion {
    int a;
    float b;
};

/* Define the extern array */
int extern_array[5] = {1, 2, 3, 4, 5};
