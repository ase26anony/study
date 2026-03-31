#ifndef TEST_GTY_H
#define TEST_GTY_H

/* Macro wrappers that expand to GTY annotations with nested delimiters */
#define PTR_ARRAY(type) type * GTY((length("len"))) []
#define NESTED_PTR_ARRAY(type, depth) type * GTY((length("len"))) [depth]
#define CALLBACK_TYPE(ret, args) ret (* GTY((skip)) args)

/* Complex GTY structure with all delimiter types */
typedef struct GTY((
  desc("%0.tag"),
  param_is(struct variant_node)
)) variant_node;

struct GTY(()) tree_node {
  int value;
  
  /* Parentheses in function pointer */
  int (* GTY((skip)) 
    traverse_callback)(struct tree_node *node, 
                      int (* GTY((skip)) inner)(int));
  
  /* Brackets for array with nested GTY */
  struct tree_node * GTY((length("child_count"))) 
    children[];
  
  /* Braces for embedded union */
  union {
    int tag;
    void * GTY((tag("0"))) data;
    struct tree_node * GTY((tag("1"))) node_ref;
  } GTY((desc("%0.tag"))) variant;
  
  /* Macro-expanded array with brackets */
  PTR_ARRAY(struct tree_node) grandchildren;
  
  /* Nested array with multiple bracket levels */
  int GTY((length("matrix_dim"))) matrix[][3];
  
  /* Function pointer with complex signature (parentheses) */
  CALLBACK_TYPE(int, 
    (struct tree_node * GTY((skip)), 
     int (* GTY((skip))(int, int)))
  ) complex_handler;
};

/* Union type with GTY attributes containing string literals */
union GTY((desc("%0.type"),
          param_is(union typed_data))) typed_data {
  int int_val;
  double GTY((tag("1"))) double_val;
  char * GTY((tag("2"))) string_val;
  struct tree_node * GTY((tag("3"))) node_val;
};

/* Recursive structure definition */
struct GTY(()) graph_node {
  int id;
  struct graph_node * GTY((chain_next("%h.next"), 
                          chain_prev("%h.prev"))) 
    next, *prev;
  
  /* Array of pointers with GTY length attribute */
  struct graph_node ** GTY((length("neighbor_count"))) 
    neighbors;
  
  /* Union inside structure with tag */
  union {
    struct tree_node * GTY((tag("TREE"))) tree;
    union typed_data GTY((tag("DATA"))) data;
  } GTY((desc("%0.node_type"))) content;
};

/* Global variables with GTY markers */
extern struct tree_node * GTY(()) global_tree_root;
extern union typed_data GTY(()) global_data_array[];

/* Function pointer type with GTY skip */
typedef void (* GTY((skip)) 
  visitor_func)(struct tree_node *, 
                void * GTY((skip)) user_data);

#endif /* TEST_GTY_H */
