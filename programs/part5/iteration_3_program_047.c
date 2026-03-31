/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to exercise the type
 * enumeration logic in gengtype.cc, specifically targeting the switch
 * statement that counts occurrences of different type kinds.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this would be the
 * actual garbage collector annotation */
#define GTY(x) 

/* Forward declarations to create pointer cycles and complex type graphs */
struct ForwardDeclared;
union ForwardUnion;

/* TYPE_SCALAR: Basic scalar types */
typedef struct {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    unsigned uint_field;
} GTY(()) ScalarStruct;

/* TYPE_STRING: String types */
typedef struct {
    const char* static_string;
    char* dynamic_string;
    char fixed_string[64];
} GTY(()) StringStruct;

/* TYPE_STRUCT: Regular structure */
typedef struct {
    int id;
    ScalarStruct scalars;
    struct ForwardDeclared* next;  /* TYPE_POINTER to forward declared */
} GTY(()) RegularStruct;

/* TYPE_UNION: Union type */
typedef union {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;  /* TYPE_POINTER */
    char as_string[32]; /* TYPE_ARRAY */
} GTY(()) DataUnion;

/* TYPE_ARRAY: Various array types */
typedef struct {
    int simple_array[10];           /* Fixed-size array */
    float* pointer_array[5];        /* Array of pointers */
    struct { int x; char y; } struct_array[3]; /* Array of structs */
    union { int a; float b; } union_array[4];  /* Array of unions */
    int flexible_array[];           /* Flexible array member */
} GTY(()) ArrayStruct;

/* TYPE_POINTER: Complex pointer types */
typedef struct {
    /* Pointers to various types */
    int* int_ptr;
    void* void_ptr;
    struct ForwardDeclared** double_ptr;  /* Pointer to pointer */
    const volatile char* cv_ptr;          /* Qualified pointer */
    
    /* Function pointer - TYPE_CALLBACK */
    int (*compare_func)(const void*, const void*);
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to function returning pointer */
    void* (*allocator)(size_t);
} GTY(()) PointerStruct;

/* TYPE_CALLBACK: More function pointers */
typedef struct {
    /* Various function pointer signatures */
    void (*void_callback)(void);
    int (*int_callback)(int, char*);
    float* (*float_ptr_callback)(double);
    
    /* Callback with struct parameter */
    void (*struct_callback)(RegularStruct*);
    
    /* Callback returning callback */
    int (*(*meta_callback)(void))(int, int);
} GTY(()) CallbackStruct;

/* TYPE_USER_STRUCT: User-defined structure with tag */
struct GTY(()) UserTaggedStruct {
    int tag;
    union {
        int int_value;
        struct UserTaggedStruct* self_ptr;  /* Recursive pointer */
    } data;
};

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef struct GTY(()) LangSpecific {
    int lang_id;
    void* lang_data;
    /* Simulating language-specific extensions */
    struct {
        int flags;
        unsigned long features;
    } GTY(()) extensions;
} LangStruct;

/* Now define the forward declared structures */
struct GTY(()) ForwardDeclared {
    int marker;
    RegularStruct* regular;      /* TYPE_POINTER to regular struct */
    DataUnion data;              /* TYPE_UNION */
    struct ForwardDeclared* next; /* Recursive pointer */
};

union GTY(()) ForwardUnion {
    int as_int;
    struct ForwardDeclared* as_struct;  /* TYPE_POINTER */
    CallbackStruct callbacks;           /* TYPE_CALLBACK members */
};

/* Complex nested structure combining all types */
typedef struct GTY(()) SuperComplex {
    /* Direct members of various types */
    ScalarStruct scalars;           /* TYPE_STRUCT */
    StringStruct strings;           /* TYPE_STRUCT with TYPE_STRING members */
    ArrayStruct arrays;             /* TYPE_STRUCT with TYPE_ARRAY members */
    PointerStruct pointers;         /* TYPE_STRUCT with TYPE_POINTER members */
    CallbackStruct callbacks;       /* TYPE_STRUCT with TYPE_CALLBACK members */
    
    /* Nested structures */
    struct {
        int depth;
        struct SuperComplex* parent;  /* Recursive pointer */
        struct {
            short level;
            char label[20];
        } GTY(()) inner;
    } GTY(())) nested;
    
    /* Union field */
    union {
        RegularStruct regular;
        UserTaggedStruct user;
        LangStruct lang;
    } GTY(())) variant;
    
    /* Array of unions containing structs */
    union {
        ScalarStruct s;
        PointerStruct p;
        CallbackStruct c;
    } GTY(())) union_array[5];
    
    /* Pointer to array of function pointers */
    void (*(*func_ptr_array)[10])(void);
    
    /* Flexible array of pointers */
    DataUnion* flexible_ptrs[];
} SuperComplex;

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_type_sizes(void* ptr) {
    /* This function forces the compiler to consider all types */
    (void)ptr;
    return 0;
}

