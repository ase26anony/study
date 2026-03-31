#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type */
struct GTY(()) base_struct {
    int GTY((skip)) id;
    char *GTY((skip)) name;
};

/* Union type */
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

/* User-defined type via typedef */
typedef struct base_struct GTY(()) base_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Structure containing union */
struct GTY(()) struct_with_union {
    int GTY((skip)) type;
    union {
        int GTY((skip)) int_member;
        float GTY((skip)) float_member;
        char *GTY((skip)) string_member;
    } GTY((tag("type"))) value;
};

/* Union containing structure */
union GTY(()) union_with_struct {
    struct {
        int GTY((skip)) x;
        int GTY((skip)) y;
    } GTY((skip)) point;
    struct {
        float GTY((skip)) r;
        float GTY((skip)) g;
        float GTY((skip)) b;
    } GTY((skip)) color;
};

/* Complex nested type */
struct GTY(()) container {
    struct linked_node *GTY((skip)) head;
    struct linked_node *GTY((skip)) tail;
    int_array GTY((skip)) counts;
    data_union_t GTY((skip)) metadata;
    callback_func GTY((skip)) notify;
};

/* String type simulation */
struct GTY(()) string_wrapper {
    const char *GTY((skip)) str;
    int GTY((skip)) length;
};

/* Scalar type in container */
struct GTY(()) scalar_container {
    long GTY((skip)) long_value;
    unsigned long long GTY((skip)) ull_value;
    double GTY((skip)) double_value;
};

/* Conditional compilation for GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("next"), chain_prev("prev"))) chained_struct {
    int GTY((skip)) value;
    struct chained_struct *GTY((skip)) next;
    struct chained_struct *GTY((skip)) prev;
};
#else
struct chained_struct {
    int value;
    struct chained_struct *next;
    struct chained_struct *prev;
};
#endif

/* Forward declarations for complex type graph */
struct GTY(()) tree_node;
union GTY(()) tree_data;

/* Self-referential types */
struct GTY(()) tree_node {
    int GTY((skip)) node_type;
    union tree_data *GTY((skip)) data;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

union GTY(()) tree_data {
    int GTY((skip)) int_data;
    double GTY((skip)) double_data;
    char *GTY((skip)) string_data;
    struct base_struct *GTY((skip)) struct_data;
};

/* Array of different pointer types */
struct GTY(()) pointer_array_container {
    void *GTY((skip)) void_ptrs[8];
    struct base_struct *GTY((skip)) struct_ptrs[4];
    callback_func GTY((skip)) func_ptrs[3];
};

/* Extern declarations */
extern struct container GTY((skip)) global_container;
extern data_union_t GTY((skip)) global_union;

#endif /* GTY_TEST_TYPES_H */
