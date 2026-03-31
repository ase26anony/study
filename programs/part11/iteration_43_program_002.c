case '?':
  // Handle invalid option or missing argument
  fprintf(stderr, "Try '%s -h' for more information.\n", program_name);
  exit(EXIT_FAILURE);
  break;
