case 'h':
  print_usage ();
  break;
case 'v':
  print_version ();
  break;
case 'l':
  flag_dump_contents = 1;
  break;
case 'p':
  flag_dump_positions = 1;
  break;
case 'r':
  flag_dump_raw = 1;
  break;
case 's':
  flag_dump_stable = 1;
  break;
default:
  fprintf (stderr, "unknown flag `%c'\n", optopt);  // Changed opt to optopt
  exit (EXIT_FAILURE);  // Added exit call for error handling
