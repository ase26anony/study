#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY marker macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED */
struct opaque_struct;

/* TYPE_SCALAR: Fundamental scalar types and enums */
typedef enum GTY(()) color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef int GTY(()) scalar_int;
typedef double GTY(()) scalar_double;
typedef float GTY(()) scalar_float;
typedef long GTY(()) scalar_long;
typedef char GTY(()) scalar_char;

/* TYPE_STRING: String types */
typedef char* GTY(()) string_ptr;
typedef const char* GTY(()) const_string_ptr;

/* TYPE_CALLBACK: Function pointer type */
typedef void (*callback_func)(int, double);
typedef callback_func GTY(()) gty_callback;

/* TYPE_UNION: Union types */
union GTY(()) basic_union {
    int int_val;
    double double_val;
    char* GTY((length("strlen($)"))) string_val;
    void* ptr_val;
};

/* Tagged union with discriminator */
struct GTY(()) tagged_union_container {
    int tag;
    union GTY((desc("%0.tag"))) {
        int as_int;
        double as_double;
        struct simple_struct* as_ptr;
    } value;
};

/* TYPE_ARRAY: Array types */
typedef int GTY(()) int_array[10];
typedef struct simple_struct* GTY(()) ptr_array[5];
typedef char GTY(()) multi_dim_array[3][4][5];

/* TYPE_POINTER: Various pointer types */
typedef void* GTY(()) void_ptr;
typedef struct simple_struct* GTY(()) struct_ptr;
typedef union basic_union* GTY(()) union_ptr;
typedef int* GTY(()) int_ptr;
typedef int** GTY(()) int_ptr_ptr;
typedef void (*func_ptr)(void);

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) simple_struct {
    int id;
    char* GTY((length("strlen($)"))) name;
    double value;
    color_t color;
    struct simple_struct* GTY((chain_next("%h.next"))) next;
    struct simple_struct* GTY((chain_prev("%h.prev"))) prev;
};

/* Struct with bit-fields */
struct GTY(()) bitfield_struct {
    unsigned int flag1 : 1;
    unsigned int flag2 : 2;
    unsigned int flag3 : 3;
    unsigned int padding : 26;
};

/* Struct with anonymous struct inside */
struct GTY(()) nested_anonymous_struct {
    int outer;
    struct {
        int inner_a;
        int inner_b;
    } inner;
    union {
        int x;
        double y;
    } anon_union;
};

/* Struct with array member */
struct GTY(()) struct_with_array {
    int count;
    int GTY((length("%h.count"))) values[];
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_defined_struct {
    int user_data;
    void* user_handle;
};

/* Complex struct with all types */
struct GTY(()) master_struct {
    /* Scalars */
    int id;
    double score;
    color_t theme;
    
    /* Strings */
    char* GTY((length("strlen($)"))) title;
    const char* GTY((length("strlen($)"))) description;
    
    /* Pointers */
    struct master_struct* GTY((chain_next("%h.next"))) next;
    struct master_struct* GTY((chain_prev("%h.prev"))) prev;
    void* arbitrary_ptr;
    
    /* Arrays */
    int GTY(()) numbers[5];
    struct simple_struct* GTY(()) struct_array[3];
    
    /* Union */
    union basic_union data;
    
    /* Callback */
    gty_callback callback;
    
    /* Undefined/opaque */
    struct opaque_struct* opaque;
    
    /* Bitfields */
    unsigned int flags : 8;
    
    /* Variable length array */
    int var_count;
    char* GTY((length("%h.var_count"))) var_data[];
};

/* Linked list node using chain_next/chain_prev */
struct GTY(()) list_node {
    int data;
    struct list_node* GTY((chain_next("%h.next"))) next;
    struct list_node* GTY((chain_prev("%h.prev"))) prev;
};

/* Tree node structure */
struct GTY(()) tree_node {
    int value;
    struct tree_node* GTY((chain_next("%h.left"))) left;
    struct tree_node* GTY((chain_prev("%h.right"))) right;
};

#endif /* TEST_TYPES_H */
