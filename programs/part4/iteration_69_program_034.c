/* test_gengtype_types.c
 * This test aims to trigger GCC's internal gengtype type counting logic
 * by defining various GC-tracked types with different attributes.
 */

/* First, let's create some basic type definitions with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;

/* Enum type (scalar) */
enum gc_color __attribute__((user("GC"))) {
    RED,
    GREEN,
    BLUE
};

/* String type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* Callback type (function pointer) */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* Simple struct */
struct __attribute__((user("GC"))) gc_point {
    gc_int_t x;
    gc_int_t y;
    gc_float_t z;
};

/* Nested struct */
struct __attribute__((user("GC"))) gc_rectangle {
    struct gc_point top_left;
    struct gc_point bottom_right;
    gc_string_t name;
};

/* Union type */
union __attribute__((user("GC"))) gc_data {
    gc_int_t as_int;
    gc_float_t as_float;
    char* as_string;
    struct gc_point as_point;
};

/* Array type */
typedef gc_int_t __attribute__((user("GC"))) gc_array_t[10];

/* Pointer type to struct */
typedef struct gc_point* __attribute__((user("GC"))) gc_point_ptr_t;

/* More complex: struct containing arrays and pointers */
struct __attribute__((user("GC"))) gc_complex {
    gc_array_t numbers;
    gc_point_ptr_t points[5];
    gc_callback_t callback;
    union gc_data data;
};

/* Create type aliases to force additional type processing */
typedef struct gc_point __attribute__((alias("gc_point"))) gc_point_alias_t;
typedef union gc_data __attribute__((weak, user("GC"))) gc_data_weak_t;

/* Language-specific struct (simulating TYPE_LANG_STRUCT) */
/* This might be recognized differently by GCC's internal passes */
struct __attribute__((user("GC"))) gc_lang_struct {
    void* tree_node;  /* Simulating GCC tree node pointer */
    int lang_specific;
};

/* Now create global variables with these types to ensure they're processed */
/* Use 'used' and 'retain' attributes to prevent optimization */
struct gc_point __attribute__((used, retain)) global_point = {1, 2, 3.0};
union gc_data __attribute__((used, retain)) global_data;
struct gc_complex __attribute__((used, retain)) global_complex;
struct gc_lang_struct __attribute__((used, retain)) global_lang_struct;

/* Function using __builtin_clear_padding which requires type layout info */
void clear_struct_padding(void) {
    __builtin_clear_padding(&global_point);
    __builtin_clear_padding(&global_complex);
}

/* Main function that references our types */
int main() {
    /* Reference all types to prevent dead code elimination */
    gc_int_t local_int = 42;
    gc_string_t local_string = "test";
    gc_callback_t local_callback = 0;
    gc_array_t local_array = {0};
    gc_point_ptr_t local_ptr = &global_point;
    
    /* Calculate checksum using addresses and sizes */
    unsigned long checksum = 0;
    
    checksum += (unsigned long)&global_point;
    checksum += (unsigned long)&global_data;
    checksum += (unsigned long)&global_complex;
    checksum += sizeof(struct gc_point);
    checksum += sizeof(union gc_data);
    checksum += sizeof(struct gc_complex);
    checksum += sizeof(gc_array_t);
    
    /* Call function that uses type-layout builtins */
    clear_struct_padding();
    
    /* Print something to prevent optimization */
    printf("Checksum: %lu\n", checksum);
    printf("Point size: %zu\n", sizeof(struct gc_point));
    printf("Complex size: %zu\n", sizeof(struct gc_complex));
    
    return 0;
}
