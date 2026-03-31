/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to exercise the type
 * enumeration logic in gengtype.cc, specifically targeting the switch
 * statement that counts occurrences of different type kinds.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x) 

/* Forward declarations to create pointer cycles and complex type graphs */
struct ForwardDeclared;
union ForwardUnion;

/* ==================== TYPE_SCALAR triggers ==================== */
struct ScalarContainer {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
    signed char schar_field;
    unsigned int uint_field;
    long double long_double_field;
};

/* ==================== TYPE_STRING triggers ==================== */
struct StringContainer {
    const char *string_literal;
    char *mutable_string;
    const char *const constant_string;
    char fixed_string[32];
};

/* ==================== TYPE_STRUCT triggers ==================== */
struct InnerStruct {
    int x;
    double y;
};

struct OuterStruct {
    struct InnerStruct inner;
    int outer_data;
};

/* Recursive structure for complex type graphs */
struct RecursiveStruct {
    int data;
    struct RecursiveStruct *next;  /* TYPE_POINTER */
};

/* ==================== TYPE_UNION triggers ==================== */
union SimpleUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
};

union NestedUnion {
    struct {
        int tag;
        union {
            int int_val;
            float float_val;
            char *string_val;  /* TYPE_POINTER to TYPE_STRING */
        } data;
    } tagged;
    long long raw_data;
};

/* ==================== TYPE_POINTER triggers ==================== */
struct PointerContainer {
    /* Basic pointers */
    int *int_ptr;
    char **char_ptr_ptr;  /* Pointer to pointer */
    
    /* Function pointers - TYPE_CALLBACK */
    int (*func_ptr)(int, char*);
    void (*void_func)(void);
    
    /* Pointers to complex types */
    struct InnerStruct *struct_ptr;
    union SimpleUnion *union_ptr;
    
    /* Pointer to forward declared type */
    struct ForwardDeclared *forward_ptr;
    
    /* Const and volatile pointers */
    const int *const_ptr;
    volatile char *volatile_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to function */
    int (*(*complex_func_ptr))(void);
};

/* ==================== TYPE_ARRAY triggers ==================== */
struct ArrayContainer {
    /* Fixed size arrays */
    int int_array[20];
    char char_array[50];
    float float_array[5][5];  /* Multi-dimensional */
    
    /* Array of pointers */
    void *ptr_array[8];
    
    /* Array of structures */
    struct InnerStruct struct_array[4];
    
    /* Array of unions */
    union SimpleUnion union_array[3];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ==================== TYPE_CALLBACK triggers ==================== */
typedef int (*callback_func_t)(const char *, void *);

struct CallbackContainer {
    callback_func_t handler;
    void (*startup)(void);
    void (*shutdown)(int);
    
    /* Array of callbacks */
    int (*callbacks[5])(void);
    
    /* Nested callback in union */
    union {
        void (*void_callback)(void);
        int (*int_callback)(int);
    } callback_union;
};

/* ==================== Complex nested type ==================== */
struct ComplexType {
    /* Scalar fields */
    int id;
    
    /* String field */
    const char *name;
    
    /* Nested structure */
    struct {
        int counter;
        struct InnerStruct data;
    } nested;
    
    /* Union field */
    union {
        int option_a;
        struct {
            float x, y;
        } option_b;
    } choice;
    
    /* Pointer to self (recursive) */
    struct ComplexType *self_ptr;
    
    /* Array of pointers to different types */
    void *heterogeneous_array[4];
    
    /* Callback */
    void (*notify)(struct ComplexType*);
    
    /* Multi-dimensional array */
    double matrix[3][3];
};

/* ==================== Forward declared types ==================== */
struct ForwardDeclared {
    int magic;
    union ForwardUnion *link;
};

union ForwardUnion {
    struct ForwardDeclared *struct_link;
    long long data;
};

/* ==================== GTY-annotated types ==================== */
/* These simulate types that would be processed by gengtype in real GCC */
GTY(()) struct GtyAnnotated {
    int gty_field;
    struct GtyAnnotated *gty_next;
    const char *gty_name;
};

GTY(()) union GtyUnion {
    struct GtyAnnotated *as_struct;
    callback_func_t as_callback;
};

/* ==================== Main driver ==================== */

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void *addr, size_t size) {
    /* Simple hash to ensure the function isn't optimized away */
    return (size_t)addr ^ size;
}

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarContainer scalar_inst = {0};
    struct StringContainer string_inst = {"literal", NULL, "const", "fixed"};
    struct OuterStruct outer_inst = {{{1, 2.0}, 3}};
    struct RecursiveStruct recursive_inst = {42, NULL};
    union SimpleUnion union_inst = {.as_int = 100};
    union NestedUnion nested_union_inst = {0};
    struct PointerContainer pointer_inst = {0};
    struct ArrayContainer *array_ptr = NULL;
    struct CallbackContainer callback_inst = {0};
    struct ComplexType complex_inst = {0};
    struct ForwardDeclared forward_inst = {999, NULL};
    struct GtyAnnotated gty_inst = {0, NULL, "test"};
    union GtyUnion gty_union_inst = {0};
    
    /* Take addresses to ensure types are referenced */
    volatile void *addresses[] = {
        &scalar_inst,
        &string_inst,
        &outer_inst,
        &recursive_inst,
        &union_inst,
        &nested_union_inst,
        &pointer_inst,
        &array_ptr,
        &callback_inst,
        &complex_inst,
        &forward_inst,
        &gty_inst,
        &gty_union_inst
    };
    
    /* Compute sizeof for all types */
    size_t size_sum = 0;
    
    size_sum += compute_checksum((void*)&scalar_inst, sizeof(struct ScalarContainer));
    size_sum += compute_checksum((void*)&string_inst, sizeof(struct StringContainer));
    size_sum += compute_checksum((void*)&outer_inst, sizeof(struct OuterStruct));
    size_sum += compute_checksum((void*)&recursive_inst, sizeof(struct RecursiveStruct));
    size_sum += compute_checksum((void*)&union_inst, sizeof(union SimpleUnion));
    size_sum += compute_checksum((void*)&nested_union_inst, sizeof(union NestedUnion));
    size_sum += compute_checksum((void*)&pointer_inst, sizeof(struct PointerContainer));
    size_sum += compute_checksum((void*)array_ptr, sizeof(struct ArrayContainer));
    size_sum += compute_checksum((void*)&callback_inst, sizeof(struct CallbackContainer));
    size_sum += compute_checksum((void*)&complex_inst, sizeof(struct ComplexType));
    size_sum += compute_checksum((void*)&forward_inst, sizeof(struct ForwardDeclared));
    size_sum += compute_checksum((void*)&gty_inst, sizeof(struct GtyAnnotated));
    size_sum += compute_checksum((void*)&gty_union_inst, sizeof(union GtyUnion));
    
    /* Access nested members to ensure full type traversal */
    scalar_inst.int_field = sizeof(scalar_inst);
    string_inst.mutable_string = (char*)&string_inst.fixed_string[0];
    outer_inst.inner.x = recursive_inst.data;
    complex_inst.self_ptr = &complex_inst;
    complex_inst.notify = NULL;
    
    /* Create pointer cycles */
    recursive_inst.next = &recursive_inst;
    
    /* Print checksum to prevent optimization */
    printf("Type analysis checksum: %zu\n", size_sum % 1000);
    printf("Number of types referenced: %zu\n", 
           sizeof(addresses)/sizeof(addresses[0]));
    
    return 0;
}
