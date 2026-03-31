/* test_plugin.c - GCC plugin to test specific plugin infrastructure code */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

int plugin_is_GPL_compatible;

/* Dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    return 0;
}

static bool dummy_pass_gate(void)
{
    return true;
}

static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-pass",
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
    .tv_id = TV_NONE,
};

/* Pass info structure for registration */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* Plugin info for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin infrastructure code"
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = NULL,
        .nelt = 0,
        .stride = 0,
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL } /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    int rc;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }
    
    /* Register for PLUGIN_PASS_MANAGER_SETUP */
    rc = plugin_register(plugin_info->base_name,
                         PLUGIN_PASS_MANAGER_SETUP,
                         &dummy_pass_info);
    if (rc != PLUGIN_SUCCESS) {
        return rc;
    }
    
    /* Register for PLUGIN_INFO */
    rc = plugin_register(plugin_info->base_name,
                         PLUGIN_INFO,
                         &plugin_info_data);
    if (rc != PLUGIN_SUCCESS) {
        return rc;
    }
    
    /* Register for PLUGIN_REGISTER_GGC_ROOTS */
    rc = plugin_register(plugin_info->base_name,
                         PLUGIN_REGISTER_GGC_ROOTS,
                         dummy_ggc_root_tab);
    if (rc != PLUGIN_SUCCESS) {
        return rc;
    }
    
    return PLUGIN_SUCCESS;
}
