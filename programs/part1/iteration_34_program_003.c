int main (int argc, char **argv)
{
  int opt;
  
  while ((opt = getopt (argc, argv, "hvplrs")) != -1)
    {
      switch (opt)
        {
        case 'h':
          print_usage ();
          exit (EXIT_SUCCESS);
          break;
        case 'v':
          print_version ();
          exit (EXIT_SUCCESS);
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
          fprintf (stderr, "unknown flag `%c'\n", opt);
          print_usage ();
          exit (EXIT_FAILURE);
        }
    }
  
  // Process remaining arguments (non-option arguments)
  for (int i = optind; i < argc; i++)
    {
      // Process file arguments here
      process_file (argv[i]);
    }
  
  return EXIT_SUCCESS;
}
