/*
 * GCC plugin to trigger uncovered code in plugin.cc
 * Specifically targets:
 * - PLUGIN_PASS_MANAGER_SETUP
 * - PLUGIN_INFO
 * - PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
 * 1. Data for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Define a simple dummy pass */
static unsigned int
execute_my_pass (void)
{
  /* Do nothing, just return */
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
    "my-dummy-pass",           /* name */
    OPTGROUP_NONE,             /* optinfo_flags */
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

/* Register pass info structure */
static struct register_pass_info pass_info = {
  .pass = &my_pass.pass,
  .reference_pass_name = "cfg",  /* Insert after the CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * 2. Data for PLUGIN_INFO
 * ============================================ */

static struct plugin_info plugin_info_data = {
  .version = "1.0",
  .help = "Coverage plugin for testing GCC plugin infrastructure\n"
          "This plugin triggers three specific events to cover code in plugin.cc"
};

/* ============================================
 * 3. Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Define a dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
  {
    .base = (void *)&my_pass,  /* Point to something that exists */
    .nelt = 1,                 /* One element */
    .stride = sizeof(my_pass), /* Size of each element */
    .cb = NULL,                /* No callback */
    .pchw = NULL               /* No PCH handling */
  },
  { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* ============================================
 * Plugin initialization function
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP, 
                    NULL,  /* No callback function needed */
                    &pass_info);
  
  /* Register callback for PLUGIN_INFO */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,
                    &plugin_info_data);
  
  /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,
                    dummy_ggc_root_tab);
  
  /* Also register for other events to ensure plugin is active */
  register_callback(plugin_name, PLUGIN_START_PARSE_FUNCTION, NULL, NULL);
  register_callback(plugin_name, PLUGIN_FINISH_PARSE_FUNCTION, NULL, NULL);
  
  return 0; /* Success */
}
