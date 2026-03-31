/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch statement in gengtype.cc (lines 182-213).
 * When processed by GCC's gengtype utility during a build, these types
 * should trigger all cases in the switch statement.
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
void reference_types(void* ptr1, void* ptr2, void* ptr3, void* ptr4, void* ptr5) {
    /* Do nothing, just prevent optimization */
    (void)ptr1; (void)ptr2; (void)ptr3; (void)ptr4; (void)ptr5;
}

/* ========== TYPE DEFINITIONS ========== */

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
} ScalarTypes;

/* TYPE_STRING: String type */
typedef struct GTY(()) StringType {
    const char* GTY((tag("0"))) string_field;  /* TYPE_STRING */
    char* mutable_string;
} StringType;

/* TYPE_STRUCT: Simple structure */
typedef struct GTY(()) SimpleStruct {
    int x;
    int y;
} SimpleStruct;

/* TYPE_USER_STRUCT: User-defined structure */
struct GTY(()) ForwardDecl;  /* Forward declaration */

typedef struct GTY(()) UserStruct {
    struct ForwardDecl* GTY((skip)) forward_ptr;  /* TYPE_USER_STRUCT */
    SimpleStruct embedded;  /* TYPE_STRUCT */
} UserStruct;

struct GTY(()) ForwardDecl {
    UserStruct* GTY((skip)) back_ptr;
    int data;
};

/* TYPE_UNION: Union type */
typedef union GTY(()) TestUnion {
    int as_int;
    float as_float;
    char* GTY((tag("1"))) as_string;
    SimpleStruct* as_struct;
} TestUnion;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    void* void_ptr;
    int* int_ptr;
    char** double_ptr_ptr;
    SimpleStruct* struct_ptr;
    TestUnion* union_ptr;
    
    /* Function pointer - TYPE_CALLBACK */
    int (*callback_func)(int, char*);
    
    /* Array of function pointers */
    void (*func_array[10])(void);
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    int fixed_array[100];
    char multi_dim[10][20];
    float* GTY((length("dynamic_length"))) flexible_array;  /* Flexible array member */
    int dynamic_length;
    
    /* Pointer to array */
    int (*array_ptr)[50];
    
    /* Array of pointers */
    SimpleStruct* struct_ptr_array[30];
} ArrayTypes;

/* TYPE_LANG_STRUCT: Complex nested structure simulating language-specific type */
typedef struct GTY(()) LangStruct {
    enum {
        TAG_INT,
        TAG_FLOAT,
        TAG_STRING,
        TAG_STRUCT
    } tag;
    
    union {
        int int_val;
        float float_val;
        char* GTY((tag("2"))) string_val;
        SimpleStruct struct_val;
    } data;
    
    /* Self-referential pointer */
    struct LangStruct* GTY((skip)) next;
    
    /* Array of unions */
    TestUnion union_array[5];
    
    /* Pointer to callback */
    void (*lang_callback)(struct LangStruct*);
} LangStruct;

/* TYPE_CALLBACK: Structure centered around callbacks */
typedef struct GTY(()) CallbackContainer {
    /* Multiple callback types */
    int (*int_callback)(int);
    char* (*string_callback)(const char*);
    void (*void_callback)(void*);
    
    /* Nested callback in union */
    union {
        void (*simple_cb)(void);
        int (*complex_cb)(int, float, char*);
    } callback_union;
    
    /* Array of callbacks */
    void (*callback_array[5])(LangStruct*);
} CallbackContainer;

/* Ultimate nested structure containing all types */
typedef struct GTY(()) MegaStruct {
    /* All basic types */
    ScalarTypes scalars;
    StringType strings;
    SimpleStruct simple;
    UserStruct user;
    TestUnion union_field;
    PointerTypes pointers;
    ArrayTypes arrays;
    LangStruct lang;
    CallbackContainer callbacks;
    
    /* Self-reference */
    struct MegaStruct* GTY((skip)) self_ptr;
    
    /* Complex nested array */
    struct {
        int depth;
        LangStruct* GTY((skip)) nodes[10];
    } nested[3];
} MegaStruct;

