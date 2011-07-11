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

#include <pspsdk.h>
#include <pspiofilemgr.h>
#include "logger.h"

#ifdef DEBUG

char _buffer_log[256];
SceUID debug_fd = -1;

void zeroCtrlInitDebug(void) {
	debug_fd = sceIoOpen(LOGFILE, PSP_O_CREAT | PSP_O_APPEND | PSP_O_WRONLY, 0777);
}

int kwrite(const char *path, void *buffer, int buflen) {
    int written = 0;
    int close = 0;
    int k1 = pspSdkSetK1(0);
    if(debug_fd < 0) {
    	zeroCtrlInitDebug();
    	close = 1;
    }
    if(debug_fd >= 0) {
        written = sceIoWrite(debug_fd, buffer, buflen);
        if(close) {
        	sceIoClose(debug_fd);
        	debug_fd = -1;
        }
    }
    pspSdkSetK1(k1);
    return written;
}

#endif
