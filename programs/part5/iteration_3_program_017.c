/* test_gengtype_coverage.c
 * 
 * This test defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types
 * should trigger multiple cases in the switch statement.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers for compilation - in real GCC build these
 * would be actual GTY(()) annotations that trigger gengtype processing */
#define GTY(x)

/* Forward declarations to create complex type relationships */
struct forward_declared_struct;
union forward_declared_union;

/* ========== TYPE_SCALAR triggers ========== */
struct GTY(()) ScalarTypes {
    int int_field;              /* TYPE_SCALAR */
    char char_field;            /* TYPE_SCALAR */
    float float_field;          /* TYPE_SCALAR */
    double double_field;        /* TYPE_SCALAR */
    _Bool bool_field;           /* TYPE_SCALAR */
    long long long_field;       /* TYPE_SCALAR */
    short short_field;          /* TYPE_SCALAR */
    unsigned uint_field;        /* TYPE_SCALAR */
};

/* ========== TYPE_STRING triggers ========== */
struct GTY(()) StringTypes {
    const char* static_string;  /* TYPE_STRING */
    char* dynamic_string;       /* TYPE_POINTER (to TYPE_SCALAR) */
    const char* const const_string_ptr; /* TYPE_POINTER (to TYPE_STRING) */
};

/* ========== TYPE_STRUCT triggers ========== */
struct GTY(()) NestedStruct {
    int data;
    struct NestedStruct* next;  /* TYPE_POINTER to TYPE_STRUCT */
};

struct GTY(()) ComplexStruct {
    struct ScalarTypes scalars; /* TYPE_STRUCT */
    struct StringTypes strings; /* TYPE_STRUCT */
    struct NestedStruct nested; /* TYPE_STRUCT */
};

/* ========== TYPE_UNION triggers ========== */
union GTY(()) SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;               /* TYPE_POINTER */
};

union GTY(()) ComplexUnion {
    struct {
        int type_tag;
        union SimpleUnion data; /* TYPE_UNION inside struct */
    } tagged;
    struct ComplexStruct as_struct; /* TYPE_STRUCT inside union */
    long long as_longlong;
};

/* ========== TYPE_ARRAY triggers ========== */
struct GTY(()) ArrayTypes {
    int fixed_array[10];        /* TYPE_ARRAY of TYPE_SCALAR */
    char* pointer_array[5];     /* TYPE_ARRAY of TYPE_POINTER */
    struct ScalarTypes struct_array[3]; /* TYPE_ARRAY of TYPE_STRUCT */
    int multi_dim[2][3][4];     /* Multi-dimensional TYPE_ARRAY */
    int flexible_array[];       /* Flexible array member - TYPE_ARRAY */
};

/* ========== TYPE_POINTER triggers ========== */
struct GTY(()) PointerTypes {
    void* void_ptr;             /* TYPE_POINTER to TYPE_NONE? */
    int* int_ptr;               /* TYPE_POINTER to TYPE_SCALAR */
    struct ComplexStruct* struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
    union ComplexUnion* union_ptr;    /* TYPE_POINTER to TYPE_UNION */
    struct forward_declared_struct* forward_ptr; /* TYPE_POINTER to forward decl */
    int (*func_ptr)(int, char); /* TYPE_POINTER to TYPE_CALLBACK */
    int (*array_of_func_ptrs[3])(void); /* TYPE_ARRAY of TYPE_POINTER to TYPE_CALLBACK */
};

/* ========== TYPE_CALLBACK triggers ========== */
typedef int (*callback_func)(int, void*); /* TYPE_POINTER to TYPE_CALLBACK */

struct GTY(()) CallbackContainer {
    callback_func handler;      /* TYPE_POINTER to TYPE_CALLBACK */
    void (*void_func)(void);    /* TYPE_POINTER to TYPE_CALLBACK */
    int (*const const_func_ptr)(double); /* TYPE_POINTER to TYPE_CALLBACK */
};

