/* test_gengtype_coverage.c
 * 
 * This program defines complex nested data structures to exercise
 * all type enumeration cases in gengtype.cc's switch statement.
 * 
 * When processed by gengtype during GCC build, this should trigger:
 * - TYPE_SCALAR (basic types)
 * - TYPE_STRING (const char*)
 * - TYPE_STRUCT (nested structs)
 * - TYPE_UNION (unions)
 * - TYPE_POINTER (pointers to various types)
 * - TYPE_ARRAY (fixed and flexible arrays)
 * - TYPE_CALLBACK (function pointers)
 * - TYPE_USER_STRUCT (user-defined structs)
 * - TYPE_LANG_STRUCT (language-specific structs)
 */

/* Dummy GTY macro for compilation - in real GCC this would be the actual GTY marker */
#define GTY(x) 

/* For TYPE_LANG_STRUCT simulation - language-specific structure */
struct GTY(()) lang_specific_base {
    int lang_tag;
    void *lang_data;
};

/* Complex union for TYPE_UNION */
union GTY(()) complex_union {
    int int_val;
    double double_val;
    void *ptr_val;
    struct {
        char flag;
        int data;
    } nested;
};

/* Callback type for TYPE_CALLBACK */
typedef int (*callback_func)(int, void*);

/* Nested struct for TYPE_STRUCT */
struct GTY(()) nested_inner {
    int inner_id;
    float inner_data;
    char inner_flags[8];  /* Fixed array */
};

/* Main complex structure containing all type variations */
struct GTY(()) complex_main_struct {
    /* TYPE_SCALAR fields */
    int scalar_int;
    char scalar_char;
    float scalar_float;
    double scalar_double;
    long scalar_long;
    
    /* TYPE_STRING field */
    const char *string_field;
    
    /* TYPE_POINTER fields */
    struct complex_main_struct *self_ptr;
    union complex_union *union_ptr;
    int *int_ptr;
    void *void_ptr;
    
    /* TYPE_ARRAY fields */
    int fixed_array[10];
    struct nested_inner struct_array[5];
    
    /* TYPE_STRUCT field */
    struct nested_inner nested_struct;
    
    /* TYPE_UNION field */
    union complex_union data_union;
    
    /* TYPE_CALLBACK field */
    callback_func callback;
    
    /* Flexible array member (special array case) */
    int flexible_array[];
};

/* Another struct with pointers to everything */
struct GTY(()) pointer_collector {
    /* Pointers to various types */
    struct complex_main_struct *main_ptr;
    union complex_union *union_ptr;
    struct nested_inner *inner_ptr;
    callback_func *callback_ptr;
    
    /* Array of pointers */
    void *ptr_array[20];
    
    /* Pointer to array */
    int (*array_ptr)[10];
    
    /* Function pointer with complex signature */
    struct complex_main_struct* (*factory_func)(int, const char*);
};

/* Union containing structs */
union GTY(()) struct_union {
    struct complex_main_struct main_data;
    struct pointer_collector ptr_data;
    struct {
        int tag;
        union {
            int i;
            double d;
        } value;
    } tagged;
};

/* Container with all types */
struct GTY(()) type_container {
    /* Direct instances */
    struct complex_main_struct main_instance;
    struct pointer_collector ptr_instance;
    union struct_union union_instance;
    
    /* Arrays of different types */
    union complex_union union_array[3];
    callback_func callback_array[4];
    
    /* Pointer to lang struct */
    struct lang_specific_base *lang_ptr;
    
    /* Nested container */
    struct type_container *next;
};

/* External function to prevent optimization */
__attribute__((noinline)) 
size_t compute_checksum(void *data, size_t size) {
    volatile unsigned char *bytes = (unsigned char *)data;
    size_t sum = 0;
    for (size_t i = 0; i < size && i < 64; i++) {
        sum += bytes[i];
    }
    return sum;
}

