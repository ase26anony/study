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
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Simple dummy pass for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* Do nothing - just a placeholder pass */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always enable this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = {
    {
        GIMPLE_PASS,
        "dummy-pass",           /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        dummy_pass_gate,        /* gate */
        dummy_pass_execute,     /* execute */
        NULL,                   /* sub */
        NULL,                   /* next */
        0,                      /* static_pass_number */
        TV_NONE,                /* tv_id */
        0,                      /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    }
};

/* Pass registration info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info dummy_pass_info = {
    .pass = &dummy_pass.pass,           /* Reference to our pass */
    .reference_pass_name = "cfg",       /* Insert after CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER     /* Position: insert after */
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Coverage plugin for testing GCC plugin infrastructure\n"
            "This plugin triggers uncovered code in plugin.cc\n"
            "Specifically targets PLUGIN_PASS_MANAGER_SETUP,\n"
            "PLUGIN_INFO, and PLUGIN_REGISTER_GGC_ROOTS events."
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy structure for GGC roots */
static struct dummy_ggc_struct {
    int value;
    tree node;
} dummy_ggc_data;

/* GGC root table for PLUGIN_REGISTER_GGC_ROOTS */
static const struct ggc_root_tab dummy_ggc_roots[] = {
    {
        .base = (void *)&dummy_ggc_data,
        .nelt = sizeof(dummy_ggc_data) / sizeof(void *),
        .stride = sizeof(void *),
        .cb = NULL,
        .pchw = NULL
    },
    /* Terminator - required by GCC */
    { NULL, 0, 0, NULL, NULL }
};

/* ============================================
   Plugin Initialization Function
   ============================================ */

int plugin_init(struct plugin_name_args *plugin_info,
                struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    
    /* Verify GCC version compatibility */
    if (!plugin_default_version_check(version, &gcc_version)) {
        fprintf(stderr, "Error: Plugin version mismatch\n");
        return 1;
    }
    
    printf("Coverage plugin '%s' initializing...\n", plugin_name);
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_PASS_MANAGER_SETUP,
        NULL,  /* No callback function needed - infrastructure handles it */
        &dummy_pass_info
    );
    
    printf("  Registered PLUGIN_PASS_MANAGER_SETUP event\n");
    
    /* ============================================
       Register callback for PLUGIN_INFO
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_INFO,
        NULL,  /* No callback function needed */
        &plugin_metadata
    );
    
    printf("  Registered PLUGIN_INFO event\n");
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       ============================================ */
    register_callback(
        plugin_name,
        PLUGIN_REGISTER_GGC_ROOTS,
        NULL,  /* No callback function needed */
        dummy_ggc_roots
    );
    
    printf("  Registered PLUGIN_REGISTER_GGC_ROOTS event\n");
    
    printf("Coverage plugin initialization complete\n");
    
    return 0; /* Success */
}
