/* gengtype-test.c - Complex type definitions to exercise gengtype type enumeration */

#include <stdio.h>
#include <stddef.h>
#include <string.h>

/* Simulate GTY markers for compilation */
#define GTY(x)

/* External function to prevent optimization */
extern void __attribute__((noinline)) use_pointer(void *ptr);
extern void __attribute__((noinline)) use_size(size_t size);

/* TYPE_SCALAR triggers */
typedef struct {
    int int_field;          /* TYPE_SCALAR */
    char char_field;        /* TYPE_SCALAR */
    float float_field;      /* TYPE_SCALAR */
    double double_field;    /* TYPE_SCALAR */
    long long_field;        /* TYPE_SCALAR */
    short short_field;      /* TYPE_SCALAR */
    unsigned uint_field;    /* TYPE_SCALAR */
} GTY(()) ScalarStruct;

/* TYPE_STRING trigger */
typedef struct {
    const char *name;       /* TYPE_STRING */
    char *dynamic_string;   /* TYPE_STRING */
    char fixed_string[32];  /* TYPE_ARRAY of TYPE_SCALAR */
} GTY(()) StringStruct;

/* TYPE_POINTER triggers */
typedef struct {
    void *void_ptr;         /* TYPE_POINTER */
    int *int_ptr;           /* TYPE_POINTER to TYPE_SCALAR */
    ScalarStruct *struct_ptr; /* TYPE_POINTER to TYPE_STRUCT */
    struct ForwardDecl *forward_ptr; /* TYPE_POINTER to forward decl */
} GTY(()) PointerStruct;

/* Forward declaration for pointer */
struct ForwardDecl;

/* TYPE_ARRAY triggers */
typedef struct {
    int fixed_array[10];           /* TYPE_ARRAY of TYPE_SCALAR */
    int multi_array[5][5];         /* TYPE_ARRAY of TYPE_ARRAY */
    ScalarStruct struct_array[3];  /* TYPE_ARRAY of TYPE_STRUCT */
    void *ptr_array[8];            /* TYPE_ARRAY of TYPE_POINTER */
} GTY(()) ArrayStruct;

/* TYPE_UNION trigger */
typedef union {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
    struct {
        int x;
        int y;
    } as_coords;
} GTY(()) ComplexUnion;

/* TYPE_STRUCT with nested types */
typedef struct GTY(()) NestedStruct {
    ScalarStruct scalars;
    PointerStruct pointers;
    ArrayStruct arrays;
    ComplexUnion union_field;
    
    /* Self-referential pointer for graph traversal */
    struct NestedStruct *next;  /* TYPE_POINTER to TYPE_STRUCT */
    
    /* Function pointer for TYPE_CALLBACK */
    int (*compare)(const struct NestedStruct *, const struct NestedStruct *);
    
    /* Array of function pointers */
    void (*callbacks[5])(void);
} NestedStruct;

/* TYPE_CALLBACK trigger - function pointer type */
typedef int (*Comparator)(const void *, const void *);

/* Another struct with callback */
typedef struct GTY(()) CallbackStruct {
    Comparator cmp_func;      /* TYPE_CALLBACK */
    void (*action)(void);     /* TYPE_CALLBACK */
    int (*transform)(int);    /* TYPE_CALLBACK */
} CallbackStruct;

/* TYPE_UNION with complex members */
typedef union GTY(()) TaggedUnion {
    struct {
        int type;
        union {
            int int_value;
            float float_value;
            char *string_value;
        } data;
    } tagged;
    
    struct {
        long long bits;
        void *metadata;
    } raw;
} TaggedUnion;

/* Deeply nested structure */
typedef struct GTY(()) TreeNode {
    int value;
    struct TreeNode *left;    /* TYPE_POINTER */
    struct TreeNode *right;   /* TYPE_POINTER */
    struct TreeNode *parent;  /* TYPE_POINTER */
    
    union {
        int color;
        void *user_data;
    } attr;                   /* TYPE_UNION */
    
    char name[20];            /* TYPE_ARRAY */
} TreeNode;

