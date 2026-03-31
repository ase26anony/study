/* test_gengtype_coverage.c
 * 
 * A test program to exercise gengtype's type enumeration logic.
 * Defines complex nested structures with GTY markers to ensure
 * all type kinds are processed during gengtype analysis.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) /* nothing */
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to prevent inlining */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile static int sink;
    sink = (int)(intptr_t)p;
}

/* ========== TYPE DEFINITIONS ========== */

/* TYPE_SCALAR: Basic scalar types */
typedef GTY(()) struct ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_prec;
    long long int64;
    unsigned short word;
    _Bool boolean;
} ScalarTypes;

/* TYPE_STRING: String types */
typedef GTY(()) struct StringTypes {
    const char *constant_string;
    char *mutable_string;
    wchar_t *wide_string;
} StringTypes;

/* TYPE_POINTER: Various pointer types */
typedef GTY(()) struct PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct PointerTypes *self_ptr;  /* Recursive pointer */
    void (**func_ptr_array)(void);  /* Array of function pointers */
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef GTY(()) struct ArrayTypes {
    int fixed_array[10];
    char multi_dim[5][5];
    float flexible_array[];  /* Flexible array member */
} ArrayTypes;

/* TYPE_STRUCT: Nested structure */
typedef GTY(()) struct NestedStruct {
    int depth;
    struct InnerStruct {
        int inner_data;
        struct DeeperStruct {
            char deepest;
        } deeper;
    } inner;
} NestedStruct;

/* TYPE_UNION: Union types */
typedef GTY(()) union ComplexUnion {
    int as_int;
    float as_float;
    struct {
        char byte1;
        char byte2;
    } as_bytes;
    void *as_pointer;
} ComplexUnion;

/* TYPE_USER_STRUCT: User-defined structure with tag */
struct user_defined;  /* Forward declaration */
typedef GTY(()) struct UserStructWrapper {
    struct user_defined *user_ptr;
    int wrapper_data;
} UserStructWrapper;

/* Actual user-defined structure */
typedef GTY(()) struct user_defined {
    int user_id;
    char user_name[32];
} user_defined;

/* TYPE_CALLBACK: Function pointer types */
typedef GTY(()) struct CallbackContainer {
    int (*comparator)(const void *, const void *);
    void (*callback)(void *data);
    char *(*string_processor)(char *);
} CallbackContainer;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef GTY(()) struct LangSpecific {
    void *tree_node;      /* Simulating GCC's tree_node */
    void *rtx_code;       /* Simulating RTL */
    struct LangSpecific *next;
} LangSpecific;

/* Master structure containing all types */
typedef GTY(()) struct MasterType {
    /* Scalar section */
    ScalarTypes scalars;
    
    /* String section */
    StringTypes strings;
    
    /* Pointer section */
    PointerTypes pointers;
    
    /* Array section */
    ArrayTypes *array_ptr;  /* Pointer to array struct */
    
    /* Nested structure */
    NestedStruct nested;
    
    /* Union */
    ComplexUnion union_data;
    
    /* User struct */
    UserStructWrapper user_struct_wrapper;
    
    /* Callbacks */
    CallbackContainer callbacks;
    
    /* Language-specific */
    LangSpecific *lang_struct;
    
    /* Direct arrays and pointers */
    int direct_array[20];
    void *direct_pointer;
    
    /* Self-reference for graph traversal */
    struct MasterType *next;
    struct MasterType *prev;
} MasterType;

/* TYPE_UNDEFINED: Forward declared incomplete type */
typedef struct UndefinedType UndefinedType;

/* Container that might reference undefined type */
typedef GTY(()) struct UndefinedContainer {
    UndefinedType *undefined_ptr;  /* TYPE_UNDEFINED when processed */
    int defined_data;
} UndefinedContainer;

/* ========== FUNCTION DEFINITIONS ========== */

