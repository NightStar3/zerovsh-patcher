/*
 * This file is part of ZeroVSH Patcher.

 * ZeroVSH Patcher is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * ZeroVSH Patcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ZeroVSH Patcher. If not, see <http://www.gnu.org/licenses/ .
 */

//Our header
#include "blacklist.h"
#include "resolver.h"

//Global for blacklisting
int g_blacklisted = 0;

static const char *g_blacklist_mod[] = {
        "sceHVNetfront_Module",
};

//OK
void zeroCtrlSetBlackListItems(char *item) {
    for (int i = 0; i < ITEMSOF(g_blacklist_mod); i++) {
        zeroCtrlWriteDebug("-> Item %d: %s\n", i + 1, g_blacklist_mod[i]);

        if (!strcmp(item, g_blacklist_mod[i])) {
            g_blacklisted = 1;
        }
    }
}
//OK
int zeroCtrlIsBlacklistedFound(void) {
    if (g_blacklisted == 1) {
        zeroCtrlWriteDebug("Found\n\n");
        return 1;
    }

    return 0;
}
