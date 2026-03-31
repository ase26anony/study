case ARGUMENT_PACK_SELECT:
  indent_to (file, indent + 3);
  fprintf (file, "index %d", ARGUMENT_PACK_SELECT_INDEX (node));
  break;
