/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc code */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass that does nothing */
static unsigned int
execute_dummy_pass (void)
{
  /* This pass does nothing, just for demonstration */
  return 0;
}

/* Define the dummy pass structure */
static struct gimple_opt_pass dummy_pass =
{
  {
    GIMPLE_PASS,          /* type */
    "dummy-pass",         /* name */
    OPTGROUP_NONE,        /* optinfo_flags */
    NULL,                 /* gate */
    execute_dummy_pass,   /* execute */
    NULL,                 /* sub */
    NULL,                 /* next */
    0,                    /* static_pass_number */
    TV_NONE,              /* tv_id */
    0,                    /* properties_required */
    0,                    /* properties_provided */
    0,                    /* properties_destroyed */
    0,                    /* todo_flags_start */
    0                     /* todo_flags_finish */
  }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass.pass,           /* Reference to our pass */
  .reference_pass_name = "cfg",       /* Insert after the CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER     /* Insert after the reference pass */
};

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Coverage plugin for testing GCC plugin infrastructure\n"
          "This plugin triggers uncovered code in plugin.cc\n"
          "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
          "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Roots Table
   ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_tree = NULL_TREE;

/* GGC roots table - must be NULL-terminated */
static const struct ggc_root_tab dummy_ggc_roots[] = {
  {
    .base = (void *)&dummy_tree,  /* Base pointer */
    .nelt = 1,                    /* Number of elements */
    .stride = sizeof(tree),       /* Size of each element */
    .cb = NULL,                   /* Optional callback */
    .pchw = NULL                  /* Optional PCH callback */
  },
  { NULL, 0, 0, NULL, NULL }     /* NULL terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Check GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version))
    return 1;
  
  /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
  register_callback (plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,           /* No callback function needed */
                     &pass_info);
  
  /* Register callback for PLUGIN_INFO */
  register_callback (plugin_name,
                     PLUGIN_INFO,
                     NULL,           /* No callback function needed */
                     &plugin_info_data);
  
  /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
  register_callback (plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,           /* No callback function needed */
                     dummy_ggc_roots);
  
  /* Also register for other events to ensure plugin is active */
  register_callback (plugin_name, PLUGIN_START_PARSE_FUNCTION, NULL, NULL);
  register_callback (plugin_name, PLUGIN_FINISH_PARSE_FUNCTION, NULL, NULL);
  
  return 0; /* Success */
}
