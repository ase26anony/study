/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, it should trigger
 * all cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile int sink = (int)(intptr_t)p;
    (void)sink;
}

/* ========== TYPE_SCALAR definitions ========== */
struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
};

/* ========== TYPE_STRING definitions ========== */
struct GTY(()) StringTypes {
    const char *string_literal;
    char *mutable_string;
    const char *const constant_string;
    char char_array[32];
};

/* ========== TYPE_STRUCT definitions ========== */
struct GTY(()) InnerStruct {
    int x;
    double y;
};

struct GTY(()) OuterStruct {
    struct InnerStruct inner;
    int outer_field;
};

/* ========== TYPE_UNION definitions ========== */
union GTY(()) SimpleUnion {
    int as_int;
    float as_float;
    void *as_pointer;
};

struct GTY(()) StructWithUnion {
    int tag;
    union {
        int int_value;
        double double_value;
        struct InnerStruct struct_value;
    } GTY((desc("tag"))) data;
};

/* ========== TYPE_POINTER definitions ========== */
struct GTY(()) PointerFest {
    int *int_ptr;
    struct OuterStruct *struct_ptr;
    union SimpleUnion *union_ptr;
    void *void_ptr;
    struct PointerFest *self_ptr;  /* Recursive pointer */
    int **double_ptr;
    
    /* Function pointer - TYPE_CALLBACK */
    int (*compare_func)(const void *, const void *);
    void (*callback)(int, char);
};

/* ========== TYPE_ARRAY definitions ========== */
struct GTY(()) ArrayTypes {
    int simple_array[10];
    struct InnerStruct struct_array[5];
    union SimpleUnion union_array[8];
    char *pointer_array[20];
    
    /* Multi-dimensional arrays */
    int matrix[3][3];
    struct OuterStruct nested_array[2][2];
    
    /* Flexible array member */
    int flexible_array[];
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* In GCC context, TYPE_USER_STRUCT refers to types defined elsewhere
 * that are referenced via typedefs or similar */
typedef struct GTY(()) OuterStruct UserDefinedStruct;
typedef union GTY(()) SimpleUnion UserDefinedUnion;

struct GTY(()) Container {
    UserDefinedStruct user_struct;
    UserDefinedUnion user_union;
    UserDefinedStruct *user_struct_ptr;
};

/* ========== Complex nested type ========== */
struct GTY(()) UltimateType {
    /* Scalar fields */
    int id;
    float weight;
    
    /* String field */
    const char *name;
    
    /* Struct field */
    struct InnerStruct inner;
    
    /* Union field */
    union SimpleUnion variant;
    
    /* Pointer fields */
    struct UltimateType *next;
    struct UltimateType **prev;
    int (*operation)(struct UltimateType *);
    
    /* Array fields */
    int scores[5];
    struct InnerStruct items[3];
    
    /* Nested struct with union */
    struct StructWithUnion nested;
    
    /* Array of pointers to different types */
    void *generic_pointers[4];
    
    /* Callback function pointer */
    void (*notify)(struct UltimateType *, int);
};

/* ========== TYPE_LANG_STRUCT simulation ========== */
/* In GCC, TYPE_LANG_STRUCT represents language-specific structures.
 * We'll create a structure that mimics this by having opaque pointers
 * and language-specific fields */
struct GTY(()) LangLikeStruct {
    void *language_data;
    int language_tag;
    struct {
        int lang_specific_field;
        void *lang_opaque;
    } GTY((skip)) lang_info;
};

/* ========== Main function to reference all types ========== */
int main(void) {
    volatile size_t total_size = 0;
    
    /* Declare instances of all types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct OuterStruct outer = {0};
    union SimpleUnion simple_union;
    struct StructWithUnion struct_with_union = {0};
    struct PointerFest pointers = {0};
    struct ArrayTypes *array_ptr = NULL;
    struct Container container = {0};
    struct UltimateType ultimate = {0};
    struct LangLikeStruct lang_struct = {0};
    
    /* Array with flexible member needs special allocation */
    struct ArrayTypes *flex_array = (struct ArrayTypes *)
        malloc(sizeof(struct ArrayTypes) + 10 * sizeof(int));
    
    if (!flex_array) {
        return 1;
    }
    
    /* Take addresses and compute sizes to force type analysis */
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct OuterStruct);
    total_size += sizeof(union SimpleUnion);
    total_size += sizeof(struct StructWithUnion);
    total_size += sizeof(struct PointerFest);
    total_size += sizeof(struct ArrayTypes);
    total_size += sizeof(struct Container);
    total_size += sizeof(struct UltimateType);
    total_size += sizeof(struct LangLikeStruct);
    
    /* Take addresses to ensure types are referenced */
    use_pointer(&scalars);
    use_pointer(&strings);
    use_pointer(&outer);
    use_pointer(&simple_union);
    use_pointer(&struct_with_union);
    use_pointer(&pointers);
    use_pointer(&container);
    use_pointer(&ultimate);
    use_pointer(&lang_struct);
    use_pointer(flex_array);
    
    /* Reference nested members */
    use_pointer(&scalars.int_field);
    use_pointer(&strings.string_literal);
    use_pointer(&outer.inner);
    use_pointer(&struct_with_union.data);
    use_pointer(&pointers.compare_func);
    use_pointer(&container.user_struct);
    use_pointer(&ultimate.notify);
    use_pointer(&lang_struct.lang_info);
    
    /* Reference array elements */
    if (flex_array) {
        for (int i = 0; i < 10; i++) {
            flex_array->flexible_array[i] = i;
            use_pointer(&flex_array->flexible_array[i]);
        }
    }
    
    /* Function pointer assignment (TYPE_CALLBACK) */
    pointers.compare_func = (int (*)(const void *, const void *))&use_pointer;
    ultimate.notify = (void (*)(struct UltimateType *, int))&use_pointer;
    
    /* Create a complex pointer chain */
    ultimate.next = &ultimate;
    ultimate.prev = &ultimate.next;
    
    /* Print something to prevent optimization */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Address of ultimate type: %p\n", (void*)&ultimate);
    
    /* Compute and print a checksum */
    size_t checksum = (size_t)&scalars;
    checksum ^= (size_t)&strings;
    checksum ^= (size_t)&outer;
    checksum ^= total_size;
    
    printf("Checksum: 0x%zx\n", checksum);
    
    if (flex_array) {
        free(flex_array);
    }
    
    return 0;
}
