/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * Compilation for gengtype testing:
 *   gcc -O0 -g -fdump-tree-all -c -ffat-lto-objects test_gengtype_coverage.c
 * 
 * For integration into GCC build:
 *   Place in gcc/testsuite/gengtype/ directory and add to appropriate Makefile
 */

/* Dummy GTY macro for compilation outside GCC */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* ==================== TYPE_SCALAR ==================== */
struct ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
};

/* ==================== TYPE_STRING ==================== */
struct StringTypes {
    const char *string_literal;
    char *dynamic_string;
    const char *const constant_string;
};

/* ==================== TYPE_STRUCT ==================== */
struct NestedStruct {
    int depth;
    struct {
        int inner_a;
        struct {
            int inner_b;
        } deeply_nested;
    } inner;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined struct with GTY marker (simulated) */
typedef struct GTY(()) UserDefinedStruct {
    int user_id;
    char user_name[32];
    struct UserDefinedStruct *next;
} UserStruct;

/* ==================== TYPE_UNION ==================== */
union ComplexUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    struct {
        int union_struct_a;
        char union_struct_b;
    } as_struct;
};

/* ==================== TYPE_POINTER ==================== */
struct PointerTypes {
    /* Basic pointers */
    int *int_ptr;
    char **char_ptr_ptr;
    
    /* Function pointers (TYPE_CALLBACK) */
    int (*func_ptr)(int, char);
    void (*callback)(void *);
    
    /* Pointer to incomplete type */
    struct ForwardDecl *forward_ptr;
    
    /* Pointer to union */
    union ForwardUnion *union_ptr;
    
    /* Self-referential pointer */
    struct PointerTypes *self;
    
    /* Pointer array */
    void *ptr_array[10];
};

/* ==================== TYPE_ARRAY ==================== */
struct ArrayTypes {
    /* Fixed-size arrays */
    int fixed_array[100];
    char multi_dim[10][20];
    
    /* Zero-length array (GCC extension) */
    int flexible_array[0];
    
    /* Array of pointers */
    struct NestedStruct *struct_ptr_array[5];
    
    /* Array of function pointers */
    void (*func_array[3])(void);
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

struct CallbackContainer {
    Comparator compare;
    EventHandler handlers[5];
    void (*nested_callback)(int (*inner)(void));
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Simulating language-specific structure */
struct LangSpecific {
    /* Marked with language-specific attributes */
    int lang_specific_field;
    
    /* Complex nested type that might be language-specific */
    union {
        struct {
            int tree_code;
            void *lang_specific_data;
        } tree_node;
        struct {
            int rtx_code;
            long long rtx_value;
        } rtx_node;
    } u;
};

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* This structure combines all types to ensure maximum coverage */
struct GTY(()) MasterStructure {
    /* Scalar fields */
    int master_id;
    
    /* String field */
    const char *master_name;
    
    /* Nested struct */
    struct NestedStruct nested;
    
    /* User struct */
    UserStruct user;
    
    /* Union */
    union ComplexUnion data_union;
    
    /* Pointers */
    struct PointerTypes *pointers;
    
    /* Arrays */
    struct ArrayTypes arrays;
    
    /* Callbacks */
    struct CallbackContainer callbacks;
    
    /* Language-specific structure */
    struct LangSpecific lang_struct;
    
    /* Self-reference for cycles */
    struct MasterStructure *next;
    
    /* Array of self-references */
    struct MasterStructure *children[5];
    
    /* Mixed array */
    void *mixed_array[20];
};

/* ==================== FORWARD DECLARED STRUCTURES ==================== */
struct ForwardDecl {
    int forward_data;
    struct MasterStructure *link_back;
};

union ForwardUnion {
    int as_int;
    struct ForwardDecl *as_struct;
};

/* ==================== FUNCTION DEFINITIONS ==================== */
/* Prevent optimization by making functions noinline */
__attribute__((noinline)) 
static size_t compute_type_sizes(void) {
    size_t total_size = 0;
    
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct NestedStruct);
    total_size += sizeof(UserStruct);
    total_size += sizeof(union ComplexUnion);
    total_size += sizeof(struct PointerTypes);
    total_size += sizeof(struct ArrayTypes);
    total_size += sizeof(struct CallbackContainer);
    total_size += sizeof(struct LangSpecific);
    total_size += sizeof(struct MasterStructure);
    
