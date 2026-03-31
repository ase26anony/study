/* gengtype_test.c
 * Test program to trigger GCC's internal gengtype type counting logic
 * Compile with: gcc -c -O0 -fdump-tree-gimple -fdump-lang-all gengtype_test.c
 * For best results, also compile with: -fgnu-tm -fprofile-arcs
 */

/* First, define some base types with GC attributes */
typedef int __attribute__((user("GC"))) gc_int_t;
typedef float __attribute__((user("GC"))) gc_float_t;
typedef char __attribute__((user("GC"))) gc_char_t;

/* TYPE_SCALAR: Basic scalar types with GC attributes */
__attribute__((user("GC"))) int global_scalar_int;
__attribute__((user("GC"))) float global_scalar_float;
__attribute__((user("GC"))) enum color { RED, GREEN, BLUE } global_enum;

/* TYPE_STRING: String pointer type */
typedef char* __attribute__((user("GC"))) gc_string_t;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*__attribute__((user("GC"))) gc_callback_t)(int, float);

/* TYPE_STRUCT: Basic structure */
struct __attribute__((user("GC"))) gc_basic_struct {
    gc_int_t id;
    gc_float_t value;
    gc_string_t name;
};

/* TYPE_USER_STRUCT: Another structure with different attributes */
struct __attribute__((user("GC"), packed)) gc_packed_struct {
    gc_char_t byte;
    gc_int_t data;
};

/* TYPE_UNION: Union type */
union __attribute__((user("GC"))) gc_data_union {
    gc_int_t as_int;
    gc_float_t as_float;
    gc_string_t as_string;
};

/* TYPE_POINTER: Pointer typedefs */
typedef struct gc_basic_struct* __attribute__((user("GC"))) gc_struct_ptr_t;
typedef union gc_data_union* __attribute__((user("GC"))) gc_union_ptr_t;

/* TYPE_ARRAY: Array types */
typedef gc_int_t __attribute__((user("GC"))) gc_int_array_t[10];
typedef struct gc_basic_struct __attribute__((user("GC"))) gc_struct_array_t[5];

/* Complex nested type for TYPE_STRUCT */
struct __attribute__((user("GC"))) gc_complex_struct {
    gc_int_t id;
    gc_struct_ptr_t next;
    gc_int_array_t scores;
    gc_callback_t handler;
    union gc_data_union data;
};

/* TYPE_LANG_STRUCT: Simulate language-specific structure */
struct __attribute__((user("GC"), aligned(16))) gc_lang_struct {
    gc_int_t tag;
    void* __attribute__((user("GC"))) language_data;
};

/* Create type aliases to force additional type processing */
typedef struct gc_basic_struct __attribute__((user("GC"), alias("gc_basic_struct"))) gc_basic_struct_alias;
typedef union gc_data_union __attribute__((user("GC"), weak)) gc_data_union_weak;

/* Global variables to prevent optimization and force type instantiation */
__attribute__((used, retain, user("GC"))) 
struct gc_basic_struct global_gc_struct = {1, 3.14, "test"};

__attribute__((used, retain, user("GC")))
union gc_data_union global_gc_union = {.as_int = 42};

__attribute__((used, retain, user("GC")))
gc_int_array_t global_int_array = {1, 2, 3, 4, 5};

__attribute__((used, retain, user("GC")))
gc_callback_t global_callback = 0;

/* Function using __builtin_clear_padding which requires type layout info */
void clear_struct_padding(struct gc_complex_struct* s) {
    __builtin_clear_padding(s);
}

/* Main function that references all types to prevent dead code elimination */
int main() {
    /* Reference scalar types */
    global_scalar_int = 100;
    global_scalar_float = 3.14159;
    global_enum = BLUE;
    
    /* Reference struct types */
    struct gc_basic_struct local_struct = {2, 2.718, "local"};
    struct gc_complex_struct complex_struct = {0};
    
    /* Reference union type */
    union gc_data_union local_union;
    local_union.as_string = "union test";
    
    /* Reference array types */
    gc_int_array_t local_array = {0};
    local_array[0] = 10;
    
    /* Reference pointer types */
    gc_struct_ptr_t struct_ptr = &global_gc_struct;
    gc_union_ptr_t union_ptr = &global_gc_union;
    
    /* Reference callback type */
    if (global_callback) {
        global_callback(1, 2.0);
    }
    
    /* Use __builtin_clear_padding to force type analysis */
    clear_struct_padding(&complex_struct);
    
    /* Calculate checksum using addresses and sizes */
    unsigned long checksum = 0;
    checksum += (unsigned long)&global_scalar_int;
    checksum += (unsigned long)&global_scalar_float;
    checksum += (unsigned long)&global_enum;
    checksum += (unsigned long)&global_gc_struct;
    checksum += (unsigned long)&global_gc_union;
    checksum += (unsigned long)global_int_array;
    checksum += sizeof(struct gc_basic_struct);
    checksum += sizeof(union gc_data_union);
    checksum += sizeof(gc_int_array_t);
    checksum += sizeof(gc_callback_t);
    
    /* Print something observable */
    printf("Checksum: %lu\n", checksum);
    printf("Size of gc_complex_struct: %zu\n", sizeof(struct gc_complex_struct));
    
    return 0;
}
