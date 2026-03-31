/* plugin_coverage.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
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
  return 0;
}

static bool
gate_dummy_pass (void)
{
  return true;
}

/* Define the dummy pass structure */
static struct gimple_opt_pass pass_dummy_pass =
{
  {
    GIMPLE_PASS,
    "dummy-pass",               /* name */
    OPTGROUP_NONE,              /* optinfo_flags */
    gate_dummy_pass,            /* gate */
    execute_dummy_pass,         /* execute */
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
static struct register_pass_info dummy_pass_info = {
  .pass = &pass_dummy_pass.pass,
  .reference_pass_name = "cfg",  /* Insert after CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO - Plugin Information Structure
   ============================================ */

static struct plugin_info my_plugin_info = {
  .version = "1.0",
  .help = "Test plugin for coverage of plugin.cc lines 458-470"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS - GGC Roots Table
   ============================================ */

/* Dummy structure for GGC roots */
static tree dummy_tree = NULL_TREE;

/* GGC roots table - must be NULL-terminated */
static const struct ggc_root_tab dummy_ggc_roots[] = {
  {
    .base = (void *)&dummy_tree,
    .nelt = 1,
    .stride = sizeof(tree),
    .cb = NULL,
    .pchw = NULL
  },
  { NULL, 0, 0, NULL, NULL }  /* NULL terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Verify GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version))
    return 1;
  
  printf("Plugin '%s' initializing...\n", plugin_name);
  
  /* ============================================
     Register callback for PLUGIN_PASS_MANAGER_SETUP
     ============================================ */
  register_callback (plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback function needed for registration */
                     &dummy_pass_info);
  
  /* ============================================
     Register callback for PLUGIN_INFO
     ============================================ */
  register_callback (plugin_name,
                     PLUGIN_INFO,
                     NULL,
                     &my_plugin_info);
  
  /* ============================================
     Register callback for PLUGIN_REGISTER_GGC_ROOTS
     ============================================ */
  register_callback (plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,
                     dummy_ggc_roots);
  
  printf("Plugin '%s' successfully registered all three event types\n", plugin_name);
  
  return 0;  /* Success */
}
