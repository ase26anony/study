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
    int GTY((skip)) value;
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
    } GTY((skip)) data;
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
    callback_func GTY((skip)) handler;
};

/* For TYPE_LANG_STRUCT simulation - using a tag that might be treated specially */
struct GTY(()) lang_struct_sim {
    int GTY((skip)) lang_specific;
    void *GTY((skip)) lang_data;
};

/* String type handling */
struct GTY(()) string_container {
    const char *GTY((skip)) fixed_string;
    char *GTY((skip)) dynamic_string;
};

/* Scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) long_value;
    unsigned GTY((skip)) flags;
    size_t GTY((skip)) size;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_type {
    int GTY((skip)) marked_field;
    void *GTY((skip)) marked_pointer;
};
#else
struct conditional_type {
    int marked_field;
    void *marked_pointer;
};
#endif

/* Multiple GTY options */
struct GTY(()) options_test {
    int GTY((skip, tag("0"))) tag_field;
    struct linked_node *GTY((skip, length("len"))) nodes;
    int GTY((skip)) len;
    void *GTY((skip, desc("1"))) described_ptr;
};

/* Forward declarations for cross-TU testing */
struct GTY(()) forward_declared;
typedef struct forward_declared GTY(()) forward_declared_t;

/* Array of various types */
typedef union data_union GTY(()) union_array[8];
typedef callback_func GTY(()) callback_array[4];

#endif /* GTY_TEST_TYPES_H */
