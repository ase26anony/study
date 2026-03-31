/* test_gengtype_coverage.c
 * A test program to exercise type enumeration in gengtype.cc
 * Defines complex nested structures to trigger all type kind cases
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation - in real GCC this would be the actual GTY marker */
#define GTY(x)

/* Forward declarations to enable mutual recursion */
struct GTY(()) ComplexStruct;
union GTY(()) ComplexUnion;

/* ==================== TYPE_SCALAR definitions ==================== */
struct GTY(()) ScalarTypes {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_value;
    short short_value;
    unsigned int unsigned_integer;
    signed char signed_char;
};

/* ==================== TYPE_STRING definitions ==================== */
struct GTY(()) StringTypes {
    const char* constant_string;
    char* mutable_string;
    const char* const constant_string_array[3];
};

/* ==================== TYPE_POINTER definitions ==================== */
struct GTY(()) PointerTypes {
    /* Simple pointers */
    int* int_ptr;
    char** char_ptr_ptr;
    
    /* Function pointers (TYPE_CALLBACK) */
    int (*func_ptr)(int, char);
    void (*void_func_ptr)(void);
    char* (*string_func_ptr)(const char*);
    
    /* Pointer to struct */
    struct ComplexStruct* struct_ptr;
    
    /* Pointer to union */
    union ComplexUnion* union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Pointer to pointer to function */
    int (**func_ptr_ptr)(void);
    
    /* Self-referential pointer */
    struct PointerTypes* self_ptr;
};

/* ==================== TYPE_ARRAY definitions ==================== */
struct GTY(()) ArrayTypes {
    /* Fixed-size arrays */
    int fixed_array[20];
    char char_array[50];
    float float_array[5][5];  /* Multi-dimensional */
    
    /* Array of pointers */
    void* ptr_array[15];
    
    /* Array of function pointers */
    int (*func_array[8])(void);
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ==================== TYPE_STRUCT definitions ==================== */
struct GTY(()) NestedStruct {
    int level1;
    struct {
        int level2;
        struct {
            int level3;
            struct {
                int level4;
            } deepest;
        } deeper;
    } inner;
};

/* ==================== TYPE_UNION definitions ==================== */
union GTY(()) ComplexUnion {
    int as_int;
    float as_float;
    double as_double;
    void* as_pointer;
    struct {
        int x;
        int y;
    } as_struct;
    char as_array[16];
};

/* ==================== TYPE_USER_STRUCT definitions ==================== */
/* This would typically be a user-defined struct type in GCC's context */
typedef struct GTY(()) {
    int tag;
    union {
        int int_value;
        float float_value;
        struct ComplexStruct* struct_value;
    } data;
} UserDefinedStruct;

/* ==================== TYPE_LANG_STRUCT definitions ==================== */
/* Simulating language-specific structure types */
struct GTY(()) LangSpecificStruct {
    int lang_specific_field;
    void* lang_data;
    
    /* Language-specific extensions */
    struct {
        int extension_field1;
        char extension_field2;
    } GTY((tag("LANG_EXT"))) lang_extension;
};

/* ==================== Complex nested structure ==================== */
struct GTY(()) ComplexStruct {
    /* Scalar types */
    int id;
    char flags;
    
    /* String type */
    const char* name;
    
    /* Pointer types */
    struct ComplexStruct* next;
    struct ComplexStruct* prev;
    void** void_ptr_ptr;
    
    /* Array types */
    int scores[10];
    struct ComplexStruct* children[5];
    
    /* Nested struct */
    struct {
        int x, y, z;
    } coordinates;
    
    /* Nested union */
    union {
        int int_view;
        float float_view;
        char bytes[4];
    } data_union;
    
    /* Function pointer */
    int (*compare)(struct ComplexStruct*, struct ComplexStruct*);
    
    /* Array of function pointers */
    void (*handlers[5])(struct ComplexStruct*);
    
    /* Pointer to array */
    int (*matrix_ptr)[3][3];
    
    /* Reference to another complex type */
    union ComplexUnion* union_ref;
    
    /* For mutual recursion */
    struct ComplexStruct* recursive_ptr;
    
    /* Flexible array of pointers */
    struct ComplexStruct* variable_list[];
};

/* ==================== TYPE_CALLBACK definitions ==================== */
/* Using typedef for function pointer type */
typedef int (*Comparator)(const void*, const void*);

struct GTY(()) CallbackContainer {
    Comparator sorter;
    void (*initializer)(void*);
    void (*destructor)(void*);
    
