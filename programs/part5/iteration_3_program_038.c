/* test_gengtype_coverage.c
 * Complex type definitions to exercise gengtype type enumeration logic
 */

/* Simulate GTY markers for compilation */
#define GTY(x)

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/* Prevent optimization */
#define NOINLINE __attribute__((noinline))
#define USED __attribute__((used))

/* TYPE_SCALAR triggers */
typedef int scalar_int;
typedef char scalar_char;
typedef float scalar_float;
typedef double scalar_double;

/* TYPE_STRING triggers */
typedef const char* string_type;

/* TYPE_CALLBACK triggers (function pointers) */
typedef int (*callback_func)(int, void*);
typedef void (*simple_callback)(void);

/* Basic struct for TYPE_STRUCT */
struct GTY(()) basic_struct {
    scalar_int a;
    scalar_char b;
    scalar_float c;
    scalar_double d;
};

/* Union for TYPE_UNION */
union GTY(()) basic_union {
    scalar_int as_int;
    scalar_float as_float;
    struct basic_struct* as_struct_ptr;
};

/* Nested struct with all type kinds */
struct GTY(()) complex_nested {
    /* TYPE_SCALAR */
    int count;
    float ratio;
    double precision;
    
    /* TYPE_POINTER */
    struct complex_nested* GTY((skip)) next;
    void* GTY((skip)) user_data;
    
    /* TYPE_ARRAY */
    int GTY((skip)) fixed_array[10];
    char GTY((skip)) name[32];
    
    /* TYPE_STRING */
    const char* GTY((skip)) description;
    
    /* Nested TYPE_STRUCT */
    struct basic_struct GTY((skip)) embedded;
    
    /* Nested TYPE_UNION */
    union basic_union GTY((skip)) choice;
    
    /* TYPE_ARRAY of pointers */
    struct basic_struct* GTY((skip)) struct_array[5];
    
    /* TYPE_ARRAY of arrays */
    int GTY((skip)) matrix[3][3];
    
    /* TYPE_CALLBACK */
    callback_func GTY((skip)) handler;
    
    /* Flexible array member (special array case) */
    int GTY((skip)) flexible_array[];
};

/* TYPE_USER_STRUCT simulation */
typedef struct GTY(()) user_defined {
    int id;
    struct complex_nested* GTY((skip)) data;
    union basic_union GTY((skip)) value;
} user_struct_t;

/* Another layer of nesting with pointers to different types */
struct GTY(()) container {
    /* Array of different pointer types */
    void* GTY((skip)) void_ptrs[4];
    
    /* Pointer to array */
    int (*GTY((skip)) array_ptr)[10];
    
    /* Function pointer array */
    simple_callback GTY((skip)) callbacks[3];
    
    /* Nested anonymous struct */
    struct {
        int x;
        int y;
    } GTY((skip)) point;
    
    /* Nested anonymous union */
    union {
        long as_long;
        double as_double;
    } GTY((skip)) numeric;
    
    /* Reference to user struct */
    user_struct_t* GTY((skip)) user;
};

/* Union containing various types */
union GTY(()) mega_union {
    struct basic_struct as_struct;
    struct complex_nested* as_complex_ptr;
    user_struct_t as_user;
    int as_int_array[20];
    callback_func as_callback;
};

/* Array-specific types */
typedef int GTY((skip)) int_matrix[4][4];
typedef struct basic_struct GTY((skip)) struct_array_t[8];

/* External function to prevent optimization */
NOINLINE void* use_types(
    struct basic_struct* bs,
    union basic_union* bu,
    struct complex_nested* cn,
    user_struct_t* us,
    struct container* c,
    union mega_union* mu,
    int_matrix* im,
    struct_array_t* sa
) {
    volatile void* result = NULL;
    
    /* Force consideration of all types */
    result = (void*)&bs->a;
    result = (void*)&bu->as_int;
    result = (void*)&cn->count;
    result = (void*)&us->id;
    result = (void*)&c->point.x;
    result = (void*)&mu->as_struct.a;
    result = (void*)(*im)[0];
    result = (void*)&(*sa)[0];
    
    return result;
}

/* Main function that references all types */
int main() {
    /* Declare instances of all complex types */
    struct basic_struct bs = {1, 'A', 3.14f, 2.71828};
    union basic_union bu;
    bu.as_int = 42;
    
    struct complex_nested* cn = NULL;
    user_struct_t us = {1001, NULL, {.as_int = 0}};
    
    struct container c = {
        .void_ptrs = {&bs, &bu, &us, NULL},
        .array_ptr = NULL,
        .callbacks = {NULL, NULL, NULL},
        .point = {10, 20},
        .numeric = {.as_long = 100},
        .user = &us
    };
    
    union mega_union mu;
    mu.as_struct = bs;
    
    int_matrix im = {{0}};
    struct_array_t sa = {{0}};
    
    /* Calculate sizeof all types */
    size_t total_size = 0;
    
    total_size += sizeof(struct basic_struct);
    total_size += sizeof(union basic_union);
    total_size += sizeof(struct complex_nested);
    total_size += sizeof(user_struct_t);
    total_size += sizeof(struct container);
    total_size += sizeof(union mega_union);
    total_size += sizeof(int_matrix);
    total_size += sizeof(struct_array_t);
    
    /* Take addresses of everything */
    void* addresses[] = {
        &bs, &bu, &cn, &us, &c, &mu, &im, &sa,
        &bs.a, &bu.as_int, &us.id, &c.point,
        &mu.as_struct, im[0], sa[0].a
    };
    
    /* Use the types to prevent optimization */
    void* result = use_types(&bs, &bu, cn, &us, &c, &mu, &im, &sa);
    
    /* Print checksum */
    printf("Type coverage test:\n");
    printf("Total size of all types: %zu bytes\n", total_size);
    printf("Number of type references: %zu\n", sizeof(addresses)/sizeof(addresses[0]));
    printf("Result pointer: %p\n", result);
    
    /* Reference string type */
    const char* test_string = "This is a TYPE_STRING";
    printf("String test: %s\n", test_string);
    
    /* Reference callback type */
    callback_func func_ptr = NULL;
    printf("Callback function pointer size: %zu\n", sizeof(func_ptr));
    
    return 0;
}
