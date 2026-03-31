/* test_plugin.c - GCC plugin to test uncovered plugin infrastructure code */

#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

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
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
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
    /* Terminator entry */
    { NULL, 0, 0, NULL, NULL }
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }

    /* Register for PLUGIN_INFO event */
    if (register_callback(plugin_info->base_name,
                          PLUGIN_INFO,
                          NULL,
                          &plugin_info_data)) {
        fprintf(stderr, "Failed to register PLUGIN_INFO callback\n");
        return 1;
    }

    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    if (register_callback(plugin_info->base_name,
                          PLUGIN_PASS_MANAGER_SETUP,
                          NULL,
                          &dummy_pass_info)) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP callback\n");
        return 1;
    }

    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    if (register_callback(plugin_info->base_name,
                          PLUGIN_REGISTER_GGC_ROOTS,
                          NULL,
                          dummy_ggc_root_tab)) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS callback\n");
        return 1;
    }

    return 0; /* PLUGIN_SUCCESS */
}
