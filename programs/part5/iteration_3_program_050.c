/* test_gengtype_coverage.c
 * 
 * This test defines complex nested data structures to exercise the
 * type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types should
 * trigger all TYPE_* cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x)

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *ptr) {
    /* Do nothing but prevent optimization */
    (void)ptr;
}

/* ========== TYPE_SCALAR definitions ========== */
struct ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    _Bool bool_field;
};

/* ========== TYPE_STRING definitions ========== */
struct StringTypes {
    const char *string1;
    char *string2;
    const char *const string3;
};

/* ========== TYPE_STRUCT definitions ========== */
struct InnerStruct {
    int x;
    double y;
};

struct OuterStruct {
    struct InnerStruct inner;
    int counter;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct with GTY marker (simulated) */
typedef struct GTY(()) UserDefinedStruct {
    int id;
    char name[32];
    struct UserDefinedStruct *next;
} UserDefinedStruct;

/* ========== TYPE_UNION definitions ========== */
union DataUnion {
    int int_val;
    float float_val;
    double double_val;
    char *string_val;
    void *ptr_val;
};

struct UnionContainer {
    int type;
    union DataUnion data;
};

/* ========== TYPE_POINTER definitions ========== */
struct PointerTypes {
    int *int_ptr;
    char **char_ptr_ptr;
    void *void_ptr;
    struct InnerStruct *struct_ptr;
    union DataUnion *union_ptr;
    
    /* Function pointer */
    int (*func_ptr)(int, char);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to struct */
    struct OuterStruct **nested_ptr;
};

/* ========== TYPE_ARRAY definitions ========== */
struct ArrayTypes {
    /* Fixed-size arrays */
    int int_array[20];
    char char_array[50];
    float float_array[5][5];  /* 2D array */
    
    /* Array of pointers */
    void *ptr_array[8];
    
    /* Array of structs */
    struct InnerStruct struct_array[4];
    
    /* Array of unions */
    union DataUnion union_array[3];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Callback function types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

struct CallbackContainer {
    Comparator compare;
    CallbackFunc callback;
    void *user_data;
};

/* ========== TYPE_LANG_STRUCT definitions ========== */
/* Simulating language-specific structure */
struct LangSpecific {
    /* Language-specific fields */
    void *lang_data;
    int lang_tag;
    
    /* Nested anonymous struct (C11) */
    struct {
        int anonymous_field1;
        double anonymous_field2;
    };
    
    /* Bitfields */
    unsigned int flags : 4;
    unsigned int mode : 3;
};

/* ========== Complex Nested Type ========== */
/* This should trigger multiple type kinds */
struct ComplexNestedType {
    /* Scalar fields */
    int id;
    double value;
    
    /* String field */
    const char *name;
    
    /* Nested struct */
    struct {
        int nested_id;
        float nested_data[3];
    } inner;
    
    /* Union field */
    union {
        int as_int;
        double as_double;
        char *as_string;
    } variant;
    
    /* Pointer to self (recursive type) */
    struct ComplexNestedType *next;
    
    /* Array of pointers to callbacks */
    CallbackFunc callbacks[5];
    
    /* 2D array */
    int matrix[3][3];
    
    /* Pointer array with different types */
    void *mixed_ptrs[4];
    