/* ========== TYPE_USER_STRUCT triggers ========== */
/* In GCC context, TYPE_USER_STRUCT might refer to user-defined types
 * with special handling. We'll create typedef'd structs. */
typedef struct GTY(()) {
    int user_data;
    char* user_name;
} UserDefinedStruct;

typedef union GTY(()) {
    UserDefinedStruct as_user_struct;
    struct ComplexStruct as_complex;
} UserDefinedUnion;

/* ========== Complete the forward declarations ========== */
struct GTY(()) forward_declared_struct {
    int magic_number;
    struct forward_declared_struct* self_ptr; /* Recursive pointer */
    union forward_declared_union* union_ptr;
};

union GTY(()) forward_declared_union {
    struct forward_declared_struct as_struct;
    int as_int;
    void* as_void_ptr;
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* TYPE_LANG_STRUCT might be language-specific structures.
 * We'll create a struct that could be interpreted as such. */
struct GTY(()) LangLikeStruct {
    enum {
        LANG_C,
        LANG_CPP,
        LANG_JAVA
    } language_tag;
    
    union {
        /* C-specific fields */
        struct {
            void* c_context;
            int c_flags;
        } c_info;
        
        /* C++-specific fields */
        struct {
            void* vtable_ptr;
            unsigned int virtual_count;
        } cpp_info;
    } lang_data;
};

/* ========== Complex nested type with all categories ========== */
struct GTY(()) UltimateType {
    /* Scalars */
    int id;
    float value;
    
    /* Strings */
    const char* name;
    
    /* Structs */
    struct ScalarTypes scalars;
    struct ComplexStruct complex;
    
    /* Unions */
    union SimpleUnion simple_union;
    union ComplexUnion complex_union;
    
    /* Arrays */
    int scores[5];
    struct ComplexStruct* ptr_array[3];
    
    /* Pointers */
    struct UltimateType* self;  /* Recursive */
    void* generic_ptr;
    callback_func callback;
    
    /* Nested anonymous struct/union */
    struct {
        int anonymous_data;
        union {
            int anon_int;
            float anon_float;
        } anon_union;
    } anonymous;
    
    /* Flexible array at the end */
    char extra_data[];
};

/* ========== Function to prevent optimization ========== */
/* This function takes addresses of various types to ensure
 * they're referenced and not optimized away */
__attribute__((noinline)) 
static size_t collect_type_info(
    struct ScalarTypes* scalars,
    struct StringTypes* strings,
    struct ComplexStruct* complex,
    union SimpleUnion* simple_u,
    union ComplexUnion* complex_u,
    struct ArrayTypes* arrays,
    struct PointerTypes* pointers,
    struct CallbackContainer* callbacks,
    UserDefinedStruct* user_struct,
    UserDefinedUnion* user_union,
    struct forward_declared_struct* forward,
    union forward_declared_union* forward_u,
    struct LangLikeStruct* lang_struct,
    struct UltimateType* ultimate)
{
    /* Compute sizes to force type analysis */
    size_t total_size = 0;
    
    total_size += sizeof(*scalars);
    total_size += sizeof(*strings);
    total_size += sizeof(*complex);
    total_size += sizeof(*simple_u);
    total_size += sizeof(*complex_u);
    total_size += sizeof(*arrays) + sizeof(int[10]); /* Account for fixed array */
    total_size += sizeof(*pointers);
    total_size += sizeof(*callbacks);
    total_size += sizeof(*user_struct);
    total_size += sizeof(*user_union);
    total_size += sizeof(*forward);
    total_size += sizeof(*forward_u);
    total_size += sizeof(*lang_struct);
    total_size += sizeof(*ultimate) + sizeof(char[10]); /* Account for flexible array */
    
    /* Take addresses of members to ensure they're considered */
    volatile void* addr_keep;
    