    /* Array of callbacks */
    void (*callbacks[10])(int, void*);
};

/* ==================== Global instances ==================== */
/* Global variables to ensure types are used */
GTY(()) struct ScalarTypes global_scalars = {0};
GTY(()) struct StringTypes global_strings = {0};
GTY(()) struct PointerTypes global_pointers = {0};
GTY(()) struct ArrayTypes* global_array_ptr = NULL;
GTY(()) struct NestedStruct global_nested = {0};
GTY(()) union ComplexUnion global_union = {0};
GTY(()) UserDefinedStruct global_user_struct = {0};
GTY(()) struct LangSpecificStruct global_lang_struct = {0};
GTY(()) struct ComplexStruct global_complex = {0};
GTY(()) struct CallbackContainer global_callbacks = {0};

/* ==================== External function to prevent optimization ==================== */
/* Use noinline attribute to prevent inlining and ensure types are referenced */
__attribute__((noinline)) 
static size_t compute_type_sizes(void) {
    size_t total_size = 0;
    
    /* Take addresses and compute sizes of all types */
    total_size += sizeof(struct ScalarTypes);
    total_size += sizeof(struct StringTypes);
    total_size += sizeof(struct PointerTypes);
    total_size += sizeof(struct ArrayTypes);
    total_size += sizeof(struct NestedStruct);
    total_size += sizeof(union ComplexUnion);
    total_size += sizeof(UserDefinedStruct);
    total_size += sizeof(struct LangSpecificStruct);
    total_size += sizeof(struct ComplexStruct);
    total_size += sizeof(struct CallbackContainer);
    
    /* Take addresses of global instances */
    volatile void* ptr;
    ptr = &global_scalars;
    ptr = &global_strings;
    ptr = &global_pointers;
    ptr = &global_nested;
    ptr = &global_union;
    ptr = &global_user_struct;
    ptr = &global_lang_struct;
    ptr = &global_complex;
    ptr = &global_callbacks;
    
    (void)ptr; /* Suppress unused variable warning */
    
    return total_size;
}

/* ==================== Function using function pointers ==================== */
static int sample_comparator(const void* a, const void* b) {
    return *(const int*)a - *(const int*)b;
}

static void sample_initializer(void* data) {
    *(int*)data = 42;
}

/* ==================== Main function ==================== */
int main(void) {
    size_t total_size = 0;
    
    /* Initialize some data */
    global_scalars.integer = 100;
    global_scalars.floating = 3.14159f;
    
    global_strings.constant_string = "Hello, gengtype!";
    
    global_pointers.func_ptr = NULL;
    global_pointers.self_ptr = &global_pointers;
    
    /* Initialize function pointers */
    global_callbacks.sorter = sample_comparator;
    global_callbacks.initializer = sample_initializer;
    
    /* Complex struct initialization */
    global_complex.id = 1;
    global_complex.name = "TestStruct";
    global_complex.next = &global_complex;
    global_complex.prev = &global_complex;
    
    /* Union usage */
    global_union.as_int = 42;
    global_union.as_float = 2.71828f;
    
    /* User struct */
    global_user_struct.tag = 1;
    global_user_struct.data.int_value = 100;
    
    /* Lang struct */
    global_lang_struct.lang_specific_field = 99;
    
    /* Compute total size of all types */
    total_size = compute_type_sizes();
    
    /* Additional type operations to ensure all types are referenced */
    printf("Type analysis test program\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    
    /* More operations to prevent optimization */
    printf("Size of ComplexStruct: %zu\n", sizeof(struct ComplexStruct));
    printf("Size of ComplexUnion: %zu\n", sizeof(union ComplexUnion));
    printf("Size of PointerTypes: %zu\n", sizeof(struct PointerTypes));
    printf("Size of ArrayTypes (base): %zu\n", offsetof(struct ArrayTypes, flexible_array));
    
    /* Take addresses of nested members */
    printf("Offset of inner in NestedStruct: %zu\n", 
           offsetof(struct NestedStruct, inner));
    printf("Offset of deeper in inner: %zu\n", 
           offsetof(struct NestedStruct, inner.deeper));
    
    /* Use function pointers */
    if (global_callbacks.sorter) {
        int a = 5, b = 10;
        int result = global_callbacks.sorter(&a, &b);
        printf("Comparator result: %d\n", result);
    }
    
    /* Array operations */
    for (int i = 0; i < 10; i++) {
        global_complex.scores[i] = i * 10;
    }
    
    /* Pointer arithmetic */
    int* int_ptr = global_complex.scores;
    printf("First score: %d\n", *int_ptr);
    
    return 0;
}
