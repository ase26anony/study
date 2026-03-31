/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by gengtype during GCC build, it should trigger
 * counts for all type kinds.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation */
#define GTY(x)

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* ==================== TYPE_SCALAR ==================== */
/* Basic scalar types */
GTY(())
struct Scalars {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_int;
    unsigned int unsigned_int;
    _Bool boolean;
    int8_t int8;
    int64_t int64;
};

/* ==================== TYPE_STRING ==================== */
/* String types */
GTY(())
struct Strings {
    const char *c_string;
    char *mutable_string;
    const char *const constant_string;
    char fixed_string[32];
};

/* ==================== TYPE_STRUCT ==================== */
/* Basic structure */
GTY(())
struct BasicStruct {
    int id;
    char name[64];
};

/* Nested structures */
GTY(())
struct OuterStruct {
    int outer_id;
    struct BasicStruct inner;
    struct {
        int anonymous_id;
        float anonymous_float;
    } anonymous_member;
};

/* ==================== TYPE_USER_STRUCT ==================== */
/* User-defined structure with typedef */
typedef GTY(()) struct {
    int user_id;
    char user_name[32];
    struct BasicStruct *nested;
} UserDefinedStruct;

/* Another user struct with pointer to itself */
typedef GTY(()) struct SelfRefStruct {
    int value;
    struct SelfRefStruct *next;
    struct SelfRefStruct *prev;
} SelfRefStruct;

/* ==================== TYPE_UNION ==================== */
/* Basic union */
GTY(())
union BasicUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    char as_char;
};

/* Tagged union (discriminated union) */
GTY(())
struct TaggedUnion {
    enum { INT_TYPE, FLOAT_TYPE, STRING_TYPE } tag;
    union {
        int int_value;
        float float_value;
        char *string_value;
    } data;
};

/* Union within struct */
GTY(())
struct StructWithUnion {
    int type;
    union {
        struct {
            int x, y;
        } point;
        struct {
            float radius;
        } circle;
        struct {
            int width, height;
        } rectangle;
    } shape;
};

/* ==================== TYPE_POINTER ==================== */
/* Various pointer types */
GTY(())
struct PointerFest {
    int *int_ptr;
    char **char_ptr_ptr;
    void *void_ptr;
    const int *const_int_ptr;
    volatile char *volatile_char_ptr;
    struct BasicStruct *struct_ptr;
    union BasicUnion *union_ptr;
    UserDefinedStruct *user_struct_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to function (TYPE_CALLBACK will handle function pointers) */
    int (*func_ptr)(int, char);
    
    /* Complex pointer types */
    struct ForwardDecl *forward_ptr;
    union ForwardUnion *forward_union_ptr;
    
    /* Pointer to pointer to struct */
    struct BasicStruct ***struct_ptr_ptr_ptr;
    
    /* Const pointer to volatile struct */
    const volatile struct BasicStruct *cv_struct_ptr;
};

/* ==================== TYPE_ARRAY ==================== */
/* Various array types */
GTY(())
struct ArrayCollection {
    /* Fixed size arrays */
    int fixed_array[100];
    char char_array[256];
    float float_array[16][16];
    double multi_dim[3][4][5];
    
    /* Array of pointers */
    int *pointer_array[20];
    struct BasicStruct *struct_ptr_array[50];
    
    /* Array of arrays */
    int matrix[10][10];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* Array of structures */
GTY(())
struct ArrayOfStructs {
    struct BasicStruct elements[100];
    UserDefinedStruct user_elements[50];
};

/* ==================== TYPE_CALLBACK ==================== */
/* Function pointer types */
typedef int (*BinaryOp)(int, int);
typedef void (*CallbackFunc)(void *data, int result);
typedef char *(*StringProcessor)(const char *input);

GTY(())
struct CallbackStruct {
    /* Simple function pointers */
    int (*compare)(const void *, const void *);
    void (*logger)(const char *message);
    
    /* Typedef function pointers */
    BinaryOp arithmetic_op;
    CallbackFunc completion_callback;
    StringProcessor string_processor;
    
    /* Array of function pointers */
    void (*handlers[10])(void);
    
    /* Function pointer returning pointer */
    struct BasicStruct *(*allocator)(void);
    
    /* Function pointer with complex parameters */
    int (*complex_func)(struct BasicStruct **, union BasicUnion *, int (*)(int));
};

/* ==================== TYPE_LANG_STRUCT ==================== */
/* Language-specific structure (simulating GCC internal types) */
#ifdef __cplusplus
extern "C" {
#endif

/* Simulating a language-specific structure type */
GTY(())
struct LangSpecific {
    /* Language-specific fields would go here */
    void *lang_data;
    int lang_tag;
    
    /* Nested language-specific type */
    struct {
        int lang_specific_field;
        void *lang_opaque;
    } lang_inner;
};

#ifdef __cplusplus
}
#endif

/* ==================== COMPLEX NESTED STRUCTURE ==================== */
/* Master structure containing all types */
GTY(())
struct MasterType {
    /* Scalars */
    struct Scalars scalars;
    
    /* Strings */
    struct Strings strings;
    
    /* Basic struct */
    struct BasicStruct basic;
    
    /* User struct */
    UserDefinedStruct user_struct;
    
    /* Union */
    union BasicUnion basic_union;
    
    /* Pointers */
    struct PointerFest pointers;
    
    /* Arrays */
    struct ArrayCollection arrays;
    
    /* Callbacks */
    struct CallbackStruct callbacks;
    
    /* Language struct */
    struct LangSpecific lang_struct;
    
    /* Self-referential */
    struct MasterType *next;
    struct MasterType *prev;
    
    /* Array of unions */
    union BasicUnion union_array[20];
    
