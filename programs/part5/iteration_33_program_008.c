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
static void my_pass_execute(void);

/* ============================================
 * 1. Data for PLUGIN_PASS_MANAGER_SETUP
 * ============================================ */

/* Simple dummy pass structure */
struct my_pass_data {
    struct opt_pass pass;
};

/* Pass execution function */
static void
my_pass_execute(void)
{
    /* Do nothing - just a dummy pass for coverage */
    if (dump_file)
        fprintf(dump_file, "My dummy pass executed\n");
}

/* Create the pass instance */
static struct opt_pass *
make_my_pass(void)
{
    struct my_pass_data *pass_data;
    
    pass_data = XNEW(struct my_pass_data);
    
    pass_data->pass.type = GIMPLE_PASS;
    pass_data->pass.name = "my-dummy-pass";
    pass_data->pass.optinfo_flags = OPTGROUP_NONE;
    pass_data->pass.tv_id = TV_NONE;
    pass_data->pass.properties_required = 0;
    pass_data->pass.properties_provided = 0;
    pass_data->pass.properties_destroyed = 0;
    pass_data->pass.todo_flags_start = 0;
    pass_data->pass.todo_flags_finish = 0;
    pass_data->pass.execute = my_pass_execute;
    
    return &pass_data->pass;
}

/* Register pass info structure */
static struct register_pass_info my_pass_info = {
    .pass = NULL,  /* Will be set in plugin_init */
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
 * 2. Data for PLUGIN_INFO
 * ============================================ */

static struct plugin_info my_plugin_info = {
    .version = "1.0",
    .help = "GCC coverage plugin for testing plugin infrastructure\n"
            "This plugin triggers PLUGIN_PASS_MANAGER_SETUP, PLUGIN_INFO,\n"
            "and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
 * 3. Data for PLUGIN_REGISTER_GGC_ROOTS
 * ============================================ */

/* Dummy structure that GCC will track for garbage collection */
static struct dummy_ggc_struct {
    int data;
    tree some_tree;
} *dummy_ggc_ptr = NULL;

/* GGC root table entry */
static const struct ggc_root_tab my_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_ptr,
        .nelt = 1,
        .stride = sizeof(struct dummy_ggc_struct *),
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
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
     * Trigger PLUGIN_PASS_MANAGER_SETUP
     * ============================================ */
    
    /* Create and set up the pass */
    my_pass_info.pass = make_my_pass();
    
    /* Register callback for pass manager setup */
    register_callback(plugin_name,
                     PLUGIN_PASS_MANAGER_SETUP,
                     NULL,  /* No callback needed - infrastructure handles it */
                     &my_pass_info);
    
    /* ============================================
     * Trigger PLUGIN_INFO
     * ============================================ */
    
    register_callback(plugin_name,
                     PLUGIN_INFO,
                     NULL,  /* No callback needed */
                     &my_plugin_info);
    
    /* ============================================
     * Trigger PLUGIN_REGISTER_GGC_ROOTS
     * ============================================ */
    
    register_callback(plugin_name,
                     PLUGIN_REGISTER_GGC_ROOTS,
                     NULL,  /* No callback needed */
                     (void *)my_ggc_roots);
    
    /* Additional callback to verify plugin is active */
    register_callback(plugin_name,
                     PLUGIN_START_PARSE_FUNCTION,
                     NULL,  /* Simple NULL callback for demonstration */
                     NULL);
    
    printf("Plugin %s registered all target events\n", plugin_name);
    
    return 0;  /* Success */
}
