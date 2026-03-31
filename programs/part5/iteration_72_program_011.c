#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Array type - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Pointer type within structure - TYPE_POINTER */
struct GTY(()) list_node {
    struct list_node *GTY((skip)) next;  /* Recursive pointer */
    struct list_node *GTY((skip)) prev;  /* Another pointer */
    union data_union data;
    int_array scores;  /* Array type field */
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_fn)(int, struct base_struct*);
typedef int (*GTY(()) compare_fn)(const void*, const void*);

/* Structure containing callback */
struct GTY(()) processor {
    callback_fn GTY((skip)) process;  /* Function pointer field */
    compare_fn GTY((skip)) compare;
    struct list_node *GTY((skip)) data_list;
};

/* Nested structure with union */
struct GTY(()) container {
    struct base_struct header;
    union {
        int int_data;
        struct list_node *GTY((skip)) node_data;
    } GTY((tag("0"))) content;  /* Union with tag option */
    int GTY((length("get_length"))) variable_array[];  /* Variable length array */
};

/* Scalar type in structure - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    unsigned long GTY((skip)) flags;
    double GTY((skip)) precision;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) message;  /* String pointer */
    char *GTY((skip)) buffer;
};

/* Complex nested types */
struct GTY(()) tree_node {
    int type;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union value;
};

/* Array of different pointer types */
struct GTY(()) pointer_collection {
    void *GTY((skip)) void_ptrs[5];
    struct base_struct *GTY((skip)) struct_ptrs[3];
    callback_fn GTY((skip)) callbacks[2];
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) linked_item {
    int data;
    struct linked_item *GTY((skip)) next;
    struct linked_item *GTY((skip)) prev;
};
#else
struct linked_item {
    int data;
    struct linked_item *next;
    struct linked_item *prev;
};
#endif

/* Multiple GTY options chained */
struct GTY((skip, desc("1"), tag("NODE_TYPE"))) tagged_node {
    int type_tag;
    struct tagged_node *GTY((skip)) link;
};

/* Forward declaration that will be defined elsewhere */
struct GTY(()) forward_declared;

#endif /* GTY_TEST_TYPES_H */
