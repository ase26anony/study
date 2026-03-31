/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * the type enumeration switch in gengtype.cc (lines 182-213).
 * When processed by gengtype during a GCC build, these types should
 * trigger all cases in the switch statement.
 */

/* Dummy GTY macro for compilation - in real GCC build this would be
 * the actual garbage collector annotation */
#define GTY(x) 

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization from removing type references */
#define KEEP_ALIVE(x) asm volatile("" : : "r"(x) : "memory")

/* External function to ensure types are referenced */
__attribute__((noinline)) 
void use_pointer(void *p) {
    volatile static int sink;
    sink = (int)(intptr_t)p;
}

/* TYPE_SCALAR: Basic scalar types */
typedef struct GTY(()) ScalarTypes {
    int int_field;
    char char_field;
    float float_field;
    double double_field;
    _Bool bool_field;
    long long_field;
    short short_field;
    signed char s_char_field;
    unsigned int uint_field;
} ScalarTypes;

/* TYPE_STRING: String types */
typedef struct GTY(()) StringTypes {
    const char *const_string;
    char *mutable_string;
    wchar_t *wide_string;
    char fixed_string[64];
} StringTypes;

/* TYPE_POINTER: Various pointer types */
typedef struct GTY(()) PointerTypes {
    void *void_ptr;
    int *int_ptr;
    struct PointerTypes *self_ptr;
    void (**func_ptr_array)(void);
    int *const const_ptr;
    volatile int *volatile_ptr;
} PointerTypes;

/* TYPE_ARRAY: Array types */
typedef struct GTY(()) ArrayTypes {
    int simple_array[10];
    char multi_dim[5][10];
    struct ArrayTypes *ptr_array[20];
    int flexible_array[];
} ArrayTypes;

/* TYPE_STRUCT: Nested structures */
typedef struct GTY(()) InnerStruct {
    int inner_data;
    double inner_value;
} InnerStruct;

typedef struct GTY(()) OuterStruct {
    InnerStruct nested;
    InnerStruct *nested_ptr;
    struct {
        int anonymous_member;
        float anonymous_float;
    } anonymous;
    struct NamedInner GTY(()) {
        char name[32];
        int id;
    } named;
} OuterStruct;

/* TYPE_UNION: Union types */
typedef union GTY(()) DataUnion {
    int as_int;
    float as_float;
    double as_double;
    void *as_ptr;
    struct {
        int low;
        int high;
    } as_struct;
} DataUnion;

typedef struct GTY(()) UnionContainer {
    DataUnion data;
    union {
        char small;
        long large;
    } inline_union;
} UnionContainer;

/* TYPE_USER_STRUCT: User-defined structure type */
struct ForwardDecl;
typedef struct ForwardDecl GTY(()) ForwardDecl;

typedef struct GTY(()) UserDefined {
    struct ForwardDecl *forward_ref;
    enum {
        STATE_A,
        STATE_B,
        STATE_C
    } state;
    void (*callback)(struct UserDefined *);
} UserDefined;

struct ForwardDecl {
    UserDefined *owner;
    int magic;
};

/* TYPE_CALLBACK: Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*EventHandler)(int event_id, void *user_data);

typedef struct GTY(()) CallbackContainer {
    Comparator compare_func;
    EventHandler handlers[5];
    void (*complex_callback)(struct CallbackContainer *, int, ...);
    int (*method)(struct CallbackContainer *self, float param);
} CallbackContainer;

/* Complex nested type combining everything */
typedef struct GTY(()) SuperComplexType {
    /* TYPE_SCALAR */
    int counter;
    double precision;
    
    /* TYPE_STRING */
    const char *name;
    char buffer[256];
    
    /* TYPE_POINTER */
    struct SuperComplexType *next;
    void **pointer_array;
    
    /* TYPE_ARRAY */
    int matrix[3][3];
    union {
        int x;
        float y;
    } union_array[10];
    
    /* TYPE_STRUCT */
    InnerStruct inner;
    OuterStruct outer;
    
    /* TYPE_UNION */
    DataUnion variant;
    
    /* TYPE_USER_STRUCT */
    UserDefined *user_data;
    
    /* TYPE_CALLBACK */
    int (*processor)(struct SuperComplexType *);
    
    /* Flexible array member (special array case) */
    unsigned char extra_data[];
} SuperComplexType;

/* TYPE_LANG_STRUCT: Simulating language-specific structure */
typedef struct GTY(()) LangSpecific {
    /* This would normally have GCC internal types */
    void *tree_node;
    void *rtx_value;
    struct LangSpecific *chain;
    int lang_specific_flags;
} LangSpecific;

