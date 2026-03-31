/* test_gengtype_coverage.c
 * Complex type definitions to trigger type enumeration in gengtype.cc
 * Specifically targets lines 182-213 (switch on typekind)
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers used in GCC internals */
#define GTY(x) /* nothing for test compilation */

/* Forward declarations to create pointer cycles */
struct ComplexStruct;
union NestedUnion;

/* TYPE_SCALAR triggers */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING triggers */
typedef const char* string_type;

/* TYPE_CALLBACK triggers (function pointer) */
typedef void (*callback_type)(int, const char*);

/* TYPE_UNION triggers */
union SimpleUnion {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_STRUCT triggers */
struct SimpleStruct {
    scalar_int id;              /* TYPE_SCALAR */
    scalar_char initial;        /* TYPE_SCALAR */
    scalar_float weight;        /* TYPE_SCALAR */
    scalar_double precision;    /* TYPE_SCALAR */
};

/* TYPE_USER_STRUCT triggers (user-defined struct type) */
typedef struct SimpleStruct UserStructType;

/* TYPE_ARRAY triggers */
struct ArrayContainer {
    int fixed_array[10];        /* TYPE_ARRAY */
    scalar_float float_array[5][3]; /* TYPE_ARRAY (multi-dimensional) */
    char* pointer_array[8];     /* TYPE_ARRAY of TYPE_POINTER */
};

/* TYPE_POINTER triggers */
struct PointerContainer {
    struct SimpleStruct* struct_ptr;    /* TYPE_POINTER to TYPE_STRUCT */
    union SimpleUnion* union_ptr;       /* TYPE_POINTER to TYPE_UNION */
    callback_type func_ptr;             /* TYPE_POINTER (function) */
    void* void_ptr;                     /* TYPE_POINTER */
    int* int_ptr;                       /* TYPE_POINTER to scalar */
    struct ComplexStruct* forward_ptr;  /* TYPE_POINTER to forward decl */
};

/* TYPE_STRUCT with nested union */
struct StructWithUnion {
    int tag;
    union {
        int int_value;
        float float_value;
        struct SimpleStruct nested_struct;
    } data;                            /* TYPE_UNION inside TYPE_STRUCT */
};

/* TYPE_UNION with struct member */
union UnionWithStruct {
    struct {
        int x;
        int y;
    } point;                           /* TYPE_STRUCT inside TYPE_UNION */
    int coordinates[2];
};

/* Complex nested structure to trigger multiple types */
struct ComplexStruct GTY(()) {
    /* TYPE_SCALAR */
    int id;
    unsigned long count;
    
    /* TYPE_STRING */
    const char* name;
    char* dynamic_string;
    
    /* TYPE_POINTER */
    struct ComplexStruct* next;
    struct ComplexStruct* prev;
    void** void_ptr_ptr;
    
    /* TYPE_ARRAY */
    int scores[20];
    struct SimpleStruct struct_array[5];
    
    /* TYPE_UNION */
    union SimpleUnion value_union;
    
    /* TYPE_STRUCT */
    struct SimpleStruct embedded_struct;
    
    /* TYPE_CALLBACK */
    callback_type notify;
    
    /* TYPE_ARRAY of pointers */
    union NestedUnion* union_array[4];
    
