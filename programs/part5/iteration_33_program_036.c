/* coverage_plugin.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Required plugin metadata */
int plugin_is_GPL_compatible;

/* =========================================== */
/* 1. Data for PLUGIN_PASS_MANAGER_SETUP event */
/* =========================================== */

/* Simple dummy pass structure */
struct dummy_pass_data {
    struct opt_pass pass;
};

/* Dummy pass execution function */
static unsigned int
execute_dummy_pass (void)
{
    /* Do nothing, just return */
    return 0;
}

/* Create a dummy pass instance */
static struct dummy_pass_data dummy_pass_instance = {
    .pass = {
        .type = GIMPLE_PASS,
        .name = "dummy-coverage-pass",
        .optinfo_flags = OPTGROUP_NONE,
        .tv_id = TV_NONE,
        .properties_required = 0,
        .properties_provided = 0,
        .properties_destroyed = 0,
        .todo_flags_start = 0,
        .todo_flags_finish = 0,
        .execute = execute_dummy_pass,
    }
};

/* Register pass info structure */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass_instance.pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* =========================================== */
/* 2. Data for PLUGIN_INFO event */
/* =========================================== */

static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "Coverage test plugin for GCC plugin infrastructure\n"
            "This plugin triggers uncovered code paths in plugin.cc"
};

/* =========================================== */
/* 3. Data for PLUGIN_REGISTER_GGC_ROOTS event */
/* =========================================== */

/* Dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_pass_instance,
        .nelt = sizeof(dummy_pass_instance) / sizeof(void *),
        .stride = sizeof(void *),
        .cb = NULL,
        .pchw = NULL
    },
    { NULL, 0, 0, NULL, NULL }  /* Terminator */
};

/* =========================================== */
/* Plugin initialization function */
/* =========================================== */

int
plugin_init (struct plugin_name_args *plugin_info,
             struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check (version, &gcc_version))
        return 1;
    
    /* Register callback for PLUGIN_PASS_MANAGER_SETUP */
    register_callback (plugin_name, 
                       PLUGIN_PASS_MANAGER_SETUP,
                       NULL,  /* No callback function needed */
                       &pass_info);
    
    /* Register callback for PLUGIN_INFO */
    register_callback (plugin_name,
                       PLUGIN_INFO,
                       NULL,  /* No callback function needed */
                       &plugin_info_data);
    
    /* Register callback for PLUGIN_REGISTER_GGC_ROOTS */
    register_callback (plugin_name,
                       PLUGIN_REGISTER_GGC_ROOTS,
                       NULL,  /* No callback function needed */
                       dummy_ggc_root_tab);
    
    /* Also register for other events to ensure plugin is active */
    register_callback (plugin_name, PLUGIN_START_PARSE_FUNCTION, NULL, NULL);
    register_callback (plugin_name, PLUGIN_FINISH_PARSE_FUNCTION, NULL, NULL);
    
    return 0;  /* Success */
}
