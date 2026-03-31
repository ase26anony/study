default:
  fprintf(stderr, "Error: Unknown option `%c'\n", opt);
  print_usage();
  exit(EXIT_FAILURE);
  break;
