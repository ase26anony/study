/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct SimpleStruct {
    int id;
    float value;
    char name[32];
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    int x;
    int y;
} Point;

/* TYPE_UNION: Union with several members */
union DataUnion {
    int int_val;
    float float_val;
    char char_val;
    void *ptr_val;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct SimpleStruct *struct_ptr;
void *void_ptr;
const char *const_string_ptr;
volatile int *volatile_int_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_matrix[5][5];
struct SimpleStruct struct_array[3];
Point *pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enum type (also scalar) */
enum Color {
    RED,
    GREEN,
    BLUE
};

/* TYPE_STRING: String literals in initializers */
const char *greeting = "Hello, World!";
char filename[] = "test.txt";

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* Complex callback with struct return */
struct SimpleStruct* (*struct_factory)(int, const char*);

/* TYPE_LANG_STRUCT: GCC-specific constructs with attributes */
struct __attribute__((packed, aligned(4))) PackedStruct {
    char a;
    int b;
    char c;
};

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* More complex nested structures for deep traversal */
struct ComplexNode {
    int data;
    struct ComplexNode *next;
    union DataUnion value;
    Comparator compare;
};

/* Struct with array of function pointers */
struct OperationSet {
    const char *name;
    int (*operations[5])(int, int);
};

/* Union with struct member */
union NestedUnion {
    struct {
        int type;
        void *data;
    } header;
    struct SimpleStruct body;
};

/* Volatile and const qualified types */
volatile const int volatile_const_int = 42;
const volatile char *const volatile_string = "constant";

/* Using #pragma pack */
#pragma pack(push, 1)
struct PackedWithPragma {
    char c;
    int i;
    short s;
};
#pragma pack(pop)

/* Chain of pointers */
struct ChainElement {
    int value;
    struct ChainElement ***triple_ptr;
};

/* Array of unions */
union DataUnion union_array[4];

/* Function pointer returning pointer to array */
int (*func_returning_array_ptr(void))[10] {
    static int arr[10];
    return &arr;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    struct SimpleStruct s = {1, 3.14f, "test"};
    prevent_optimization += s.id;
    
    /* Use user struct */
    Point p = {10, 20};
    prevent_optimization += p.x;
    
    /* Use union */
    union DataUnion u;
    u.int_val = 42;
    prevent_optimization += u.int_val;
    
    /* Use pointers */
    int_ptr = &prevent_optimization;
    struct_ptr = &s;
    prevent_optimization += *int_ptr;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[0][0] = 'A';
    prevent_optimization += int_array[0];
    
    /* Use scalars */
    scalar_int = 100;
    prevent_optimization += scalar_int;
    
    /* Use enum */
    enum Color color = RED;
    prevent_optimization += color;
    
    /* Use strings */
    prevent_optimization += greeting[0];
    prevent_optimization += filename[0];
    
    /* Use callback (declare one but don't call unsafe ones) */
    Comparator cmp = NULL;
    if (cmp) {
        cmp(&prevent_optimization, &prevent_optimization);
    }
    
    /* Use GCC-specific structs */
    struct PackedStruct ps = {'a', 42, 'b'};
    prevent_optimization += ps.a;
    
    /* Use complex nested structure */
    struct ComplexNode node = {0};
    node.data = 42;
    prevent_optimization += node.data;
    
    /* Use volatile/const */
    prevent_optimization += volatile_const_int;
    
    /* Use packed with pragma */
    struct PackedWithPragma pwp = {'x', 123, 456};
    prevent_optimization += pwp.c;
    
    /* Use chain */
    struct ChainElement chain = {0};
    prevent_optimization += chain.value;
    
    /* Use array of unions */
    union_array[0].int_val = 99;
    prevent_optimization += union_array[0].int_val;
    
    /* Call function returning array pointer */
    int (*arr_ptr)[10] = func_returning_array_ptr();
    prevent_optimization += (*arr_ptr)[0];
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional function using transparent union */
void use_transparent_union(TransparentUnion tu) {
    /* Transparent unions can be called with any member type */
    int *ip = tu.int_ptr;
    (void)ip;
}

/* Static variables at file scope (ensures gengtype sees them) */
static struct SimpleStruct static_struct = {2, 2.718f, "static"};
static union DataUnion static_union = {.float_val = 3.14159f};
static int static_array[5] = {1, 2, 3, 4, 5};
static const char *static_string = "Static string";

/* Extern declaration (simulating multiple translation units) */
extern struct SimpleStruct external_struct;

/* Struct with bitfields (another GCC extension) */
struct BitfieldStruct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 3;
    unsigned int flag3 : 4;
    int regular_member;
};

/* Anonymous struct/union (C11/GCC extension) */
struct Container {
    struct {
        int inner_x;
        int inner_y;
    };
    union {
        int as_int;
        float as_float;
    };
};

/* Flexible array member (C99) */
struct FlexibleArray {
    int length;
    char data[];  /* Flexible array member */
};

/* Aligned attribute directly on variables */
int __attribute__((aligned(16))) aligned_var = 0;

/* Weak attribute */
extern int __attribute__((weak)) weak_symbol;

/* Cleanup attribute */
void cleanup_func(int *p) {
    *p = 0;
}

/* Section attribute */
int __attribute__((section(".mysection"))) section_var = 123;

/* Used attribute to prevent elimination */
static int __attribute__((used)) used_var = 456;
