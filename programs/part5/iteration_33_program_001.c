/* plugin_coverage.c - GCC plugin to trigger uncovered code in plugin.cc */
#include "gcc-plugin.h"
#include "plugin-version.h"
#include "tree.h"
#include "tree-pass.h"
#include "context.h"
#include "gimple.h"
#include "cgraph.h"
#include "ggc.h"

/* Mandatory plugin license declaration */
int plugin_is_GPL_compatible = 1;

/* ============================================
   PLUGIN_PASS_MANAGER_SETUP Implementation
   ============================================ */

/* Dummy pass structure for PLUGIN_PASS_MANAGER_SETUP */
static unsigned int dummy_pass_execute(void)
{
    /* This pass does nothing */
    return 0;
}

static bool dummy_pass_gate(void)
{
    /* Always run this pass */
    return true;
}

static struct gimple_opt_pass dummy_pass = {
    {
        GIMPLE_PASS,
        "dummy-plugin-pass",           /* name */
        OPTGROUP_NONE,                 /* optinfo_flags */
        dummy_pass_gate,               /* gate */
        dummy_pass_execute,            /* execute */
        NULL,                          /* sub */
        NULL,                          /* next */
        0,                             /* static_pass_number */
        TV_NONE,                       /* tv_id */
        0,                             /* properties_required */
        0,                             /* properties_provided */
        0,                             /* properties_destroyed */
        0,                             /* todo_flags_start */
        0                              /* todo_flags_finish */
    }
};

/* Register pass info for PLUGIN_PASS_MANAGER_SETUP */
static struct register_pass_info pass_info = {
    .pass = &dummy_pass.pass,          /* Reference to our dummy pass */
    .reference_pass_name = "cfg",      /* Insert after the CFG pass */
    .ref_pass_instance_number = 1,
    .pos_op = PASS_POS_INSERT_AFTER    /* Position: insert after reference pass */
};

/* ============================================
   PLUGIN_INFO Implementation
   ============================================ */

static struct plugin_info plugin_metadata = {
    .version = "1.0",
    .help = "Test plugin for coverage of plugin.cc lines 458-470\n"
            "This plugin triggers three specific event types:\n"
            "1. PLUGIN_PASS_MANAGER_SETUP\n"
            "2. PLUGIN_INFO\n"
            "3. PLUGIN_REGISTER_GGC_ROOTS"
};

/* ============================================
   PLUGIN_REGISTER_GGC_ROOTS Implementation
   ============================================ */

/* Dummy GGC root table entry */
static const struct ggc_root_tab dummy_ggc_root_tab[] = {
    {
        .base = (void *)&dummy_pass,   /* Base pointer */
        .nelt = 1,                     /* Number of elements */
        .stride = sizeof(dummy_pass),  /* Size of each element */
        .cb = NULL,                    /* No callback */
        .pchw = NULL                   /* No PCH handling */
    },
    { NULL, 0, 0, NULL, NULL }        /* Terminator (required) */
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
        fprintf(stderr, "Plugin %s: incompatible GCC version\n", plugin_name);
        return 1;
    }
    
    printf("Plugin %s initializing...\n", plugin_name);
    
    /* ============================================
       Register callback for PLUGIN_PASS_MANAGER_SETUP
       This triggers lines 458-461 in plugin.cc
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_PASS_MANAGER_SETUP,
                      NULL,  /* No callback function needed for registration */
                      &pass_info);
    
    /* ============================================
       Register callback for PLUGIN_INFO
       This triggers lines 462-465 in plugin.cc
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_INFO,
                      NULL,  /* No callback function needed for registration */
                      &plugin_metadata);
    
    /* ============================================
       Register callback for PLUGIN_REGISTER_GGC_ROOTS
       This triggers lines 466-470 in plugin.cc
       ============================================ */
    register_callback(plugin_name,
                      PLUGIN_REGISTER_GGC_ROOTS,
                      NULL,  /* No callback function needed for registration */
                      dummy_ggc_root_tab);
    
    printf("Plugin %s successfully registered all three event types\n", plugin_name);
    printf("  - PLUGIN_PASS_MANAGER_SETUP with dummy pass\n");
    printf("  - PLUGIN_INFO with version/help metadata\n");
    printf("  - PLUGIN_REGISTER_GGC_ROOTS with dummy GGC root table\n");
    
    return 0; /* Success */
}