    /* Flexible array of structs */
    struct InnerStruct items[];
};

/* ========== Another complex type ========== */
typedef struct GTY(()) TreeNode {
    int value;
    struct TreeNode *left;
    struct TreeNode *right;
    struct TreeNode *parent;
    char *data;
    int children_count;
    struct TreeNode *children[];
} TreeNode;

/* ========== Function declarations ========== */
int sample_comparator(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

void sample_callback(int event, void *data) {
    /* Empty callback */
    (void)event;
    (void)data;
}

/* ========== Main function ========== */
int main() {
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {"Hello", "World", "Test"};
    struct OuterStruct outer = {{{1, 2.0}, 3}};
    UserDefinedStruct user_struct = {42, "TestUser", NULL};
    struct UnionContainer union_cont = {0};
    struct PointerTypes pointers = {0};
    struct ArrayTypes *array_ptr = NULL;
    struct CallbackContainer callbacks = {sample_comparator, sample_callback, NULL};
    struct LangSpecific lang_struct = {NULL, 0, {1, 2.0}, 0, 0};
    struct ComplexNestedType *complex_ptr = NULL;
    TreeNode *tree_root = NULL;
    
    /* Initialize union */
    union_cont.type = 1;
    union_cont.data.int_val = 42;
    
    /* Initialize pointers */
    int int_val = 100;
    char *char_ptr = "PointerTest";
    pointers.int_ptr = &int_val;
    pointers.char_ptr_ptr = &char_ptr;
    pointers.func_ptr = NULL;
    
    /* Calculate sizes to force type consideration */
    size_t total_size = 0;
    
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct OuterStruct);
    total_size += sizeof(UserDefinedStruct);
    total_size += sizeof(struct UnionContainer);
    total_size += sizeof(struct PointerTypes);
    total_size += sizeof(struct ArrayTypes);
    total_size += sizeof(struct CallbackContainer);
    total_size += sizeof(struct LangSpecific);
    total_size += sizeof(struct ComplexNestedType);
    total_size += sizeof(TreeNode);
    
    /* Take addresses of all instances and members */
    use_pointer(&scalars);
    use_pointer(&strings);
    use_pointer(&outer);
    use_pointer(&user_struct);
    use_pointer(&union_cont);
    use_pointer(&pointers);
    use_pointer(&callbacks);
    use_pointer(&lang_struct);
    
    /* Take addresses of specific members to ensure they're considered */
    use_pointer(&scalars.int_field);
    use_pointer(&strings.string1);
    use_pointer(&outer.inner.x);
    use_pointer(&user_struct.name);
    use_pointer(&union_cont.data);
    use_pointer(&pointers.func_ptr);
    use_pointer(&callbacks.compare);
    use_pointer(&lang_struct.lang_data);
    
    /* Force consideration of array types */
    int sample_array[5] = {1, 2, 3, 4, 5};
    int (*array_2d)[3][3] = NULL;
    void *void_ptr_array[2] = {&scalars, &outer};
    
    use_pointer(sample_array);
    use_pointer(array_2d);
    use_pointer(void_ptr_array);
    
    /* Force consideration of function pointer types */
    int (*func_ptr_array[2])(int) = {NULL, NULL};
    use_pointer(func_ptr_array);
    
    /* Create a complex nested structure on stack */
    struct {
        int header;
        struct ComplexNestedType data;
    } stack_complex = {0};
    
    use_pointer(&stack_complex);
    
    /* Print checksum to prevent optimization */
    printf("Type coverage test - Total size sum: %zu\n", total_size);
    printf("Address checksum: %p %p %p\n", 
           (void*)&scalars, 
           (void*)&outer, 
           (void*)&user_struct);
    
    /* Return something based on sizes to ensure computation isn't optimized away */
    return (total_size > 0) ? 0 : 1;
}

/* ========== Additional type definitions outside main ========== */
/* These ensure TYPE_UNDEFINED might be triggered for forward declarations */

/* Forward declaration */
struct ForwardDeclared;

/* Struct with pointer to forward declared type */
struct HasForwardPtr {
    int id;
    struct ForwardDeclared *fwd_ptr;
};

/* Later definition */
struct ForwardDeclared {
    int value;
    struct HasForwardPtr *back_ptr;
};

/* Anonymous struct/union test */
struct AnonymousTest {
    union {
        int x;
        float y;
    };
    struct {
        char a;
        char b;
    };
};

/* Const volatile qualified types */
struct QualifiedTypes {
    const int const_field;
    volatile int volatile_field;
    const volatile int cv_field;
    int *restrict restrict_ptr;
};

/* Aligned types */
struct AlignedTypes {
    int __attribute__((aligned(64))) aligned_field;
    double __attribute__((aligned(32))) aligned_double;
};

/* Packed struct */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Vector types (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));