/* Another external function that takes various type pointers */
__attribute__((noinline))
void reference_all_types(
    ScalarStruct* s1,
    StringStruct* s2,
    RegularStruct* s3,
    DataUnion* u1,
    ArrayStruct* s4,
    PointerStruct* s5,
    CallbackStruct* s6,
    struct UserTaggedStruct* s7,
    LangStruct* s8,
    struct ForwardDeclared* s9,
    union ForwardUnion* u2,
    SuperComplex* sc
) {
    /* Volatile operations to prevent optimization */
    volatile size_t total = 0;
    
    total += sizeof(*s1);
    total += sizeof(*s2);
    total += sizeof(*s3);
    total += sizeof(*u1);
    total += sizeof(*s4);
    total += sizeof(*s5);
    total += sizeof(*s6);
    total += sizeof(*s7);
    total += sizeof(*s8);
    total += sizeof(*s9);
    total += sizeof(*u2);
    total += sizeof(*sc);
    
    /* Take addresses of members to ensure they're considered */
    (void)&s1->int_field;
    (void)&s2->static_string;
    (void)&s3->next;
    (void)&u1->as_int;
    (void)&s4->simple_array;
    (void)&s5->compare_func;
    (void)&s6->void_callback;
    (void)&s7->tag;
    (void)&s8->lang_id;
    (void)&s9->marker;
    (void)&u2->as_int;
    (void)&sc->nested;
    
    /* Use the total to prevent dead code elimination */
    if (total > 1000) {
        printf("Types referenced\n");
    }
}

int main(void) {
    /* Declare instances of all complex types */
    ScalarStruct scalars = {0};
    StringStruct strings = {0};
    RegularStruct regular = {0};
    DataUnion data_union;
    ArrayStruct* arrays = NULL;  /* Pointer to handle flexible array */
    PointerStruct pointers = {0};
    CallbackStruct callbacks = {0};
    struct UserTaggedStruct user_struct = {0};
    LangStruct lang_struct = {0};
    struct ForwardDeclared forward = {0};
    union ForwardUnion forward_union;
    SuperComplex* super_complex = NULL;
    
    /* Take addresses to ensure types are referenced */
    ScalarStruct* s1 = &scalars;
    StringStruct* s2 = &strings;
    RegularStruct* s3 = &regular;
    DataUnion* u1 = &data_union;
    ArrayStruct* s4 = arrays;
    PointerStruct* s5 = &pointers;
    CallbackStruct* s6 = &callbacks;
    struct UserTaggedStruct* s7 = &user_struct;
    LangStruct* s8 = &lang_struct;
    struct ForwardDeclared* s9 = &forward;
    union ForwardUnion* u2 = &forward_union;
    SuperComplex* sc = super_complex;
    
    /* Compute sizes of all types */
    size_t total_size = 0;
    
    total_size += sizeof(ScalarStruct);
    total_size += sizeof(StringStruct);
    total_size += sizeof(RegularStruct);
    total_size += sizeof(DataUnion);
    total_size += offsetof(ArrayStruct, flexible_array);  /* Size without flexible array */
    total_size += sizeof(PointerStruct);
    total_size += sizeof(CallbackStruct);
    total_size += sizeof(struct UserTaggedStruct);
    total_size += sizeof(LangStruct);
    total_size += sizeof(struct ForwardDeclared);
    total_size += sizeof(union ForwardUnion);
    total_size += offsetof(SuperComplex, flexible_ptrs);  /* Size without flexible array */
    
    /* Create pointer cycles to make type graph more complex */
    regular.next = &forward;
    forward.next = &forward;  /* Self-reference */
    forward.regular = &regular;
    
    /* Reference all types through external function */
    reference_all_types(s1, s2, s3, u1, s4, s5, s6, s7, s8, s9, u2, sc);
    
    /* Use compute_type_sizes to prevent optimization */
    compute_type_sizes(&total_size);
    
    /* Print something to ensure execution */
    printf("Total type footprint: %zu bytes\n", total_size);
    printf("Type coverage test completed.\n");
    
    return 0;
}
