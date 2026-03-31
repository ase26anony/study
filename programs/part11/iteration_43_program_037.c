default:
  fprintf(stderr, "unknown flag `%c'\n", opt);
  fprintf(stderr, "Try `%s -h' for more information.\n", program_name);
  exit(EXIT_FAILURE);
  break;
