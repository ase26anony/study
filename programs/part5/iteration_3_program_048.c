/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to trigger
 * all type enumeration cases in gengtype.cc's switch statement.
 * When processed by gengtype during a GCC build, it should cause
 * the uncovered lines to be executed.
 */

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* External function to prevent optimization */
extern void use_pointer(void *p) __attribute__((noinline));

/* TYPE_SCALAR triggers */
typedef struct GTY(()) ScalarTypes {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;   /* TYPE_SCALAR */
    _Bool bool_field;      /* TYPE_SCALAR */
    long long_field;       /* TYPE_SCALAR */
} ScalarTypes;

/* TYPE_STRING trigger */
typedef struct GTY(()) StringTypes {
    const char *string1;    /* TYPE_STRING */
    char *string2;          /* TYPE_STRING */
    const char * const constant_string; /* TYPE_STRING */
} StringTypes;

/* TYPE_STRUCT and nested structures */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    float inner_float;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;     /* TYPE_STRUCT */
    int outer_data;
} OuterStruct;

/* TYPE_USER_STRUCT - A struct that references itself */
typedef struct GTY(()) SelfRefStruct {
    int data;
    struct SelfRefStruct *next;  /* Self-reference for TYPE_USER_STRUCT */
} SelfRefStruct;

/* TYPE_UNION */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
} DataUnion;

/* TYPE_POINTER variations */
typedef struct GTY(()) PointerTypes {
    int *int_ptr;           /* TYPE_POINTER to TYPE_SCALAR */
    char **char_ptr_ptr;    /* TYPE_POINTER to TYPE_POINTER */
    void *generic_ptr;      /* TYPE_POINTER */
    struct PointerTypes *self_ptr; /* TYPE_POINTER to TYPE_USER_STRUCT */
    
    /* Function pointer - TYPE_CALLBACK */
    int (*compare_func)(const void *, const void *);
    
    /* Another function pointer */
    void (*callback)(int, char);
} PointerTypes;

/* TYPE_ARRAY variations */
typedef struct GTY(()) ArrayTypes {
    int fixed_array[10];           /* TYPE_ARRAY of TYPE_SCALAR */
    char *ptr_array[5];            /* TYPE_ARRAY of TYPE_POINTER */
    float multi_dim[3][4];         /* TYPE_ARRAY of TYPE_ARRAY */
    
    /* Flexible array member */
    int flexible_array[];          /* TYPE_ARRAY */
} ArrayTypes;

/* TYPE_LANG_STRUCT - Simulating GCC language-specific structure */
typedef struct GTY(()) LangStruct {
    /* Language-specific fields */
    unsigned lang_specific : 8;
    unsigned another_flag : 1;
    
    /* Mixed types */
    union {
        int lang_int;
        void *lang_ptr;
    } u;
    
    /* Array of function pointers */
    void (*lang_callbacks[3])(void);
} LangStruct;

/* Complex nested type combining everything */
typedef struct GTY(()) MegaStruct {
    /* TYPE_SCALAR */
    int id;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_STRUCT */
    InnerStruct inner;
    
    /* TYPE_UNION */
    DataUnion data;
    
    /* TYPE_POINTER */
    struct MegaStruct *parent;
    MegaStruct *children[5];  /* TYPE_ARRAY of TYPE_POINTER */
    
    /* TYPE_ARRAY */
    int scores[20];
    
    /* TYPE_CALLBACK */
    void (*cleanup)(struct MegaStruct *);
    
    /* TYPE_LANG_STRUCT */
    LangStruct lang_info;
    
    /* Nested anonymous struct */
    struct {
        int anonymous_data;
        float anonymous_float;
    } anonymous;
    
    /* Bitfields - TYPE_SCALAR */
    unsigned flags : 4;
    unsigned status : 2;
} MegaStruct;