    /* Flexible array member (special array case) */
    int flexible_array[];
};

/* Another union type */
union NestedUnion {
    int as_int;
    struct ComplexStruct* as_complex;
    struct ArrayContainer as_array_container;
    callback_type as_callback;
};

/* TYPE_LANG_STRUCT simulation (language-specific structure) */
struct LangSpecificStruct {
    int lang_tag;
    void* lang_data;
    /* Simulate language-specific fields */
    union {
        long long ll_value;
        double d_value;
    } lang_union;
};

/* Global variables to ensure types are used */
volatile struct ComplexStruct* global_complex_ptr = NULL;
volatile union NestedUnion global_union_var;
volatile struct LangSpecificStruct global_lang_struct;

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_type_sizes(void) {
    size_t total_size = 0;
    
    /* Force consideration of all types through sizeof */
    total_size += sizeof(scalar_int);
    total_size += sizeof(scalar_char);
    total_size += sizeof(scalar_float);
    total_size += sizeof(scalar_double);
    total_size += sizeof(string_type);
    total_size += sizeof(callback_type);
    total_size += sizeof(union SimpleUnion);
    total_size += sizeof(struct SimpleStruct);
    total_size += sizeof(UserStructType);
    total_size += sizeof(struct ArrayContainer);
    total_size += sizeof(struct PointerContainer);
    total_size += sizeof(struct StructWithUnion);
    total_size += sizeof(union UnionWithStruct);
    total_size += sizeof(struct ComplexStruct);
    total_size += sizeof(union NestedUnion);
    total_size += sizeof(struct LangSpecificStruct);
    
    return total_size;
}

/* Another noinline function that takes addresses */
__attribute__((noinline))
void take_addresses(
    struct ComplexStruct* cs,
    union NestedUnion* nu,
    struct LangSpecificStruct* ls,
    callback_type cb
) {
    /* Volatile operations to prevent optimization */
    volatile void* ptr1 = &cs->id;
    volatile void* ptr2 = &nu->as_int;
    volatile void* ptr3 = &ls->lang_tag;
    volatile void* ptr4 = &cb;
    
    (void)ptr1;
    (void)ptr2;
    (void)ptr3;
    (void)ptr4;
}

int main(void) {
    /* Declare instances of all complex types */
    struct SimpleStruct simple = {1, 'A', 3.14f, 2.71828};
    union SimpleUnion simple_union = {.as_int = 42};
    UserStructType user_struct = {2, 'B', 1.414f, 3.14159};
    struct ArrayContainer arrays = {0};
    struct PointerContainer pointers = {0};
    struct StructWithUnion struct_union = {0};
    union UnionWithStruct union_struct = {0};
    struct ComplexStruct complex = {0};
    union NestedUnion nested_union = {0};
    struct LangSpecificStruct lang_struct = {0};
    
    /* Initialize some values */
    simple_union.as_float = 3.14159f;
    arrays.fixed_array[0] = 100;
    arrays.float_array[0][0] = 1.0f;
    pointers.struct_ptr = &simple;
    pointers.union_ptr = &simple_union;
    pointers.func_ptr = NULL;
    struct_union.tag = 1;
    struct_union.data.int_value = 999;
    union_struct.point.x = 10;
    union_struct.point.y = 20;
    complex.id = 123;
    complex.name = "Test Name";
    complex.next = &complex;  /* Self-reference for pointer cycle */
    complex.notify = NULL;
    nested_union.as_complex = &complex;
    lang_struct.lang_tag = 0xABCD;
    
    /* Take addresses of all instances */
    struct ComplexStruct* complex_ptr = &complex;
    union NestedUnion* nested_ptr = &nested_union;
    struct LangSpecificStruct* lang_ptr = &lang_struct;
    
    /* Call function that takes addresses */
    take_addresses(complex_ptr, nested_ptr, lang_ptr, NULL);
    
    /* Compute total size of all types */
    size_t total_size = compute_type_sizes();
    
    /* Additional operations to ensure type usage */
    volatile size_t offset1 = offsetof(struct ComplexStruct, name);
    volatile size_t offset2 = offsetof(union NestedUnion, as_complex);
    volatile size_t offset3 = offsetof(struct LangSpecificStruct, lang_union);
    
    /* Create pointer chains */
    void* ptr_chain[] = {
        &simple,
        &simple_union,
        &user_struct,
        &arrays,
        &pointers,
        &struct_union,
        &union_struct,
        &complex,
        &nested_union,
        &lang_struct,
        complex_ptr->next,
        pointers.struct_ptr,
        nested_ptr->as_complex
    };
    
    /* Calculate a simple checksum */
    unsigned long checksum = 0;
    for (size_t i = 0; i < sizeof(ptr_chain)/sizeof(ptr_chain[0]); i++) {
        checksum += (unsigned long)ptr_chain[i];
    }
    checksum += total_size;
    checksum += offset1 + offset2 + offset3;
    
    printf("Type coverage test complete. Checksum: %lu\n", checksum % 1000);
    
    return 0;
}
