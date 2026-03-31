/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, it should trigger
 * all cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would
 * be the actual garbage collector annotation */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile int sink = *(volatile int*)&p;
    (void)sink;
}

/* ========== TYPE DEFINITIONS ========== */

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
} ScalarTypes;

/* TYPE_STRING: String type */
typedef struct GTY(()) StringType {
    const char * GTY((tag("0"))) string_ptr;
    char string_array[64];
} StringType;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct PointerTypes * GTY((skip)) self_ptr;
    void (* GTY((callback)) func_ptr)(void);
    int (*array_ptr)[10];
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    int fixed_array[20];
    char multi_dim[5][10];
    struct ArrayTypes *pointer_array[15];
    int flexible_array[];
} ArrayTypes;

/* TYPE_STRUCT: Nested structures */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    double inner_value;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;
    InnerStruct *nested_ptr;
    struct {
        int anonymous_member;
        float anonymous_float;
    } GTY((tag("1"))) anonymous;
} OuterStruct;

/* TYPE_UNION: Union types */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    void *as_pointer;
    struct {
        int part1;
        int part2;
    } as_struct;
} DataUnion;

typedef struct GTY(()) UnionContainer {
    DataUnion data;
    union {
        char small;
        long large;
    } inline_union;
} UnionContainer;

/* TYPE_USER_STRUCT: User-defined structure with callbacks */
typedef int (* GTY((callback)) CompareFunc)(const void*, const void*);

typedef struct GTY(()) UserStruct {
    CompareFunc comparator;
    void * GTY((skip)) user_data;
    const char *name;
} UserStruct;

/* TYPE_CALLBACK: Function pointer types */
typedef struct GTY(()) CallbackContainer {
    void (* GTY((callback)) callback)(int, float);
    int (* GTY((callback)) another_callback)(const char*);
    struct CallbackContainer * GTY((chain_next)) next;
} CallbackContainer;

/* Complex nested type combining everything */
typedef struct GTY(()) MegaType {
    /* TYPE_SCALAR */
    int id;
    
    /* TYPE_STRING */
    const char * GTY((tag("2"))) description;
    
    /* TYPE_POINTER */
    struct MegaType * GTY((skip)) parent;
    void **pointer_array;
    
    /* TYPE_ARRAY */
    int matrix[3][3];
    DataUnion union_array[5];
    
    /* TYPE_STRUCT */
    OuterStruct outer;
    
    /* TYPE_UNION */
    DataUnion variant;
    
    /* TYPE_USER_STRUCT */
    UserStruct user;
    
    /* TYPE_CALLBACK */
    void (* GTY((callback)) handler)(struct MegaType*);
    
    /* Flexible array member */
    char extra_data[];
} MegaType;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef struct GTY(()) LangStruct {
    void * GTY((length("len"))) data;
    size_t len;
    int lang_specific_flags;
    struct LangStruct * GTY((chain_next)) chain;
} LangStruct;

/* Forward declaration for circular reference */
typedef struct GTY(()) TreeNode TreeNode;

struct GTY(()) TreeNode {
    int value;
    TreeNode * GTY((skip)) left;
    TreeNode * GTY((skip)) right;
    TreeNode * GTY((skip)) parent;
};

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Declare instances of all types */
    ScalarTypes scalars = {0};
    StringType strings = {0};
    PointerTypes pointers = {0};
    ArrayTypes *arrays = NULL;
    OuterStruct outer = {0};
    UnionContainer union_container = {0};
    UserStruct user_struct = {0};
    CallbackContainer callbacks = {0};
    MegaType *mega = NULL;
    LangStruct lang_struct = {0};
    TreeNode tree_node = {0};
    
    /* Force compiler to consider all types */
    size_t total_size = 0;
    
    /* TYPE_SCALAR references */
    total_size += sizeof(ScalarTypes);
    scalars.int_field = 42;
    scalars.float_field = 3.14159f;
    KEEP_ALIVE(&scalars);
    
    /* TYPE_STRING references */
    total_size += sizeof(StringType);
    strings.string_ptr = "Hello, gengtype!";
    KEEP_ALIVE(&strings);
    
    /* TYPE_POINTER references */
    total_size += sizeof(Pointers);
    pointers.self_ptr = &pointers;
    use_pointer(&pointers);
    
    /* TYPE_ARRAY references */
    arrays = (ArrayTypes*)malloc(sizeof(ArrayTypes) + 50 * sizeof(int));
    if (arrays) {
        total_size += sizeof(ArrayTypes);
        for (int i = 0; i < 20; i++) {
            arrays->fixed_array[i] = i * i;
        }
        KEEP_ALIVE(arrays);
    }
    
    /* TYPE_STRUCT references */
    total_size += sizeof(OuterStruct);
    outer.nested.inner_data = 100;
    outer.anonymous.anonymous_member = 200;
    KEEP_ALIVE(&outer);
    
    /* TYPE_UNION references */
    total_size += sizeof(UnionContainer);
    union_container.data.as_int = 0xDEADBEEF;
    union_container.inline_union.large = 1ULL << 40;
    KEEP_ALIVE(&union_container);
    
    /* TYPE_USER_STRUCT references */
    total_size += sizeof(UserStruct);
    user_struct.name = "TestUserStruct";
    KEEP_ALIVE(&user_struct);
    
    /* TYPE_CALLBACK references */
    total_size += sizeof(CallbackContainer);
    callbacks.next = &callbacks; /* Self-reference */
    KEEP_ALIVE(&callbacks);
    
    /* Complex MegaType reference */
    mega = (MegaType*)malloc(sizeof(MegaType) + 100 * sizeof(char));
    if (mega) {
        total_size += sizeof(MegaType);
        mega->id = 999;
        mega->description = "MegaType instance";
        mega->variant.as_float = 2.71828f;
        KEEP_ALIVE(mega);
    }
    
    /* TYPE_LANG_STRUCT references */
    total_size += sizeof(LangStruct);
    lang_struct.len = 256;
    lang_struct.data = malloc(lang_struct.len);
    KEEP_ALIVE(&lang_struct);
    
    /* Tree node for circular references */
    total_size += sizeof(TreeNode);
    tree_node.value = 42;
    tree_node.left = &tree_node; /* Self-reference */
    KEEP_ALIVE(&tree_node);
    
    /* Calculate and print checksum */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: 0x%08lx\n", (unsigned long)total_size * 0x12345678);
    
    /* Cleanup */
    if (arrays) free(arrays);
    if (mega) free(mega);
    if (lang_struct.data) free(lang_struct.data);
    
    return 0;
}
