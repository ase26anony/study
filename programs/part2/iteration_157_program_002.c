struct GTY(()) marked_struct {
  int field;
  struct marked_struct *GTY((skip)) next;  /* skip this field in GC */
  tree GTY((tag("TREE_TYPE"))) type_field; /* tree node with tag */
};