    return total_size;
}

__attribute__((noinline))
static void take_addresses(
    struct ScalarTypes *scalar,
    struct StringTypes *string,
    struct NestedStruct *nested,
    UserStruct *user,
    union ComplexUnion *cunion,
    struct PointerTypes *pointer,
    struct ArrayTypes *array,
    struct CallbackContainer *callback,
    struct LangSpecific *lang,
    struct MasterStructure *master
) {
    /* Force addresses to be taken by volatile pointers */
    volatile void *volatile_ptr;
    
    vol_ptr = scalar;
    vol_ptr = string;
    vol_ptr = nested;
    vol_ptr = user;
    vol_ptr = cunion;
    vol_ptr = pointer;
    vol_ptr = array;
    vol_ptr = callback;
    vol_ptr = lang;
    vol_ptr = master;
    
    /* Access nested members to ensure full type traversal */
    if (master) {
        vol_ptr = &master->nested.inner.deeply_nested.inner_b;
        vol_ptr = master->pointers->func_ptr;
        vol_ptr = master->arrays.struct_ptr_array[0];
        vol_ptr = master->callbacks.handlers[0];
        vol_ptr = &master->lang_struct.u.tree_node.tree_code;
    }
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    /* Declare instances of all complex types */
    struct ScalarTypes scalar_instance = {0};
    struct StringTypes string_instance = {0};
    struct NestedStruct nested_instance = {0};
    UserStruct user_instance = {0};
    union ComplexUnion union_instance = {0};
    struct PointerTypes pointer_instance = {0};
    struct ArrayTypes array_instance = {0};
    struct CallbackContainer callback_instance = {0};
    struct LangSpecific lang_instance = {0};
    struct MasterStructure master_instance = {0};
    
    /* Create pointer cycles */
    master_instance.next = &master_instance;
    master_instance.children[0] = &master_instance;
    pointer_instance.self = &pointer_instance;
    user_instance.next = &user_instance;
    
    /* Initialize arrays to prevent undefined behavior */
    for (int i = 0; i < 100; i++) {
        array_instance.fixed_array[i] = i;
    }
    
    /* Take addresses of all instances and their members */
    take_addresses(
        &scalar_instance,
        &string_instance,
        &nested_instance,
        &user_instance,
        &union_instance,
        &pointer_instance,
        &array_instance,
        &callback_instance,
        &lang_instance,
        &master_instance
    );
    
    /* Compute total size of all types */
    size_t total_size = compute_type_sizes();
    
    /* Create a checksum based on addresses and sizes */
    uintptr_t checksum = 0;
    checksum += (uintptr_t)&scalar_instance;
    checksum += (uintptr_t)&string_instance;
    checksum += (uintptr_t)&nested_instance;
    checksum += (uintptr_t)&user_instance;
    checksum += (uintptr_t)&union_instance;
    checksum += (uintptr_t)&pointer_instance;
    checksum += (uintptr_t)&array_instance;
    checksum += (uintptr_t)&callback_instance;
    checksum += (uintptr_t)&lang_instance;
    checksum += (uintptr_t)&master_instance;
    checksum += total_size;
    
    /* Print result to prevent optimization */
    printf("Type analysis checksum: 0x%lx\n", (unsigned long)checksum);
    printf("Total size of all types: %zu bytes\n", total_size);
    
    return 0;
}

/* ==================== ADDITIONAL TYPE DEFINITIONS ==================== */
/* More types to ensure TYPE_UNDEFINED might be triggered in edge cases */

/* Incomplete array type */
extern int external_array[];

/* Type with attribute that might affect gengtype processing */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

/* Anonymous struct/union */
struct AnonymousContainer {
    struct {
        int x;
        int y;
    } point;
    union {
        int i;
        float f;
    } value;
};

/* Const volatile qualified types */
const volatile struct {
    int read_only;
    int write_only;
} cv_type;

/* Aligned types */
struct __attribute__((aligned(64))) AlignedStruct {
    double data[8];
};
