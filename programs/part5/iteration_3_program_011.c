/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * Compilation for gengtype testing:
 * 1. Place in GCC source tree (e.g., gcc/testsuite/gengtype/)
 * 2. Use: gcc -c -O0 -g -fdump-tree-all -ffat-lto-objects test_gengtype_coverage.c
 * 3. Process with gengtype during GCC build
 */

/* Dummy GTY macro for compilation outside GCC */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* External function to prevent dead code elimination */
NOINLINE void use_pointer(void *ptr);
NOINLINE void use_pointer(void *ptr) {
    volatile int sink = (int)(intptr_t)ptr;
    (void)sink;
}

/* ==================== TYPE DEFINITIONS ==================== */

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_prec;
    long long_value;
    unsigned int unsigned_int;
    _Bool boolean;
} ScalarTypes;

/* TYPE_STRING: String types */
typedef struct GTY(()) StringTypes {
    const char *constant_string;
    char *mutable_string;
    wchar_t *wide_string;
} StringTypes;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct PointerTypes *self_ptr;
    void (*func_ptr)(void);
    int (*int_func_ptr)(int, int);
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    int fixed_array[10];
    char multi_dim[5][5];
    struct ArrayTypes *pointer_array[8];
    int flexible_array[]; /* Flexible array member */
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
    } anonymous;
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

/* TYPE_USER_STRUCT: User-defined structure with tags */
struct GTY((tag("USER_DEFINED"))) UserDefinedStruct {
    int user_id;
    char *user_name;
};

typedef struct GTY(()) UserStructContainer {
    struct UserDefinedStruct *user_struct;
} UserStructContainer;

/* TYPE_CALLBACK: Function pointer structures */
typedef int (*Comparator)(const void *, const void *);

typedef struct GTY(()) CallbackContainer {
    Comparator compare_func;
    void (*callback)(void *data, int result);
    struct {
        void (*nested_callback)(int);
    } callback_group;
} CallbackContainer;

/* TYPE_LANG_STRUCT: Language-specific structure (simulated) */
typedef struct GTY(()) LangSpecific {
    void *lang_data;
    int lang_tag;
    struct LangSpecific *next;
} LangSpecific;

/* Complex nested structure combining all types */
typedef struct GTY(()) MasterStructure {
    /* TYPE_SCALAR */
    int master_id;
    double master_value;
    
    /* TYPE_STRING */
    const char *master_name;
    
    /* TYPE_POINTER */
    struct MasterStructure *self_reference;
    void **void_ptr_ptr;
    
    /* TYPE_ARRAY */
    ScalarTypes scalar_array[3];
    PointerTypes *pointer_list[5];
    
    /* TYPE_STRUCT */
    OuterStruct outer;
    
    /* TYPE_UNION */
    DataUnion variant;
    
    /* TYPE_USER_STRUCT */
    struct UserDefinedStruct user;
    
    /* TYPE_CALLBACK */
    void (*master_callback)(struct MasterStructure*);
    
    /* TYPE_LANG_STRUCT */
    LangSpecific *lang_struct;
    
    /* Nested anonymous union */
    union {
        int as_int;
        struct {
            char flag1 : 1;
            char flag2 : 1;
            char flags : 6;
        } bits;
    } options;
} MasterStructure;

/* Recursive structure for deep traversal */
typedef struct GTY(()) TreeNode {
    int value;
    struct TreeNode *left;
    struct TreeNode *right;
    struct TreeNode *children[4];
} TreeNode;

/* Union with struct members */
typedef union GTY(()) ComplexUnion {
    MasterStructure as_master;
    struct {
        int type_tag;
        union {
            int int_data;
            double float_data;
            void *ptr_data;
        } payload;
    } tagged;
} ComplexUnion;

/* ==================== MAIN FUNCTION ==================== */

