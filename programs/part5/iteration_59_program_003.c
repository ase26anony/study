#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type) type * GTY((length("nested_len"), nested)) *[]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY structure with deeply nested annotations */
typedef struct node node_t;

struct GTY((
  desc("%0.tag"),
  param_is(struct variant),
  chain_next("next"),
  chain_prev("prev")
)) node {
  int value;
  
  /* Parentheses for function pointer with complex signature */
  int (* GTY((skip)) callback)(
    struct node *child, 
    int depth, 
    void (* GTY((skip)) nested_cb)(int)
  );
  
  /* Brackets for array bounds - nested arrays */
  struct node * GTY((length("child_count"))) children[];
  
  /* Using macro expansion for more complex array */
  PTR_ARRAY(struct node) grandchildren;
  
  /* Braces for nested union within structure */
  union {
    int tag;
    void * GTY((tag("0"))) data;
    struct {
      int x;
      int y;
      int (* GTY((skip)) coord_calc)(int, int);
    } GTY((skip)) point;
  } variant;
  
  /* Another level of nesting with all delimiter types */
  struct {
    char * GTY((length("str_len"))) name;
    struct node * GTY((chain_next("link_next"), chain_prev("link_prev"))) links[4];
    union {
      int ival;
      double fval;
      struct node * GTY((reorder("node_cmp"))) sorted_list;
    } data;
  } metadata;
  
  struct node *next;
  struct node *prev;
};

/* Union type with GTY attributes containing string literals */
union GTY((
  desc("%0.tag == 0 ? \"INT\" : %0.tag == 1 ? \"PTR\" : \"OTHER\""),
  param_is(struct node)
)) gty_union {
  int int_val;
  struct node * GTY((tag("1"))) node_ptr;
  void * GTY((skip)) other_ptr;
};

/* Template-like structure using macros */
struct GTY(()) tree_container {
  int len;
  int nested_len;
  
  /* Multiple levels of nested arrays */
  NESTED_PTR_ARRAY(struct node) complex_array;
  
  /* Function pointer array with parentheses */
  int (* GTY((length("callback_count"), skip)) callbacks[])(
    struct node *,
    int,
    union gty_union *
  );
  
  /* Nested structure with its own GTY markers */
  struct {
    int depth;
    struct node * GTY((reorder("depth_cmp"))) *nodes;
    struct tree_container * GTY((skip)) parent;
  } GTY((skip)) context;
};

/* Global variable with GTY marker */
extern struct node * GTY(()) global_tree_root;

/* Function pointer type with complex signature */
typedef void (* GTY((skip)) traversal_func_t)(
  struct node *root,
  int (* GTY((skip)) visit)(struct node *, void *),
  void *user_data,
  struct {
    int max_depth;
    int options;
  } config
);

#endif /* TEST_GTY_H */
