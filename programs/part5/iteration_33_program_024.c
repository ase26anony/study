/* coverage_plugin.c - GCC plugin to trigger uncovered plugin.cc events */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP - Custom Pass Definition
   ============================================ */

/* Simple dummy pass structure */
static struct gimple_opt_pass dummy_pass =
{
  {
    GIMPLE_PASS,
    "dummy_pass",               /* name */
    OPTGROUP_NONE,              /* optinfo_flags */
    NULL,                       /* gate */
    NULL,                       /* execute */
    NULL,                       /* sub */
    NULL,                       /* next */
    0,                          /* static_pass_number */
    TV_NONE,                    /* tv_id */
    0,                          /* properties_required */
    0,                          /* properties_provided */
    0,                          /* properties_destroyed */
    0,                          /* todo_flags_start */
    0                           /* todo_flags_finish */
  }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
  .pass = &dummy_pass.pass,     /* Reference to our dummy pass */
  .reference_pass_name = "cfg", /* Insert after the CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Coverage test plugin for plugin.cc uncovered lines\n"
          "This plugin triggers PLUGIN_PASS_MANAGER_SETUP,\n"
          "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Root Table
   ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_tree = NULL_TREE;
static int dummy_int = 0;

/* GGC root table with one dummy entry, terminated with NULL */
static const struct ggc_root_tab dummy_ggc_roots[] = {
  {
    .base = (void *)&dummy_tree,  /* Base pointer */
    .nelt = 1,                    /* Number of elements */
    .stride = sizeof(tree),       /* Size of each element */
    .cb = NULL,                   /* Optional callback */
    .pchw = NULL                  /* PCH handling */
  },
  {
    .base = (void *)&dummy_int,
    .nelt = 1,
    .stride = sizeof(int),
    .cb = NULL,
    .pchw = NULL
  },
  { NULL, 0, 0, NULL, NULL }     /* Terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Verify GCC version compatibility */
  if (!plugin_default_version_check(version, &gcc_version)) {
    fprintf(stderr, "Error: Plugin version mismatch\n");
    return 1;
  }
  
  printf("Coverage plugin '%s' initializing...\n", plugin_name);
  
  /* ============================================
     Trigger PLUGIN_PASS_MANAGER_SETUP event
     ============================================ */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,  /* No callback needed - infrastructure handles it */
                    &pass_info);
  
  /* ============================================
     Trigger PLUGIN_INFO event
     ============================================ */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,  /* No callback needed */
                    &plugin_info_data);
  
  /* ============================================
     Trigger PLUGIN_REGISTER_GGC_ROOTS event
     ============================================ */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,  /* No callback needed */
                    dummy_ggc_roots);
  
  printf("Coverage plugin registered all three target events\n");
  
  return 0;  /* Success */
}
