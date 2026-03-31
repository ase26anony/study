/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to trigger type enumeration
 * in gengtype.cc's switch statement (lines 182-213).
 * 
 * Compilation for gengtype testing:
 * 1. Place in GCC source tree where gengtype processes headers
 * 2. Use: gcc -c -O0 -g -fdump-tree-all -ffat-lto-objects test_gengtype_coverage.c
 * 3. Or integrate into GCC build with GTY markers
 */

/* Dummy GTY macro for standalone compilation - would be GCC's actual GTY in real build */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* ========== TYPE DEFINITIONS TO COVER ALL SWITCH CASES ========== */

/* TYPE_SCALAR: Basic scalar types */
struct GTY(()) ScalarContainer {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    _Bool bool_field;       /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
};

/* TYPE_STRING: String types */
struct GTY(()) StringContainer {
    const char* static_string;      /* TYPE_STRING */
    char* dynamic_string;           /* TYPE_POINTER (to scalar) */
    const char* const const_string; /* TYPE_STRING */
};

/* TYPE_STRUCT: Nested structures */
struct GTY(()) InnerStruct {
    int x;
    int y;
};

struct GTY(()) OuterStruct {
    struct InnerStruct inner;      /* TYPE_STRUCT */
    struct InnerStruct* inner_ptr; /* TYPE_POINTER */
};

/* TYPE_USER_STRUCT: User-defined structure */
typedef struct GTY(()) UserDefined {
    int id;
    char name[32];
} UserDefined_t;

/* TYPE_UNION: Union types */
union GTY(()) DataUnion {
    int as_int;
    float as_float;
    void* as_pointer;
    struct InnerStruct as_struct;
};

/* TYPE_POINTER: Various pointer types */
struct GTY(()) PointerContainer {
    int* int_ptr;                   /* TYPE_POINTER to scalar */
    struct InnerStruct* struct_ptr; /* TYPE_POINTER to struct */
    union DataUnion* union_ptr;     /* TYPE_POINTER to union */
    void* void_ptr;                 /* TYPE_POINTER */
    int** double_ptr;               /* TYPE_POINTER to pointer */
};

/* TYPE_ARRAY: Array types */
struct GTY(()) ArrayContainer {
    int fixed_array[10];            /* TYPE_ARRAY of scalars */
    struct InnerStruct struct_array[5]; /* TYPE_ARRAY of structs */
    char* pointer_array[8];         /* TYPE_ARRAY of pointers */
    int flexible_array[];           /* Flexible array member */
};

/* TYPE_CALLBACK: Function pointers */
typedef int (*callback_func)(int, void*);

struct GTY(()) CallbackContainer {
    callback_func func_ptr;         /* TYPE_CALLBACK */
    void (*void_func)(void);        /* TYPE_CALLBACK */
    int (*array_func[3])(void);     /* TYPE_ARRAY of callbacks */
};

/* TYPE_LANG_STRUCT: Language-specific structure (simulated) */
struct GTY(()) LangSpecific {
    void* lang_data;
    int lang_tag;
};

/* TYPE_UNDEFINED: Forward declaration creates undefined type initially */
struct GTY(()) ForwardDeclared;
struct GTY(()) UsesForward {
    struct ForwardDeclared* fwd_ptr; /* TYPE_POINTER to undefined */
};

struct GTY(()) ForwardDeclared {
    int defined_now;
};

/* ========== COMPLEX NESTED STRUCTURE ========== */

/* Master structure containing all type kinds */
struct GTY(()) MasterType {
    /* Scalars */
    int master_id;
    double master_value;
    
    /* Strings */
    const char* master_name;
    
    /* Structures */
    struct ScalarContainer scalars;
    struct StringContainer strings;
    struct OuterStruct nested_struct;
    
    /* User struct */
    UserDefined_t user_struct;
    
    /* Union */
    union DataUnion data_union;
    
    /* Pointers */
    struct PointerContainer* ptr_container;
    
    /* Arrays */
    struct ArrayContainer array_container;
    int multi_dim_array[3][4][5];
    
