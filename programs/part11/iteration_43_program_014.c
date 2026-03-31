case '?':  // getopt already handles unknown options
  // getopt prints its own error, but you might want custom handling
  print_usage();
  exit(EXIT_FAILURE);
  break;