/* Global variables to ensure types are instantiated */
ScalarTypes GTY(()) g_scalar = {0};
StringTypes GTY(()) g_string = {0};
PointerTypes GTY(()) g_pointer = {0};
ArrayTypes GTY(()) *g_array_ptr = NULL;
OuterStruct GTY(()) g_outer = {0};
UnionContainer GTY(()) g_union = {0};
UserDefined GTY(()) g_user = {0};
CallbackContainer GTY(()) g_callback = {0};
SuperComplexType GTY(()) *g_complex = NULL;
LangSpecific GTY(()) g_lang = {0};

/* Function to create complex type instances */
__attribute__((noinline))
SuperComplexType* create_complex_type(void) {
    static SuperComplexType instance;
    instance.counter = 42;
    instance.name = "Complex Instance";
    instance.next = &instance;
    instance.processor = NULL;
    return &instance;
}

int main(void) {
    /* Force consideration of all types by the compiler */
    
    /* TYPE_SCALAR references */
    ScalarTypes scalar_local = {1, 'A', 3.14f, 2.71828, 1, 100L, 2, -1, 42U};
    use_pointer(&scalar_local);
    size_t scalar_size = sizeof(ScalarTypes);
    
    /* TYPE_STRING references */
    StringTypes string_local = {"Hello", "World", L"Wide", "Fixed"};
    use_pointer(&string_local);
    size_t string_size = sizeof(StringTypes);
    
    /* TYPE_POINTER references */
    int dummy_int = 42;
    PointerTypes pointer_local = {&dummy_int, &dummy_int, NULL, NULL, &dummy_int, &dummy_int};
    use_pointer(&pointer_local);
    size_t pointer_size = sizeof(PointerTypes);
    
    /* TYPE_ARRAY references */
    ArrayTypes *array_local = (ArrayTypes*)malloc(sizeof(ArrayTypes) + 50 * sizeof(int));
    if (array_local) {
        for (int i = 0; i < 10; i++) array_local->simple_array[i] = i;
        use_pointer(array_local);
        size_t array_size = sizeof(ArrayTypes) + 50 * sizeof(int);
        free(array_local);
    }
    
    /* TYPE_STRUCT references */
    OuterStruct outer_local = {
        .nested = {1, 2.0},
        .nested_ptr = NULL,
        .anonymous = {3, 4.0f},
        .named = {"Test", 5}
    };
    use_pointer(&outer_local);
    size_t outer_size = sizeof(OuterStruct);
    
    /* TYPE_UNION references */
    UnionContainer union_local = {
        .data = {.as_int = 42},
        .inline_union = {.large = 1000L}
    };
    use_pointer(&union_local);
    size_t union_size = sizeof(UnionContainer);
    
    /* TYPE_USER_STRUCT references */
    UserDefined user_local = {NULL, STATE_A, NULL};
    ForwardDecl forward_local = {&user_local, 123};
    user_local.forward_ref = &forward_local;
    use_pointer(&user_local);
    size_t user_size = sizeof(UserDefined);
    
    /* TYPE_CALLBACK references */
    CallbackContainer callback_local = {NULL, {NULL}, NULL, NULL};
    use_pointer(&callback_local);
    size_t callback_size = sizeof(CallbackContainer);
    
    /* TYPE_LANG_STRUCT references */
    LangSpecific lang_local = {NULL, NULL, NULL, 0xABCD};
    use_pointer(&lang_local);
    size_t lang_size = sizeof(LangSpecific);
    
    /* Create and reference the super complex type */
    SuperComplexType *complex_local = create_complex_type();
    use_pointer(complex_local);
    size_t complex_size = sizeof(SuperComplexType);
    
    /* Calculate checksum to ensure all code is used */
    size_t total_size = 
        scalar_size + string_size + pointer_size + outer_size +
        union_size + user_size + callback_size + lang_size + complex_size;
    
    /* Use volatile to prevent optimization */
    volatile size_t result = total_size;
    
    printf("Type analysis coverage test complete.\n");
    printf("Total size of all types: %zu bytes\n", result);
    
    /* Reference global instances */
    KEEP_ALIVE(g_scalar);
    KEEP_ALIVE(g_string);
    KEEP_ALIVE(g_pointer);
    KEEP_ALIVE(g_array_ptr);
    KEEP_ALIVE(g_outer);
    KEEP_ALIVE(g_union);
    KEEP_ALIVE(g_user);
    KEEP_ALIVE(g_callback);
    KEEP_ALIVE(g_complex);
    KEEP_ALIVE(g_lang);
    
    return 0;
}
