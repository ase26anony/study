/* test_gengtype_coverage.c
 * 
 * This program defines complex, nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * Compile with: gcc -O0 -g -c -ffat-lto-objects test_gengtype_coverage.c
 * Or integrate into GCC build with appropriate GTY markers.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Dummy GTY macro for compilation outside GCC build system */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declarations to create pointer cycles */
struct ForwardDecl;
union ForwardUnion;

/* ========== TYPE_SCALAR definitions ========== */
struct ScalarTypes GTY(()) {
    int integer;
    char character;
    float floating;
    double double_precision;
    long long_int;
    unsigned int unsigned_int;
    _Bool boolean;
};

/* ========== TYPE_STRING definitions ========== */
struct StringTypes GTY(()) {
    const char *constant_string;
    char *mutable_string;
    const char *const constant_string_array[3];
};

/* ========== TYPE_STRUCT definitions ========== */
struct InnerStruct GTY(()) {
    int inner_data;
    float inner_float;
};

struct OuterStruct GTY(()) {
    struct InnerStruct nested;
    int outer_data;
};

/* ========== TYPE_USER_STRUCT definitions ========== */
/* User-defined struct with complex nesting */
struct UserDefined GTY(()) {
    struct {
        int anonymous_member;
        struct InnerStruct another_nested;
    } anonymous_struct;
    
    struct OuterStruct *pointer_to_struct;
};

/* ========== TYPE_UNION definitions ========== */
union ComplexUnion GTY(()) {
    int as_int;
    float as_float;
    double as_double;
    void *as_pointer;
    struct {
        int union_nested_int;
        char union_nested_char;
    } nested_in_union;
};

/* ========== TYPE_POINTER definitions ========== */
struct PointerFest GTY(()) {
    /* Simple pointers */
    int *int_ptr;
    char **char_ptr_ptr;
    
    /* Function pointers (TYPE_CALLBACK) */
    int (*func_ptr)(int, char);
    void (*void_func_ptr)(void);
    
    /* Pointer to struct */
    struct OuterStruct *struct_ptr;
    
    /* Pointer to union */
    union ComplexUnion *union_ptr;
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Self-referential pointer */
    struct PointerFest *next;
    
    /* Forward declaration pointer */
    struct ForwardDecl *forward_ptr;
    
    /* Pointer to incomplete type */
    void *opaque_ptr;
};

/* ========== TYPE_ARRAY definitions ========== */
struct ArrayTypes GTY(()) {
    /* Fixed-size arrays */
    int fixed_array[20];
    char char_array[50];
    struct InnerStruct struct_array[5];
    union ComplexUnion union_array[3];
    
    /* Multi-dimensional arrays */
    int matrix[3][4];
    char cube[2][3][4];
    
    /* Array of pointers */
    int *pointer_array[8];
    struct OuterStruct *struct_ptr_array[5];
    
    /* Flexible array member (C99) */
    int flexible_array[];
};

/* ========== TYPE_CALLBACK definitions ========== */
/* Function pointer types */
typedef int (*Comparator)(const void *, const void *);
typedef void (*CallbackFunc)(int, void *);

struct CallbackContainer GTY(()) {
    Comparator compare_func;
    CallbackFunc callback;
    void (*signal_handler)(int);
    
    /* Array of function pointers */
    int (*operation[5])(int, int);
};

/* ========== TYPE_LANG_STRUCT definitions ========== */
/* Simulating language-specific structure */
struct LangSpecificBase GTY(()) {
    int lang_tag;
    void *lang_data;
};

/* ========== Complex nested type with all categories ========== */
struct UltimateType GTY(()) {
    /* TYPE_SCALAR */
    int counter;
    
    /* TYPE_STRING */
    const char *name;
    
    /* TYPE_STRUCT */
    struct InnerStruct inner;
    
    /* TYPE_UNION */
    union ComplexUnion variant;
    
    /* TYPE_POINTER */
    struct UltimateType *self;
    void **generic_pointer;
    
    /* TYPE_ARRAY */
    int scores[10];
    struct InnerStruct items[5];
    
    /* TYPE_CALLBACK */
    int (*processor)(struct UltimateType *);
    
