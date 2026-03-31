/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "plugin-version.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declaration for our dummy pass */
static struct opt_pass my_dummy_pass;

/* ============================================
 * 1. Data for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Dummy gate function (always returns true) */
static bool
my_pass_gate (void)
{
  return true;
}

/* Dummy execute function */
static unsigned int
my_pass_execute (void)
{
  /* Do nothing - just a dummy pass */
  return 0;
}

/* Define our dummy pass structure */
static struct opt_pass my_dummy_pass = 
{
  .type = GIMPLE_PASS,
  .name = "my-dummy-pass",
  .optinfo_flags = OPTGROUP_NONE,
  .tv_id = TV_NONE,
  .properties_required = 0,
  .properties_provided = 0,
  .properties_destroyed = 0,
  .todo_flags_start = 0,
  .todo_flags_finish = 0,
  .gate = my_pass_gate,
  .execute = my_pass_execute,
  .sub = NULL,
  .next = NULL,
  .static_pass_number = 0
};

/* Register pass info structure */
static struct register_pass_info my_pass_info = 
{
  .pass = &my_dummy_pass,
  .reference_pass_name = "cfg",
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * 2. Data for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = 
{
  .version = "1.0",
  .help = "This plugin triggers uncovered code in GCC's plugin infrastructure"
};

/* ============================================
 * 3. Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab my_ggc_root_tab[] = 
{
  {
    .base = (void *)&my_dummy_pass,
    .nelt = 1,
    .stride = sizeof(struct opt_pass),
    .cb = NULL,
    .pchw = NULL
  },
  /* Required NULL terminator */
  { NULL, 0, 0, NULL, NULL }
};

/* ============================================
 * Plugin Initialization Function
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Verify GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version))
    return 1;
  
  /* ============================================
   * Register callback for PLUGIN_PASS_MANAGER_SETUP
   * ============================================ */
  register_callback (plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP, 
                     NULL,  /* No callback function needed */
                     &my_pass_info);
  
  /* ============================================
   * Register callback for PLUGIN_INFO
   * ============================================ */
  register_callback (plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* No callback function needed */
                     &my_plugin_info);
  
  /* ============================================
   * Register callback for PLUGIN_REGISTER_GGC_ROOTS
   * ============================================ */
  register_callback (plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* No callback function needed */
                     my_ggc_root_tab);
  
  return 0; /* Success */
}
