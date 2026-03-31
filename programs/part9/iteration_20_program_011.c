/* base-types.gtype - Basic type definitions for gengtype testing */

/* TYPE_SCALAR examples */
typedef int my_scalar_int;
typedef unsigned long my_scalar_ulong;
typedef double my_scalar_double;

/* TYPE_STRING */
typedef const char *my_string_type;

/* TYPE_POINTER */
typedef struct my_basic_struct *my_pointer_type;

/* TYPE_ARRAY - both fixed and variable length */
struct my_array_container {
    int fixed_array[10];
    struct my_basic_struct **variable_array GTY((length("var_len")));
    int var_len;
};

/* TYPE_STRUCT with various fields */
struct my_basic_struct GTY(()) {
    my_scalar_int id;
    my_string_type name;
    struct my_basic_struct *next GTY((skip));  /* skip annotation */
    struct my_basic_struct *prev GTY((chain_prev("next")));
    struct my_union_type *data_ptr;
    int fixed_array[5];
};

/* TYPE_UNION */
union my_union_type GTY((desc("type_tag"))) {
    int int_val;
    double double_val;
    struct my_basic_struct *struct_ptr;
    int type_tag;
};

/* Forward declaration for TYPE_UNDEFINED test */
struct forward_declared_struct;

/* Struct using forward declared type to trigger TYPE_UNDEFINED */
struct uses_forward_decl {
    struct forward_declared_struct *fwd_ptr;
    int value;
};

/* Now define the forward declared struct */
struct forward_declared_struct GTY(()) {
    int data;
    struct uses_forward_decl *back_ref;
};

/* Linked list example with chain_next */
struct linked_list GTY(()) {
    int value;
    struct linked_list *next GTY((chain_next));
};

/* Nested structure for complex traversal */
struct outer_struct GTY(()) {
    struct my_basic_struct inner;
    struct outer_struct *sibling;
    union my_union_type data;
    struct linked_list *list_head;
};
