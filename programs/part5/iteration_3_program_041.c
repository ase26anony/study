/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by gengtype during GCC build, it should trigger:
 * - TYPE_UNDEFINED
 * - TYPE_SCALAR
 * - TYPE_STRING
 * - TYPE_STRUCT
 * - TYPE_USER_STRUCT
 * - TYPE_UNION
 * - TYPE_POINTER
 * - TYPE_ARRAY
 * - TYPE_CALLBACK
 * - TYPE_LANG_STRUCT
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations to create pointer cycles and complex types */
struct forward_declared;
union forward_union;

/* ========== TYPE_SCALAR definitions ========== */
GTY(())
struct ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    _Bool boolean;
    long long int64;
    unsigned short uint16;
};

/* ========== TYPE_STRING definitions ========== */
GTY(())
struct StringTypes {
    const char *constant_string;
    char *mutable_string;
    const char *array_of_strings[5];
};

/* ========== TYPE_STRUCT definitions ========== */
GTY(())
struct InnerStruct {
    int x;
    double y;
};

GTY(())
struct OuterStruct {
    struct InnerStruct nested;
    int extra_field;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct with typedef */
typedef GTY(()) struct {
    int user_id;
    char user_name[32];
} UserDefinedStruct;

/* Another user struct with function pointers */
typedef GTY(()) struct {
    int (*compare)(const void *, const void *);
    void (*destructor)(void *);
} UserStructWithCallbacks;

/* ========== TYPE_UNION definitions ========== */
GTY(())
union VariantUnion {
    int as_int;
    double as_double;
    void *as_pointer;
    struct {
        int tag;
        char data[16];
    } as_struct;
};

/* Tagged union for discriminated types */
GTY(())
struct TaggedUnionContainer {
    int type_tag;
    union {
        int int_value;
        float float_value;
        char *string_value;
        struct InnerStruct struct_value;
    } data;
};

/* ========== TYPE_POINTER definitions ========== */
GTY(())
struct PointerMadness {
    /* Simple pointers */
    int *int_ptr;
    char **double_ptr_to_char;
    
    /* Pointer to struct */
    struct OuterStruct *struct_ptr;
    
    /* Pointer to union */
    union VariantUnion *union_ptr;
    
    /* Pointer to pointer */
    void ***triple_void_ptr;
    
    /* Self-referential pointer */
    struct PointerMadness *next;
    
    /* Pointer to forward declared */
    struct forward_declared *forward_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer (TYPE_CALLBACK) */
    int (*callback)(int, char **);
};

/* ========== TYPE_ARRAY definitions ========== */
GTY(())
struct ArrayTypes {
    /* Fixed-size arrays */
    int fixed_array[100];
    char char_matrix[10][20];
    struct InnerStruct struct_array[5];
    
    /* Multi-dimensional */
    double cube[3][4][5];
    
    /* Array of pointers */
    void *pointer_array[8];
    
    /* Array of function pointers */
    void (*func_array[4])(void);
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

GTY(())
struct CallbackContainer {
    Comparator sorter;
    EventHandler handlers[5];
    void (*simple_callback)(void);
    
    /* Nested callback in struct */
    struct {
        int (*validate)(const char *);
        void (*log_message)(const char *);
    } validators;
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* Language-specific structures (simulating GCC internal types) */
GTY(())
struct LangSpecificBase {
    int lang_specific_tag;
    void *lang_data;
};

/* Tree-like structure for language AST */
GTY(())
struct LangTreeNode {
    int node_type;
    struct LangTreeNode *first_child;
    struct LangTreeNode *next_sibling;
    union {
        int int_value;
        double float_value;
        char *string_value;
    } attribute;
};

/* ========== TYPE_UNDEFINED simulation ========== */
/* Incomplete/forward declared types */
struct forward_declared {
    int magic;
    struct forward_declared *next;
};

union forward_union {
    int a;
    struct forward_declared *b;
};

/* ========== Complex nested type ========== */
GTY(())
struct UltimateType {
    /* Scalar mix */
    int id;
    double weight;
    
    /* String */
    const char *name;
    
    /* Struct */
    struct InnerStruct inner;
    
    /* User struct */
    UserDefinedStruct user;
    
    /* Union */
    union VariantUnion variant;
    