/* Callback function implementations */
int sample_comparator(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

void sample_callback(void *data) {
    volatile static int counter;
    counter++;
}

char *sample_string_processor(char *str) {
    if (str) str[0] = 'X';
    return str;
}

/* ========== MAIN FUNCTION ========== */

int main() {
    /* Declare instances of all complex types */
    ScalarTypes scalars = {0};
    StringTypes strings = {0};
    PointerTypes pointers = {0};
    ArrayTypes *array_ptr = NULL;
    NestedStruct nested = {0};
    ComplexUnion union_data = {0};
    UserStructWrapper user_wrapper = {0};
    user_defined user_def = {0};
    CallbackContainer callbacks = {0};
    LangSpecific lang_spec = {0};
    MasterType master = {0};
    UndefinedContainer undefined_container = {0};
    
    /* Initialize pointers */
    strings.constant_string = "Hello, gengtype!";
    
    /* Set up callbacks */
    callbacks.comparator = sample_comparator;
    callbacks.callback = sample_callback;
    callbacks.string_processor = sample_string_processor;
    
    /* Link structures */
    master.scalars = scalars;
    master.strings = strings;
    master.pointers = pointers;
    master.nested = nested;
    master.union_data = union_data;
    master.user_struct_wrapper.user_ptr = &user_def;
    master.callbacks = callbacks;
    master.lang_struct = &lang_spec;
    
    /* Create circular reference for graph traversal */
    master.next = &master;
    master.prev = &master;
    
    /* Force compiler to consider all types through various operations */
    
    /* 1. sizeof operations on all types */
    size_t total_size = 0;
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringTypes);
    total_size += sizeof(PointerTypes);
    total_size += sizeof(NestedStruct);
    total_size += sizeof(ComplexUnion);
    total_size += sizeof(UserStructWrapper);
    total_size += sizeof(user_defined);
    total_size += sizeof(CallbackContainer);
    total_size += sizeof(LangSpecific);
    total_size += sizeof(MasterType);
    total_size += sizeof(UndefinedContainer);
    
    /* 2. Take addresses of all instances and members */
    void *addresses[] = {
        &scalars, &scalars.integer, &scalars.character,
        &strings, &strings.constant_string,
        &pointers, &pointers.self_ptr,
        &nested, &nested.inner, &nested.inner.deeper,
        &union_data, &union_data.as_int, &union_data.as_bytes,
        &user_wrapper, &user_def,
        &callbacks, &callbacks.comparator,
        &lang_spec,
        &master, &master.direct_array, &master.direct_pointer,
        &undefined_container
    };
    
    /* 3. Use pointers to force type analysis */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_pointer(addresses[i]);
    }
    
    /* 4. Access all structure members to ensure they're used */
    scalars.integer = 42;
    scalars.character = 'A';
    scalars.floating = 3.14f;
    
    nested.depth = 10;
    nested.inner.inner_data = 20;
    nested.inner.deeper.deepest = 'Z';
    
    union_data.as_int = 0xDEADBEEF;
    
    user_def.user_id = 100;
    user_def.user_name[0] = 'T';
    
    /* 5. Compute offsetof to ensure layout is considered */
    size_t offsets[] = {
        offsetof(MasterType, scalars),
        offsetof(MasterType, strings),
        offsetof(MasterType, pointers),
        offsetof(MasterType, nested),
        offsetof(MasterType, union_data),
        offsetof(MasterType, user_struct_wrapper),
        offsetof(MasterType, callbacks),
        offsetof(MasterType, lang_struct),
        offsetof(NestedStruct, inner),
        offsetof(NestedStruct, inner.deeper)
    };
    
    for (size_t i = 0; i < sizeof(offsets)/sizeof(offsets[0]); i++) {
        total_size += offsets[i];
    }
    
    /* 6. Prevent dead code elimination */
    KEEP_ALIVE(&scalars);
    KEEP_ALIVE(&strings);
    KEEP_ALIVE(&nested);
    KEEP_ALIVE(&union_data);
    KEEP_ALIVE(&user_def);
    KEEP_ALIVE(&callbacks);
    KEEP_ALIVE(&master);
    
    /* Print something to prevent optimization */
    printf("Type analysis test - Total size metric: %zu\n", total_size);
    printf("Address of master structure: %p\n", (void*)&master);
    
    return (int)(total_size % 256);
}
