/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to exercise the type
 * enumeration logic in gengtype.cc, specifically targeting the switch
 * statement that counts occurrences of different type kinds.
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation macro */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to ensure types are referenced */
__attribute__((noinline)) 
void reference_types(void* ptr1, void* ptr2, void* ptr3, void* ptr4) {
    /* Do nothing meaningful, just reference the pointers */
    volatile int dummy = 0;
    if (ptr1) dummy++;
    if (ptr2) dummy++;
    if (ptr3) dummy++;
    if (ptr4) dummy++;
    (void)dummy;
}

/* ========== TYPE DEFINITIONS ========== */

/* Basic scalar types - should trigger TYPE_SCALAR */
typedef struct GTY(()) ScalarStruct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    _Bool bool_field;       /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
} ScalarStruct;

/* String type - should trigger TYPE_STRING */
typedef struct GTY(()) StringStruct {
    const char* string_field;      /* TYPE_STRING */
    char* mutable_string;          /* TYPE_POINTER (to TYPE_SCALAR) */
    const char* const const_string; /* TYPE_STRING */
} StringStruct;

/* Nested struct - should trigger TYPE_STRUCT */
typedef struct GTY(()) InnerStruct {
    int x;
    int y;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct inner;      /* TYPE_STRUCT */
    int extra;
} OuterStruct;

/* User-defined struct - should trigger TYPE_USER_STRUCT */
struct GTY(()) ForwardDeclared;  /* Forward declaration */

typedef struct GTY(()) UserStructContainer {
    struct ForwardDeclared* fd_ptr;  /* TYPE_POINTER to TYPE_USER_STRUCT */
    void* opaque;
} UserStructContainer;

struct GTY(()) ForwardDeclared {
    int data;
    UserStructContainer* back_ref;  /* Circular reference */
};

/* Union type - should trigger TYPE_UNION */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
} DataUnion;

/* Pointer types - should trigger TYPE_POINTER */
typedef struct GTY(()) PointerStruct {
    int* int_ptr;                   /* TYPE_POINTER to TYPE_SCALAR */
    ScalarStruct* struct_ptr;       /* TYPE_POINTER to TYPE_STRUCT */
    DataUnion* union_ptr;          /* TYPE_POINTER to TYPE_UNION */
    void (*func_ptr)(void);        /* TYPE_POINTER (function pointer) */
    void* generic_ptr;             /* TYPE_POINTER */
    struct PointerStruct* self_ptr; /* Self-referential pointer */
} PointerStruct;

/* Array types - should trigger TYPE_ARRAY */
typedef struct GTY(()) ArrayStruct {
    int fixed_array[10];           /* TYPE_ARRAY of TYPE_SCALAR */
    char string_array[5][20];      /* TYPE_ARRAY of TYPE_ARRAY of TYPE_SCALAR */
    ScalarStruct struct_array[3];  /* TYPE_ARRAY of TYPE_STRUCT */
    DataUnion union_array[4];      /* TYPE_ARRAY of TYPE_UNION */
    int* pointer_array[8];         /* TYPE_ARRAY of TYPE_POINTER */
    int flexible_array[];          /* Flexible array member */
} ArrayStruct;

/* Callback type - should trigger TYPE_CALLBACK */
typedef int (*comparator_t)(const void*, const void*);  /* TYPE_POINTER */

typedef struct GTY(()) CallbackStruct {
    comparator_t compare;          /* TYPE_POINTER (function pointer) */
    void (*callback)(int, char*);  /* TYPE_POINTER */
    int (*method)(struct CallbackStruct*); /* Method pointer */
} CallbackStruct;

/* Complex nested structure combining all types */
typedef struct GTY(()) MegaStruct {
    /* Scalar fields */
    int id;
    char tag;
    
    /* String field */
    const char* name;              /* TYPE_STRING */
    
    /* Nested struct */
    InnerStruct position;
    
    /* Union field */
    DataUnion data;
    
    /* Pointer fields */
    struct MegaStruct* next;       /* TYPE_POINTER */
    int* data_ptr;
    void (*action)(void);
    
    /* Array fields */
    int scores[5];
    char matrix[3][3];
    
    /* Callback field */
    comparator_t sorter;
    
    /* Nested anonymous union */
    union {
        int as_int;
        float as_float;
    } variant;
    
    /* Bitfields (scalar) */
    unsigned int flags : 4;
    unsigned int status : 2;
} MegaStruct;

