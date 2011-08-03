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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <string.h>
#include <stdio.h>

#ifdef DEBUG

#define LOGFILE "ms0:/PSP/VSH/debug_patcher.txt"

extern char _buffer_log[256];

void zeroCtrlInitDebug();
int kwrite(const char *path, void *buffer, int buflen);

#define zeroCtrlWriteDebug(format, ...) do { \
    sprintf(_buffer_log, "%s: "format, __func__, ## __VA_ARGS__); \
    kwrite(LOGFILE, _buffer_log, strlen(_buffer_log)); \
} while(0)

#else

#define zeroCtrlInitDebug()
#define zeroCtrlWriteDebug(format, ...)
// uncomment this to use the logger with psplink
//#define zeroCtrlWriteDebug(format, ...) printf(format, ## __VA_ARGS__)

#endif

#endif /* LOGGER_H_ */