/* Structure with flexible array member */
typedef struct GTY(()) FlexArrayStruct {
    int count;
    double average;
    int scores[];             /* Flexible array - TYPE_ARRAY */
} FlexArrayStruct;

/* Structure containing all type kinds */
typedef struct GTY(()) AllTypesStruct {
    /* TYPE_SCALAR */
    int id;
    char code;
    
    /* TYPE_STRING */
    const char *description;
    
    /* TYPE_POINTER */
    void *data;
    AllTypesStruct *self_ptr;
    
    /* TYPE_ARRAY */
    int numbers[100];
    char buffer[256];
    
    /* TYPE_STRUCT */
    ScalarStruct scalar_member;
    
    /* TYPE_UNION */
    ComplexUnion union_member;
    
    /* TYPE_CALLBACK */
    void (*handler)(AllTypesStruct *);
    
    /* Nested anonymous struct */
    struct {
        int flags;
        void *private;
    } internal;
    
    /* Pointer to array */
    int (*matrix_ptr)[10][10];
    
    /* Complex pointer type */
    char *(*get_name)(void);
    
} AllTypesStruct;

/* External function definitions to prevent optimization */
void __attribute__((noinline)) use_pointer(void *ptr) {
    volatile void *temp = ptr;
    (void)temp;
}

void __attribute__((noinline)) use_size(size_t size) {
    volatile size_t temp = size;
    (void)temp;
}

