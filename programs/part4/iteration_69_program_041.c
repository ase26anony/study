/* test_gengtype_types.c
 * This program defines various GC-tracked types to trigger type counting
 * in GCC's gengtype machinery during compilation.
 */

/* Phase 1: Basic scalar and string types */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef char* __attribute__((user("GC"))) gc_string_t;

/* Enum type (treated as scalar) */
typedef enum __attribute__((user("GC"))) {
    GC_ENUM_A,
    GC_ENUM_B,
    GC_ENUM_C
} gc_enum_t;

/* Phase 2: Struct types with various members */
struct __attribute__((user("GC"))) gc_basic_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
};

/* Nested struct */
struct __attribute__((user("GC"))) gc_nested_struct {
    struct gc_basic_struct inner;
    gc_int_t count;
    struct gc_nested_struct* __attribute__((user("GC"))) next;
};

/* Phase 3: Union types */
union __attribute__((user("GC"))) gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_basic_struct as_struct;
};

/* Phase 4: User-defined struct (TYPE_USER_STRUCT) */
/* Using typedef to create user struct type */
typedef struct gc_basic_struct __attribute__((user("GC"))) gc_user_struct_t;

/* Phase 5: Pointer types */
typedef struct gc_basic_struct* __attribute__((user("GC"))) gc_struct_ptr_t;
typedef union gc_data_union* __attribute__((user("GC"))) gc_union_ptr_t;
typedef void* __attribute__((user("GC"))) gc_void_ptr_t;

/* Phase 6: Array types */
typedef gc_int_t __attribute__((user("GC"))) gc_int_array_t[10];
typedef struct gc_basic_struct __attribute__((user("GC"))) gc_struct_array_t[5];

/* Phase 7: Callback/function pointer types */
typedef void (* __attribute__((user("GC"))) gc_callback_t)(int, gc_string_t);
typedef gc_int_t (* __attribute__((user("GC"))) gc_func_ptr_t)(struct gc_basic_struct*);

/* Phase 8: Complex nested type for thorough testing */
struct __attribute__((user("GC"))) gc_complex_type {
    gc_int_array_t numbers;
    gc_struct_ptr_t children[3];
    union gc_data_union data;
    gc_callback_t handler;
    struct {
        gc_int_t x;
        gc_int_t y;
    } __attribute__((user("GC"))) point;
};

/* Phase 9: Create type aliases to force additional processing */
typedef struct gc_complex_type __attribute__((alias("gc_complex_type"))) gc_alias_t;
typedef gc_user_struct_t __attribute__((weak)) gc_weak_struct_t;

/* Phase 10: Global variables to prevent elimination */
struct gc_basic_struct __attribute__((used, retain, user("GC"))) global_gc_struct = {1, 3.14, "test"};
union gc_data_union __attribute__((used, retain, user("GC"))) global_gc_union = {.as_int = 42};
gc_int_array_t __attribute__((used, retain, user("GC"))) global_array = {0};
gc_callback_t __attribute__((used, retain, user("GC"))) global_callback = 0;

/* Phase 11: Force padding analysis */
struct __attribute__((user("GC"))) gc_padded_struct {
    char c;
    gc_int_t i;
    short s;
};

/* Helper function using builtin that requires type info */
void analyze_types(void) {
    struct gc_padded_struct padded = {'a', 100, 200};
    /* This builtin requires precise type layout information */
    __builtin_clear_padding(&padded);
}

/* Phase 12: Language-specific struct simulation */
/* Using transactional memory attribute to simulate lang struct processing */
struct __attribute__((user("GC"), transaction_safe)) gc_transactional_struct {
    gc_int_t counter;
    gc_string_t data;
};

/* Main function - mostly to create valid executable */
int main() {
    /* Reference global variables to prevent elimination */
    volatile void* ptrs[] = {
        &global_gc_struct,
        &global_gc_union,
        &global_array,
        &global_callback
    };
    
    /* Use sizeof on various types */
    size_t sizes[] = {
        sizeof(gc_int_t),
        sizeof(struct gc_basic_struct),
        sizeof(union gc_data_union),
        sizeof(gc_int_array_t),
        sizeof(gc_callback_t),
        sizeof(struct gc_complex_type)
    };
    
    /* Force type analysis */
    analyze_types();
    
    /* Simple checksum for observable output */
    unsigned long checksum = 0;
    for (int i = 0; i < 4; i++) {
        checksum += (unsigned long)ptrs[i];
    }
    for (int i = 0; i < 6; i++) {
        checksum += sizes[i];
    }
    
    /* Print something to prevent optimization */
    printf("Type analysis test - checksum: %lu\n", checksum);
    
    return 0;
}
