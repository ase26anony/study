case TRAIT_EXPR:
  indent_to (file, indent+4);
  fprintf (file, "kind %d", TRAIT_EXPR_KIND (node));
  print_node (file, "type 1", TRAIT_EXPR_TYPE1 (node), indent+4);
  if (TRAIT_EXPR_TYPE2 (node))
    print_node (file, "type 2", TRAIT_EXPR_TYPE2 (node), indent+4);
  break;