/* Function using all types */
void process_all_types(void) {
    /* Declare instances of all types */
    ScalarStruct scalar_instance = {0};
    StringStruct string_instance = {0};
    PointerStruct pointer_instance = {0};
    ArrayStruct array_instance = {0};
    ComplexUnion union_instance = {0};
    NestedStruct nested_instance = {0};
    CallbackStruct callback_instance = {0};
    TaggedUnion tagged_instance = {0};
    TreeNode tree_instance = {0};
    AllTypesStruct all_types_instance = {0};
    
    /* Take addresses to ensure types are considered */
    ScalarStruct *scalar_ptr = &scalar_instance;
    StringStruct *string_ptr = &string_instance;
    PointerStruct *pointer_ptr = &pointer_instance;
    ArrayStruct *array_ptr = &array_instance;
    ComplexUnion *union_ptr = &union_instance;
    NestedStruct *nested_ptr = &nested_instance;
    CallbackStruct *callback_ptr = &callback_instance;
    TaggedUnion *tagged_ptr = &tagged_instance;
    TreeNode *tree_ptr = &tree_instance;
    AllTypesStruct *all_types_ptr = &all_types_instance;
    
    /* Use sizeof on all types */
    size_t total_size = 0;
    total_size += sizeof(ScalarStruct);
    total_size += sizeof(StringStruct);
    total_size += sizeof(PointerStruct);
    total_size += sizeof(ArrayStruct);
    total_size += sizeof(ComplexUnion);
    total_size += sizeof(NestedStruct);
    total_size += sizeof(CallbackStruct);
    total_size += sizeof(TaggedUnion);
    total_size += sizeof(TreeNode);
    total_size += sizeof(AllTypesStruct);
    
    /* Take addresses of members to ensure full type traversal */
    use_pointer(&scalar_instance.int_field);
    use_pointer(&string_instance.name);
    use_pointer(&pointer_instance.void_ptr);
    use_pointer(&array_instance.fixed_array);
    use_pointer(&union_instance.as_int);
    use_pointer(&nested_instance.scalars);
    use_pointer(&callback_instance.cmp_func);
    use_pointer(&tagged_instance.tagged);
    use_pointer(&tree_instance.left);
    use_pointer(&all_types_instance.handler);
    
    /* Use sizes */
    use_size(total_size);
    use_size(sizeof(scalar_instance));
    use_size(sizeof(string_instance.fixed_string));
    use_size(sizeof(array_instance.multi_array));
    use_size(sizeof(nested_instance.callbacks));
    
    /* Create pointer chains for graph traversal */
    nested_instance.next = &nested_instance;
    tree_instance.left = &tree_instance;
    tree_instance.right = &tree_instance;
    tree_instance.parent = &tree_instance;
    all_types_instance.self_ptr = &all_types_instance;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        array_instance.fixed_array[i] = i;
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            array_instance.multi_array[i][j] = i * j;
        }
    }
    
    /* Initialize strings */
    string_instance.name = "Test String";
    strcpy(string_instance.fixed_string, "Fixed string content");
    
    /* Use all pointers */
    pointer_instance.void_ptr = &total_size;
    pointer_instance.int_ptr = &scalar_instance.int_field;
    pointer_instance.struct_ptr = &scalar_instance;
    
    /* Set callback pointers */
    callback_instance.cmp_func = (Comparator)strcmp;
    
    /* Use union */
    union_instance.as_int = 42;
    union_instance.as_float = 3.14f;
    union_instance.as_ptr = &union_instance;
    
    /* Complex nested access */
    nested_instance.scalars.int_field = 100;
    nested_instance.arrays.fixed_array[0] = 999;
    nested_instance.union_field.as_int = 123;
    
    /* Tagged union usage */
    tagged_instance.tagged.type = 1;
    tagged_instance.tagged.data.int_value = 456;
    tagged_instance.raw.bits = 0xDEADBEEF;
    
    /* Tree structure setup */
    tree_instance.value = 777;
    strcpy(tree_instance.name, "TreeNode");
    
    /* All types struct initialization */
    all_types_instance.id = 1;
    all_types_instance.code = 'A';
    all_types_instance.description = "All types included";
    all_types_instance.data = &total_size;
    all_types_instance.handler = NULL;
    all_types_instance.internal.flags = 0xFF;
    
    /* Initialize matrix pointer */
    static int matrix[10][10];
    all_types_instance.matrix_ptr = &matrix;
    
    /* Prevent dead code elimination */
    volatile int checksum = 0;
    checksum += scalar_instance.int_field;
    checksum += array_instance.fixed_array[0];
    checksum += nested_instance.scalars.int_field;
    checksum += tree_instance.value;
    checksum += all_types_instance.id;
    
    /* Print something to ensure code isn't optimized away */
    printf("Type processing test - total size: %zu, checksum: %d\n", 
           total_size, checksum);
}

/* Main function that forces consideration of all types */
int main(void) {
    printf("Starting gengtype type enumeration test...\n");
    
    /* Process all complex types */
    process_all_types();
    
    /* Additional type references */
    {
        /* Reference typedefs */
        typedef void (*ComplexFuncPtr)(int, char**, double[10]);
        ComplexFuncPtr func_ptr = NULL;
        use_pointer(&func_ptr);
        
        /* Reference anonymous struct in union */
        union {
            struct {
                int a;
                int b;
            } s;
            long long ll;
        } anon_union;
        anon_union.s.a = 1;
        anon_union.s.b = 2;
        use_pointer(&anon_union);
        
        /* Multi-level pointer */
        int ***triple_ptr = NULL;
        use_pointer(&triple_ptr);
        
        /* Array of pointers to functions */
        int (*func_array[5])(void);
        for (int i = 0; i < 5; i++) {
            func_array[i] = NULL;
        }
        use_pointer(&func_array);
        
        /* Const and volatile qualified types */
        const volatile int cv_int = 42;
        use_pointer((void*)&cv_int);
        
        /* Restrict qualified pointer */
        int *restrict restrict_ptr = NULL;
        use_pointer(&restrict_ptr);
    }
    
    printf("Test completed.\n");
    return 0;
}
