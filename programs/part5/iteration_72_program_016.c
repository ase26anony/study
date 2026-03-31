#ifndef GTY_TYPES_H
#define GTY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic GTY-marked structure - TYPE_STRUCT */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip attribute */
};

/* Union type - TYPE_UNION */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *GTY((skip)) struct_ptr;
};

/* Typedef creating user-defined type - TYPE_USER_STRUCT */
typedef struct base_struct GTY(()) base_struct_t;

/* Array type definition */
typedef int GTY(()) int_array[10];

/* Function pointer (callback) type - TYPE_CALLBACK */
typedef void (*GTY(()) callback_func)(int, const char*);

/* Complex nested structure with recursive pointer */
struct GTY(()) tree_node {
    int value;
    struct tree_node *GTY((skip)) left;
    struct tree_node *GTY((skip)) right;
    union data_union data;
};

/* Structure containing array of pointers */
struct GTY(()) pointer_container {
    struct base_struct *GTY((skip)) items[5];
    int_array numbers;
};

/* Union containing structure */
union GTY(()) nested_union {
    struct base_struct s;
    struct tree_node t;
    callback_func cb;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((length("strlen(x)"))) str;  /* Using GTY options */
    int GTY((tag("1"))) tagged_field;
};
#else
struct conditional_struct {
    int x;
    char *str;
    int tagged_field;
};
#endif

/* Scalar type in container - TYPE_SCALAR */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    double GTY((skip)) double_value;
    unsigned int GTY((skip)) flags;
};

/* String type handling - TYPE_STRING */
struct GTY(()) string_container {
    const char *GTY((skip)) constant_string;
    char *GTY((skip)) dynamic_string;
};

/* Array of unions */
typedef union data_union GTY(()) union_array[4];

/* Self-referential union */
union GTY(()) self_ref_union {
    int value;
    union self_ref_union *GTY((skip)) next;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
