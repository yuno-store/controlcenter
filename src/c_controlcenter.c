/***********************************************************************
 *          C_CONTROLCENTER.C
 *          Controlcenter GClass.
 *
 *          Control Center of Yuneta Systems
 *
 *          Copyright (c) 2020 Niyamaka.
 *          All Rights Reserved.
 ***********************************************************************/
#include <string.h>
#include <stdio.h>
#include "c_controlcenter.h"

#include "schema_gest_controlcenter.c"

/***************************************************************************
 *              Constants
 ***************************************************************************/

/***************************************************************************
 *              Structures
 ***************************************************************************/

/***************************************************************************
 *              Prototypes
 ***************************************************************************/

/***************************************************************************
 *          Data: config, public data, private data
 ***************************************************************************/
PRIVATE json_t *cmd_help(hgobj gobj, const char *cmd, json_t *kw, hgobj src);

PRIVATE sdata_desc_t pm_help[] = {
/*-PM----type-----------name------------flag------------default-----description---------- */
SDATAPM (ASN_OCTET_STR, "cmd",          0,              0,          "command about you want help."),
SDATAPM (ASN_UNSIGNED,  "level",        0,              0,          "command search level in childs"),
SDATA_END()
};

PRIVATE const char *a_help[] = {"h", "?", 0};

PRIVATE sdata_desc_t command_table[] = {
/*-CMD---type-----------name----------------alias---------------items-----------json_fn---------description---------- */
SDATACM (ASN_SCHEMA,    "help",             a_help,             pm_help,        cmd_help,       "Command's help"),
SDATA_END()
};


/*---------------------------------------------*
 *      Attributes - order affect to oid's
 *---------------------------------------------*/
PRIVATE sdata_desc_t tattr_desc[] = {
/*-ATTR-type------------name----------------flag----------------default-----description---------- */
SDATA (ASN_OCTET_STR,   "company",          SDF_RD,             "default",  "Company"),

SDATA (ASN_COUNTER64,   "txMsgs",           SDF_RD|SDF_PSTATS,  0,          "Messages transmitted"),
SDATA (ASN_COUNTER64,   "rxMsgs",           SDF_RD|SDF_RSTATS,  0,          "Messages receiveds"),

SDATA (ASN_COUNTER64,   "txMsgsec",         SDF_RD|SDF_RSTATS,  0,          "Messages by second"),
SDATA (ASN_COUNTER64,   "rxMsgsec",         SDF_RD|SDF_RSTATS,  0,          "Messages by second"),
SDATA (ASN_COUNTER64,   "maxtxMsgsec",      SDF_WR|SDF_RSTATS,  0,          "Max Tx Messages by second"),
SDATA (ASN_COUNTER64,   "maxrxMsgsec",      SDF_WR|SDF_RSTATS,  0,          "Max Rx Messages by second"),

SDATA (ASN_INTEGER,     "timeout",          SDF_RD,             1*1000,     "Timeout"),
SDATA (ASN_POINTER,     "user_data",        0,                  0,          "user data"),
SDATA (ASN_POINTER,     "user_data2",       0,                  0,          "more user data"),
SDATA_END()
};

/*---------------------------------------------*
 *      GClass trace levels
 *---------------------------------------------*/
enum {
    TRACE_MESSAGES = 0x0001,
};
PRIVATE const trace_level_t s_user_trace_level[16] = {
{"messages",        "Trace messages"},
{0, 0},
};


/*---------------------------------------------*
 *              Private data
 *---------------------------------------------*/
typedef struct _PRIVATE_DATA {
    hgobj timer;
    int32_t timeout;

    hgobj gobj_top_side;
    hgobj treedb_gest;

    hgobj gobj_tranger;
    json_t *tranger;

    uint64_t *ptxMsgs;
    uint64_t *prxMsgs;
    uint64_t txMsgsec;
    uint64_t rxMsgsec;
} PRIVATE_DATA;




            /******************************
             *      Framework Methods
             ******************************/




/***************************************************************************
 *      Framework Method create
 ***************************************************************************/