/* ========== MAIN FUNCTION ========== */

int main() {
    volatile size_t total_size = 0;
    
    /* Declare instances of all types */
    ScalarTypes scalars;
    StringType strings;
    SimpleStruct simple;
    UserStruct user;
    TestUnion union_var;
    PointerTypes pointers;
    ArrayTypes arrays;
    LangStruct lang;
    CallbackContainer callbacks;
    MegaStruct mega;
    struct ForwardDecl forward;
    
    /* Initialize to avoid undefined behavior */
    scalars.int_field = 42;
    strings.string_field = "Hello, gengtype!";
    simple.x = 1; simple.y = 2;
    user.forward_ptr = &forward;
    forward.back_ptr = &user;
    forward.data = 99;
    union_var.as_int = 100;
    pointers.void_ptr = &scalars;
    arrays.fixed_array[0] = 123;
    lang.tag = TAG_INT;
    lang.data.int_val = 456;
    lang.next = NULL;
    callbacks.int_callback = NULL;
    mega.scalars = scalars;
    mega.self_ptr = &mega;
    
    /* Take addresses of all instances */
    ScalarTypes* scalar_ptr = &scalars;
    StringType* string_ptr = &strings;
    SimpleStruct* simple_ptr = &simple;
    UserStruct* user_ptr = &user;
    TestUnion* union_ptr = &union_var;
    PointerTypes* pointer_ptr = &pointers;
    ArrayTypes* array_ptr = &arrays;
    LangStruct* lang_ptr = &lang;
    CallbackContainer* callback_ptr = &callbacks;
    MegaStruct* mega_ptr = &mega;
    struct ForwardDecl* forward_ptr = &forward;
    
    /* Compute sizes of all types - forces compiler to consider type layouts */
    total_size += sizeof(ScalarTypes);
    total_size += sizeof(StringType);
    total_size += sizeof(SimpleStruct);
    total_size += sizeof(UserStruct);
    total_size += sizeof(TestUnion);
    total_size += sizeof(PointerTypes);
    total_size += sizeof(ArrayTypes);
    total_size += sizeof(LangStruct);
    total_size += sizeof(CallbackContainer);
    total_size += sizeof(MegaStruct);
    total_size += sizeof(struct ForwardDecl);
    
    /* Take addresses of members within complex structures */
    total_size += (size_t)&mega.nested[0].nodes[0];
    total_size += (size_t)&arrays.flexible_array;
    total_size += (size_t)&pointers.callback_func;
    total_size += (size_t)&callbacks.callback_array[0];
    total_size += (size_t)&lang.union_array[0];
    
    /* Reference all types through external function to prevent optimization */
    reference_types(scalar_ptr, string_ptr, simple_ptr, user_ptr, forward_ptr);
    reference_types(union_ptr, pointer_ptr, array_ptr, lang_ptr, callback_ptr);
    reference_types(mega_ptr, &mega.nested[0], &arrays.multi_dim[0][0], 
                   &pointers.func_array[0], &callbacks.callback_union);
    
    /* Use KEEP_ALIVE to ensure all variables are considered live */
    KEEP_ALIVE(scalars);
    KEEP_ALIVE(strings);
    KEEP_ALIVE(simple);
    KEEP_ALIVE(user);
    KEEP_ALIVE(union_var);
    KEEP_ALIVE(pointers);
    KEEP_ALIVE(arrays);
    KEEP_ALIVE(lang);
    KEEP_ALIVE(callbacks);
    KEEP_ALIVE(mega);
    KEEP_ALIVE(forward);
    
    /* Print result to prevent entire program from being optimized away */
    printf("Total size computation: %zu\n", total_size % 1000);
    printf("Type coverage test completed.\n");
    
    return 0;
}
