/* gengtype_test.c - Test program to trigger gengtype type counting logic */

/* Phase 1: Basic scalar and string types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef char* __attribute__((user("GC"))) gc_string_t;

/* Enum type (treated as scalar) */
typedef enum __attribute__((user("GC"))) {
    GC_STATE_INIT,
    GC_STATE_ACTIVE,
    GC_STATE_FINALIZED
} gc_state_t;

/* Phase 2: Struct types with GC attributes */
struct __attribute__((user("GC"))) gc_base_struct {
    gc_int_t id;
    gc_string_t name;
    gc_state_t state;
};

/* Nested struct with pointer members */
struct __attribute__((user("GC"))) gc_complex_struct {
    struct gc_base_struct base;
    struct gc_complex_struct* __attribute__((user("GC"))) next;
    gc_float_t values[10];
    void (*callback)(int) __attribute__((user("GC")));
};

/* Phase 3: Union types with GC attributes */
union __attribute__((user("GC"))) gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
    struct gc_base_struct as_struct;
};

/* Phase 4: Array types with GC attributes */
typedef gc_int_t __attribute__((user("GC"))) gc_int_array_t[100];
typedef struct gc_base_struct __attribute__((user("GC"))) gc_struct_array_t[50];

/* Phase 5: Pointer types with GC attributes */
typedef struct gc_complex_struct* __attribute__((user("GC"))) gc_complex_ptr_t;
typedef union gc_data_union* __attribute__((user("GC"))) gc_union_ptr_t;

/* Phase 6: Callback/function pointer types with GC attributes */
typedef void (* __attribute__((user("GC"))) gc_callback_t)(struct gc_complex_struct*, gc_int_t);
typedef gc_int_t (* __attribute__((user("GC"))) gc_transform_t)(gc_float_t*, int);

/* Phase 7: Complex nested type with all categories */
struct __attribute__((user("GC"))) gc_master_type {
    /* Scalar */
    gc_int_t counter;
    
    /* String */
    gc_string_t description;
    
    /* Struct */
    struct gc_base_struct base;
    
    /* Union */
    union gc_data_union data;
    
    /* Pointer */
    gc_complex_ptr_t complex_ptr;
    
    /* Array */
    gc_int_array_t numbers;
    
    /* Callback */
    gc_callback_t handler;
    
    /* Nested struct pointer array */
    struct gc_base_struct* __attribute__((user("GC"))) items[20];
    
    /* Union array */
    union gc_data_union __attribute__((user("GC"))) variants[5];
};

/* Phase 8: Create type aliases to force additional type processing */
typedef struct gc_master_type __attribute__((user("GC"))) gc_master_alias_t 
    __attribute__((alias("gc_master_type")));

typedef union gc_data_union __attribute__((user("GC"))) gc_union_alias_t
    __attribute__((weak, alias("gc_data_union")));

/* Phase 9: Global variables with GC attributes to force processing */
struct gc_base_struct __attribute__((user("GC"), used, retain)) global_base = {
    .id = 1,
    .name = "Global Base",
    .state = GC_STATE_ACTIVE
};

struct gc_master_type __attribute__((user("GC"), used, retain)) global_master;

union gc_data_union __attribute__((user("GC"), used, retain)) global_union;

gc_int_array_t __attribute__((user("GC"), used, retain)) global_array;

/* Phase 10: Function using __builtin_clear_padding to force type layout analysis */
void __attribute__((used)) analyze_gc_types(void) {
    /* Force type layout analysis for GC-tracked types */
    __builtin_clear_padding(&global_base);
    __builtin_clear_padding(&global_master);
    __builtin_clear_padding(&global_union);
    
    /* Access all type sizes to ensure they're processed */
    volatile size_t sizes[] = {
        sizeof(gc_int_t),
        sizeof(gc_float_t),
        sizeof(gc_string_t),
        sizeof(gc_state_t),
        sizeof(struct gc_base_struct),
        sizeof(struct gc_complex_struct),
        sizeof(union gc_data_union),
        sizeof(gc_int_array_t),
        sizeof(gc_struct_array_t),
        sizeof(gc_complex_ptr_t),
        sizeof(gc_callback_t),
        sizeof(struct gc_master_type)
    };
    (void)sizes; /* Suppress unused warning */
}

/* Phase 11: Multiple translation unit simulation using inline functions */
static inline void __attribute__((always_inline, used)) 
process_complex_struct(struct gc_complex_struct* s) {
    if (s && s->callback) {
        s->callback(s->base.id);
    }
}

static inline union gc_data_union __attribute__((always_inline, used))
create_data_union(gc_int_t val) {
    union gc_data_union u;
    u.as_int = val;
    __builtin_clear_padding(&u);
    return u;
}

/* Phase 12: Main function that references all GC types */
int main(int argc, char** argv) {
    /* Initialize global master struct */
    global_master.counter = 1000;
    global_master.description = "Master GC Structure";
    global_master.base = global_base;
    global_master.data = create_data_union(42);
    global_master.handler = 0; /* No callback by default */
    
    /* Fill array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
        if (i < 20) {
            global_master.items[i] = &global_base;
        }
        if (i < 5) {
            global_master.variants[i].as_int = i * 10;
        }
    }
    
    /* Force type analysis */
    analyze_gc_types();
    
    /* Calculate checksum using addresses and sizes */
    unsigned long checksum = 0;
    checksum += (unsigned long)&global_base;
    checksum += (unsigned long)&global_master;
    checksum += (unsigned long)&global_union;
    checksum += (unsigned long)global_array;
    
    checksum += sizeof(struct gc_base_struct);
    checksum += sizeof(struct gc_complex_struct);
    checksum += sizeof(union gc_data_union);
    checksum += sizeof(struct gc_master_type);
    
    /* Use the checksum to prevent optimization */
    if (checksum != 0) {
        printf("GC Type Test - Checksum: %lu\n", checksum);
        printf("Type sizes - Base: %zu, Complex: %zu, Master: %zu\n",
               sizeof(struct gc_base_struct),
               sizeof(struct gc_complex_struct),
               sizeof(struct gc_master_type));
    }
    
    return 0;
}
