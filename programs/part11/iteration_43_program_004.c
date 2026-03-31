case 'h':
  print_usage();
  exit(EXIT_SUCCESS);  // Exit after help
  break;
case 'v':
  print_version();
  exit(EXIT_SUCCESS);  // Exit after version
  break;
// ... other cases ...
default:
  fprintf(stderr, "unknown option `-%c'\n", opt);
  print_usage();  // Show usage on error
  exit(EXIT_FAILURE);  // Exit with error code