    /* Pointers */
    struct UltimateType *self_ptr;
    void **generic_ptrs[3];
    
    /* Arrays */
    int scores[7];
    struct InnerStruct nested_array[2][2];
    
    /* Callbacks */
    int (*processor)(struct UltimateType *);
    
    /* Language struct */
    struct LangTreeNode *ast_node;
    
    /* Pointer to undefined/forward declared */
    struct forward_declared *mystery;
    
    /* Array of unions */
    union {
        int i;
        float f;
    } choice_array[4];
    
    /* Nested anonymous struct */
    struct {
        int counter;
        char flags[8];
    } state;
};

/* ========== Function to prevent optimization ========== */
/* Use noinline and volatile to ensure types are referenced */
__attribute__((noinline)) 
static size_t compute_type_sizes(void) {
    volatile size_t total_size = 0;
    
    /* Force evaluation of all type sizes */
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct InnerStruct);
    total_size += sizeof(struct OuterStruct);
    total_size += sizeof(UserDefinedStruct);
    total_size += sizeof(UserStructWithCallbacks);
    total_size += sizeof(union VariantUnion);
    total_size += sizeof(struct TaggedUnionContainer);
    total_size += sizeof(struct PointerMadness);
    total_size += offsetof(struct ArrayTypes, flexible_array); /* Don't include flexible array */
    total_size += sizeof(struct CallbackContainer);
    total_size += sizeof(struct LangSpecificBase);
    total_size += sizeof(struct LangTreeNode);
    total_size += sizeof(struct UltimateType);
    
    return total_size;
}

/* ========== External function to ensure type usage ========== */
/* This function's definition is elsewhere, forcing the compiler
   to generate complete type information */
extern void use_type_info(
    struct ScalarTypes *s,
    struct StringTypes *str,
    struct UltimateType *ult,
    struct LangTreeNode *lang
);

/* ========== Main driver ========== */
int main(void) {
    /* Declare instances of all complex types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct InnerStruct inner = {0};
    struct OuterStruct outer = {0};
    UserDefinedStruct user_struct = {0};
    UserStructWithCallbacks user_callbacks = {0};
    union VariantUnion variant = {0};
    struct TaggedUnionContainer tagged = {0};
    struct PointerMadness pointers = {0};
    static struct ArrayTypes arrays; /* Static to allow flexible array */
    struct CallbackContainer callbacks = {0};
    struct LangSpecificBase lang_base = {0};
    struct LangTreeNode lang_node = {0};
    struct UltimateType ultimate = {0};
    
    /* Take addresses to ensure types are referenced */
    volatile void *addresses[] = {
        &scalars, &strings, &inner, &outer,
        &user_struct, &user_callbacks, &variant,
        &tagged, &pointers, &arrays, &callbacks,
        &lang_base, &lang_node, &ultimate
    };
    
    /* Access members to ensure complete type usage */
    scalars.integer = 42;
    strings.constant_string = "Hello, gengtype!";
    inner.x = 10;
    outer.nested.y = 3.14159;
    user_struct.user_id = 1001;
    variant.as_int = 0xDEADBEEF;
    tagged.type_tag = 1;
    tagged.data.int_value = 99;
    pointers.next = &pointers; /* Self-reference */
    
    /* Compute and print type sizes */
    size_t total_size = compute_type_sizes();
    
    /* Use addresses to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        /* Cast to volatile char* and read first byte */
        char first_byte = *(volatile char*)addresses[i];
        total_size += (size_t)first_byte; /* Use in computation */
    }
    
    printf("Total type footprint: %zu bytes\n", total_size);
    printf("Test program completed. If built with gengtype,\n");
    printf("all type enumeration cases should have been triggered.\n");
    
    return 0;
}

/* ========== Additional type definitions for completeness ========== */
/* Define the forward declared types */
struct forward_declared {
    int magic;
    struct forward_declared *next;
    union forward_union *union_ref;
};

union forward_union {
    int a;
    struct forward_declared *b;
    struct UltimateType *ultimate_ref;
};

/* Array of unknown size (incomplete array type) */
extern int external_array[];

/* Const volatile qualified types */
GTY(())
struct CVQualified {
    const volatile int cv_int;
    volatile char *volatile_ptr;
    const struct InnerStruct *const_struct_ptr;
};
