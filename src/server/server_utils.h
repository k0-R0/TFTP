#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "commons/commons.h"

Status handle_connect(FileContext *ctx);
Status handle_get(FileContext *ctx);
Status handle_put(FileContext *ctx);
Status handle_mode(FileContext *ctx, TransferMode *server_mode);
Status handle_quit(FileContext *ctx);

#endif