/* Another external function */
__attribute__((noinline))
void touch_memory(void *ptr) {
    volatile int sink = 0;
    if (ptr) {
        sink = 1;
    }
}

/* Callback function implementation */
int sample_callback(int value, void *context) {
    return value * 2 + (context != NULL);
}

/* Factory function implementation */
struct complex_main_struct* create_complex_struct(int id, const char *name) {
    static struct complex_main_struct instance;
    instance.scalar_int = id;
    instance.string_field = name;
    return &instance;
}

int main(void) {
    /* Declare instances of all complex types */
    struct type_container container;
    struct complex_main_struct main_struct;
    struct pointer_collector ptr_collector;
    union struct_union big_union;
    
    /* Initialize some data */
    main_struct.scalar_int = 42;
    main_struct.scalar_char = 'A';
    main_struct.scalar_float = 3.14f;
    main_struct.string_field = "Test string";
    main_struct.self_ptr = &main_struct;
    main_struct.callback = sample_callback;
    
    /* Fill arrays */
    for (int i = 0; i < 10; i++) {
        main_struct.fixed_array[i] = i * 2;
    }
    
    for (int i = 0; i < 5; i++) {
        main_struct.struct_array[i].inner_id = i;
        main_struct.struct_array[i].inner_data = i * 1.5f;
    }
    
    /* Initialize union */
    main_struct.data_union.int_val = 100;
    
    /* Set up pointer collector */
    ptr_collector.main_ptr = &main_struct;
    ptr_collector.factory_func = create_complex_struct;
    
    /* Initialize container */
    container.main_instance = main_struct;
    container.ptr_instance = ptr_collector;
    container.union_instance.main_data = main_struct;
    container.next = &container;  /* Self-reference */
    
    /* Take addresses of everything to ensure they're used */
    volatile void *addresses[] = {
        &container,
        &main_struct,
        &ptr_collector,
        &big_union,
        &main_struct.nested_struct,
        &main_struct.data_union,
        main_struct.fixed_array,
        main_struct.struct_array,
        &main_struct.callback,
        ptr_collector.ptr_array,
        &container.union_array,
        &container.callback_array
    };
    
    /* Compute sizeof all types to ensure they're considered */
    size_t size_checksum = 0;
    size_checksum += sizeof(struct type_container);
    size_checksum += sizeof(struct complex_main_struct);
    size_checksum += sizeof(struct pointer_collector);
    size_checksum += sizeof(union struct_union);
    size_checksum += sizeof(union complex_union);
    size_checksum += sizeof(struct nested_inner);
    size_checksum += sizeof(callback_func);
    size_checksum += sizeof(struct lang_specific_base);
    
    /* Use callback */
    int callback_result = main_struct.callback(21, &main_struct);
    size_checksum += callback_result;
    
    /* Use factory function */
    struct complex_main_struct *factory_result = 
        ptr_collector.factory_func(99, "Factory created");
    if (factory_result) {
        size_checksum += factory_result->scalar_int;
    }
    
    /* Touch all memory regions to prevent optimization */
    for (size_t i = 0; i < sizeof(addresses)/sizeof(addresses[0]); i++) {
        touch_memory((void*)addresses[i]);
    }
    
    /* Compute checksum of some memory */
    size_checksum += compute_checksum(&container, sizeof(container));
    size_checksum += compute_checksum(&main_struct, sizeof(main_struct));
    
    /* Print result to prevent entire program from being optimized away */
    printf("Type coverage test checksum: %zu\n", size_checksum);
    printf("Structure sizes:\n");
    printf("  type_container: %zu\n", sizeof(struct type_container));
    printf("  complex_main_struct: %zu\n", sizeof(struct complex_main_struct));
    printf("  pointer_collector: %zu\n", sizeof(struct pointer_collector));
    printf("  struct_union: %zu\n", sizeof(union struct_union));
    
    return 0;
}
