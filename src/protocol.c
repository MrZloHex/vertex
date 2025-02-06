/* =============================================================================
 *                          ░▒▓█ _VERTEX_ █▓▒░
 *
 *   File       : protocol.c
 *   Author     : MrZloHex
 *   Date       : 2025-02-01
 *
 *   Description:
 *      MLP - MonoLith's Protocol
 *
 * =============================================================================
 */

#include "protocol.h"

// #include "trace.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char *MLP_MsgType_STR[] =
{
    "UNKNOWN", "CMD", "LOG", "OK", "ERR"
};

const char *MLP_MsgAction_STR[] =
{
    "UNKNOWN", "REG", "GET", "SET", "HRT"
};

static MLP_MsgType
mlp_get_msg_type(char *msg)
{
    char *end = strchr(msg, ':');
    // if (!end)
    // { TRACE_ERROR("FAIELD TO PARSE TYPE"); }

    *end = 0;
    // TRACE_DEBUG("TYPE\t`%s`", msg);
    MLP_MsgType type = MT_UNKNOWN;
    
    if (strcmp(msg, "CMD") == 0)
    { type = MT_CMD; }
    if (strcmp(msg, "LOG") == 0)
    { type = MT_LOG; }
    if (strcmp(msg, "OK") == 0)
    { type = MT_OK; }
    if (strcmp(msg, "ERR") == 0)
    { type = MT_ERR; }

    *end = ':';
    return type;
}

static
MLP_MsgAction
mlp_get_msg_action(char *msg)
{
    msg = strchr(msg, ':') +1;
    char *end = strchr(msg, ':');
    // if (!end || !msg)
    // { TRACE_ERROR("FAILED TO PARSE ACTION"); }

    *end = 0;
    // TRACE_DEBUG("ACT\t\t`%s`", msg);
    MLP_MsgAction action = MA_UNKNOWN;
    
    if (strcmp(msg, "REG") == 0)
    { action = MA_REG; }
    if (strcmp(msg, "GET") == 0)
    { action = MA_GET; }
    if (strcmp(msg, "SET") == 0)
    { action = MA_SET; }
    if (strcmp(msg, "HRT") == 0)
    { action = MA_HRT; }

    *end = ':';
    return action;
}

static void
mlp_get_msg_params(MLP_Msg *mlp, char *msg)
{
    msg = strchr(msg, ':') +1;
    msg = strchr(msg, ':') +1;
    // if (!msg)
    // { TRACE_ERROR("FAIELD TO PARSE PARAMS"); }

    for (size_t i = 0; i < MAX_PARAMS; ++i)
    {
        char *end = strchr(msg, ':');
        if (!end && !*msg)
        { break; }
        else if (!end && *msg)
        { end = strrchr(msg, 0); }

        *end = 0;
        // TRACE_DEBUG("PARAM\t`%s`", msg);
        
        mlp->params[i] = msg;
        msg = end+1;
    }
}

MLP_Msg
mlp_parse_msg(char *msg)
{
    MLP_MsgType   type   = mlp_get_msg_type(msg);
    MLP_MsgAction action = mlp_get_msg_action(msg);

    // if (!type || !action)
    // { TRACE_ERROR("FAILED TO PARSE MSG"); }

    MLP_Msg mlp =
    {
        .type   = type,
        .action = action
    };

    mlp_get_msg_params(&mlp, msg);

    return mlp;
}

char *
mlp_make_msg(MLP_Msg msg)
{
    char *buffer = malloc(128);
    // if (!buffer)
    // { TRACE_ERROR("FAILED TO ALLOC MSG"); }

    int n = snprintf
    (
        buffer, 128, "%s:%s",
        MLP_MsgType_STR[msg.type],
        MLP_MsgAction_STR[msg.action]
    );

    for (size_t i = 0; i < MAX_PARAMS; ++i)
    {
        if (!msg.params[i])
        { continue; }

        n += snprintf
        (
            buffer + n, 128 - n, ":%s",
            msg.params[i]
        );
    }

    snprintf(buffer + n, 128 -n, "\n");

    return buffer;
}



