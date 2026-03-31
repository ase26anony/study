/*
 * GCC Plugin to trigger uncovered lines in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "intl.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Data Structure
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int
execute_my_pass (void)
{
  /* Do nothing - just a dummy pass */
  return 0;
}

static bool
gate_my_pass (void)
{
  /* Always run this pass */
  return true;
}

static struct gimple_opt_pass my_pass =
{
  {
    GIMPLE_PASS,
    "my_dummy_pass",           /* name */
    gate_my_pass,              /* gate */
    execute_my_pass,           /* execute */
    NULL,                      /* sub */
    NULL,                      /* next */
    0,                         /* static_pass_number */
    TV_NONE,                   /* tv_id */
    0,                         /* properties_required */
    0,                         /* properties_provided */
    0,                         /* properties_destroyed */
    0,                         /* todo_flags_start */
    0                          /* todo_flags_finish */
  }
};

static struct opt_pass *
make_my_pass (void)
{
  return &my_pass.pass;
}

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
  .pass = &my_pass.pass,
  .reference_pass_name = "cfg",  /* Insert after CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Data Structure
   ============================================ */

static struct plugin_info my_plugin_info = {
  .version = "1.0",
  .help = "Coverage plugin for testing GCC plugin infrastructure\n"
          "Triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Data Structure
   ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
  {
    .base = (void *)&my_pass,  /* Point to something */
    .nelt = 1,
    .stride = sizeof(my_pass),
    .cb = NULL,
    .pchw = NULL
  },
  { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP,
                    NULL,  /* No callback function needed for registration */
                    &my_pass_info);
  
  /* Register callback for PLUGIN_INFO */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,
                    &my_plugin_info);
  
  /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,
                    dummy_ggc_root_tab);
  
  return 0;  /* Success */
}
