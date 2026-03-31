#ifndef GTY_TEST_TYPES_H
#define GTY_TEST_TYPES_H

/* Basic structure type */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* TYPE_STRING */
};

/* Union type */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;  /* TYPE_POINTER */
    struct base_struct *GTY((skip)) struct_ptr;  /* Nested pointer */
};

/* Recursive structure for TYPE_POINTER */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;   /* Self-referential pointer */
    struct tree_node *GTY((skip)) right;  /* Self-referential pointer */
    union data_union GTY((skip)) data;    /* TYPE_UNION */
};

/* Array type definition */
typedef int GTY(()) int_array[10];  /* TYPE_ARRAY */

/* Multi-dimensional array */
typedef struct tree_node *GTY(()) node_matrix[5][5];

/* Callback function type */
typedef void (*GTY(()) callback_func)(int, const char *);  /* TYPE_CALLBACK */

/* User-defined struct type via typedef */
typedef struct base_struct GTY(()) base_struct_t;  /* TYPE_USER_STRUCT */

/* Scalar wrapper */
struct GTY(()) scalar_container {
    long GTY((skip)) scalar_value;  /* TYPE_SCALAR */
    unsigned GTY((skip)) flags;
};

/* Complex nested structure */
struct GTY(()) complex_type {
    struct base_struct GTY((skip)) base;
    union data_union GTY((skip)) union_field;
    int_array GTY((skip)) numbers;      /* TYPE_ARRAY */
    callback_func GTY((skip)) handler;  /* TYPE_CALLBACK */
    struct complex_type *GTY((skip)) next;  /* Recursive pointer */
};

/* Forward declaration for mutual recursion */
struct GTY(()) list_item;
struct GTY(()) list_head;

/* Mutual recursion structures */
struct GTY(()) list_item {
    int data;
    struct list_head *GTY((skip)) parent;
    struct list_item *GTY((skip)) next;
};

struct GTY(()) list_head {
    int count;
    struct list_item *GTY((skip)) first;
    struct list_item *GTY((skip)) last;
};

/* Conditional compilation for GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    float y;
};
#else
struct conditional_struct {
    int x;
    float y;
};
#endif

/* Enumeration type (scalar) */
enum gty_test_enum {
    GTY_ENUM_A,
    GTY_ENUM_B,
    GTY_ENUM_C
};

/* Structure with enum */
struct GTY(()) enum_container {
    enum gty_test_enum GTY((skip)) state;  /* TYPE_SCALAR */
    int value;
};

/* Function pointer with complex signature */
typedef int (*GTY(()) complex_callback)(
    struct base_struct *GTY((skip)),
    union data_union *GTY((skip)),
    callback_func GTY((skip))
);

/* Array of function pointers */
typedef callback_func GTY(()) callback_array[5];

/* Structure with bitfields (scalar types) */
struct GTY(()) bitfield_struct {
    unsigned int GTY((skip)) flag1 : 1;
    unsigned int GTY((skip)) flag2 : 2;
    unsigned int GTY((skip)) flag3 : 3;
    int regular_field;
};

/* Opaque pointer type */
typedef void *GTY(()) opaque_ptr;

#endif /* GTY_TEST_TYPES_H */
