default:
  fprintf(stderr, "unknown flag `%c'\n", opt);
  print_usage();
  exit(EXIT_FAILURE);  // Add exit to prevent continuing with bad input
