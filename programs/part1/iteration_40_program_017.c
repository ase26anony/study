/* test_gty.h - Comprehensive GTY annotation test for gengtype coverage */

/* Ensure GTY macro is defined */
#ifndef GTY
#define GTY(x) 
#endif

/* Dummy definitions for GCC internal types */
typedef int tree;
typedef void* rtx;
typedef void* gimple;

/* ========== TYPE_UNDEFINED ========== */
/* Forward declaration without definition */
struct GTY(()) undefined_struct;

/* ========== TYPE_SCALAR ========== */
typedef int GTY((user)) my_scalar_t;

/* ========== TYPE_STRING ========== */
const char * GTY((length)) my_string;

/* ========== TYPE_STRUCT ========== */
struct GTY((tag("my_struct"))) my_struct {
    int field;
    tree node;  /* Use dummy GCC type */
};

/* ========== TYPE_USER_STRUCT ========== */
typedef struct my_struct GTY((user)) my_user_struct_t;

/* ========== TYPE_UNION ========== */
union GTY((desc("0"))) my_union {
    int a;
    char * GTY((skip)) b;
    rtx insn;  /* Use dummy GCC type */
};

/* ========== TYPE_POINTER ========== */
struct my_struct * GTY((skip)) my_pointer;
tree * GTY((chain_next("tree"), chain_prev("tree"))) tree_chain;

/* ========== TYPE_ARRAY ========== */
int GTY((length)) my_array[10];
struct my_struct GTY((length)) struct_array[5];

/* ========== TYPE_CALLBACK ========== */
typedef void (*GTY((user)) my_callback_fn)(int);
typedef tree (*GTY((user)) tree_callback)(rtx, gimple);

/* ========== TYPE_LANG_STRUCT ========== */
/* Language-specific structure with special marker */
struct GTY((special("lang_struct"))) lang_specific_struct {
    int lang_code;
    union {
        tree ast_node;
        rtx code;
    } GTY((desc("%1.lang_code"))) u;
};

/* Complex nested structure to exercise more parsing paths */
struct GTY(()) container {
    /* Scalar */
    my_scalar_t scalar_field;
    
    /* String */
    const char * GTY((length)) name;
    
    /* Pointer */
    struct my_struct * GTY((skip)) data_ptr;
    
    /* Array */
    int GTY((length)) values[20];
    
    /* Union */
    union GTY((desc("0"))) choice {
        int int_val;
        char * GTY((skip)) str_val;
    } selection;
    
    /* Callback */
    my_callback_fn GTY((user)) callback;
    
    /* Nested struct */
    struct GTY(()) nested {
        int id;
        tree node;
    } inner;
    
    /* Language struct */
    struct lang_specific_struct GTY((skip)) lang_data;
};

/* Another test with variable length array */
struct GTY(()) varray_container {
    int count;
    tree GTY((length("%h.count"))) elements[1];
};

/* Test for param_is/param1_is usage */
struct GTY((param_is(T))) template_struct {
    T* data;
};

/* Test for atomic and maybe_undef */
struct GTY(()) atomic_test {
    int GTY((atomic)) atomic_field;
    struct undefined_struct * GTY((maybe_undef)) maybe_undefined;
};

/* Test for nested pointers with chain_next */
struct GTY(()) chain_test {
    struct chain_test * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) next;
    struct chain_test * GTY((chain_next("%h.next"), chain_prev("%h.prev"))) prev;
    int value;
};

/* Test array of pointers */
tree * GTY((length)) tree_ptr_array[50];

/* Test for skip and nested attributes */
struct GTY(()) skip_test {
    int important;
    char * GTY((skip("debug"))) debug_info;
    rtx GTY((skip)) skipped_rtx;
};

/* Test for desc with enum */
enum my_enum { ENUM_A, ENUM_B, ENUM_C };
union GTY((desc("%0.etype"))) enum_union {
    enum my_enum etype;
    int ival;
    char * GTY((skip)) sval;
};

/* Test structure containing the enum union */
struct GTY(()) enum_container {
    enum my_enum etype;
    union enum_union GTY((desc("%h.etype"))) data;
};

/* Test for callback with nested structure */
typedef struct GTY(()) {
    int id;
    tree node;
} (*GTY((user)) complex_callback)(rtx, int);

/* Test for array of unions */
union GTY((desc("0"))) multi_union {
    int i;
    double d;
    char * GTY((skip)) s;
} GTY((length)) union_array[10];

/* Test for pointer to array */
int (*GTY((skip)) ptr_to_array)[10];

/* Test for nested array in struct */
struct GTY(()) nested_array_struct {
    int dimensions[3][4];
    tree nodes[2][5];
};

/* Test for use_def/use_params */
struct GTY((use_params("T"))) generic_struct {
    void* data;
};

/* Final test: structure referencing all types */
struct GTY((tag("master"))) master_struct {
    /* Reference to undefined */
    struct undefined_struct * GTY((maybe_undef)) undef_ref;
    
    /* Scalar */
    my_scalar_t scalar;
    
    /* String */
    const char * GTY((length)) title;
    
    /* Direct struct */
    struct my_struct direct;
    
    /* User struct */
    my_user_struct_t user;
    
    /* Union */
    union my_union choice;
    
    /* Pointer */
    struct my_struct * GTY((skip)) ptr;
    
    /* Array */
    int GTY((length)) numbers[100];
    
    /* Callback */
    my_callback_fn handler;
    
    /* Lang struct */
    struct lang_specific_struct lang;
    
    /* Nested container */
    struct container nested;
    
    /* Chain */
    struct chain_test * GTY((chain_next("%h.chain"))) chain;
    
    /* Array of pointers */
    tree * GTY((length)) tree_ptrs[25];
    
    /* Enum union */
    struct enum_container enum_data;
    
    /* Array of unions */
    union multi_union GTY((length)) multi_data[5];
    
    /* Pointer to array */
    int (*GTY((skip)) matrix_ptr)[20][30];
    
    /* Nested array */
    struct nested_array_struct arrays;
    
    /* Atomic field */
    int GTY((atomic)) counter;
    
    /* Skip field */
    char * GTY((skip)) internal_debug;
};
