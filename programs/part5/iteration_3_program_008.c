/* gengtype_test.c - Test program to exercise gengtype type enumeration */

/* Dummy GTY macro for compilation - in real GCC this marks GC-able types */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* External function to prevent optimization */
NOINLINE void use_pointer(void *ptr) {
    volatile int sink = (int)(intptr_t)ptr;
    (void)sink;
}

/* TYPE_SCALAR triggers */
typedef struct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
} GTY(()) ScalarStruct;

/* TYPE_STRING trigger */
typedef struct {
    const char *name;       /* TYPE_STRING */
    char *dynamic_str;      /* TYPE_POINTER -> TYPE_STRING */
} GTY(()) StringStruct;

/* TYPE_STRUCT and nested structures */
typedef struct GTY(()) NestedInner {
    int inner_data;
    struct NestedInner *self_ptr;  /* TYPE_POINTER to TYPE_STRUCT */
} NestedInner;

typedef struct GTY(()) OuterStruct {
    NestedInner inner;      /* TYPE_STRUCT */
    int outer_data;
} OuterStruct;

/* TYPE_UNION */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;       /* TYPE_POINTER */
} DataUnion;

/* TYPE_USER_STRUCT - struct defined elsewhere */
struct ForwardDeclared;      /* Forward declaration */
typedef struct ForwardDeclared GTY(()) ForwardDeclared;

/* TYPE_ARRAY with various dimensions */
typedef struct GTY(()) ArrayContainer {
    int fixed_array[10];                /* TYPE_ARRAY of TYPE_SCALAR */
    char *ptr_array[5];                 /* TYPE_ARRAY of TYPE_POINTER */
    struct NestedInner *struct_array[3]; /* TYPE_ARRAY of TYPE_POINTER to TYPE_STRUCT */
    int multi_dim[2][3][4];             /* Multi-dimensional TYPE_ARRAY */
    int flexible_array[];               /* Flexible array member - TYPE_ARRAY */
} ArrayContainer;

/* TYPE_POINTER variations */
typedef struct GTY(()) PointerStruct {
    void *void_ptr;                     /* TYPE_POINTER */
    int *int_ptr;                       /* TYPE_POINTER to TYPE_SCALAR */
    struct OuterStruct *struct_ptr;     /* TYPE_POINTER to TYPE_STRUCT */
    union DataUnion *union_ptr;         /* TYPE_POINTER to TYPE_UNION */
    ArrayContainer *array_ptr;          /* TYPE_POINTER to TYPE_ARRAY container */
    void (*func_ptr)(void);             /* TYPE_POINTER (function pointer) */
} PointerStruct;

/* TYPE_CALLBACK - function pointer with complex signature */
typedef int (*Comparator)(const void *, const void *);  /* TYPE_POINTER (function) */

typedef struct GTY(()) CallbackContainer {
    Comparator compare;                 /* TYPE_POINTER (function) */
    void (*callback)(int, char *);      /* TYPE_POINTER (function) */
    int (*method)(struct CallbackContainer *); /* Self-referential function pointer */
} CallbackContainer;

/* TYPE_LANG_STRUCT - simulate GCC language-specific structure */
typedef struct GTY(()) LangSpecific {
    void *tree_node;                    /* TYPE_POINTER - simulate GCC tree node */
    int lang_specific_flags;
    struct LangSpecific *next;          /* TYPE_POINTER to TYPE_LANG_STRUCT */
} LangSpecific;

/* Complex nested type combining everything */
typedef struct GTY(()) MegaStruct {
    /* Scalar fields */
    int id;
    char tag;
    
    /* String fields */
    const char *description;
    
    /* Nested struct */
    ScalarStruct scalars;
    
    /* Union field */
    DataUnion data;
    
    /* Array fields */
    int scores[5];
    PointerStruct *pointers[2];
    
    /* Pointer fields */
    OuterStruct *outer;
    CallbackContainer *callbacks;
    
    /* Language-specific structure */
    LangSpecific *lang_data;
    
    /* Self-reference */
    struct MegaStruct *parent;
    
    /* Function pointer */
    void (*cleanup)(struct MegaStruct *);
} MegaStruct;