int main(void) {
    volatile size_t total_size = 0;
    
    /* Declare instances of all types */
    ScalarTypes scalars = {0};
    StringTypes strings = {0};
    PointerTypes pointers = {0};
    ArrayTypes *arrays = NULL;
    OuterStruct outer = {0};
    UnionContainer union_container = {0};
    UserStructContainer user_container = {0};
    CallbackContainer callbacks = {0};
    LangSpecific lang_specific = {0};
    MasterStructure master = {0};
    TreeNode tree = {0};
    ComplexUnion complex_union = {0};
    
    /* Force consideration of all types */
    
    /* TYPE_SCALAR references */
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(scalars.integer);
    total_size += sizeof(scalars.double_prec);
    use_pointer(&scalars);
    
    /* TYPE_STRING references */
    strings.constant_string = "Test string";
    strings.mutable_string = (char*)"Mutable";
    total_size += sizeof(StringTypes);
    use_pointer(&strings);
    
    /* TYPE_POINTER references */
    pointers.self_ptr = &pointers;
    pointers.func_ptr = (void(*)(void))0x1234;
    total_size += sizeof(PointerTypes);
    use_pointer(&pointers);
    
    /* TYPE_ARRAY references */
    total_size += sizeof(ArrayTypes);
    total_size += sizeof(int[10]);  /* fixed_array type */
    use_pointer(&arrays);
    
    /* TYPE_STRUCT references */
    outer.nested.inner_data = 42;
    total_size += sizeof(OuterStruct);
    total_size += sizeof(InnerStruct);
    use_pointer(&outer);
    
    /* TYPE_UNION references */
    union_container.data.as_int = 100;
    union_container.inline_union.large = 999;
    total_size += sizeof(UnionContainer);
    total_size += sizeof(DataUnion);
    use_pointer(&union_container);
    
    /* TYPE_USER_STRUCT references */
    user_container.user_struct = NULL;
    total_size += sizeof(UserStructContainer);
    total_size += sizeof(struct UserDefinedStruct);
    use_pointer(&user_container);
    
    /* TYPE_CALLBACK references */
    callbacks.compare_func = (Comparator)NULL;
    total_size += sizeof(CallbackContainer);
    use_pointer(&callbacks);
    
    /* TYPE_LANG_STRUCT references */
    lang_specific.lang_tag = 1;
    total_size += sizeof(LangSpecific);
    use_pointer(&lang_specific);
    
    /* Master structure with all types */
    master.master_id = 1;
    master.master_name = "Master";
    master.self_reference = &master;
    master.variant.as_float = 3.14f;
    master.master_callback = NULL;
    total_size += sizeof(MasterStructure);
    use_pointer(&master);
    
    /* Recursive structure */
    tree.value = 0;
    tree.left = &tree;
    tree.right = &tree;
    total_size += sizeof(TreeNode);
    use_pointer(&tree);
    
    /* Complex union */
    complex_union.as_master.master_id = 2;
    complex_union.tagged.type_tag = 3;
    total_size += sizeof(ComplexUnion);
    use_pointer(&complex_union);
    
    /* Take addresses of all members to ensure type analysis */
    volatile void *addresses[] = {
        &scalars,
        &strings,
        &pointers,
        &arrays,
        &outer,
        &union_container,
        &user_container,
        &callbacks,
        &lang_specific,
        &master,
        &tree,
        &complex_union,
        &scalars.integer,
        &strings.constant_string,
        &pointers.func_ptr,
        &outer.nested,
        &union_container.data,
        &master.variant,
        &tree.left,
        &complex_union.tagged.payload
    };
    
    /* Use addresses to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        volatile int sink = (int)(intptr_t)addresses[i];
        (void)sink;
    }
    
    /* Print checksum to prevent optimization */
    printf("Type coverage test - Total size sum: %zu\n", total_size);
    printf("Address count: %zu\n", sizeof(addresses)/sizeof(addresses[0]));
    
    return 0;
}