    /* Nested anonymous struct/union */
    union {
        int as_int;
        struct {
            char a;
            char b;
        } as_chars;
    } anonymous_union;
    
    /* Bitfields (scalar special case) */
    unsigned int flags : 4;
    unsigned int status : 2;
    
    /* Zero-length array (GCC extension) */
    char extra_data[0];
};

/* ========== Complete forward declarations ========== */
struct ForwardDecl GTY(()) {
    int data;
    struct ForwardDecl *next;
    struct PointerFest *link_back;
};

union ForwardUnion GTY(()) {
    int x;
    struct ForwardDecl *fd;
};

/* ========== TYPE_UNDEFINED simulation ========== */
/* Incomplete type that might be processed */
struct IncompleteType;
extern struct IncompleteType *external_incomplete;

/* ========== Function to prevent optimization ========== */
/* Use noinline attribute to ensure function isn't optimized away */
__attribute__((noinline)) 
static size_t compute_checksum(void *ptr1, void *ptr2, void *ptr3) {
    /* Simple operation to use the pointers */
    return ((size_t)ptr1 ^ (size_t)ptr2) + (size_t)ptr3;
}

/* ========== Main function ========== */
int main(void) {
    /* Declare instances of all complex types */
    volatile struct ScalarTypes scalars = {0};
    volatile struct StringTypes strings = {0};
    volatile struct OuterStruct outer = {0};
    volatile struct UserDefined user = {0};
    volatile union ComplexUnion cunion = {0};
    volatile struct PointerFest pointers = {0};
    volatile struct ArrayTypes arrays = {0};
    volatile struct CallbackContainer callbacks = {0};
    volatile struct LangSpecificBase lang_struct = {0};
    volatile struct UltimateType ultimate = {0};
    volatile struct ForwardDecl forward = {0};
    volatile union ForwardUnion funion = {0};
    
    /* Take addresses to ensure types are referenced */
    void *addresses[] = {
        &scalars, &strings, &outer, &user, &cunion,
        &pointers, &arrays, &callbacks, &lang_struct,
        &ultimate, &forward, &funion
    };
    
    /* Compute sizeof all types to ensure they're considered */
    size_t sizes[] = {
        sizeof(struct ScalarTypes),
        sizeof(struct StringTypes),
        sizeof(struct OuterStruct),
        sizeof(struct UserDefined),
        sizeof(union ComplexUnion),
        sizeof(struct PointerFest),
        offsetof(struct ArrayTypes, flexible_array), /* Don't include flexible array */
        sizeof(struct CallbackContainer),
        sizeof(struct LangSpecificBase),
        sizeof(struct UltimateType),
        sizeof(struct ForwardDecl),
        sizeof(union ForwardUnion),
        sizeof(int*),
        sizeof(int(*)(int, char)),
        sizeof(int[10]),
        sizeof(struct InnerStruct[5])
    };
    
    /* Access members to create type dependencies */
    scalars.integer = 42;
    strings.constant_string = "Hello, gengtype!";
    outer.nested.inner_data = 100;
    user.anonymous_struct.anonymous_member = 200;
    cunion.as_int = 300;
    pointers.int_ptr = &scalars.integer;
    arrays.fixed_array[0] = 999;
    callbacks.compare_func = NULL;
    lang_struct.lang_tag = 1;
    ultimate.counter = 1234;
    ultimate.name = "Ultimate";
    forward.data = 555;
    funion.x = 777;
    
    /* Create pointer cycles */
    ultimate.self = (struct UltimateType*)&ultimate;
    pointers.next = (struct PointerFest*)&pointers;
    forward.next = &forward;
    forward.link_back = (struct PointerFest*)&pointers;
    
    /* Compute a checksum to prevent dead code elimination */
    size_t checksum = 0;
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        checksum = compute_checksum((void*)checksum, addresses[i], (void*)i);
    }
    
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        checksum ^= sizes[i];
    }
    
    /* Use volatile to ensure computations aren't optimized away */
    volatile size_t final_checksum = checksum;
    
    /* Print something to ensure execution */
    printf("Type analysis test complete. Checksum: %zu\n", (size_t)final_checksum);
    printf("Total types defined: %zu\n", sizeof(addresses)/sizeof(addresses[0]));
    
    return 0;
}
