/* test_gengtype_coverage.c - Comprehensive type declarations for gengtype coverage */

#include <stddef.h>

/* TYPE_STRUCT: Plain struct with multiple members */
struct plain_struct {
    int a;
    float b;
    double c;
    char d;
};

/* TYPE_USER_STRUCT: Typedef creating a user-defined struct type */
typedef struct {
    long x;
    short y;
} user_struct_t;

/* TYPE_UNION: Union with several members */
union data_union {
    int i;
    float f;
    char str[20];
    void *ptr;
};

/* TYPE_POINTER: Various pointer types */
int *int_ptr;
struct plain_struct *struct_ptr;
void *void_ptr;
volatile int *volatile_ptr;
const char *const_str_ptr;
volatile int *const volatile_const_ptr;

/* TYPE_ARRAY: Arrays of different dimensions */
int int_array[10];
char char_array[5][5];
float float_3d_array[3][3][3];
struct plain_struct struct_array[5];

/* TYPE_SCALAR: Scalar types */
int scalar_int;
float scalar_float;
double scalar_double;

/* Enumeration type (also scalar) */
enum color {
    RED,
    GREEN,
    BLUE
} color_enum;

/* TYPE_STRING: String literal in initializer context */
const char *greeting = "Hello, gengtype!";
char message[] = "Test string";

/* TYPE_CALLBACK: Function pointer type */
typedef int (*callback_t)(int, char*);
typedef void (*simple_callback)(void);

/* Complex callback returning pointer to struct */
typedef struct plain_struct* (*struct_callback)(int);

/* TYPE_LANG_STRUCT: GCC-specific constructs */
#ifdef __GNUC__
/* Packed struct with attributes */
struct __attribute__((packed, aligned(2))) packed_struct {
    char a;
    int b;
    short c;
} __attribute__((aligned(16)));

/* Transparent union */
union __attribute__((transparent_union)) transparent_union_t {
    int *int_ptr;
    void *void_ptr;
};

/* Struct with vector attribute */
struct vector_struct {
    int data[4] __attribute__((vector_size(16)));
};
#endif

/* Complex nesting and chains */

/* Struct containing pointer to another struct */
struct container {
    struct plain_struct *inner;
    int count;
    callback_t callback;
};

/* Struct containing array of unions */
struct union_container {
    union data_union items[10];
    int size;
};

/* Typedef for function pointer returning pointer to struct */
typedef struct container* (*container_factory)(int);

/* More complex nested type */
struct complex_nested {
    struct container *containers[5];
    union data_union (*get_union)(int);
    int (*processor)(callback_t, int*);
};

/* Global variables using these types */
struct plain_struct global_struct = {1, 2.0f, 3.0, 'A'};
user_struct_t global_user_struct = {100, 200};
union data_union global_union = {.i = 42};
struct container global_container = {&global_struct, 1, NULL};
struct union_container global_union_container;

#ifdef __GNUC__
struct packed_struct global_packed = {'X', 999, 777};
#endif

/* Function pointer variables */
callback_t global_callback = NULL;
container_factory global_factory = NULL;

/* Array of function pointers */
simple_callback callbacks[3];

/* Volatile and const qualified variables */
volatile int volatile_counter = 0;
const int const_max_value = 100;
volatile int* const volatile_const_ptr2 = &volatile_counter;

/* Using #pragma pack */
#pragma pack(push, 1)
struct pragma_packed {
    char a;
    int b;
    char c;
};
#pragma pack(pop)

struct pragma_packed packed_with_pragma = {'Z', 123, 'Y'};

/* Function definitions that use the types */

/* Function that takes a callback */
int execute_callback(callback_t cb, int value) {
    if (cb) {
        return cb(value, "test");
    }
    return -1;
}

/* Function returning pointer to struct */
struct plain_struct* get_struct_ptr(int id) {
    static struct plain_struct local_struct = {id, id * 1.0f, id * 2.0, 'S'};
    return &local_struct;
}

/* Function using transparent union (GCC-specific) */
#ifdef __GNUC__
void use_transparent_union(union transparent_union_t u) {
    /* Access through void pointer */
    void *ptr = u.void_ptr;
    (void)ptr;
}
#endif

/* Main function that uses all types to prevent optimization */
int main(void) {
    /* Use struct */
    global_struct.a = 10;
    global_struct.b = 20.5f;
    
    /* Use user struct */
    global_user_struct.x = 1000;
    global_user_struct.y = 500;
    
    /* Use union */
    global_union.f = 3.14159f;
    
    /* Use pointers */
    int_ptr = &global_struct.a;
    struct_ptr = &global_struct;
    void_ptr = (void*)&global_user_struct;
    
    /* Use arrays */
    int_array[0] = 1;
    char_array[1][2] = 'X';
    float_3d_array[0][1][2] = 1.5f;
    struct_array[0] = global_struct;
    
    /* Use scalars */
    scalar_int = 42;
    scalar_float = 3.14f;
    color_enum = GREEN;
    
    /* Use string */
    message[0] = 'M';
    
    /* Use callbacks */
    callbacks[0] = NULL;
    
    /* Use nested types */
    global_container.inner = &global_struct;
    global_container.count = 5;
    
    /* Use union container */
    global_union_container.items[0].i = 99;
    global_union_container.size = 1;
    
    /* Use volatile/const */
    volatile_counter++;
    int temp = const_max_value;
    
    /* Use packed structs */
#ifdef __GNUC__
    global_packed.a = 'Y';
    global_packed.b = 888;
#endif
    
    packed_with_pragma.a = 'A';
    packed_with_pragma.b = 456;
    
    /* Call functions */
    struct plain_struct *ret_struct = get_struct_ptr(7);
    (void)ret_struct;
    
    int result = execute_callback(NULL, 5);
    (void)result;
    
#ifdef __GNUC__
    /* Use transparent union */
    union transparent_union_t tu;
    tu.int_ptr = &scalar_int;
    use_transparent_union(tu);
#endif
    
    /* Prevent dead code elimination */
    volatile int keep_alive = 0;
    keep_alive += global_struct.a;
    keep_alive += global_user_struct.x;
    keep_alive += global_union.i;
    keep_alive += *int_ptr;
    
    return keep_alive == 0 ? 0 : 0;  /* Always return 0 */
}

/* Additional type in different linkage to test cross-file scenarios */
static struct static_only_struct {
    int hidden;
    double secret;
} static_var = {42, 3.14};

/* External declaration to simulate multi-file scenario */
extern int external_function(void);

/* Inline function using complex types */
static inline void process_complex(struct complex_nested *cn) {
    if (cn && cn->containers[0]) {
        cn->containers[0]->count++;
    }
}
