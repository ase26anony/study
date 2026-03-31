/* test_types.c - Comprehensive type coverage for gengtype */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* ========== TYPE DEFINITIONS ========== */

/* Scalar types (TYPE_SCALAR) */
volatile int global_int = 42;
volatile float global_float = 3.14f;
volatile double global_double = 2.71828;
volatile char global_char = 'A';
volatile long long global_llong = 1234567890123LL;
volatile _Bool global_bool = 1;

/* String type (TYPE_STRING) */
volatile char *global_string = "Hello, World!";
volatile const char *global_const_string = "Constant string";

/* Struct types (TYPE_STRUCT) */
struct AnonymousStruct {
    int x;
    float y;
    char z;
};

struct NestedStruct {
    int id;
    struct AnonymousStruct inner;
    double data;
};

/* User struct types (TYPE_USER_STRUCT) */
typedef struct {
    int counter;
    float value;
    char name[32];
} UserStruct;

typedef struct ComplexStruct {
    UserStruct base;
    struct ComplexStruct *next;
    int array[10];
} ComplexStruct;

/* Union types (TYPE_UNION) */
union AnonymousUnion {
    int as_int;
    float as_float;
    char as_char[4];
};

typedef union {
    long long timestamp;
    double precision;
    void *pointer;
} TypedefUnion;

/* Pointer types (TYPE_POINTER) */
volatile int *int_ptr = &global_int;
volatile float **float_ptr_ptr = NULL;
volatile struct AnonymousStruct *struct_ptr = NULL;
volatile union AnonymousUnion *union_ptr = NULL;
volatile void *void_ptr = NULL;
volatile char **string_array = NULL;

/* Array types (TYPE_ARRAY) */
volatile int int_array[100] = {0};
volatile float float_array[50][20];
volatile struct AnonymousStruct struct_array[10];
volatile UserStruct user_struct_array[5];
volatile int *pointer_array[25];
volatile char char_array[] = "Array of chars";

/* Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*SimpleCallback)(int, float);

void (*global_callback)(int) = NULL;
Comparator global_comparator = NULL;

/* Language-specific struct (TYPE_LANG_STRUCT) */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int int_val;
    float float_val;
};

/* Undefined type reference (TYPE_UNDEFINED) */
struct ForwardDeclared;  /* Forward declaration creates undefined type */

/* ========== FUNCTION DECLARATIONS ========== */

/* Functions using various types */
struct AnonymousStruct __attribute__((noinline)) process_struct(struct AnonymousStruct s) {
    s.x++;
    s.y *= 2.0f;
    s.z = 'B';
    return s;
}

UserStruct __attribute__((noinline)) process_user_struct(UserStruct us) {
    us.counter++;
    us.value += 1.0f;
    us.name[0] = 'X';
    return us;
}

void __attribute__((noinline)) process_union(union AnonymousUnion u) {
    u.as_int = 0xDEADBEEF;
}

void __attribute__((noinline)) process_array(int arr[10]) {
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
}

void __attribute__((noinline)) process_pointer(int **ptr) {
    static int target = 100;
    *ptr = &target;
}

int __attribute__((noinline)) compare_ints(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

void __attribute__((noinline)) simple_callback_handler(int x, float y, SimpleCallback cb) {
    if (cb) cb(x, y);
}

/* Function that uses all types to ensure they're in the type table */
void __attribute__((noinline, used)) use_all_types(void) {
    /* Use scalars */
    volatile int local_int = global_int + 1;
    volatile float local_float = global_float * 2.0f;
    
    /* Use string */
    volatile char first_char = global_string[0];
    
    /* Use structs */
    struct AnonymousStruct as = {1, 2.0f, 'C'};
    as = process_struct(as);
    
    UserStruct us = {0};
    us = process_user_struct(us);
    
    /* Use unions */
    union AnonymousUnion au;
    process_union(au);
    
    TypedefUnion tu;
    tu.timestamp = 1234567890LL;
    
    /* Use pointers */
    volatile int *local_ptr = int_ptr;
    if (local_ptr) local_int = *local_ptr;
    
    /* Use arrays */
    process_array(int_array);
    volatile int first_elem = int_array[0];
    
    /* Use callback */
    global_comparator = compare_ints;
    if (global_comparator) {
        int a = 1, b = 2;
        global_comparator(&a, &b);
    }
    
    /* Use language-specific struct */
    struct PackedStruct ps = {'A', 42, 'B'};
    volatile char ps_a = ps.a;
    
    /* Prevent optimization */
    asm volatile("" : : "r"(local_int), "r"(local_float), "r"(first_char), 
                   "r"(first_elem), "r"(ps_a));
}

/* ========== MAIN FUNCTION ========== */

int main(int argc, char *argv[]) {
    /* Initialize some pointers */
    static int static_int = 999;
    int_ptr = &static_int;
    
    /* Initialize string array */
    static char *strings[] = {"one", "two", "three", NULL};
    string_array = strings;
    
    /* Initialize struct array */
    for (int i = 0; i < 10; i++) {
        struct_array[i].x = i;
        struct_array[i].y = i * 1.5f;
        struct_array[i].z = 'A' + i;
    }
    
    /* Initialize pointer array */
    for (int i = 0; i < 25; i++) {
        pointer_array[i] = &int_array[i % 100];
    }
    
    /* Use all types */
    use_all_types();
    
    /* Create and use a complex nested type */
    ComplexStruct cs = {{0, 1.0f, "test"}, NULL, {0}};
    cs.next = &cs;  /* Self-reference */
    
    /* Use transparent union */
    struct TransparentUnion tu_attr;
    tu_attr.int_val = 42;
    
    /* Reference forward-declared type (TYPE_UNDEFINED) */
    struct ForwardDeclared *fd_ptr = NULL;
    
    /* Opaque computation to prevent optimization */
    volatile unsigned long checksum = 0;
    checksum += global_int;
    checksum += (unsigned long)global_string[0];
    checksum += cs.base.counter;
    checksum += (unsigned long)fd_ptr;
    
    printf("Checksum: %lu\n", checksum);
    printf("Test completed - all type categories should be processed\n");
    
    return 0;
}
