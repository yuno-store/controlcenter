/****************************************************************************
 *          MAIN_CONTROLCENTER.C
 *          controlcenter main
 *
 *          Control Center of Yuneta Systems
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ****************************************************************************/
#include <yuneta_tls.h>
#include "c_controlcenter.h"
#include "yuno_controlcenter.h"

/***************************************************************************
 *                      Names
 ***************************************************************************/
#define APP_NAME        ROLE_CONTROLCENTER
#define APP_DOC         "Control Center of Yuneta Systems"

#define APP_VERSION     "4.3.4"
#define APP_SUPPORT     "<niyamaka at yuneta.io>"
#define APP_DATETIME    __DATE__ " " __TIME__

/***************************************************************************
 *                      Default config
 ***************************************************************************/
PRIVATE char fixed_config[]= "\
{                                                                   \n\
    'yuno': {                                                       \n\
        'yuno_role': '"ROLE_CONTROLCENTER"',                        \n\
        'classifiers': ['realm', 'app']                             \n\
    }                                                               \n\
}                                                                   \n\
";
PRIVATE char variable_config[]= "\
{                                                                   \n\
    'environment': {                                                \n\
        'use_system_memory': true,                                  \n\
        'log_gbmem_info': true,                                     \n\
        'MEM_MIN_BLOCK': 32,                                        \n\
        'MEM_MAX_BLOCK': 65536,             #^^ 64*K                \n\
        'MEM_SUPERBLOCK': 131072,           #^^ 128*K               \n\
        'MEM_MAX_SYSTEM_MEMORY': 1048576,   #^^ 1*M                 \n\
        'console_log_handlers': {                                   \n\
            'to_stdout': {                                          \n\
                'handler_type': 'stdout',                           \n\
                'handler_options': 255                              \n\
            }                                                       \n\
        },                                                          \n\
        'daemon_log_handlers': {                                    \n\
            'to_file': {                                            \n\
                'handler_type': 'file',                             \n\
                'filename_mask': 'controlcenter-MM-DD.log',         \n\
                'handler_options': 255                              \n\
            },                                                      \n\
            'to_udp': {                                             \n\
                'handler_type': 'udp',                              \n\
                'url': 'udp://127.0.0.1:1992',                      \n\
                'handler_options': 255                              \n\
            }                                                       \n\
        }                                                           \n\
    },                                                              \n\
    'yuno': {                                                       \n\
        'required_services': [],                                    \n\
        'public_services': [],                                      \n\
        'service_descriptor': {                                     \n\
        },                                                          \n\
        'trace_levels': {                                           \n\
            'Tcp0': ['connections'],                                \n\
            'TcpS0': ['listen', 'not-accepted', 'accepted'],        \n\
            'Tcp1': ['connections'],                                \n\
            'TcpS1': ['listen', 'not-accepted', 'accepted']         \n\
        }                                                           \n\
    },                                                              \n\
    'global': {                                                     \n\
    },                                                              \n\
    'services': [                                                   \n\
        {                                                           \n\
            'name': 'controlcenter',                                \n\
            'gclass': 'Controlcenter',                              \n\
            'default_service': true,                                \n\
            'autostart': true,                                      \n\
            'autoplay': false,                                      \n\
            'kw': {                                                 \n\
            },                                                      \n\
            'zchilds': [                                            \n\
            ]                                                       \n\
        },                                                          \n\
        {                                                           \n\
            'name': 'authz',                                        \n\
            'gclass': 'Authz',                                      \n\
            'default_service': false,                               \n\
            'autostart': true,                                      \n\
            'autoplay': true,                                       \n\
            'kw': {                                                 \n\
                'jwt_public_key': '-----BEGIN PUBLIC KEY-----\\nMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAj0ZkOtmWlsDLJiJWXTEJ\\ntyXxlVY7iLseG982eaFgDaAdtE3Z5J+WDvzni7v4MPR55oMyi/oeAvTKIsVOv3aw\\nobRJ/Mr45Qh6I0j4Hn+rFfPW4wbmxRmFeyRrfMzYAZZoibZ3m7EFlj2RINvJFIgE\\npIoTf4UneXmlSDbUU9MTZe1mULfCfEZVa5V9W86BluAAib1mYJU7aJ20KPkbQAvW\\nXqC82AE9ga66HnQ2n56mv1kPyvNGKvvM6vD2IXQeLIYgudYT2KlGKd8isrOEkrno\\nXtPKMSaRhVccO73Wbo7krhjGV5MTpMvvOM+wDprslFkODm0MORsHORVxfcVGWSpU\\ngQIDAQAB\\n-----END PUBLIC KEY-----\\n', \n\
                'initial_load': [                                       \n\
                    {                                                   \n\
                        'id': 'yuneta',             #^^ user            \n\
                        'system_user': true,                            \n\
                        'permissions': [                                \n\
                            {                                           \n\
                                'id': '*',          #^^ gclass/service  \n\
                                'allow': true,                          \n\
                                'constraints': {}                       \n\
                            }                                           \n\
                        ]                                               \n\
                    },                                                  \n\
                    {                                                   \n\
                        'id': 'ginsmar@mulesol.es', #^^ user            \n\
                        'system_user': false,                           \n\
                        'permissions': [                                \n\
                            {                                           \n\
                                'id': '*',          #^^ gclass/service  \n\
                                'allow': true,                          \n\
                                'constraints': {}                       \n\
                            }                                           \n\
                        ]                                               \n\
                    }                                                   \n\
                ]                                                       \n\
            },                                                      \n\
            'zchilds': [                                            \n\
            ]                                                       \n\
        }                                                           \n\
    ]                                                               \n\
}                                                                   \n\
";

