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
    int int_val;
    float float_val;
    char* str_val;
    void* ptr_val;
};

/* TYPE_POINTER: Various pointer types */
int* int_ptr;
struct BasicStruct* struct_ptr;
void* void_ptr;
volatile int* volatile volatile_int_ptr;
const char* const const_string_ptr = "constant";

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct BasicStruct struct_array[3];
int* pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int = 42;
float scalar_float = 3.14f;
double scalar_double = 2.71828;

/* Enumeration type (also scalar) */
enum Color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRING: String literal in initializer context */
const char* greeting = "Hello, gengtype!";
char message[] = "Test message";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);
typedef void (*CallbackFunc)(int, void*);

/* Complex callback returning pointer to struct */
typedef struct BasicStruct* (*StructFactory)(int id);

/* TYPE_LANG_STRUCT: GCC-specific constructs */
#ifdef __GNUC__
/* Struct with GCC attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int* int_ptr;
    void* any_ptr;
} TransparentUnion;

/* Struct with vector attribute (GCC extension) */
typedef int v4si __attribute__((vector_size(16)));

struct VectorStruct {
    v4si vectors[2];
    int count;
};
#endif

/* Complex nesting and chains */

/* Struct containing pointer to another struct */
struct Container {
    struct BasicStruct* item;
    int count;
    union DataUnion data;
};

/* Struct containing array of unions */
struct UnionContainer {
    union DataUnion items[4];
    int active_index;
};

/* Typedef for function pointer returning pointer to struct */
typedef struct Container* (*ContainerFactory)(int size);

/* More complex nested type */
struct ComplexType {
    struct Container** containers;  /* Pointer to pointer */
    Comparator compare_func;
    CallbackFunc callback;
    int (*array_of_funcs[3])(void);
    volatile int* volatile volatile_member;
};

/* Global variables using all types */
struct BasicStruct global_struct = {1, 3.14f, "test"};
UserStruct global_user_struct = {100, 2.71828};
union DataUnion global_union = {.int_val = 42};
enum Color global_color = BLUE;

#ifdef __GNUC__
struct PackedStruct global_packed = {'A', 123, 456};
#endif

struct Container global_container = {&global_struct, 1, {.int_val = 99}};
struct UnionContainer global_union_container;

/* Function pointer variables */
Comparator global_comparator = NULL;
ContainerFactory global_factory = NULL;

/* Array initializers */
int initialized_array[5] = {1, 2, 3, 4, 5};
struct BasicStruct initialized_structs[2] = {
    {1, 1.1f, "first"},
    {2, 2.2f, "second"}
};

/* Function using callback */
static int compare_ints(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

/* Function returning pointer to struct */
static struct BasicStruct* create_basic_struct(int id) {
    static struct BasicStruct instance;
    instance.id = id;
    instance.value = id * 1.5f;
    return &instance;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 10;
    prevent_optimization += global_struct.id;
    
    /* Use user struct */
    global_user_struct.counter++;
    prevent_optimization += (int)global_user_struct.counter;
    
    /* Use union */
    global_union.int_val = 100;
    prevent_optimization += global_union.int_val;
    
    /* Use pointers */
    if (int_ptr) prevent_optimization++;
    if (struct_ptr) prevent_optimization++;
    
    /* Use arrays */
    int_array[0] = 1;
    prevent_optimization += int_array[0];
    
    char_matrix[0][0] = 'X';
    prevent_optimization += char_matrix[0][0];
    
    /* Use scalars */
    scalar_int++;
    prevent_optimization += scalar_int;
    
    /* Use enum */
    global_color = RED;
    prevent_optimization += global_color;
    
    /* Use strings */
    prevent_optimization += greeting[0];
    prevent_optimization += message[0];
    
    /* Use callbacks */
    global_comparator = compare_ints;
    if (global_comparator) {
        int a = 1, b = 2;
        prevent_optimization += global_comparator(&a, &b);
    }
    
    /* Use complex types */
    global_container.item = &global_struct;
    prevent_optimization += global_container.count;
    
    /* Initialize union container */
    for (int i = 0; i < 4; i++) {
        global_union_container.items[i].int_val = i * 10;
    }
    prevent_optimization += global_union_container.active_index;
    
#ifdef __GNUC__
    /* Use GCC-specific types */
    global_packed.a = 'B';
    prevent_optimization += global_packed.a;
#endif
    
    /* Use function returning struct pointer */
    struct BasicStruct* new_struct = create_basic_struct(5);
    prevent_optimization += new_struct->id;
    
    /* Use volatile/const qualifiers */
    if (const_string_ptr) prevent_optimization++;
    if (volatile_int_ptr) prevent_optimization++;
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional declarations in file scope for more coverage */
static struct {
    int hidden;
    char secret[10];
} anonymous_struct = {123, "hidden"};

/* Multi-dimensional pointer array */
void* complex_pointer_array[2][3];

/* Const array of function pointers */
static const int (*const const_func_ptrs[2])(void) = {NULL, NULL};

/* Mixed declarations */
static volatile int volatile_array[5] = {0};
const volatile int const_volatile_var = 999;

/* Forward declaration to test TYPE_UNDEFINED? */
struct ForwardDeclared;
extern struct ForwardDeclared* external_ptr;
