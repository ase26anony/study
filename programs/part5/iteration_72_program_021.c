#ifndef GTY_TYPES_H
#define GTY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic GTY-marked structure (TYPE_STRUCT) */
struct GTY(()) base_struct {
    int id;
    char *GTY((skip)) name;  /* Pointer with skip option */
};

/* User-defined struct type (TYPE_USER_STRUCT) */
typedef struct base_struct GTY(()) base_struct_t;

/* Union type (TYPE_UNION) */
union GTY(()) data_union {
    int int_val;
    float float_val;
    void *GTY((skip)) ptr_val;
    struct base_struct *struct_ptr;
};

/* Pointer type within structure (TYPE_POINTER) */
struct GTY(()) linked_node {
    struct linked_node *GTY((skip)) next;  /* Recursive pointer */
    struct linked_node *GTY((skip)) prev;
    union data_union data;
    int priority;
};

/* Array type definition (TYPE_ARRAY) */
typedef int GTY(()) int_array[10];
typedef struct linked_node *GTY(()) node_ptr_array[5];

/* Function pointer/callback type (TYPE_CALLBACK) */
typedef void (*GTY(()) callback_func)(int, void*);
typedef int (*GTY(()) compare_func)(const void*, const void*);

/* Scalar type in container (TYPE_SCALAR) */
struct GTY(()) scalar_container {
    long GTY((skip)) counter;
    unsigned GTY((skip)) flags;
    double GTY((skip)) value;
};

/* String type handling (TYPE_STRING) */
struct GTY(()) string_holder {
    const char *GTY((skip)) message;
    char *GTY((skip)) buffer;
};

/* Nested complex type */
struct GTY(()) complex_type {
    struct base_struct base;
    union data_union variant;
    struct linked_node *GTY((skip)) node_list;
    int_array numbers;
    callback_func GTY((skip)) handler;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_MARKERS
struct GTY(()) conditional_struct {
    int x;
    char *GTY((skip)) y;
};
#else
struct conditional_struct {
    int x;
    char *y;
};
#endif

/* Multiple GTY options chained */
struct GTY(()) options_test {
    struct linked_node *GTY((skip, tag("special"))) special_node;
    void *GTY((skip)) opaque_data;
    int GTY((skip)) metadata;
};

/* Simulating lang_struct pattern - using naming convention */
struct GTY(()) tree_lang_struct {
    int code;
    union {
        struct base_struct *base;
        struct linked_node *node;
    } GTY((desc("code"))) u;
};

/* Forward declarations for mutual recursion */
struct GTY(()) type_a;
struct GTY(()) type_b;

/* Mutually recursive structures */
struct GTY(()) type_a {
    int id;
    struct type_b *GTY((skip)) partner;
    struct type_a *GTY((skip)) next;
};

struct GTY(()) type_b {
    int id;
    struct type_a *GTY((skip)) partner;
    callback_func GTY((skip)) callback;
};

/* Array of different types */
union GTY(()) variant_array_element {
    struct base_struct base;
    struct linked_node *node;
    callback_func callback;
    int scalar;
};

typedef union variant_array_element GTY(()) variant_array[8];

/* External declarations */
extern struct base_struct GTY(()) global_base;
extern union data_union GTY(()) global_union;
extern struct linked_node *GTY((skip)) global_list;

/* Function prototypes using GTY types */
void register_callback(callback_func GTY((skip)) func);
void process_structure(struct base_struct *GTY((skip)) s);
union data_union create_data_union(int type);

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
