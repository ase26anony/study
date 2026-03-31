/* test_types.h - Comprehensive GTY type definitions for gengtype coverage */

#ifndef TEST_TYPES_H
#define TEST_TYPES_H

/* Include GCC's GTY macros */
#ifndef GTY
#define GTY(x) 
#endif

/* Forward declaration for TYPE_UNDEFINED case */
struct opaque;  /* TYPE_UNDEFINED */

/* TYPE_SCALAR: Fundamental scalar types */
typedef enum color {
    RED,
    GREEN,
    BLUE
} color_t;

typedef GTY(()) enum status {
    STATUS_OK,
    STATUS_ERROR,
    STATUS_PENDING
} status_t;

/* TYPE_STRUCT: Basic struct with various members */
struct GTY(()) basic_struct {
    int id;                     /* scalar */
    char name[32];              /* array */
    float weight;               /* scalar */
    double price;               /* scalar */
    color_t color;              /* enum scalar */
    
    /* Bit-fields */
    unsigned int flags : 4;
    unsigned int mode : 2;
    
    /* Anonymous struct */
    struct {
        int x;
        int y;
    } GTY(()) position;
};

/* TYPE_STRUCT with nested struct */
struct GTY(()) complex_struct {
    struct basic_struct GTY(()) base;
    struct complex_struct *GTY((skip)) next;  /* pointer with skip option */
    struct complex_struct *GTY((chain_next)) chain_next;  /* chain_next option */
    struct complex_struct *GTY((chain_prev)) chain_prev;  /* chain_prev option */
    
    /* Array within struct */
    int GTY(()) scores[10];
    
    /* Flexible array member */
    char GTY((length("strlen($) + 1"))) data[];
};

/* TYPE_USER_STRUCT: User-defined struct */
struct GTY((user)) user_struct {
    int user_id;
    char *GTY(()) user_name;
    void *GTY(()) user_data;
};

/* TYPE_UNION: Basic union */
union GTY(()) data_union {
    int int_value;
    float float_value;
    double double_value;
    char *GTY(()) string_value;
    struct basic_struct GTY(()) struct_value;
};

/* TYPE_UNION: Tagged union within struct */
struct GTY(()) tagged_union_container {
    int tag;
    union {
        int as_int;
        float as_float;
        char *GTY(()) as_string;
        struct basic_struct GTY(()) as_struct;
    } GTY(()) data;
};

/* TYPE_POINTER: Various pointer types */
typedef struct basic_struct *GTY(()) basic_ptr;
typedef union data_union *GTY(()) union_ptr;
typedef void (*GTY(()) callback_func)(int, char *);  /* function pointer */

/* TYPE_ARRAY: Various array types */
typedef int GTY(()) int_array[10];
typedef struct basic_struct GTY(()) struct_array[5];
typedef char *GTY(()) string_array[3];

/* Multi-dimensional arrays */
typedef int GTY(()) matrix[3][3];
typedef struct basic_struct GTY(()) struct_matrix[2][2];

/* TYPE_STRING: String types */
typedef char *GTY((string)) gty_string;
typedef const char *GTY((string)) const_gty_string;

/* TYPE_CALLBACK: Callback function pointer type */
typedef void (*GTY(()) event_callback)(void *GTY(()) data, int event_type);

/* Struct containing callback */
struct GTY(()) event_handler {
    char *GTY(()) handler_name;
    event_callback GTY(()) callback;
    void *GTY(()) user_data;
};

/* Struct with desc option for discriminated union */
struct GTY(()) variant {
    int tag;
    union {
        int as_int;
        float as_float;
        char *GTY(()) as_string;
    } GTY((desc("tag"))) data;
};

/* Linked list example */
struct GTY(()) linked_list {
    int value;
    struct linked_list *GTY((skip)) next;
    struct linked_list *GTY((skip)) prev;
};

/* Tree structure */
struct GTY(()) tree_node {
    int key;
    void *GTY(()) data;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    struct tree_node *GTY((skip)) parent;
};

#endif /* TEST_TYPES_H */