/* Union containing structs */
typedef union GTY(()) ComplexUnion {
    ScalarTypes as_scalars;
    OuterStruct as_struct;
    ArrayTypes as_array;
    PointerTypes as_pointers;
} ComplexUnion;

/* Forward declaration for circular reference */
typedef struct GTY(()) ForwardDeclared ForwardDeclared;

struct GTY(()) ForwardDeclared {
    int data;
    ForwardDeclared *next;  /* Circular reference */
};

/* External function definition to prevent optimization */
void use_pointer(void *p) {
    /* Volatile to prevent optimization */
    volatile int sink = 0;
    if (p) sink = 1;
}

/* Function to calculate checksum of type sizes */
size_t calculate_type_sizes(void) {
    size_t total_size = 0;
    
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringTypes);
    total_size += sizeof(InnerStruct);
    total_size += sizeof(OuterStruct);
    total_size += sizeof(SelfRefStruct);
    total_size += sizeof(DataUnion);
    total_size += sizeof(PointerTypes);
    total_size += sizeof(ArrayTypes);
    total_size += sizeof(LangStruct);
    total_size += sizeof(MegaStruct);
    total_size += sizeof(ComplexUnion);
    total_size += sizeof(ForwardDeclared);
    
    return total_size;
}

int main(void) {
    /* Declare instances of all types */
    ScalarTypes scalars = {0};
    StringTypes strings = {0};
    InnerStruct inner = {0};
    OuterStruct outer = {0};
    SelfRefStruct self_ref = {0};
    DataUnion data_union = {0};
    PointerTypes pointers = {0};
    ArrayTypes arrays = {0};
    LangStruct lang_struct = {0};
    MegaStruct mega_struct = {0};
    ComplexUnion complex_union = {0};
    ForwardDeclared forward_declared = {0};
    
    /* Take addresses to ensure types are referenced */
    volatile void *addresses[] = {
        &scalars,
        &strings,
        &inner,
        &outer,
        &self_ref,
        &data_union,
        &pointers,
        &arrays,
        &lang_struct,
        &mega_struct,
        &complex_union,
        &forward_declared,
        &scalars.int_field,
        &strings.string1,
        &outer.nested,
        &pointers.compare_func,
        &arrays.fixed_array,
        &mega_struct.children,
        &mega_struct.cleanup,
        &lang_struct.lang_callbacks
    };
    
    /* Use sizeof on all types */
    size_t type_sizes[] = {
        sizeof(ScalarTypes),
        sizeof(StringTypes),
        sizeof(InnerStruct),
        sizeof(OuterStruct),
        sizeof(SelfRefStruct),
        sizeof(DataUnion),
        sizeof(PointerTypes),
        sizeof(ArrayTypes),
        sizeof(LangStruct),
        sizeof(MegaStruct),
        sizeof(ComplexUnion),
        sizeof(ForwardDeclared),
        sizeof(int),
        sizeof(char*),
        sizeof(void(*)(void)),
        sizeof(int[10]),
        sizeof(struct MegaStruct*[5])
    };
    
    /* Calculate total size as checksum */
    size_t total_size = calculate_type_sizes();
    
    /* Use all addresses to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_pointer((void*)addresses[i]);
    }
    
    /* Use type sizes */
    size_t size_sum = 0;
    for (size_t i = 0; i < sizeof(type_sizes)/sizeof(type_sizes[0]); i++) {
        size_sum += type_sizes[i];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Type coverage test:\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Sum of individual sizes: %zu bytes\n", size_sum);
    printf("Address of mega_struct: %p\n", (void*)&mega_struct);
    
    /* Create some circular references */
    self_ref.next = &self_ref;
    forward_declared.next = &forward_declared;
    mega_struct.parent = &mega_struct;
    
    /* Initialize array elements */
    for (int i = 0; i < 10; i++) {
        arrays.fixed_array[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        mega_struct.children[i] = &mega_struct;
    }
    
    return 0;
}
