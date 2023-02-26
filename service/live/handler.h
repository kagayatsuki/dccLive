/*
 * Copyright (c) shinsya.G 
 */

#ifndef DCCLIVE_HANDLER_H
#define DCCLIVE_HANDLER_H

#include "../router/router.h"
#include "../../auth/admin.h"

int live_info(RequestRouter &router, Admin *auth);
int live_list(RequestRouter &router, Admin *auth);
int live_new(RequestRouter &router, Admin *auth);
int live_delete(RequestRouter &router, Admin *auth);
int live_export_access(RequestRouter &router, Admin *auth);

#endif //DCCLIVE_HANDLER_H
