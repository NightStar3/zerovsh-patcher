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


//Global for blacklisting
int g_blacklisted = 0;


//OK
void zeroCtrlSetBlackListItems(char *item, const char *list[], int nitems)
{
	int i;

	zeroCtrlWriteDebug("Blacklist items: %d\n", nitems);

	for(i = 0; i < nitems; i++)
	{
		zeroCtrlWriteDebug("-> Item %d: %s\n", i + 1, list[i]);

		if(!strcmp(item, list[i]))
		{			
			g_blacklisted = 1;
		}
	}	
}
//OK
int zeroCtrlIsBlacklistedFound(void)
{
	if(g_blacklisted == 1)
	{
		zeroCtrlWriteDebug("Found\n\n");
		return 1;
	}

	return 0;
}