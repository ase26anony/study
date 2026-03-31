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

/* Required for GCC plugin compatibility */
int plugin_is_GPL_compatible = 1;

/* Forward declarations */
static struct opt_pass *make_my_pass(void);

/* ============================================
 * 1. Data for PLUGIN_PASS_MANAGER_SETUP
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

static struct gimple_opt_pass my_pass =
{
    .pass.type = GIMPLE_PASS,
    .pass.name = "my-dummy-pass",
    .pass.gate = gate_my_pass,
    .pass.execute = execute_my_pass,
    .pass.todo_flags_start = 0,
    .pass.todo_flags_finish = 0
};

static struct opt_pass *
make_my_pass (void)
{
    return &my_pass.pass;
}

/* Register pass info structure */
static struct register_pass_info my_pass_info = {
    .pass = make_my_pass(),
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * 2. Data for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage analysis"
};

/* ============================================
 * 3. Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&my_pass_info,
        .nelt = 1,
        .stride = sizeof(struct register_pass_info),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator */
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
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    
    /* ============================================
     * Register callback for PLUGIN_PASS_MANAGER_SETUP
     * This triggers the first uncovered case
     * ============================================ */
    register_callback (plugin_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback needed - infrastructure handles it */
                      &my_pass_info);
    
    /* ============================================
     * Register callback for PLUGIN_INFO
     * This triggers the second uncovered case
     * ============================================ */
    register_callback (plugin_name,
                      PLUGIN_INFO,
                      NULL,
                      &my_plugin_info);
    
    /* ============================================
     * Register callback for PLUGIN_REGISTER_GGC_ROOTS
     * This triggers the third uncovered case
     * ============================================ */
    register_callback (plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,
                      dummy_ggc_root_tab);
    
    return 0;
}
