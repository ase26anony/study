/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct BasicStruct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    long counter;
    double data;
} UserStruct;

/* TYPE_UNION: Union with several members */
union DataUnion {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct BasicStruct *struct_ptr;
void *void_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct BasicStruct struct_array[3];
int (*array_of_pointers)[10];

/* TYPE_SCALAR: Scalar types */
enum Color { RED, GREEN, BLUE };
typedef enum Color ColorEnum;
float scalar_float;
double scalar_double;

/* TYPE_STRING: String literal in initializer */
const char *greeting = "Hello, gengtype!";
char initialized_string[] = "Test string";

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);
typedef struct BasicStruct* (*StructFactory)(void);

/* TYPE_LANG_STRUCT: GCC-specific constructs with attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
};

/* Complex nesting and chains */
struct ComplexNode {
    int value;
    struct ComplexNode *next;
    union DataUnion data;
    Comparator compare;
    char name[20];
};

struct Container {
    struct ComplexNode *nodes[10];
    UserStruct user_data;
    CallbackFunc callback;
    volatile int status;
};

/* More complex type combinations */
typedef union DataUnion* (*UnionProcessor)(struct Container*, int);
typedef int (*(*ComplexCallback)[5])(void);

/* Global variables with initializers */
struct BasicStruct global_struct = {1, 3.14f, "test"};
UserStruct global_user_struct = {100, 2.71828};
int global_array[3] = {1, 2, 3};
union DataUnion global_union = {.as_int = 42};

/* Function pointer variable */
Comparator global_comparator = NULL;

/* Array of function pointers */
CallbackFunc callbacks[3];

/* Struct with bitfields (another GCC extension) */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int count : 8;
    unsigned int : 4;  /* padding */
    unsigned int value : 16;
} __attribute__((packed));

/* Opaque forward declaration */
struct OpaqueStruct;
extern struct OpaqueStruct *external_opaque;

/* Const pointer to volatile struct */
volatile struct Container *const volatile_container_ptr;

/* Main function to ensure all types are referenced */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Reference each major type to ensure they're not optimized away */
    struct BasicStruct local_struct = global_struct;
    UserStruct local_user = global_user_struct;
    union DataUnion local_union = global_union;
    
    /* Use pointers */
    int_ptr = &prevent_optimization;
    struct_ptr = &local_struct;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[0][0] = 'A';
    
    /* Use scalar types */
    ColorEnum color = RED;
    scalar_float = 3.14f;
    
    /* Use string */
    const char *local_greeting = greeting;
    
    /* Use function pointer if safe */
    if (global_comparator) {
        /* Would call if initialized, but we'll just reference */
        Comparator temp = global_comparator;
        (void)temp;
    }
    
    /* Use complex nested types */
    struct ComplexNode node = {0};
    node.value = 42;
    
    struct Container container = {0};
    container.status = 1;
    
    /* Use GCC-specific structs */
    struct PackedStruct packed = {0};
    packed.a = 'X';
    
    struct BitfieldStruct bitfield = {0};
    bitfield.flag1 = 1;
    
    /* Prevent unused variable warnings */
    (void)local_struct;
    (void)local_user;
    (void)local_union;
    (void)local_greeting;
    (void)color;
    (void)node;
    (void)container;
    (void)packed;
    (void)bitfield;
    (void)void_ptr;
    (void)volatile_int_ptr;
    (void)const_string_ptr;
    (void)array_of_pointers;
    (void)scalar_double;
    (void)initialized_string;
    (void)callbacks;
    (void)volatile_container_ptr;
    
    return prevent_optimization;
}

/* Additional function to create more type references */
static void helper_function(void) {
    /* Create more type instances */
    static struct ComplexNode static_node = {0};
    static UnionProcessor processor = NULL;
    static ComplexCallback complex_cb = NULL;
    
    /* Reference transparent union */
    union TransparentUnion trans_union;
    trans_union.int_ptr = NULL;
    
    (void)static_node;
    (void)processor;
    (void)complex_cb;
    (void)trans_union;
}

/* Callback function definition */
static int sample_comparator(const void *a, const void *b) {
    return (*(const int*)a - *(const int*)b);
}

/* Initialize function pointers */
__attribute__((constructor)) static void init_pointers(void) {
    global_comparator = sample_comparator;
    callbacks[0] = NULL;
}