    addr_keep = &scalars->int_field;
    addr_keep = &strings->static_string;
    addr_keep = &complex->nested.data;
    addr_keep = &simple_u->as_int;
    addr_keep = &complex_u->tagged.data.as_int;
    addr_keep = &arrays->fixed_array[0];
    addr_keep = &pointers->void_ptr;
    addr_keep = &callbacks->handler;
    addr_keep = &user_struct->user_data;
    addr_keep = &user_union->as_user_struct;
    addr_keep = &forward->magic_number;
    addr_keep = &forward_u->as_int;
    addr_keep = &lang_struct->language_tag;
    addr_keep = &ultimate->anonymous.anonymous_data;
    
    (void)addr_keep; /* Suppress unused warning */
    
    return total_size;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct ComplexStruct complex = {0};
    union SimpleUnion simple_u = {0};
    union ComplexUnion complex_u = {0};
    
    /* For ArrayTypes with flexible array, we need to allocate extra space */
    struct {
        struct ArrayTypes base;
        int flex_data[5];
    } array_wrapper = {0};
    struct ArrayTypes* arrays = &array_wrapper.base;
    
    struct PointerTypes pointers = {0};
    struct CallbackContainer callbacks = {0};
    UserDefinedStruct user_struct = {0};
    UserDefinedUnion user_union = {0};
    struct forward_declared_struct forward = {0};
    union forward_declared_union forward_u = {0};
    struct LangLikeStruct lang_struct = {0};
    
    /* For UltimateType with flexible array */
    struct {
        struct UltimateType base;
        char extra[20];
    } ultimate_wrapper = {0};
    struct UltimateType* ultimate = &ultimate_wrapper.base;
    
    /* Initialize some values to avoid undefined behavior */
    scalars.int_field = 42;
    strings.static_string = "Hello, gengtype!";
    complex.nested.data = 100;
    simple_u.as_int = 255;
    complex_u.tagged.type_tag = 1;
    arrays->fixed_array[0] = 1;
    pointers.void_ptr = (void*)0x1000;
    user_struct.user_data = 999;
    forward.magic_number = 0xCAFEBABE;
    forward_u.as_int = 12345;
    lang_struct.language_tag = LANG_C;
    ultimate->id = 1;
    ultimate->name = "Ultimate Type";
    
    /* Create a recursive pointer */
    ultimate->self = ultimate;
    forward.self_ptr = &forward;
    
    /* Call function that takes addresses of all types */
    size_t total_type_size = collect_type_info(
        &scalars, &strings, &complex, &simple_u, &complex_u,
        arrays, &pointers, &callbacks, &user_struct, &user_union,
        &forward, &forward_u, &lang_struct, ultimate);
    
    /* Print result to prevent optimization and verify execution */
    printf("Total type size calculated: %zu bytes\n", total_type_size);
    printf("Type coverage test completed.\n");
    
    /* Additional operations to ensure all types are referenced */
    printf("Sizes of individual types:\n");
    printf("  ScalarTypes: %zu\n", sizeof(struct ScalarTypes));
    printf("  StringTypes: %zu\n", sizeof(struct StringTypes));
    printf("  ComplexStruct: %zu\n", sizeof(struct ComplexStruct));
    printf("  SimpleUnion: %zu\n", sizeof(union SimpleUnion));
    printf("  ComplexUnion: %zu\n", sizeof(union ComplexUnion));
    printf("  ArrayTypes (base): %zu\n", sizeof(struct ArrayTypes));
    printf("  PointerTypes: %zu\n", sizeof(struct PointerTypes));
    printf("  CallbackContainer: %zu\n", sizeof(struct CallbackContainer));
    printf("  UserDefinedStruct: %zu\n", sizeof(UserDefinedStruct));
    printf("  UserDefinedUnion: %zu\n", sizeof(UserDefinedUnion));
    printf("  forward_declared_struct: %zu\n", sizeof(struct forward_declared_struct));
    printf("  forward_declared_union: %zu\n", sizeof(union forward_declared_union));
    printf("  LangLikeStruct: %zu\n", sizeof(struct LangLikeStruct));
    printf("  UltimateType (base): %zu\n", sizeof(struct UltimateType));
    
    return 0;
}
