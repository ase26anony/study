#ifndef GTY_TYPES_H
#define GTY_TYPES_H

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* TYPE_POINTER with skip option */
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

/* User-defined struct type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Recursive structure with pointer to self */
struct GTY(()) linked_node {
    int data;
    struct linked_node *GTY((skip)) next;
    struct linked_node *GTY((skip)) prev;
};

/* Structure containing union */
struct GTY(()) container {
    int type;
    union data_union GTY((tag("type"))) value;  /* TYPE_UNION with tag option */
};

/* Callback type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, void*);

/* Structure with callback field */
struct GTY(()) event_handler {
    callback_func GTY((skip)) handler;
    void *GTY((skip)) user_data;
};

/* Scalar type in structure - TYPE_SCALAR */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar1;
    unsigned long long GTY((skip)) scalar2;
    double GTY((skip)) scalar3;
};

/* String type - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) static_string;
    char *GTY((skip)) dynamic_string;
};

/* Complex nested structure */
struct GTY(()) complex_nested {
    struct base_struct base;
    union data_union data;
    int_array numbers;
    struct_ptr_array pointers;
    struct complex_nested *GTY((skip)) self_ref;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip, length("x"))) variable_array;
};
#else
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip)) variable_array;
};
#endif

/* Forward declaration for mutual recursion */
struct GTY(()) tree_node;
struct GTY(()) tree_child;

/* Mutual recursion structures */
struct GTY(()) tree_node {
    int value;
    struct tree_child *GTY((skip)) first_child;
};

struct GTY(()) tree_child {
    struct tree_node *GTY((skip)) node;
    struct tree_child *GTY((skip)) next;
};

/* Array with length option */
struct GTY(()) array_with_length {
    int count;
    int *GTY((skip, length("count"))) items;
};

/* Structure with nested anonymous union */
struct GTY(()) anon_union_container {
    int type;
    union {
        int int_member;
        float float_member;
        void *ptr_member;
    } GTY((tag("type"))) data;
};

/* Extern declarations */
extern struct base_struct GTY(()) global_base;
extern int_array GTY(()) global_array;

#endif /* GTY_TYPES_H */