PRIVATE void mt_create(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    helper_quote2doublequote(schema_gest_controlcenter);

    /*
     *  Chequea schema fichador, exit si falla.
     */
    json_t *jn_schema_gest_controlcenter;
    jn_schema_gest_controlcenter = legalstring2json(schema_gest_controlcenter, TRUE);
    if(!jn_schema_gest_controlcenter) {
        exit(-1);
    }

    priv->timer = gobj_create(gobj_name(gobj), GCLASS_TIMER, 0, gobj);
    priv->ptxMsgs = gobj_danger_attr_ptr(gobj, "txMsgs");
    priv->prxMsgs = gobj_danger_attr_ptr(gobj, "rxMsgs");

    /*---------------------------*
     *  Create Timeranger
     *---------------------------*/
    const char *company = gobj_read_str_attr(gobj, "company");
    if(empty_string(company)) {
        log_critical(LOG_OPT_EXIT_ZERO,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_PARAMETER_ERROR,
            "msg",          "%s", "Attribute 'company' is REQUIRED!",
            NULL
        );
    }
    char path[PATH_MAX];
    snprintf(path, sizeof(path),
        "/yuneta/store/controlcenter/%s/%s",
        company,
        gobj_yuno_role_plus_name()
    );
    json_t *kw_tranger = json_pack("{s:s, s:s, s:b, s:i}",
        "path", path,
        "filename_mask", "%Y",
        "master", 1,
        "on_critical_error", (int)(LOG_OPT_EXIT_ZERO)
    );
    priv->gobj_tranger = gobj_create_service(
        "tranger",
        GCLASS_TRANGER,
        kw_tranger,
        gobj
    );

    /*----------------------*
     *  Create Treedb
     *----------------------*/
    const char *treedb_name = kw_get_str(
        jn_schema_gest_controlcenter,
        "id",
        "gest_controlcenter",
        KW_REQUIRED
    );
    json_t *kw_resource = json_pack("{s:s, s:o, s:i}",
        "treedb_name", treedb_name,
        "treedb_schema", jn_schema_gest_controlcenter,
        "exit_on_error", LOG_OPT_EXIT_ZERO
    );

    priv->treedb_gest = gobj_create_service(
        treedb_name,
        GCLASS_NODE,
        kw_resource,
        gobj
    );

    /*
     *  Do copy of heavy used parameters, for quick access.
     *  HACK The writable attributes must be repeated in mt_writing method.
     */
    SET_PRIV(timeout,               gobj_read_int32_attr)
}

/***************************************************************************
 *      Framework Method writing
 ***************************************************************************/
PRIVATE void mt_writing(hgobj gobj, const char *path)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    IF_EQ_SET_PRIV(timeout,             gobj_read_int32_attr)
    END_EQ_SET_PRIV()
}

/***************************************************************************
 *      Framework Method destroy
 ***************************************************************************/
PRIVATE void mt_destroy(hgobj gobj)
{
}

/***************************************************************************
 *      Framework Method start
 ***************************************************************************/
