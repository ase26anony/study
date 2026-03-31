#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic GTY-marked structure */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* GTY-marked union */
union GTY(()) data_union {
    int GTY((skip)) int_val;
    float GTY((skip)) float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Pointer type within structure (recursive) */
struct GTY(()) linked_node {
    int GTY((skip)) data;
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
};

/* Array type definition */
typedef int GTY(()) int_array[10];
typedef struct base_struct *GTY(()) struct_ptr_array[5];

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;

/* Function pointer (callback) type */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Complex nested structure */
struct GTY(()) container {
    union data_union GTY((skip)) data;
    struct linked_node *GTY((skip)) list_head;
    int_array GTY((skip)) numbers;
    callback_func GTY((skip)) handler;
};

/* Self-referential union */
union GTY(()) tree_node {
    int GTY((skip)) leaf_value;
    struct GTY(()) {
        union tree_node *GTY((skip)) left;
        union tree_node *GTY((skip)) right;
    } GTY((skip)) children;
};

/* Structure with array of pointers */
struct GTY(()) pointer_array_struct {
    int count;
    struct base_struct *GTY((skip)) items[10];
    struct base_struct **GTY((skip)) dynamic_items;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int GTY((skip)) value;
    char GTY((skip)) tag;
};
#else
struct conditional_struct {
    int value;
    char tag;
};
#endif

/* GTY with multiple options */
struct GTY(()) complex_options {
    int GTY((skip, tag("0"))) type;
    union data_union GTY((skip, tag("1"))) data;
    struct linked_node *GTY((chain_next("next"), chain_prev("prev"))) list;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) dynamic_string;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned long GTY((skip)) ulong_value;
    size_t GTY((skip)) size_value;
};

/* Simulating lang_struct pattern - using a naming convention */
struct GTY(()) lang_struct_sim {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* Forward declarations for mutual recursion */
struct GTY(()) type_a;
struct GTY(()) type_b;

/* Mutually recursive structures */
struct GTY(()) type_a {
    int GTY((skip)) a_data;
    struct type_b *GTY((skip)) b_ptr;
};

struct GTY(()) type_b {
    int GTY((skip)) b_data;
    struct type_a *GTY((skip)) a_ptr;
    struct type_a GTY((skip)) a_embedded;
};

#endif /* GTY_TEST_TYPES_H */
