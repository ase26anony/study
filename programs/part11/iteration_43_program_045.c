default:
  fprintf(stderr, "Error: unknown option '-%c'\n", opt);
  print_usage();
  exit(EXIT_FAILURE);
