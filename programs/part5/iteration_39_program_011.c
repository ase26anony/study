#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree-pass.h"
#include "context.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible = 1;

/* Dummy pass definition */
static unsigned int dummy_pass_execute(void)
{
    return 0;
}

static bool dummy_pass_gate(void)
{
    return true;
}

const pass_data dummy_pass_data = {
    .type = GIMPLE_PASS,
    .name = "dummy",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
};

struct gimple_opt_pass dummy_pass = {
    .pass = {
        .type = GIMPLE_PASS,
        .name = "dummy",
        .optinfo_flags = OPTGROUP_NONE,
        .tv_id = TV_NONE,
        .properties_required = 0,
        .properties_provided = 0,
        .properties_destroyed = 0,
        .todo_flags_start = 0,
        .todo_flags_finish = 0,
        .execute = dummy_pass_execute,
        .gate = dummy_pass_gate,
    },
    .static_pass_number = 0,
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,
    .reference_pass_name = "ssa",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER,
};

/* Plugin info for PLUGIN_INFO */
struct plugin_info plugin_info = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin infrastructure",
};

/* Minimal GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = NULL,
        .nelt = 0,
        .stride = 0,
        .cb = NULL,
        .pchw = NULL,
    },
    { NULL, 0, 0, NULL, NULL } /* Terminator */
};

/* Plugin initialization function */
int plugin_init(struct plugin_name_args *plugin_info_args,
                struct plugin_gcc_version *version)
{
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        return 1;
    }

    /* Register for PLUGIN_INFO event */
    if (register_callback(plugin_info_args->base_name,
                          PLUGIN_INFO,
                          NULL,
                          &plugin_info)) {
        return 1;
    }

    /* Register for PLUGIN_PASS_MANAGER_SETUP event */
    if (register_callback(plugin_info_args->base_name,
                          PLUGIN_PASS_MANAGER_SETUP,
                          NULL,
                          &pass_info)) {
        return 1;
    }

    /* Register for PLUGIN_REGISTER_GGC_ROOTS event */
    if (register_callback(plugin_info_args->base_name,
                          PLUGIN_REGISTER_GGC_ROOTS,
                          NULL,
                          dummy_ggc_root_tab)) {
        return 1;
    }

    return 0; /* PLUGIN_SUCCESS */
}