/* Function to use all types - prevents optimization */
NOINLINE size_t compute_type_sizes(void) {
    size_t total = 0;
    
    /* Declare instances of each type */
    ScalarStruct scalar_instance = {0};
    StringStruct string_instance = {0};
    OuterStruct outer_instance = {0};
    DataUnion union_instance = {0};
    ArrayContainer *array_instance = NULL;
    PointerStruct pointer_instance = {0};
    CallbackContainer callback_instance = {0};
    LangSpecific lang_instance = {0};
    MegaStruct mega_instance = {0};
    
    /* Take addresses and compute sizes */
    total += sizeof(ScalarStruct);
    total += sizeof(StringStruct);
    total += sizeof(OuterStruct);
    total += sizeof(DataUnion);
    total += sizeof(ArrayContainer);
    total += sizeof(PointerStruct);
    total += sizeof(CallbackContainer);
    total += sizeof(LangSpecific);
    total += sizeof(MegaStruct);
    
    /* Take addresses to ensure types are referenced */
    use_pointer(&scalar_instance);
    use_pointer(&string_instance);
    use_pointer(&outer_instance);
    use_pointer(&union_instance);
    use_pointer(&pointer_instance);
    use_pointer(&callback_instance);
    use_pointer(&lang_instance);
    use_pointer(&mega_instance);
    
    /* Pointer operations */
    ScalarStruct *scalar_ptr = &scalar_instance;
    StringStruct *string_ptr = &string_instance;
    OuterStruct *outer_ptr = &outer_instance;
    
    use_pointer(scalar_ptr);
    use_pointer(string_ptr);
    use_pointer(outer_ptr);
    
    /* Array operations */
    int local_array[10] = {0};
    total += sizeof(local_array);
    use_pointer(local_array);
    
    /* Nested member access */
    total += sizeof(scalar_instance.int_field);
    total += sizeof(string_instance.name);
    total += sizeof(outer_instance.inner);
    
    return total;
}

/* Global variables to ensure types are used */
USED ScalarStruct global_scalar = {1, 'A', 3.14f, 2.718, 100};
USED StringStruct global_string = {"Test", NULL};
USED DataUnion global_union = {.as_int = 42};
USED MegaStruct global_mega = {
    .id = 1,
    .tag = 'M',
    .description = "Mega struct instance",
    .scalars = {1, 'S', 1.0f, 1.0, 1},
    .data = {.as_int = 100},
    .scores = {1, 2, 3, 4, 5},
    .cleanup = NULL
};

/* Function pointer usage */
NOINLINE void dummy_cleanup(MegaStruct *ms) {
    (void)ms;
}

/* Main function that references all types */
int main(void) {
    printf("Testing gengtype type enumeration coverage\n");
    
    /* Compute total size of all types */
    size_t total_size = compute_type_sizes();
    printf("Total size of all types: %zu bytes\n", total_size);
    
    /* Additional type operations */
    
    /* Array type operations */
    int matrix[3][4] = {{1,2,3,4}, {5,6,7,8}, {9,10,11,12}};
    use_pointer(matrix);
    
    /* Pointer arithmetic */
    int *int_ptr = &global_scalar.int_field;
    use_pointer(int_ptr + 1);
    
    /* Union type punning */
    DataUnion u;
    u.as_int = 65;
    use_pointer(&u.as_float);  /* Reinterpret as float */
    
    /* Nested structure access */
    global_mega.scalars.float_field = 3.14159f;
    global_mega.data.as_double = 2.71828;
    
    /* Function pointer assignment */
    global_mega.cleanup = dummy_cleanup;
    
    /* Simulate callback usage */
    CallbackContainer cb = {NULL, NULL, NULL};
    use_pointer(&cb);
    
    /* Take address of array element */
    use_pointer(&global_mega.scores[2]);
    
    /* Offsetof operations - force compiler to consider layout */
    printf("Offsets:\n");
    printf("  ScalarStruct.int_field: %zu\n", offsetof(ScalarStruct, int_field));
    printf("  MegaStruct.description: %zu\n", offsetof(MegaStruct, description));
    printf("  MegaStruct.scalars: %zu\n", offsetof(MegaStruct, scalars));
    
    /* Return checksum based on operations */
    int checksum = (int)total_size 
                   + (int)offsetof(ScalarStruct, int_field)
                   + (int)offsetof(MegaStruct, description);
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

/* Forward declared structure definition */
struct ForwardDeclared {
    int data;
    struct ForwardDeclared *next;
};
