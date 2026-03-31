/* plugin_coverage.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass my_pass;
static void register_my_pass(void);

/* ============================================
   PART 1: PLUGIN_PASS_MANAGER_SETUP
   ============================================ */

/* Simple dummy pass that does nothing */
static unsigned int
execute_my_pass (void)
{
  /* This pass does nothing, just returns */
  return 0;
}

static bool
gate_my_pass (void)
{
  /* Always enable this pass */
  return true;
}

static struct opt_pass my_pass =
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
  .execute = execute_my_pass,
  .gate = gate_my_pass,
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = {
  .pass = &my_pass,
  .reference_pass_name = "cfg",  /* Insert after the CFG pass */
  .ref_pass_instance_number = 1,
  .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PART 2: PLUGIN_INFO
   ============================================ */

static struct plugin_info my_plugin_info = {
  .version = "1.0",
  .help = "Test plugin for coverage analysis\n"
          "This plugin triggers uncovered code in GCC's plugin infrastructure."
};

/* ============================================
   PART 3: PLUGIN_REGISTER_GGC_ROOTS
   ============================================ */

/* Dummy GGC root table entry */
static GTY(()) tree dummy_tree = NULL_TREE;

static const struct ggc_root_tab my_ggc_roots[] = {
  {
    .base = (void *)&dummy_tree,
    .nelt = 1,
    .stride = sizeof(dummy_tree),
    .cb = NULL,
    .pchw = NULL
  },
  /* Terminator */
  { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   PLUGIN INITIALIZATION FUNCTION
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
     REGISTER CALLBACKS FOR THE THREE EVENTS
     ============================================ */
  
  /* 1. Register PLUGIN_PASS_MANAGER_SETUP event */
  register_callback(plugin_name, 
                    PLUGIN_PASS_MANAGER_SETUP, 
                    NULL,  /* No callback needed for registration */
                    &my_pass_info);
  
  /* 2. Register PLUGIN_INFO event */
  register_callback(plugin_name,
                    PLUGIN_INFO,
                    NULL,
                    &my_plugin_info);
  
  /* 3. Register PLUGIN_REGISTER_GGC_ROOTS event */
  register_callback(plugin_name,
                    PLUGIN_REGISTER_GGC_ROOTS,
                    NULL,
                    my_ggc_roots);
  
  printf("Plugin '%s' registered all three target events\n", plugin_name);
  
  return 0; /* Success */
}