    /* Callbacks */
    struct CallbackContainer callbacks;
    
    /* Language-specific */
    struct LangSpecific lang_struct;
    
    /* Forward reference handling */
    struct UsesForward forward_user;
    
    /* Self-reference (circular) */
    struct MasterType* self_ptr;
    
    /* Union containing struct */
    union {
        struct InnerStruct as_inner;
        int as_int;
    } anonymous_union;
    
    /* Nested anonymous struct */
    struct {
        int anonymous_field;
        struct MasterType* parent_ptr;
    } anonymous_struct;
};

/* ========== FUNCTION TO FORCE TYPE USAGE ========== */

NOINLINE USED
static size_t compute_type_sizes(void) {
    size_t total_size = 0;
    
    /* Declare instances of all types */
    volatile struct ScalarContainer scalars;
    volatile struct StringContainer strings;
    volatile struct OuterStruct outer;
    volatile UserDefined_t user_def;
    volatile union DataUnion data_union;
    volatile struct PointerContainer pointers;
    volatile struct ArrayContainer arrays;
    volatile struct CallbackContainer callbacks;
    volatile struct LangSpecific lang;
    volatile struct UsesForward forward_use;
    volatile struct ForwardDeclared forward_def;
    volatile struct MasterType master;
    
    /* Take addresses to ensure types are considered */
    volatile void* addresses[] = {
        &scalars, &strings, &outer, &user_def, &data_union,
        &pointers, &arrays, &callbacks, &lang, &forward_use,
        &forward_def, &master
    };
    
    /* Compute sizes of all types */
    total_size += sizeof(struct ScalarContainer);
    total_size += sizeof(struct StringContainer);
    total_size += sizeof(struct OuterStruct);
    total_size += sizeof(UserDefined_t);
    total_size += sizeof(union DataUnion);
    total_size += sizeof(struct PointerContainer);
    total_size += sizeof(struct ArrayContainer) - sizeof(int); /* exclude flexible */
    total_size += sizeof(struct CallbackContainer);
    total_size += sizeof(struct LangSpecific);
    total_size += sizeof(struct UsesForward);
    total_size += sizeof(struct ForwardDeclared);
    total_size += sizeof(struct MasterType);
    
    /* Access members to ensure they're not optimized away */
    scalars.int_field = 1;
    strings.static_string = "test";
    outer.inner.x = 2;
    user_def.id = 3;
    data_union.as_int = 4;
    
    if (pointers.int_ptr) {
        *pointers.int_ptr = 5;
    }
    
    arrays.fixed_array[0] = 6;
    callbacks.func_ptr = NULL;
    lang.lang_tag = 7;
    forward_use.fwd_ptr = NULL;
    forward_def.defined_now = 8;
    master.master_id = 9;
    
    /* Use addresses to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        if (addresses[i]) {
            total_size += (size_t)addresses[i] % 256;
        }
    }
    
    return total_size;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Force type usage and get a checksum */
    size_t type_size_sum = compute_type_sizes();
    
    /* Print result to prevent optimization */
    printf("Type size checksum: %zu\n", type_size_sum);
    printf("Structure sizes:\n");
    printf("  ScalarContainer: %zu\n", sizeof(struct ScalarContainer));
    printf("  StringContainer: %zu\n", sizeof(struct StringContainer));
    printf("  OuterStruct: %zu\n", sizeof(struct OuterStruct));
    printf("  UserDefined_t: %zu\n", sizeof(UserDefined_t));
    printf("  DataUnion: %zu\n", sizeof(union DataUnion));
    printf("  PointerContainer: %zu\n", sizeof(struct PointerContainer));
    printf("  ArrayContainer (base): %zu\n", sizeof(struct ArrayContainer) - sizeof(int));
    printf("  CallbackContainer: %zu\n", sizeof(struct CallbackContainer));
    printf("  LangSpecific: %zu\n", sizeof(struct LangSpecific));
    printf("  MasterType: %zu\n", sizeof(struct MasterType));
    
    return (int)(type_size_sum % 256);
}
