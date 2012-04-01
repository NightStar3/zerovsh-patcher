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
char _logfile[128];
static int stdout_output;

static SceUID debug_fd = -1;

void zeroCtrlInitDebug(int model, int always_opened, int use_stdout) {
	stdout_output = use_stdout;
	if(!use_stdout) {
		strcpy(_logfile, model == 4 ? LOGFILE_GO : LOGFILE);
		if(always_opened) {
			debug_fd = sceIoOpen(_logfile, PSP_O_CREAT | PSP_O_APPEND | PSP_O_WRONLY, 0777);
		}
	}
}

int kwrite(const char *path, void *buffer, int buflen) {
    int written = 0;
    int closed = 0;
    int k1 = pspSdkSetK1(0);

    if(stdout_output) {
    	written = sceIoWrite(1, buffer, buflen);
    } else {
		if(debug_fd < 0) {
			debug_fd = sceIoOpen(path, PSP_O_CREAT | PSP_O_APPEND | PSP_O_WRONLY, 0777);
			closed = 1;
		}
		if(debug_fd >= 0) {
			written = sceIoWrite(debug_fd, buffer, buflen);
			if(closed) {
				sceIoClose(debug_fd);
				debug_fd = -1;
			}
		}
    }
    pspSdkSetK1(k1);
    return written;
}

#endif
