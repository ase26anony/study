/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types should
 * trigger multiple cases in the switch statement.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#ifdef __GNUC__
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))
#else
#define NOINLINE
#define USED
#endif

/* External function to prevent dead code elimination */
NOINLINE void use_pointer(const void *ptr);

/* TYPE_SCALAR: Basic scalar types */
GTY(()) struct ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    long long_field;
    short short_field;
    unsigned uint_field;
    _Bool bool_field;
};

/* TYPE_STRING: String type */
GTY(()) struct StringType {
    const char *string_field;  /* TYPE_STRING */
    char *mutable_string;
};

/* TYPE_STRUCT: Regular structure */
GTY(()) struct RegularStruct {
    int id;
    struct ScalarTypes scalars;  /* Nested struct */
};

/* TYPE_USER_STRUCT: User-defined structure */
typedef GTY(()) struct UserDefinedStruct {
    int user_id;
    char user_name[32];  /* TYPE_ARRAY */
    struct RegularStruct *nested;  /* TYPE_POINTER to TYPE_STRUCT */
} UserStruct;

/* TYPE_UNION: Union type */
GTY(()) union ComplexUnion {
    int int_val;
    float float_val;
    double double_val;
    void *ptr_val;  /* TYPE_POINTER */
    struct {
        char data[16];  /* TYPE_ARRAY inside anonymous struct */
    } struct_in_union;
};

/* TYPE_ARRAY: Array types */
GTY(()) struct ArrayTypes {
    int fixed_array[10];           /* Fixed-size array */
    int multi_dim[3][4][5];        /* Multi-dimensional array */
    struct RegularStruct struct_array[5];  /* Array of structs */
    union ComplexUnion union_array[8];     /* Array of unions */
};