/* Language-specific structure placeholder */
/* In real GCC, this would be marked with language-specific GTY flags */
typedef struct GTY(()) LangStructPlaceholder {
    int lang_specific_data;
    void* lang_ops;
} LangStructPlaceholder;

/* Undefined type reference */
typedef struct GTY(()) UndefinedContainer {
    void* undefined_ref;  /* Could point to undefined type */
} UndefinedContainer;

/* ========== MAIN FUNCTION ========== */

int main() {
    size_t total_size = 0;
    
    /* Declare instances of all types */
    ScalarStruct scalar_instance = {0};
    StringStruct string_instance = {"Hello", NULL, "World"};
    OuterStruct outer_instance = {{{1, 2}, 3}};
    
    /* Forward declared struct instances */
    struct ForwardDeclared fd_instance = {42, NULL};
    UserStructContainer user_struct_instance = {&fd_instance, NULL};
    fd_instance.back_ref = &user_struct_instance;
    
    DataUnion union_instance = {.as_int = 100};
    PointerStruct pointer_instance = {NULL, NULL, NULL, NULL, NULL, NULL};
    ArrayStruct* array_instance = NULL;  /* Will allocate with flexible array */
    
    CallbackStruct callback_instance = {NULL, NULL, NULL};
    MegaStruct mega_instance = {
        .id = 1,
        .tag = 'A',
        .name = "Mega",
        .position = {10, 20},
        .data = {.as_float = 3.14f},
        .next = NULL,
        .data_ptr = NULL,
        .action = NULL,
        .scores = {1, 2, 3, 4, 5},
        .matrix = {{'a', 'b', 'c'}, {'d', 'e', 'f'}, {'g', 'h', 'i'}},
        .sorter = NULL,
        .variant = {.as_int = 99},
        .flags = 5,
        .status = 1
    };
    
    LangStructPlaceholder lang_instance = {0, NULL};
    UndefinedContainer undefined_instance = {NULL};
    
    /* Calculate sizes to force type consideration */
    total_size += sizeof(ScalarStruct);
    total_size += sizeof(StringStruct);
    total_size += sizeof(OuterStruct);
    total_size += sizeof(struct ForwardDeclared);
    total_size += sizeof(UserStructContainer);
    total_size += sizeof(DataUnion);
    total_size += sizeof(PointerStruct);
    total_size += sizeof(ArrayStruct) + 5 * sizeof(int);  /* With flexible array */
    total_size += sizeof(CallbackStruct);
    total_size += sizeof(MegaStruct);
    total_size += sizeof(LangStructPlaceholder);
    total_size += sizeof(UndefinedContainer);
    
    /* Take addresses to ensure types are referenced */
    void* addresses[] = {
        &scalar_instance,
        &string_instance,
        &outer_instance,
        &fd_instance,
        &user_struct_instance,
        &union_instance,
        &pointer_instance,
        &mega_instance,
        &lang_instance,
        &undefined_instance
    };
    
    /* Reference all types through external function */
    reference_types(addresses[0], addresses[1], addresses[2], addresses[3]);
    reference_types(addresses[4], addresses[5], addresses[6], addresses[7]);
    reference_types(addresses[8], addresses[9], NULL, NULL);
    
    /* Use KEEP_ALIVE to prevent optimization */
    KEEP_ALIVE(&scalar_instance.int_field);
    KEEP_ALIVE(string_instance.string_field);
    KEEP_ALIVE(outer_instance.inner.x);
    KEEP_ALIVE(fd_instance.data);
    KEEP_ALIVE(union_instance.as_float);
    KEEP_ALIVE(mega_instance.scores[0]);
    
    /* Print result to prevent complete optimization */
    printf("Total type size sum: %zu\n", total_size);
    printf("Type coverage test completed.\n");
    
    return 0;
}
