case STATIC_ASSERT:
  if (location_t loc = STATIC_ASSERT_SOURCE_LOCATION (node)) {
    indent_to (file, indent+4);
    fprintf (file, "location: %d", loc);
  }
  print_node (file, "condition", STATIC_ASSERT_CONDITION (node), indent+4);
  print_node (file, "message", STATIC_ASSERT_MESSAGE (node), indent+4);
  break;
