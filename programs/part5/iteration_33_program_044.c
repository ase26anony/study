/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
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

/* Forward declarations */
static struct opt_pass my_pass_instance;
static struct ggc_root_tab dummy_roots[];

/* ============================================
 * 1. Custom Pass for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Gate function - always returns true to enable the pass */
static bool
my_pass_gate (void)
{
  return true;
}

/* Execute function - does nothing but required */
static unsigned int
my_pass_exec (void)
{
  /* This is a dummy pass that does nothing */
  return 0;
}

/* Define our custom pass */
static struct opt_pass my_pass = 
{
  GIMPLE_PASS,               /* type */
  "my-dummy-pass",           /* name */
  OPTGROUP_NONE,             /* optinfo_flags */
  my_pass_gate,              /* gate */
  my_pass_exec,              /* execute */
  NULL,                      /* sub */
  NULL,                      /* next */
  0,                         /* static_pass_number */
  0,                         /* tv_id */
  0,                         /* properties_required */
  0,                         /* properties_provided */
  0,                         /* properties_destroyed */
  0,                         /* todo_flags_start */
  0                          /* todo_flags_finish */
};

static struct opt_pass my_pass_instance = my_pass;

/* Pass registration info */
static struct register_pass_info my_pass_info = 
{
  &my_pass_instance,         /* pass */
  "ssa",                     /* reference_pass_name */
  1,                         /* ref_pass_instance_number */
  PASS_POS_INSERT_AFTER      /* pos_op */
};

/* ============================================
 * 2. Plugin Info for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = 
{
  "1.0",                     /* version */
  "This plugin triggers uncovered code in GCC's plugin infrastructure"  /* help */
};

/* ============================================
 * 3. GGC Roots for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy static variable to use as GGC root */
static tree dummy_tree = NULL_TREE;

static struct ggc_root_tab dummy_roots[] = 
{
  {
    &dummy_tree,             /* base */
    1,                       /* nelt */
    sizeof(tree),            /* stride */
    ggc_mark_tree_node,      /* mark */
    NULL,                    /* pchw */
    NULL                     /* relocate */
  },
  { NULL, 0, 0, NULL, NULL, NULL }  /* Terminator */
};

/* ============================================
 * Main Plugin Initialization
 * ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
  const char *plugin_name = plugin_info->base_name;
  
  /* Check GCC version compatibility */
  if (!plugin_default_version_check (version, &gcc_version))
    return 1;
  
  /* ============================================
   * Register callback for PLUGIN_PASS_MANAGER_SETUP
   * ============================================ */
  register_callback (plugin_name, 
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* callback - NULL for registration events */
                     &my_pass_info);
  
  /* ============================================
   * Register callback for PLUGIN_INFO
   * ============================================ */
  register_callback (plugin_name,
                     PLUGIN_INFO,
                     NULL,
                     &my_plugin_info);
  
  /* ============================================
   * Register callback for PLUGIN_REGISTER_GGC_ROOTS
   * ============================================ */
  register_callback (plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,
                     dummy_roots);
  
  return 0;
}