/***************************************************************************
 *                      Register
 ***************************************************************************/
static void register_yuno_and_more(void)
{
    /*------------------------*
     *  Register yuneta-tls
     *------------------------*/
    yuneta_register_c_tls();

    /*-------------------*
     *  Register yuno
     *-------------------*/
    register_yuno_controlcenter();

    /*--------------------*
     *  Register service
     *--------------------*/
    gobj_register_gclass(GCLASS_CONTROLCENTER);
}

/***************************************************************************
 *                      Main
 ***************************************************************************/
int main(int argc, char *argv[])
{
    /*------------------------------------------------*
     *  To trace memory
     *------------------------------------------------*/
#ifdef DEBUG
    static uint32_t mem_list[] = {0};
    gbmem_trace_alloc_free(0, mem_list);
#endif

    /*------------------------------------------------*
     *          Traces
     *------------------------------------------------*/
    gobj_set_gclass_no_trace(GCLASS_TIMER, "machine", TRUE);  // Avoid timer trace, too much information

// Samples
//     gobj_set_gclass_trace(GCLASS_IEVENT_CLI, "ievents2", TRUE);
//     gobj_set_gclass_trace(GCLASS_IEVENT_SRV, "ievents2", TRUE);
//     gobj_set_gclass_trace(GCLASS_CONTROLCENTER, "messages", TRUE);

//     gobj_set_gobj_trace(0, "create_delete", TRUE, 0);
//     gobj_set_gobj_trace(0, "create_delete2", TRUE, 0);
//     gobj_set_gobj_trace(0, "start_stop", TRUE, 0);
//     gobj_set_gobj_trace(0, "subscriptions", TRUE, 0);
//     gobj_set_gobj_trace(0, "machine", TRUE, 0);
//     gobj_set_gobj_trace(0, "ev_kw", TRUE, 0);
//     gobj_set_gobj_trace(0, "libuv", TRUE, 0);

    //set_auto_kill_time(5);

    /*------------------------------------------------*
     *          Start yuneta
     *------------------------------------------------*/
    helper_quote2doublequote(fixed_config);
    helper_quote2doublequote(variable_config);
    yuneta_setup(
        dbattrs_startup,
        dbattrs_end,
        dbattrs_load_persistent,
        dbattrs_save_persistent,
        dbattrs_remove_persistent,
        dbattrs_list_persistent,
        command_parser,
        stats_parser,
        authz_checker,
        authz_allow,
        authz_deny,
        authzs_list
    );
    return yuneta_entry_point(
        argc, argv,
        APP_NAME, APP_VERSION, APP_SUPPORT, APP_DOC, APP_DATETIME,
        fixed_config,
        variable_config,
        register_yuno_and_more
    );
}
