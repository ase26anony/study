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
    int x, y;
    double z;
} UserStruct;

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
int *pointer_array[8];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enum type (also scalar) */
enum Color { RED, GREEN, BLUE };
enum Color color_scalar;

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, gengtype!";
char filename[] = "test.txt";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

/* Complex callback type */
typedef struct SimpleStruct* (*FactoryFunc)(int id, const char *name);

/* TYPE_LANG_STRUCT: GCC language extension constructs */
#ifdef __GNUC__
/* Struct with GCC attributes */
struct __attribute__((packed, aligned(8))) PackedStruct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union (GCC extension) */
typedef union __attribute__((transparent_union)) TransparentUnion {
    int *int_ptr;
    void *void_ptr;
} TransparentUnion;

/* Struct with vector extension (if available) */
#ifdef __SSE2__
struct VectorStruct {
    int __attribute__((vector_size(16))) v1;
    float __attribute__((vector_size(16))) v2;
};
#endif

/* Use #pragma pack */
#pragma pack(push, 1)
struct PackedWithPragma {
    char c;
    int i;
    short s;
};
#pragma pack(pop)
#endif

/* Complex nesting and chains */

/* Struct containing pointer to another struct */
struct Container {
    struct SimpleStruct *item;
    int count;
    UserStruct *users;
    union DataUnion data;
};

/* Struct containing array of unions */
struct UnionContainer {
    union DataUnion items[10];
    int size;
};

/* Typedef for function pointer returning pointer to struct */
typedef struct Container* (*GetContainerFunc)(int id);

/* More complex type with const and volatile qualifiers */
const volatile int *const volatile_qualified;
struct SimpleStruct *const const_struct_ptr;
volatile UserStruct *volatile_user_struct;

/* Global variables using the types */
struct SimpleStruct global_struct = {1, 3.14f, "test"};
UserStruct global_user_struct = {10, 20, 30.5};
union DataUnion global_union = {.int_val = 42};
struct Container global_container = {&global_struct, 1, &global_user_struct, {.int_val = 100}};
Comparator global_comparator = NULL;
FactoryFunc global_factory = NULL;

#ifdef __GNUC__
struct PackedStruct global_packed = {'a', 123, 456};
#endif

/* Function using callback */
static int compare_ints(const void *a, const void *b) {
    return *(const int*)a - *(const int*)b;
}

/* Function that could be used as factory */
static struct SimpleStruct* create_struct(int id, const char *name) {
    static struct SimpleStruct result;
    result.id = id;
    result.value = 0.0f;
    if (name) {
        for (int i = 0; i < 31 && name[i]; i++) {
            result.name[i] = name[i];
        }
        result.name[31] = '\0';
    }
    return &result;
}

/* Main function to ensure all types are "used" */
int main(void) {
    volatile int prevent_optimization = 0;
    
    /* Use struct */
    global_struct.id = 100;
    global_struct.value = 2.718f;
    
    /* Use user struct */
    global_user_struct.x = 5;
    global_user_struct.y = 10;
    global_user_struct.z = 15.5;
    
    /* Use union */
    global_union.float_val = 3.14159f;
    
    /* Use pointers */
    int_ptr = &scalar_int;
    struct_ptr = &global_struct;
    void_ptr = (void*)&global_user_struct;
    
    /* Use arrays */
    int_array[0] = 1;
    char_matrix[0][0] = 'A';
    struct_array[0].id = 1;
    pointer_array[0] = &scalar_int;
    
    /* Use scalars */
    scalar_int = 42;
    scalar_float = 1.234f;
    scalar_double = 5.678;
    color_scalar = GREEN;
    
    /* Use strings */
    prevent_optimization += greeting[0];
    prevent_optimization += filename[0];
    
    /* Use callbacks */
    global_comparator = compare_ints;
    global_factory = create_struct;
    
    /* Use complex nesting */
    global_container.item = &global_struct;
    global_container.count = 5;
    
    /* Use qualified pointers */
    volatile_qualified = &scalar_int;
    const_struct_ptr = &global_struct;
    
    #ifdef __GNUC__
    /* Use GCC-specific types */
    global_packed.a = 'b';
    global_packed.b = 456;
    global_packed.c = 789;
    #endif
    
    /* Actually use the callback */
    int a = 5, b = 10;
    if (global_comparator) {
        prevent_optimization += global_comparator(&a, &b);
    }
    
    /* Use factory function */
    if (global_factory) {
        struct SimpleStruct *s = global_factory(1, "test");
        prevent_optimization += s->id;
    }
    
    return prevent_optimization == 0 ? 0 : 1;
}

/* Additional type in different linkage to test cross-file scenarios */
static struct StaticType {
    int internal;
    const char *message;
} static_var = {999, "static message"};

/* Function with complex return type */
static union DataUnion* get_data_union(void) {
    return &global_union;
}

/* Inline function with structure parameter */
static inline void process_struct(struct SimpleStruct *s) {
    if (s) {
        s->id++;
    }
}
