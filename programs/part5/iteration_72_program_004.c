#ifndef GTY_TYPES_H
#define GTY_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Basic scalar type in container */
struct GTY(()) scalar_box {
    long GTY((skip)) value;
    double GTY((skip)) dvalue;
};

/* String type */
struct GTY(()) string_container {
    const char * GTY((skip)) message;
    char * GTY((skip)) buffer;
};

/* Array types */
typedef int GTY(()) int_array[10];
typedef struct scalar_box * GTY(()) box_ptr_array[5];

/* Callback (function pointer) type */
typedef void (*GTY(()) callback_fn)(int, const char*);
typedef int (*GTY(()) compare_fn)(const void*, const void*);

/* Pointer type */
struct GTY(()) list_node;

/* Forward declaration for recursive structure */
struct GTY(()) tree_node;

/* Union type */
union GTY(()) data_union {
    int GTY((skip)) i;
    float GTY((skip)) f;
    double GTY((skip)) d;
    void * GTY((skip)) p;
    struct tree_node * GTY((skip)) node;
};

/* Structure type */
struct GTY(()) my_struct {
    int GTY((skip)) a;
    char * GTY((skip)) b;
    union data_union GTY((skip)) u;
    struct list_node * GTY((skip)) next;
};

/* Recursive structure */
struct GTY(()) tree_node {
    int GTY((skip)) id;
    struct tree_node * GTY((skip)) left;
    struct tree_node * GTY((skip)) right;
    union data_union GTY((skip)) data;
};

/* Linked list structure */
struct GTY(()) list_node {
    struct list_node * GTY((skip)) next;
    struct list_node * GTY((skip)) prev;
    struct my_struct GTY((skip)) data;
    callback_fn GTY((skip)) callback;
};

/* Array structure */
struct GTY(()) array_container {
    int GTY((skip)) count;
    int_array GTY((skip)) fixed_array;
    struct tree_node * GTY((skip)) node_array[20];
    box_ptr_array GTY((skip)) box_ptrs;
};

/* Conditional compilation with GTY */
#ifdef USE_GTY_OPTIONS
struct GTY((chain_next("%h.next"), chain_prev("%h.prev"))) chained_struct {
    int GTY((skip)) value;
    struct chained_struct * GTY((skip)) next;
    struct chained_struct * GTY((skip)) prev;
};
#endif

/* User-defined struct type via typedef */
typedef struct my_struct GTY(()) my_struct_t;
typedef union data_union GTY(()) data_union_t;

/* Complex nested type */
struct GTY(()) nested_container {
    struct {
        int GTY((skip)) x;
        int GTY((skip)) y;
    } GTY((skip)) point;
    
    union {
        int GTY((skip)) i;
        struct my_struct GTY((skip)) s;
    } GTY((skip)) data;
    
    struct array_container GTY((skip)) arrays;
};

/* For TYPE_LANG_STRUCT simulation - using a special tag pattern */
struct GTY(()) lang_tree_node {
    enum { LANG_EXPR, LANG_DECL, LANG_TYPE } GTY((skip)) kind;
    union {
        struct {
            int GTY((skip)) op;
            struct lang_tree_node * GTY((skip)) kids[2];
        } GTY((skip)) expr;
        struct {
            const char * GTY((skip)) name;
            struct lang_tree_node * GTY((skip)) type;
        } GTY((skip)) decl;
        struct {
            int GTY((skip)) size;
            int GTY((skip)) align;
        } GTY((skip)) type;
    } GTY((skip)) u;
};

#ifdef __cplusplus
}
#endif

#endif /* GTY_TYPES_H */
