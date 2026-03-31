/* gengtype_test.c - Comprehensive type coverage for gengtype.cc */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

/* ========== TYPE DEFINITIONS ========== */

/* 1. Scalar types (TYPE_SCALAR) */
volatile int global_int = 42;
static float global_float = 3.14f;
double global_double = 2.71828;
char global_char = 'A';
long long global_llong = 0xDEADBEEF;
_Bool global_bool = 1;

/* 2. String type (TYPE_STRING) */
char *global_string = "Hello, gengtype!";
const char *const_string = "Constant string";

/* 3. Struct types (TYPE_STRUCT) */
struct AnonymousStruct {
    int x;
    float y;
    char z;
};

/* 4. User struct types (TYPE_USER_STRUCT) */
typedef struct {
    int id;
    double value;
    char name[32];
} UserStruct;

typedef struct ComplexStruct {
    int tag;
    union {
        int int_val;
        float float_val;
        void *ptr_val;
    } data;
    struct ComplexStruct *next;
} ComplexStruct;

/* 5. Union types (TYPE_UNION) */
union AnonymousUnion {
    int as_int;
    float as_float;
    void *as_ptr;
};

typedef union {
    uint64_t as_uint64;
    double as_double;
    struct {
        uint32_t low;
        uint32_t high;
    } as_parts;
} TypedefUnion;

/* 6. Pointer types (TYPE_POINTER) */
int *int_ptr = &global_int;
float **float_ptr_ptr = &(&global_float);
char ***triple_char_ptr = NULL;
void *void_ptr = NULL;
struct AnonymousStruct *struct_ptr = NULL;
UserStruct **user_struct_ptr_ptr = NULL;

/* 7. Array types (TYPE_ARRAY) */
int int_array[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
float float_array[5][3];
struct AnonymousStruct struct_array[4];
UserStruct *pointer_array[8];
int (*function_ptr_array[3])(void);

/* 8. Callback types (TYPE_CALLBACK) */
typedef int (*Comparator)(const void *, const void *);
typedef void (*VoidCallback)(int, char *);

int compare_ints(const void *a, const void *b) {
    return *(int*)a - *(int*)b;
}

void sample_callback(int x, char *msg) {
    printf("Callback: %d, %s\n", x, msg);
}

/* Function pointer variables */
Comparator global_comparator = compare_ints;
VoidCallback global_void_callback = sample_callback;
void (*direct_fp)(int) = NULL;

/* 9. Language-specific structs (TYPE_LANG_STRUCT) */
/* GCC extensions for language-specific types */
struct __attribute__((packed)) PackedStruct {
    char a;
    int b;
    char c;
};

struct __attribute__((transparent_union)) TransparentUnion {
    int int_member;
    float float_member;
};

/* 10. Undefined/forward declarations (TYPE_UNDEFINED) */
struct ForwardDeclared;  /* Incomplete/undefined type */

/* ========== TYPE USAGE FUNCTIONS ========== */

/* Force usage of all types to prevent optimization */
__attribute__((noinline)) 
static void use_scalars(int seed) {
    global_int += seed;
    global_float *= 1.1f;
    global_double /= 2.0;
    global_char ^= seed;
    global_llong <<= 1;
    global_bool = !global_bool;
}

__attribute__((noinline))
static void use_strings(void) {
    char local_copy[64];
    for (int i = 0; global_string[i] && i < 63; i++) {
        local_copy[i] = global_string[i] ^ 0x55;
    }
    printf("String: %s\n", const_string);
}

__attribute__((noinline))
static struct AnonymousStruct use_structs(int x) {
    struct AnonymousStruct s;
    s.x = x;
    s.y = x * 1.5f;
    s.z = x & 0xFF;
    return s;
}

__attribute__((noinline))
static UserStruct *use_user_structs(void) {
    static UserStruct us = {0};
    us.id++;
    us.value += 0.5;
    us.name[0] = 'A' + (us.id % 26);
    return &us;
}

__attribute__((noinline))
static void use_unions(int mode) {
    union AnonymousUnion au;
    TypedefUnion tu;
    
    if (mode == 0) {
        au.as_int = 42;
        tu.as_uint64 = 0x123456789ABCDEF0ULL;
    } else {
        au.as_float = 3.14f;
        tu.as_double = 2.71828;
    }
    
    /* Prevent unused variable warnings */
    (void)au;
    (void)tu;
}

__attribute__((noinline))
static void use_pointers(void) {
    *int_ptr = 100;
    if (float_ptr_ptr && *float_ptr_ptr) {
        **float_ptr_ptr = 99.9f;
    }
    
    /* Chain pointer operations */
    void_ptr = (void*)&global_char;
    struct_ptr = (struct AnonymousStruct*)void_ptr;
}

__attribute__((noinline))
static int use_arrays(void) {
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += int_array[i];
        int_array[i] = sum;
    }
    
    for (int i = 0; i < 4; i++) {
        struct_array[i] = use_structs(i);
    }
    
    return sum;
}

__attribute__((noinline))
static void use_callbacks(void) {
    int a = 5, b = 10;
    if (global_comparator) {
        int result = global_comparator(&a, &b);
        printf("Comparison result: %d\n", result);
    }
    
    if (global_void_callback) {
        global_void_callback(42, "test");
    }
}

__attribute__((noinline))
static void use_lang_structs(void) {
    struct PackedStruct ps = {'X', 1234, 'Y'};
    struct TransparentUnion tu = {.int_member = 999};
    
    /* Access all bytes to ensure usage */
    unsigned char *bytes = (unsigned char*)&ps;
    for (size_t i = 0; i < sizeof(ps); i++) {
        bytes[i] ^= 0xAA;
    }
    
    (void)tu;
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    int seed = getpid();  /* Non-deterministic seed */
    
    /* Use all type categories */
    use_scalars(seed);
    use_strings();
    
    struct AnonymousStruct s = use_structs(seed);
    (void)s;
    
    UserStruct *us = use_user_structs();
    (void)us;
    
    use_unions(seed & 1);
    use_pointers();
    
    int array_sum = use_arrays();
    printf("Array sum: %d\n", array_sum);
    
    use_callbacks();
    use_lang_structs();
    
    /* Create and use a complex nested type */
    ComplexStruct cs = {
        .tag = 1,
        .data = {.int_val = 42},
        .next = NULL
    };
    
    /* Multi-dimensional pointer array access */
    for (int i = 0; i < 8; i++) {
        pointer_array[i] = us;
    }
    
    /* Function pointer in array */
    function_ptr_array[0] = (int (*)(void))getpid;
    
    /* Final checksum to use everything */
    unsigned long checksum = 0;
    checksum += global_int;
    checksum += (unsigned long)global_string[0];
    checksum += array_sum;
    checksum += (unsigned long)global_comparator;
    
    printf("Final checksum: %lu\n", checksum);
    
    return 0;
}