/* TYPE_POINTER: Various pointer types */
GTY(()) struct PointerTypes {
    int *int_ptr;                   /* Pointer to scalar */
    struct RegularStruct *struct_ptr;  /* Pointer to struct */
    union ComplexUnion *union_ptr;     /* Pointer to union */
    struct ArrayTypes *array_ptr;      /* Pointer to array struct */
    
    /* Function pointer - TYPE_CALLBACK */
    int (*callback_func)(int, char*);  /* TYPE_CALLBACK */
    
    /* Pointer to pointer */
    void **void_ptr_ptr;
    
    /* Pointer to array */
    int (*array_ptr_field)[10];
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
GTY(()) struct LangStruct {
    void *lang_data;
    int lang_tag;
    struct LangStruct *next;  /* Self-referential pointer */
};

/* Complex nested structure combining all types */
GTY(()) struct MasterType {
    /* TYPE_SCALAR */
    int master_id;
    
    /* TYPE_STRING */
    const char *master_name;
    
    /* TYPE_STRUCT */
    struct ScalarTypes scalars;
    
    /* TYPE_USER_STRUCT */
    UserStruct user_struct;
    
    /* TYPE_UNION */
    union ComplexUnion data_union;
    
    /* TYPE_ARRAY */
    struct PointerTypes *ptr_array[5];  /* Array of pointers */
    
    /* TYPE_POINTER */
    struct ArrayTypes *array_data;
    
    /* TYPE_CALLBACK */
    void (*master_callback)(struct MasterType*);
    
    /* TYPE_LANG_STRUCT */
    struct LangStruct *lang_data;
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* Function to use all types and prevent optimization */
NOINLINE void use_all_types(void) {
    /* Declare instances of each type */
    volatile struct ScalarTypes scalars = {0};
    volatile struct StringType strings = {0};
    volatile struct RegularStruct regular = {0};
    volatile UserStruct user_struct = {0};
    volatile union ComplexUnion cunion = {0};
    volatile struct ArrayTypes arrays = {0};
    volatile struct PointerTypes pointers = {0};
    volatile struct LangStruct lang_struct = {0};
    volatile struct MasterType *master_ptr = NULL;
    
    /* Take addresses to ensure types are considered */
    use_pointer(&scalars);
    use_pointer(&strings);
    use_pointer(&regular);
    use_pointer(&user_struct);
    use_pointer(&cunion);
    use_pointer(&arrays);
    use_pointer(&pointers);
    use_pointer(&lang_struct);
    use_pointer(&master_ptr);
    
    /* Compute sizes - forces compiler to consider type layouts */
    volatile size_t sizes = 0;
    sizes += sizeof(struct ScalarTypes);
    sizes += sizeof(struct StringType);
    sizes += sizeof(struct RegularStruct);
    sizes += sizeof(UserStruct);
    sizes += sizeof(union ComplexUnion);
    sizes += sizeof(struct ArrayTypes);
    sizes += sizeof(struct PointerTypes);
    sizes += sizeof(struct LangStruct);
    sizes += sizeof(struct MasterType);
    
    /* Access members through pointers to force type analysis */
    struct PointerTypes *ptrs = (struct PointerTypes*)&pointers;
    ptrs->int_ptr = (int*)&scalars.int_field;
    ptrs->struct_ptr = (struct RegularStruct*)&regular;
    ptrs->callback_func = NULL;  /* TYPE_CALLBACK */
    
    /* Array access */
    struct ArrayTypes *arrs = (struct ArrayTypes*)&arrays;
    arrs->fixed_array[0] = 42;
    arrs->struct_array[0].id = 1;
    
    /* Union access */
    union ComplexUnion *un = (union ComplexUnion*)&cunion;
    un->int_val = 100;
    un->ptr_val = (void*)&strings;
    
    /* String access */
    struct StringType *strs = (struct StringType*)&strings;
    strs->string_field = "constant string";
    strs->mutable_string = (char*)"mutable string";
    
    /* Print something to prevent complete optimization */
    printf("Type analysis test - total size: %zu\n", sizes);
}

/* Function that uses a pointer (prevents dead code elimination) */
NOINLINE void use_pointer(const void *ptr) {
    /* Volatile to prevent optimization */
    volatile const void *volatile_ptr = ptr;
    (void)volatile_ptr;  /* Suppress unused warning */
}

/* Main function that drives type usage */
int main(void) {
    printf("Starting gengtype coverage test...\n");
    
    /* Use all complex types */
    use_all_types();
    
    /* Additional type references in main */
    struct MasterType master_instance = {0};
    struct MasterType *master_array[3] = {0};
    
    /* Take addresses of various type components */
    int *int_ptr = &master_instance.master_id;
    const char **string_ptr = &master_instance.master_name;
    struct ScalarTypes *scalar_ptr = &master_instance.scalars;
    UserStruct *user_ptr = &master_instance.user_struct;
    union ComplexUnion *union_ptr = &master_instance.data_union;
    struct PointerTypes **array_elem = &master_instance.ptr_array[0];
    struct ArrayTypes **array_data_ptr = &master_instance.array_data;
    void (**callback_ptr)(struct MasterType*) = &master_instance.master_callback;
    struct LangStruct **lang_ptr = &master_instance.lang_data;
    
    /* Prevent optimization of these pointers */
    use_pointer(int_ptr);
    use_pointer(string_ptr);
    use_pointer(scalar_ptr);
    use_pointer(user_ptr);
    use_pointer(union_ptr);
    use_pointer(array_elem);
    use_pointer(array_data_ptr);
    use_pointer(callback_ptr);
    use_pointer(lang_ptr);
    
    /* Compute offsetof for various members - forces layout analysis */
    size_t offsets[] = {
        offsetof(struct MasterType, master_id),
        offsetof(struct MasterType, master_name),
        offsetof(struct MasterType, scalars),
        offsetof(struct MasterType, user_struct),
        offsetof(struct MasterType, data_union),
        offsetof(struct MasterType, ptr_array),
        offsetof(struct MasterType, array_data),
        offsetof(struct MasterType, master_callback),
        offsetof(struct MasterType, lang_data),
    };
    
    /* Use offsets to prevent optimization */
    for (size_t i = 0; i < sizeof(offsets)/sizeof(offsets[0]); i++) {
        use_pointer((const void*)offsets[i]);
    }
    
    printf("Test completed.\n");
    return 0;
}
