/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types
 * should trigger multiple cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would
 * be the actual garbage collector annotation */
#define GTY(x)

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to ensure types are referenced */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile int sink = (int)(intptr_t)p;
    (void)sink;
}

/* ========== TYPE DEFINITIONS ========== */

/* Basic scalar types - should trigger TYPE_SCALAR */
GTY(())
struct ScalarStruct {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_value;
    short short_value;
    unsigned int unsigned_integer;
    _Bool boolean;
};

/* String type - should trigger TYPE_STRING */
GTY(())
struct StringStruct {
    const char *string_literal;
    char *dynamic_string;
    const char *const constant_string;
};

/* Nested struct - should trigger TYPE_STRUCT and TYPE_USER_STRUCT */
GTY(())
struct InnerStruct {
    int inner_data;
    float inner_float;
};

GTY(())
struct OuterStruct {
    struct InnerStruct nested;
    struct InnerStruct *nested_ptr;
    int outer_data;
};

/* Union type - should trigger TYPE_UNION */
GTY(())
union DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    struct {
        int tag;
        union DataUnion *next;
    } recursive;
};

/* Pointer types - should trigger TYPE_POINTER */
GTY(())
struct PointerStruct {
    int *int_ptr;
    void *void_ptr;
    struct PointerStruct *self_ptr;
    struct PointerStruct **double_ptr;
    int (*function_ptr)(int, char);
    void (*callback)(void *);
    union DataUnion *union_ptr;
};

/* Array types - should trigger TYPE_ARRAY */
GTY(())
struct ArrayStruct {
    int fixed_array[10];
    char string_array[5][20];
    struct InnerStruct struct_array[3];
    int *pointer_array[5];
    int flexible_array[];
};

/* Callback/function pointer types - should trigger TYPE_CALLBACK */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int, void *);

GTY(())
struct CallbackStruct {
    Comparator compare_func;
    EventHandler event_handler;
    void (*simple_callback)(void);
    int (*complex_callback)(struct CallbackStruct *, int, ...);
};

/* Complex nested structure combining all types */
GTY(())
struct MasterStruct {
    /* Scalar members */
    int id;
    char tag;
    
    /* String member */
    const char *name;
    
    /* Struct members */
    struct InnerStruct inner;
    struct OuterStruct outer;
    
    /* Union member */
    union DataUnion data;
    
    /* Pointer members */
    struct MasterStruct *next;
    struct MasterStruct **prev;
    int *data_ptr;
    
    /* Array members */
    int scores[5];
    struct InnerStruct items[3];
    char buffer[256];
    
    /* Callback members */
    void (*cleanup)(struct MasterStruct *);
    int (*validate)(const struct MasterStruct *);
    
    /* Nested anonymous struct */
    struct {
        int flags;
        unsigned int counter;
    } state;
    
    /* Nested anonymous union */
    union {
        int error_code;
        void *error_data;
    } error;
};

/* Language-specific structure simulation - TYPE_LANG_STRUCT */
/* In GCC context, this would be language-specific internal types */
GTY(())
struct LangStruct {
    void *tree_node;      /* Simulating GCC's tree_node */
    void *rtx_code;       /* Simulating RTL */
    int lang_specific;
};

/* Undefined type reference - TYPE_UNDEFINED */
/* Forward declaration without definition */
struct UndefinedType;

GTY(())
struct ReferencesUndefined {
    struct UndefinedType *undefined_ptr;  /* TYPE_UNDEFINED when referenced */
    int defined_field;
};

/* ========== MAIN FUNCTION ========== */

int main(void) {
    /* Declare instances of all complex types */
    struct ScalarStruct scalar_instance = {0};
    struct StringStruct string_instance = {0};
    struct OuterStruct outer_instance = {0};
    union DataUnion union_instance = {0};
    struct PointerStruct pointer_instance = {0};
    struct ArrayStruct *array_instance = NULL;
    struct CallbackStruct callback_instance = {0};
    struct MasterStruct master_instance = {0};
    struct LangStruct lang_instance = {0};
    struct ReferencesUndefined undefined_ref = {0};
    
    /* Take addresses to ensure types are referenced */
    volatile struct ScalarStruct *volatile_scalar = &scalar_instance;
    volatile struct StringStruct *volatile_string = &string_instance;
    volatile struct OuterStruct *volatile_outer = &outer_instance;
    volatile union DataUnion *volatile_union = &union_instance;
    volatile struct PointerStruct *volatile_pointer = &pointer_instance;
    volatile struct CallbackStruct *volatile_callback = &callback_instance;
    volatile struct MasterStruct *volatile_master = &master_instance;
    volatile struct LangStruct *volatile_lang = &lang_instance;
    volatile struct ReferencesUndefined *volatile_undefined = &undefined_ref;
    
    /* Compute sizeof all types - forces compiler to consider type layouts */
    size_t sizes[] = {
        sizeof(struct ScalarStruct),
        sizeof(struct StringStruct),
        sizeof(struct InnerStruct),
        sizeof(struct OuterStruct),
        sizeof(union DataUnion),
        sizeof(struct PointerStruct),
        sizeof(struct CallbackStruct),
        sizeof(struct MasterStruct),
        sizeof(struct LangStruct),
        sizeof(struct ReferencesUndefined),
        sizeof(int*),
        sizeof(void(*)(void)),
        sizeof(int[10]),
        sizeof(struct InnerStruct[3])
    };
    
    /* Use external function to prevent optimization */
    use_pointer(&scalar_instance);
    use_pointer(&string_instance);
    use_pointer(&outer_instance);
    use_pointer(&union_instance);
    use_pointer(&pointer_instance);
    use_pointer(&callback_instance);
    use_pointer(&master_instance);
    use_pointer(&lang_instance);
    use_pointer(&undefined_ref);
    
    /* Calculate checksum to ensure all operations have effect */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Reference array type (requires allocation for flexible array) */
    array_instance = (struct ArrayStruct*)malloc(
        sizeof(struct ArrayStruct) + 10 * sizeof(int));
    if (array_instance) {
        use_pointer(array_instance);
        free(array_instance);
    }
    
    /* Prevent dead code elimination */
    KEEP_ALIVE(volatile_scalar);
    KEEP_ALIVE(volatile_string);
    KEEP_ALIVE(volatile_outer);
    KEEP_ALIVE(volatile_union);
    KEEP_ALIVE(volatile_pointer);
    KEEP_ALIVE(volatile_callback);
    KEEP_ALIVE(volatile_master);
    KEEP_ALIVE(volatile_lang);
    KEEP_ALIVE(volatile_undefined);
    
    printf("Type analysis test complete. Total size sum: %zu\n", total_size);
    printf("If processed by gengtype, this should exercise the type switch.\n");
    
    return 0;
}
