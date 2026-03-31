/* test_gengtype_types.c
 * This test triggers GCC's gengtype type counting logic by defining
 * a variety of GC-tracked types with different attributes and structures.
 */

/* Phase 1: Basic scalar and string types */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef char* __attribute__((user("GC"))) gc_string_t;

/* Enum type (treated as scalar) */
typedef enum __attribute__((user("GC"))) {
    GC_VAL_A,
    GC_VAL_B,
    GC_VAL_C
} gc_enum_t;

/* Phase 2: Struct types with GC attributes */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_string_t name;
    void* __attribute__((user("GC"))) opaque_data;
};

/* Nested struct definition */
struct __attribute__((user("GC"))) gc_complex_struct {
    struct gc_base_struct base;
    gc_float_t values[5];
    struct gc_complex_struct* __attribute__((user("GC"))) next;
};

/* User struct with typedef */
typedef struct __attribute__((user("GC"))) {
    int x, y;
    char* __attribute__((user("GC"))) label;
} gc_user_struct_t;

/* Phase 3: Union types */
union __attribute__((user("GC"))) gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct* __attribute__((user("GC"))) as_struct;
};

/* Phase 4: Pointer types with GC attributes */
typedef gc_user_struct_t* __attribute__((user("GC"))) gc_user_ptr_t;
typedef union gc_data_union* __attribute__((user("GC"))) gc_union_ptr_t;

/* Phase 5: Array types */
typedef gc_int_t __attribute__((user("GC"))) gc_int_array_t[10];
typedef struct gc_base_struct __attribute__((user("GC"))) gc_struct_array_t[5];

/* Phase 6: Callback (function pointer) type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, void*);

/* Phase 7: Complex nested type with multiple attributes */
struct __attribute__((user("GC"), packed, aligned(8))) gc_packed_struct {
    gc_enum_t type;
    union {
        gc_int_t int_val;
        gc_float_t float_val;
        gc_string_t str_val;
    } data;
    gc_callback_t callback;
    gc_int_array_t scores;
};

/* Phase 8: Create type aliases to force additional processing */
typedef struct gc_packed_struct __attribute__((user("GC"))) gc_packed_alias_t 
    __attribute__((alias("gc_packed_struct")));

/* Weak reference to create another type entry */
extern struct gc_complex_struct __attribute__((user("GC"), weak)) weak_gc_struct;

/* Global variables to force type instantiation */
struct gc_base_struct __attribute__((user("GC"), used, retain)) global_base = {0};
gc_user_struct_t __attribute__((user("GC"), used, retain)) global_user = {0};
union gc_data_union __attribute__((user("GC"), used, retain)) global_union = {0};
gc_int_array_t __attribute__((user("GC"), used, retain)) global_array = {0};

/* Function using __builtin_clear_padding to require type layout */
void clear_struct_padding(struct gc_packed_struct* s) {
    __builtin_clear_padding(s);
}

/* Main function that references all types to prevent elimination */
int main() {
    /* Reference scalar types */
    gc_int_t x = 42;
    gc_float_t y = 3.14f;
    gc_enum_t e = GC_VAL_B;
    
    /* Reference string type */
    gc_string_t str = "test";
    
    /* Reference struct types */
    struct gc_complex_struct complex = {0};
    gc_user_struct_t user = {0};
    
    /* Reference union type */
    union gc_data_union data_union;
    data_union.as_int = x;
    
    /* Reference pointer types */
    gc_user_ptr_t user_ptr = &user;
    gc_union_ptr_t union_ptr = &data_union;
    
    /* Reference array type */
    gc_int_array_t local_array = {0};
    
    /* Reference callback type */
    gc_callback_t callback = 0;
    
    /* Reference packed struct */
    struct gc_packed_struct packed = {0};
    clear_struct_padding(&packed);
    
    /* Calculate checksum using addresses/sizes */
    unsigned long checksum = 0;
    checksum += (unsigned long)&x;
    checksum += (unsigned long)&y;
    checksum += (unsigned long)&complex;
    checksum += (unsigned long)&user;
    checksum += (unsigned long)&data_union;
    checksum += sizeof(struct gc_packed_struct);
    checksum += sizeof(gc_int_array_t);
    
    /* Use the checksum to produce observable output */
    printf("Checksum: %lu\n", checksum);
    
    return (int)(checksum % 256);
}