PRIVATE int mt_start(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    gobj_start(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method stop
 ***************************************************************************/
PRIVATE int mt_stop(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    gobj_stop(priv->timer);
    return 0;
}

/***************************************************************************
 *      Framework Method play
 *  Yuneta rule:
 *  If service has mt_play then start only the service gobj.
 *      (Let mt_play be responsible to start their tree)
 *  If service has not mt_play then start the tree with gobj_start_tree().
 ***************************************************************************/
PRIVATE int mt_play(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*
     *  Start tranger/treedb
     */
    gobj_start(priv->gobj_tranger);
    priv->tranger = gobj_read_pointer_attr(priv->gobj_tranger, "tranger");
    gobj_write_pointer_attr(priv->treedb_gest, "tranger", priv->tranger);
    gobj_start(priv->treedb_gest);

    /*
     *  Start __top_side__
     */
    priv->gobj_top_side = gobj_find_service("__top_side__", TRUE);
    gobj_subscribe_event(priv->gobj_top_side, 0, 0, gobj);
    gobj_start_tree(priv->gobj_top_side);

    return 0;
}

/***************************************************************************
 *      Framework Method pause
 ***************************************************************************/
PRIVATE int mt_pause(hgobj gobj)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    /*
     *  Stop __top_side__
     */
    gobj_unsubscribe_event(priv->gobj_top_side, 0, 0, gobj);
    EXEC_AND_RESET(gobj_stop_tree, priv->gobj_top_side);

    /*
     *  Stop treeb/tranger
     */
    gobj_stop(priv->treedb_gest);
    gobj_stop(priv->gobj_tranger);
    priv->tranger = 0;

    clear_timeout(priv->timer);
    return 0;
}




            /***************************
             *      Commands
             ***************************/




/***************************************************************************
 *
 ***************************************************************************/
PRIVATE json_t *cmd_help(hgobj gobj, const char *cmd, json_t *kw, hgobj src)
{
    KW_INCREF(kw);
    json_t *jn_resp = gobj_build_cmds_doc(gobj, kw);
    return msg_iev_build_webix(
        gobj,
        0,
        jn_resp,
        0,
        0,
        kw  // owned
    );
}




            /***************************
             *      Local Methods
             ***************************/




            /***************************
             *      Actions
             ***************************/




/***************************************************************************
 *  Identity_card on from
 *      Web clients (__top_side__)
 ***************************************************************************/
PRIVATE int ac_on_open(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    if(src != priv->gobj_top_side) {
        log_error(0,
            "gobj",         "%s", gobj_full_name(gobj),
            "function",     "%s", __FUNCTION__,
            "msgset",       "%s", MSGSET_INTERNAL_ERROR,
            "msg",          "%s", "on_open NOT from GOBJ_TOP_SIDE",
            "src",          "%s", gobj_full_name(src),
            NULL
        );
    }

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *  Identity_card off from
 *      Web clients (__top_side__)
 ***************************************************************************/
PRIVATE int ac_on_close(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *
 ***************************************************************************/
PRIVATE int ac_timeout(hgobj gobj, const char *event, json_t *kw, hgobj src)
{
    PRIVATE_DATA *priv = gobj_priv_data(gobj);

    uint64_t maxtxMsgsec = gobj_read_uint64_attr(gobj, "maxtxMsgsec");
    uint64_t maxrxMsgsec = gobj_read_uint64_attr(gobj, "maxrxMsgsec");
    if(priv->txMsgsec > maxtxMsgsec) {
        gobj_write_uint64_attr(gobj, "maxtxMsgsec", priv->txMsgsec);
    }
    if(priv->rxMsgsec > maxrxMsgsec) {
        gobj_write_uint64_attr(gobj, "maxrxMsgsec", priv->rxMsgsec);
    }

    gobj_write_uint64_attr(gobj, "txMsgsec", priv->txMsgsec);
    gobj_write_uint64_attr(gobj, "rxMsgsec", priv->rxMsgsec);

    priv->rxMsgsec = 0;
    priv->txMsgsec = 0;

    KW_DECREF(kw);
    return 0;
}

/***************************************************************************
 *                          FSM
 ***************************************************************************/
PRIVATE const EVENT input_events[] = {
    // top input
    {"EV_ON_OPEN",              0,  0,  0},
    {"EV_ON_CLOSE",             0,  0,  0},
    // bottom input
    {"EV_TIMEOUT",              0,  0,  0},
    {"EV_STOPPED",              0,  0,  0},
    // internal
    {NULL, 0, 0, ""}
};
PRIVATE const EVENT output_events[] = {
    {NULL, 0, 0, ""}
};
PRIVATE const char *state_names[] = {
    "ST_IDLE",
    NULL
};

PRIVATE EV_ACTION ST_IDLE[] = {
    {"EV_ON_OPEN",                  ac_on_open,                 0},
    {"EV_ON_CLOSE",                 ac_on_close,                0},
    {"EV_TIMEOUT",                  ac_timeout,             0},
    {"EV_STOPPED",                  0,                      0},
    {0,0,0}
};

PRIVATE EV_ACTION *states[] = {
    ST_IDLE,
    NULL
};

PRIVATE FSM fsm = {
    input_events,
    output_events,
    state_names,
    states,
};

/***************************************************************************
 *              GClass
 ***************************************************************************/
/*---------------------------------------------*
 *              Local methods table
 *---------------------------------------------*/
PRIVATE LMETHOD lmt[] = {
    {0, 0, 0}
};

/*---------------------------------------------*
 *              GClass
 *---------------------------------------------*/
PRIVATE GCLASS _gclass = {
    0,  // base
    GCLASS_CONTROLCENTER_NAME,
    &fsm,
    {
        mt_create,
        0, //mt_create2,
        mt_destroy,
        mt_start,
        mt_stop,
        mt_play,
        mt_pause,
        mt_writing,
        0, //mt_reading,
        0, //mt_subscription_added,
        0, //mt_subscription_deleted,
        0, //mt_child_added,
        0, //mt_child_removed,
        0, //mt_stats,
        0, //mt_command_parser,
        0, //mt_inject_event,
        0, //mt_create_resource,
        0, //mt_list_resource,
        0, //mt_update_resource,
        0, //mt_delete_resource,
        0, //mt_add_child_resource_link
        0, //mt_delete_child_resource_link
        0, //mt_get_resource
        0, //mt_authorization_parser,
        0, //mt_authenticate,
        0, //mt_list_childs,
        0, //mt_stats_updated,
        0, //mt_disable,
        0, //mt_enable,
        0, //mt_trace_on,
        0, //mt_trace_off,
        0, //mt_gobj_created,
        0, //mt_future33,
        0, //mt_future34,
        0, //mt_publish_event,
        0, //mt_publication_pre_filter,
        0, //mt_publication_filter,
        0, //mt_authz_checker,
        0, //mt_future39,
        0, //mt_create_node,
        0, //mt_update_node,
        0, //mt_delete_node,
        0, //mt_link_nodes,
        0, //mt_future44,
        0, //mt_unlink_nodes,
        0, //mt_future46,
        0, //mt_get_node,
        0, //mt_list_nodes,
        0, //mt_shoot_snap,
        0, //mt_activate_snap,
        0, //mt_list_snaps,
        0, //mt_treedbs,
        0, //mt_treedb_topics,
        0, //mt_topic_desc,
        0, //mt_topic_links,
        0, //mt_topic_hooks,
        0, //mt_node_parents,
        0, //mt_node_childs,
        0, //mt_node_instances,
        0, //mt_save_node,
        0, //mt_topic_size,
        0, //mt_future62,
        0, //mt_future63,
        0, //mt_future64
    },
    lmt,
    tattr_desc,
    sizeof(PRIVATE_DATA),
    0,  // acl
    s_user_trace_level,
    command_table,  // command_table
    0,  // gcflag
};

/***************************************************************************
 *              Public access
 ***************************************************************************/
PUBLIC GCLASS *gclass_controlcenter(void)
{
    return &_gclass;
}
