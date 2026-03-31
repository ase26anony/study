/*
 * GCC Plugin to trigger uncovered code in plugin.cc
 * Specifically targets:
 * - PLUGIN_PASS_MANAGER_SETUP
 * - PLUGIN_INFO
 * - PLUGIN_REGISTER_GGC_ROOTS
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

/* Global plugin name */
static const char *plugin_name = "coverage_plugin";

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Define a simple dummy pass for registration */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always enable this pass */
    return true;
}

static struct opt_pass dummy_pass = {
    .type = GIMPLE_PASS,
    .name = "dummy-coverage-pass",
    .optinfo_flags = OPTGROUP_NONE,
    .tv_id = TV_NONE,
    .properties_required = 0,
    .properties_provided = 0,
    .properties_destroyed = 0,
    .todo_flags_start = 0,
    .todo_flags_finish = 0,
    .gate = dummy_pass_gate,
    .execute = dummy_pass_execute,
    .sub = NULL,
    .next = NULL,
    .static_pass_number = 0
};

/* Create register_pass_info structure for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass,
    .reference_pass_name = "cfg",
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

/* Create plugin_info structure for PLUGIN_INFO */
static struct plugin_info plugin_info_data = {
    .version = "1.0",
    .help = "This plugin triggers uncovered code in GCC's plugin infrastructure"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Define a dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_pass,
        .nelt = 1,
        .stride = sizeof(struct opt_pass),
        .cb = NULL,
        .pchw = NULL
    },
    /* Required NULL terminator */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    int ret = 0;
    
    /* Check GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin requires GCC %s\n", gcc_version.basever);
        return 1;
    }
    
    /* Store plugin name */
    plugin_name = plugin_info->base_name;
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback function needed - infrastructure handles it */
        &pass_info
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_PASS_MANAGER_SETUP callback\n");
        return ret;
    }
    
    /* ============================================
       Register callback for PLUGIN_INFO
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback function needed */
        &plugin_info_data
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_INFO callback\n");
        return ret;
    }
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    ret = register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback function needed */
        dummy_ggc_root_tab
    );
    
    if (ret != 0) {
        fprintf(stderr, "Failed to register PLUGIN_REGISTER_GGC_ROOTS callback\n");
        return ret;
    }
    
    /* Optional: Register additional callbacks to ensure plugin is active */
    ret = register_callback(
        plugin_name,
        PLUGIN_START_PARSE_FUNCTION,
        NULL,  /* Dummy callback */
        NULL
    );
    
    printf("Coverage plugin initialized successfully\n");
    return 0;
}
