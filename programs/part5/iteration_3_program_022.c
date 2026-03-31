/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to exercise the type
 * enumeration logic in gengtype.cc, specifically targeting the switch
 * statement that counts occurrences of different type kinds.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent dead code elimination */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

/* External function to force type references */
NOINLINE void use_pointer(void *ptr);
NOINLINE void use_pointer(void *ptr) {
    /* Volatile to prevent optimization */
    volatile int sink = (int)(intptr_t)ptr;
    (void)sink;
}

/* ==================== TYPE DEFINITIONS ==================== */

/* TYPE_SCALAR: Basic scalar types */
struct GTY(()) ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    _Bool boolean;
    long long int64;
};

/* TYPE_STRING: String types */
struct GTY(()) StringTypes {
    const char *constant_string;
    char *mutable_string;
    wchar_t *wide_string;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct ScalarTypes *struct_ptr;
    void (**function_ptr_array)(void);
};

/* TYPE_ARRAY: Array types */
struct GTY(()) ArrayTypes {
    int fixed_array[10];
    char multi_dim[5][20];
    struct ScalarTypes struct_array[3];
    int flexible_array[];  /* Flexible array member */
};

/* TYPE_STRUCT: Nested structure */
struct GTY(()) NestedStruct {
    int id;
    struct GTY(()) InnerStruct {
        float x;
        float y;
        struct GTY(()) DeepInner {
            char label[32];
            int depth;
        } GTY((tag("deep"))) deep;
    } GTY((tag("inner"))) inner;
    double value;
};

/* TYPE_UNION: Union types */
union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    struct {
        char data[8];
    } as_struct;
};

/* TYPE_USER_STRUCT: Typedef'd structure */
typedef struct GTY(()) {
    int counter;
    char name[64];
    union DataUnion data;
} GTY((tag("user1"))) UserDefinedStruct;

/* Complex nested type with all kinds */
struct GTY(()) MasterType {
    /* TYPE_SCALAR */
    int id;
    float priority;
    
    /* TYPE_STRING */
    const char *description;
    char *buffer;
    
    /* TYPE_POINTER */
    struct MasterType *self_ptr;
    void (*callback)(int, char*);
    int *int_ptr;
    
    /* TYPE_ARRAY */
    int scores[5];
    struct ScalarTypes scalar_array[2];
    
    /* TYPE_STRUCT */
    struct NestedStruct nested;
    
    /* TYPE_UNION */
    union DataUnion variant;
    
    /* TYPE_USER_STRUCT */
    UserDefinedStruct user_struct;
    
    /* Pointer to array */
    int (*matrix_ptr)[10][10];
    
    /* Function pointer (TYPE_CALLBACK) */
    int (*comparator)(const void*, const void*);
    
    /* Nested anonymous struct */
    struct GTY(()) {
        unsigned flags;
        char tag;
    } metadata;
    
    /* Flexible array of pointers */
    struct MasterType **children;
};

/* Another complex type for additional coverage */
union GTY(()) SuperUnion {
    struct MasterType as_master;
    struct ArrayTypes as_array;
    struct PointerTypes as_pointers;
    UserDefinedStruct as_user;
    
    struct GTY(()) {
        long type_tag;
        void *payload;
    } tagged;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
struct GTY(()) LangSpecific {
    void *tree_node;      /* Simulating GCC tree nodes */
    void *rtx_value;      /* Simulating RTL expressions */
    int lang_flag;
    struct GTY(()) {
        int dialect;
        const char *features;
    } extensions;
};

/* Global variables to ensure types are used */
GTY(()) struct MasterType global_master;
GTY(()) union SuperUnion global_union;
GTY(()) struct LangSpecific global_lang_struct;

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    volatile size_t total_size = 0;
    volatile int checksum = 0;
    
    /* Declare instances of all types */
    struct ScalarTypes scalars = {0};
    struct StringTypes strings = {0};
    struct PointerTypes pointers = {0};
    struct ArrayTypes *array_ptr = NULL;
    struct NestedStruct nested = {0};
    union DataUnion data_union;
    UserDefinedStruct user_struct = {0};
    struct MasterType master = {0};
    union SuperUnion super_union;
    struct LangSpecific lang_struct = {0};
    
    /* Force consideration of each type by taking addresses and sizes */
    
    /* TYPE_SCALAR */
    total_size += sizeof(struct ScalarTypes);
    use_pointer(&scalars);
    checksum += scalars.integer;
    
    /* TYPE_STRING */
    total_size += sizeof(struct StringTypes);
    use_pointer(&strings);
    strings.constant_string = "test string";
    checksum += (int)strings.constant_string[0];
    
    /* TYPE_POINTER */
    total_size += sizeof(struct PointerTypes);
    use_pointer(&pointers);
    pointers.void_ptr = &scalars;
    
    /* TYPE_ARRAY */
    array_ptr = (struct ArrayTypes*)malloc(sizeof(struct ArrayTypes) + 10 * sizeof(int));
    total_size += sizeof(struct ArrayTypes);
    use_pointer(array_ptr);
    if (array_ptr) {
        for (int i = 0; i < 10; i++) {
            array_ptr->fixed_array[i] = i;
            checksum += i;
        }
        free(array_ptr);
    }
    
    /* TYPE_STRUCT */
    total_size += sizeof(struct NestedStruct);
    use_pointer(&nested);
    nested.id = 42;
    checksum += nested.id;
    
    /* TYPE_UNION */
    total_size += sizeof(union DataUnion);
    use_pointer(&data_union);
    data_union.as_int = 0xDEADBEEF;
    checksum += data_union.as_int;
    
    /* TYPE_USER_STRUCT */
    total_size += sizeof(UserDefinedStruct);
    use_pointer(&user_struct);
    user_struct.counter = 100;
    checksum += user_struct.counter;
    
    /* Complex TYPE_STRUCT with everything */
    total_size += sizeof(struct MasterType);
    use_pointer(&master);
    master.id = 1;
    master.callback = NULL;
    master.comparator = NULL;
    checksum += master.id;
    
    /* TYPE_UNION with complex members */
    total_size += sizeof(union SuperUnion);
    use_pointer(&super_union);
    super_union.tagged.type_tag = 2;
    checksum += (int)super_union.tagged.type_tag;
    
    /* TYPE_LANG_STRUCT */
    total_size += sizeof(struct LangSpecific);
    use_pointer(&lang_struct);
    lang_struct.lang_flag = 3;
    checksum += lang_struct.lang_flag;
    
    /* Reference global GTY-marked variables */
    use_pointer(&global_master);
    use_pointer(&global_union);
    use_pointer(&global_lang_struct);
    
    /* Calculate final checksum */
    checksum += (int)total_size;
    
    printf("Type coverage test complete. Checksum: %d\n", checksum);
    printf("Total size of all types: %zu bytes\n", total_size);
    
    return 0;
}
