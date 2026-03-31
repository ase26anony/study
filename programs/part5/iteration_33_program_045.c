/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Targets: PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO, PLUGIN_REGISTER_GGC_ROOTS
 */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "config.h"
#include "system.h"
#include "coretypes.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin declaration */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
 * 1. PLUGIN_PASS_MANAGER_SETUP: Custom Pass
 * ============================================ */

/* Simple dummy pass structure */
static unsigned int
execute_my_pass (void)
{
    /* Do nothing, just a placeholder */
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
    .gate = gate_my_pass
};

static struct opt_pass *
make_my_pass (void)
{
    return &my_pass;
}

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info my_pass_info = 
{
    .pass = make_my_pass(),
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * 2. PLUGIN_INFO: Plugin Information
 * ============================================ */

static struct plugin_info my_plugin_info = 
{
    .version = "1.0",
    .help = "This plugin triggers uncovered code in GCC's plugin infrastructure.\n"
            "It registers a dummy pass, plugin info, and GGC roots."
};

/* ============================================
 * 3. PLUGIN_REGISTER_GGC_ROOTS: GGC Roots Table
 * ============================================ */

/* Dummy structure that will be garbage collected */
static tree dummy_tree = NULL_TREE;

/* GGC roots table - must be NULL-terminated */
static const struct ggc_root_tab my_ggc_roots[] = 
{
    {
        .base = (void *)&dummy_tree,
        .nelt = 1,
        .stride = sizeof(tree),
        .cb = NULL,
        .pchw = NULL
    },
    /* NULL terminator */
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
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback (plugin_name, 
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback function needed */
                      &my_pass_info);
    
    /* Register callback for PLUGIN_INFO */
    register_callback (plugin_name,
                      PLUGIN_INFO,
                      NULL,
                      &my_plugin_info);
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback (plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,
                      my_ggc_roots);
    
    /* Additional registration to ensure pass runs */
    register_callback (plugin_name,
                      PLUGIN_ATTRIBUTES,
                      NULL,
                      NULL);
    
    return 0;
}
