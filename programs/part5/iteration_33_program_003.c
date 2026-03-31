/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events
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

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int
execute_my_pass (void)
{
    /* This pass does nothing, just for demonstration */
    return 0;
}

static bool
gate_my_pass (void)
{
    /* Always run this pass */
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

static struct opt_pass *
make_my_pass (void)
{
    return &my_pass;
}

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info my_plugin_info = 
{
    .version = "1.0",
    .help = "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets:\n"
            "  - PLUGIN_PASS_MANAGER_SETUP\n"
            "  - PLUGIN_INFO\n"
            "  - PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy structure for GGC roots */
static GTY(()) tree dummy_tree = NULL_TREE;

static const struct ggc_root_tab my_ggc_roots[] = 
{
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
   Plugin Initialization
   ============================================ */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    struct register_pass_info pass_info;
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    
    /* ============================================
       Trigger PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    
    /* Initialize pass registration info */
    memset(&pass_info, 0, sizeof(pass_info));
    pass_info.pass = make_my_pass();
    pass_info.reference_pass_name = "cfg";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback(plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP, 
                      NULL,  /* No callback function needed */
                      &pass_info);
    
    /* ============================================
       Trigger PLUGIN_INFO
       ============================================ */
    
    /* Register callback for PLUGIN_INFO */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed */
                      &my_plugin_info);
    
    /* ============================================
       Trigger PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed */
                      my_ggc_roots);
    
    return 0;
}
