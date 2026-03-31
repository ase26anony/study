#ifndef GTY_TYPES_H
#define GTY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* GTY-marked union - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* User-defined struct type via typedef - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type definition - TYPE_ARRAY */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* Callback function pointer type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* String type - TYPE_STRING */
typedef const char *GTY(()) gty_string;

/* Scalar type container - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    double GTY((skip)) value;
};

/* Complex nested structure with multiple GTY options */
struct GTY(()) complex_node {
    int data;
    struct complex_node *GTY((tag("0"))) next;  /* Recursive pointer with tag */
    struct complex_node *GTY((skip)) prev;      /* Skipped pointer */
    union data_union GTY(()) node_data;         /* Embedded union */
    callback_func GTY(()) notify;               /* Callback field */
};

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_leaf;

/* Structure for simulating lang_struct-like behavior */
struct GTY((chain_next ("%h.next"), chain_prev ("%h.prev"))) tree_node {
    int node_type;
    gty_string GTY((skip)) label;
    struct tree_node *GTY((skip)) next;
    struct tree_node *GTY((skip)) prev;
    struct tree_leaf *GTY((skip)) first_child;
};

struct GTY(()) tree_leaf {
    int leaf_id;
    struct tree_node *GTY((skip)) parent;
    int_array GTY(()) values;  /* Array field */
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY(()) optimized_struct {
    int GTY((special("special_field"))) special;
    struct base_struct *GTY((length("len"))) items;
    int len;
};
#else
struct GTY(()) optimized_struct {
    int special;
    struct base_struct *GTY((skip)) items;
    int len;
};
#endif

/* Multiple GTY options chained together */
struct GTY(()) multi_option_struct {
    int GTY((skip, tag("1"))) hidden_field;
    void *GTY((skip, desc("tag"))) tagged_ptr;
    int tag;
};

/* External declarations */
extern struct base_struct GTY(()) global_base;
extern union data_union GTY(()) global_union;

/* Function prototypes using GTY types */
void process_callback(callback_func GTY(()) cb);
struct complex_node* GTY((skip)) create_complex_graph(void);

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
