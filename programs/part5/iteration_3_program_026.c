/* test_gengtype_coverage.c
 * 
 * This program defines complex data structures to trigger all cases
 * in the type enumeration switch statement in gengtype.cc.
 * 
 * Compile with: gcc -O0 -g -fdump-tree-all -c -ffat-lto-objects test_gengtype_coverage.c
 * Or integrate into GCC build system for full gengtype processing.
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Simulate GTY markers for compilation - in real GCC these would be
 * processed by gengtype */
#define GTY(x) 

/* Forward declarations to create complex type dependencies */
struct forward_declared;
union forward_declared_union;

/* TYPE_SCALAR: Basic scalar types */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING: String types */
typedef const char* string_type;
typedef char* mutable_string;

/* TYPE_CALLBACK: Function pointer types */
typedef void (*callback_func)(int, void*);
typedef int (*comparator_func)(const void*, const void*);

/* TYPE_STRUCT: Simple structure */
struct simple_struct {
    int x;
    char y;
    float z;
};

/* TYPE_UNION: Simple union */
union simple_union {
    int as_int;
    float as_float;
    void* as_ptr;
};

/* TYPE_USER_STRUCT: More complex user-defined structure */
struct user_defined {
    /* TYPE_SCALAR fields */
    int id;
    double weight;
    
    /* TYPE_POINTER fields */
    struct user_defined* next;
    struct forward_declared* fwd_ptr;
    
    /* TYPE_ARRAY fields */
    int scores[10];
    char name[50];
    
    /* TYPE_STRING field */
    const char* description;
    
    /* TYPE_CALLBACK field */
    callback_func handler;
    
    /* Nested TYPE_UNION */
    union {
        int tag;
        void* data;
    } payload;
    
    /* Flexible array member (TYPE_ARRAY) */
    int flexible_array[];
};

/* TYPE_STRUCT with nested structures */
struct outer_struct {
    /* TYPE_SCALAR */
    long counter;
    
    /* TYPE_POINTER to another struct */
    struct inner_struct* inner;
    
    /* TYPE_ARRAY of structs */
    struct simple_struct items[5];
    
    /* TYPE_UNION field */
    union simple_union variant;
    
    /* TYPE_CALLBACK */
    comparator_func compare;
};

/* Another structure for complex relationships */
struct inner_struct {
    /* TYPE_POINTER back to outer */
    struct outer_struct* parent;
    
    /* TYPE_ARRAY of pointers */
    void* ptr_array[8];
    
    /* TYPE_STRING array */
    const char* messages[4];
    
    /* Nested anonymous struct (TYPE_STRUCT) */
    struct {
        int x, y;
    } position;
};

/* TYPE_UNION with complex members */
union complex_union {
    /* TYPE_STRUCT */
    struct {
        int type;
        union simple_union data;
    } tagged;
    
    /* TYPE_ARRAY */
    unsigned char bytes[16];
    
    /* TYPE_POINTER to function */
    void (*func_ptr)(void);
    
    /* TYPE_POINTER to union (self-reference) */
    union complex_union* next;
};

/* Structure with all type kinds */
struct all_types_container {
    /* TYPE_SCALAR */
    uintptr_t scalar_field;
    
    /* TYPE_STRING */
    const char* string_field;
    
    /* TYPE_STRUCT */
    struct simple_struct struct_field;
    
    /* TYPE_UNION */
    union simple_union union_field;
    
    /* TYPE_POINTER */
    struct all_types_container* self_ptr;
    
    /* TYPE_ARRAY */
    double matrix[3][3];
    
    /* TYPE_CALLBACK */
    void (*operation)(struct all_types_container*);
    
    /* Pointer to TYPE_ARRAY */
    int* dynamic_array;
    
    /* Array of TYPE_POINTER */
    void* ptr_list[10];
    
    /* Nested TYPE_STRUCT with TYPE_UNION */
    struct {
        int tag;
        union {
            int num;
            char* str;
        } value;
    } variant;
};

/* Forward declared structure definition */
struct forward_declared {
    int magic;
    struct user_defined* owner;
    struct forward_declared* chain;
};

