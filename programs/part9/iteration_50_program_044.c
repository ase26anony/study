/* test_gengtype_coverage.c
 * Comprehensive type declarations to exercise gengtype's type system
 */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct PlainStruct {
    int id;
    float value;
    char name[32];
    void *data;
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    long counter;
    double precision;
    struct PlainStruct *link;
} UserStruct;

/* TYPE_UNION: Union with several members */
union MixedUnion {
    int as_int;
    float as_float;
    char as_char;
    void *as_ptr;
    struct PlainStruct as_struct;
};

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array_1d[10];
char char_array_2d[5][5];
float float_array_3d[3][3][3];

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct PlainStruct *struct_ptr;
void **void_ptr_ptr;
volatile int *volatile_int_ptr;
const char *const_string_ptr;

/* TYPE_SCALAR: Scalar types */
enum Color { RED, GREEN, BLUE };
typedef enum Color ColorEnum;

/* TYPE_STRING: String literal in initializer */
const char *greeting = "Hello, gengtype!";
char message[] = "Test message";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void*, const void*);
typedef void (*Callback)(int, const char*);

/* Complex nesting and chains */
struct ComplexNode {
    int id;
    union MixedUnion data;
    struct ComplexNode *next;
    struct ComplexNode *prev;
    Comparator compare;
};

/* TYPE_LANG_STRUCT: GCC-specific attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    double c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) {
    int *intp;
    void *voidp;
} TransparentUnionPtr;

/* Volatile and const qualifiers in complex combinations */
volatile int * const volatile_ptr_const = (volatile int*)0x1000;
const volatile long double cv_ld = 3.14159265358979323846L;

/* Nested array of function pointers */
Callback callback_array[5];

/* Struct containing array of unions */
struct UnionContainer {
    int count;
    union MixedUnion items[10];
};

/* Typedef for function pointer returning pointer to struct */
typedef struct PlainStruct* (*StructFactory)(int, const char*);

/* Global variables using all declared types */
struct PlainStruct global_struct = {1, 3.14f, "test", NULL};
UserStruct global_user_struct = {100, 2.71828, &global_struct};
union MixedUnion global_union = {.as_int = 42};
struct ComplexNode node1 = {1, {.as_int = 10}, NULL, NULL, NULL};
struct PackedStruct packed_global = {'X', 999, 1.234};

/* Function using callback */
static int compare_ints(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

/* Struct factory function */
static struct PlainStruct* create_struct(int id, const char *name) {
    static struct PlainStruct local;
    local.id = id;
    if (name) {
        for (int i = 0; i < 31 && name[i]; i++) {
            local.name[i] = name[i];
        }
    }
    return &local;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = prevent_optimization + 1;
    global_struct.value = 2.5f;
    
    /* Use user struct */
    global_user_struct.counter++;
    
    /* Use union */
    global_union.as_float = 3.14f;
    
    /* Use arrays */
    int_array_1d[0] = 100;
    char_array_2d[0][0] = 'A';
    float_array_3d[0][0][0] = 1.0f;
    
    /* Use pointers */
    if (int_ptr) *int_ptr = 5;
    if (struct_ptr) struct_ptr->id = 10;
    
    /* Use scalar/enum */
    ColorEnum color = RED;
    color = GREEN;
    
    /* Use strings */
    if (greeting[0] != '\0') prevent_optimization++;
    message[0] = 'M';
    
    /* Use callbacks */
    callback_array[0] = NULL;
    Comparator cmp = compare_ints;
    int x = 5, y = 10;
    if (cmp) cmp(&x, &y);
    
    /* Use complex nested types */
    node1.data.as_int = 20;
    node1.compare = compare_ints;
    
    /* Use packed struct */
    packed_global.a = 'Y';
    
    /* Use transparent union */
    TransparentUnionPtr tup;
    tup.intp = &x;
    
    /* Use volatile/const */
    if (volatile_ptr_const) prevent_optimization++;
    
    /* Use struct with array of unions */
    struct UnionContainer container;
    container.count = 3;
    container.items[0].as_int = 100;
    
    /* Use struct factory */
    StructFactory factory = create_struct;
    if (factory) factory(1, "test");
    
    /* Ensure all arrays are addressed */
    for (int i = 0; i < 10; i++) {
        int_array_1d[i] = i;
    }
    
    return prevent_optimization > 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct {
    int hidden;
    const char *secret;
} static_struct = {42, "hidden"};

/* One more GCC attribute for good measure */
struct __attribute__((may_alias)) AliasedStruct {
    int values[4];
};
