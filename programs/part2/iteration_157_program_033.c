struct GTY((chain_next ("chain_next"))) marked_struct {
  int field;
  struct marked_struct *chain_next;
};