/* Union using forward declaration */
union forward_declared_union {
    struct forward_declared* fwd;
    int value;
};

/* Global variables to ensure types are used */
GTY(()) struct all_types_container global_container;
GTY(()) union complex_union global_union;
GTY(()) struct user_defined* global_user_struct_list;

/* External function to prevent optimization */
__attribute__((noinline)) 
void use_types(void* ptr1, void* ptr2, size_t size) {
    volatile int sink = 0;
    sink = (ptr1 != ptr2) ? 1 : 0;
    sink += (int)size;
    /* Prevent the call from being optimized away */
    asm volatile("" : : "r"(sink) : "memory");
}

/* Another external function */
__attribute__((noinline))
size_t compute_checksum(void* data, size_t size) {
    volatile size_t result = 0;
    unsigned char* bytes = (unsigned char*)data;
    for (size_t i = 0; i < size && i < 16; i++) {
        result += bytes[i];
    }
    return result;
}

int main(void) {
    /* Declare instances of all complex types */
    struct all_types_container container_instance;
    struct user_defined user_instance = {
        .id = 1,
        .weight = 2.5,
        .next = NULL,
        .fwd_ptr = NULL,
        .description = "Test user struct",
        .handler = NULL,
        .payload.tag = 0
    };
    
    struct outer_struct outer_instance;
    struct inner_struct inner_instance;
    union complex_union union_instance;
    struct forward_declared fwd_instance;
    union forward_declared_union fwd_union_instance;
    
    /* Initialize some values */
    container_instance.scalar_field = 0xDEADBEEF;
    container_instance.string_field = "Coverage test string";
    container_instance.self_ptr = &container_instance;
    
    outer_instance.counter = 1000;
    outer_instance.inner = &inner_instance;
    outer_instance.compare = NULL;
    
    inner_instance.parent = &outer_instance;
    inner_instance.position.x = 10;
    inner_instance.position.y = 20;
    
    union_instance.tagged.type = 1;
    union_instance.next = &union_instance;
    
    fwd_instance.magic = 42;
    fwd_instance.owner = &user_instance;
    
    fwd_union_instance.value = 12345;
    
    /* Take addresses of all instances and members */
    void* addresses[] = {
        &container_instance,
        &container_instance.struct_field,
        &container_instance.union_field,
        &container_instance.matrix,
        &user_instance,
        &user_instance.scores,
        &user_instance.payload,
        &outer_instance,
        &outer_instance.items,
        &outer_instance.variant,
        &inner_instance,
        &inner_instance.ptr_array,
        &union_instance,
        &union_instance.bytes,
        &fwd_instance,
        &fwd_union_instance
    };
    
    /* Compute sizeof for all types */
    size_t sizes[] = {
        sizeof(struct all_types_container),
        sizeof(struct user_defined),
        sizeof(struct outer_struct),
        sizeof(struct inner_struct),
        sizeof(union complex_union),
        sizeof(struct forward_declared),
        sizeof(union forward_declared_union),
        sizeof(struct simple_struct),
        sizeof(union simple_union),
        sizeof(callback_func),
        sizeof(string_type),
        sizeof(scalar_int),
        sizeof(double[3][3]),
        sizeof(void*[10])
    };
    
    /* Use the types to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        use_types(addresses[i], addresses[(i+1) % 16], sizes[i % 14]);
    }
    
    /* Compute and print a checksum based on sizes */
    size_t total_size = 0;
    for (size_t i = 0; i < sizeof(sizes)/sizeof(sizes[0]); i++) {
        total_size += sizes[i];
    }
    
    /* Also compute checksum of some instance data */
    size_t data_checksum = compute_checksum(&container_instance, 
                                          sizeof(container_instance));
    data_checksum += compute_checksum(&user_instance, sizeof(user_instance));
    
    printf("Type coverage test:\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Data checksum: %zu\n", data_checksum);
    printf("Address of global_container: %p\n", (void*)&global_container);
    printf("Address of global_union: %p\n", (void*)&global_union);
    
    /* Reference global pointers */
    if (global_user_struct_list) {
        printf("Global list is not NULL\n");
    }
    
    return 0;
}