    /* Function pointer array */
    void (*func_array[5])(struct MasterType *);
    
    /* Nested anonymous struct */
    struct {
        int magic;
        void *opaque;
    } extra;
};

/* ==================== FORWARD DECLARED TYPES ==================== */
/* Now define the forward declared types */
GTY(())
struct ForwardDecl {
    int value;
    struct ForwardDecl *next;
    struct PointerFest *pointers;
};

GTY(())
union ForwardUnion {
    int as_int;
    struct ForwardDecl *as_struct;
    void *as_void;
};

/* ==================== TYPE_UNDEFINED ==================== */
/* To potentially trigger TYPE_UNDEFINED, we use incomplete types */
extern struct IncompleteType;  /* Forward declared but never defined */
extern union UnknownUnion;     /* Another incomplete type */

GTY(())
struct UsesIncomplete {
    struct IncompleteType *incomplete_ptr;  /* Pointer to undefined type */
    union UnknownUnion *unknown_union_ptr;  /* Pointer to undefined union */
    void *opaque;  /* Generic opaque pointer */
};

/* ==================== EXTERNAL FUNCTION ==================== */
/* Non-inlineable function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void *data, size_t size) {
    /* Simple checksum computation */
    unsigned char *bytes = (unsigned char *)data;
    size_t sum = 0;
    for (size_t i = 0; i < size && i < 256; i++) {
        sum += bytes[i];
    }
    return sum;
}

/* Another external function to use function pointers */
__attribute__((noinline))
int dummy_callback(int a, int b) {
    return a + b;
}

/* ==================== MAIN FUNCTION ==================== */
int main(void) {
    volatile size_t total_size = 0;
    volatile size_t checksum = 0;
    
    /* Declare instances of all types */
    struct Scalars scalars_instance = {0};
    struct Strings strings_instance = {"Hello", "World", "Constant", "Fixed"};
    struct BasicStruct basic_instance = {42, "Test"};
    UserDefinedStruct user_instance = {99, "User", &basic_instance};
    union BasicUnion union_instance = {.as_int = 100};
    struct PointerFest pointers_instance = {0};
    struct ArrayCollection arrays_instance = {0};
    struct CallbackStruct callbacks_instance = {0};
    struct LangSpecific lang_instance = {0};
    struct MasterType master_instance = {0};
    struct ForwardDecl forward_instance = {0};
    union ForwardUnion forward_union_instance = {0};
    struct UsesIncomplete incomplete_instance = {0};
    
    /* Initialize function pointers */
    callbacks_instance.compare = (int (*)(const void *, const void *))dummy_callback;
    callbacks_instance.arithmetic_op = dummy_callback;
    
    /* Take addresses of all instances */
    void *addresses[] = {
        &scalars_instance,
        &strings_instance,
        &basic_instance,
        &user_instance,
        &union_instance,
        &pointers_instance,
        &arrays_instance,
        &callbacks_instance,
        &lang_instance,
        &master_instance,
        &forward_instance,
        &forward_union_instance,
        &incomplete_instance
    };
    
    /* Compute sizeof for all types */
    size_t sizes[] = {
        sizeof(struct Scalars),
        sizeof(struct Strings),
        sizeof(struct BasicStruct),
        sizeof(UserDefinedStruct),
        sizeof(union BasicUnion),
        sizeof(struct PointerFest),
        sizeof(struct ArrayCollection),
        sizeof(struct CallbackStruct),
        sizeof(struct LangSpecific),
        sizeof(struct MasterType),
        sizeof(struct ForwardDecl),
        sizeof(union ForwardUnion),
        sizeof(struct UsesIncomplete),
        sizeof(struct OuterStruct),
        sizeof(struct TaggedUnion),
        sizeof(struct StructWithUnion),
        sizeof(struct ArrayOfStructs),
        sizeof(SelfRefStruct),
        sizeof(int*),
        sizeof(char**),
        sizeof(void*),
        sizeof(int[10]),
        sizeof(float[3][4]),
        sizeof(int(*)(int, int)),
        sizeof(void(*[5])(void))
    };
    
    /* Sum all sizes (prevents optimization) */
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Compute checksums (forces compiler to consider the data) */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        checksum += compute_checksum(addresses[i], 16);
    }
    
    /* Use all instances to prevent dead code elimination */
    scalars_instance.integer = (int)total_size;
    strings_instance.c_string = (checksum > 0) ? "Non-zero" : "Zero";
    basic_instance.id = (int)checksum;
    user_instance.user_id = (int)(total_size + checksum);
    
    /* Create pointer cycles */
    SelfRefStruct self_ref1 = {1, NULL, NULL};
    SelfRefStruct self_ref2 = {2, &self_ref1, NULL};
    self_ref1.next = &self_ref2;
    self_ref1.prev = &self_ref2;
    
    /* Use function pointers */
    if (callbacks_instance.arithmetic_op) {
        int result = callbacks_instance.arithmetic_op(10, 20);
        total_size += result;
    }
    
    /* Print results to ensure no optimization */
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Checksum: %zu\n", checksum);
    printf("Self-ref struct values: %d, %d\n", self_ref1.value, self_ref2.value);
    
    /* Return something based on the computations */
    return (total_size > 0 && checksum > 0) ? 0 : 1;
}

/* Additional unused types to ensure they're in the type graph */
GTY(())
struct UnusedType1 {
    int matrix[100][100];
    struct UnusedType1 *self_ptr[10];
};

GTY(())
union UnusedUnion1 {
    long double ld;
    struct MasterType *mt;
    void (*funcs[20])(void);
};

/* Global variables to force type inclusion */
struct MasterType GTY((skip)) global_master;
UserDefinedStruct GTY((skip)) global_user_struct;
struct CallbackStruct GTY((skip)) global_callbacks;
