/*
 *  By Per Jessen <per@computer.org> with changes by the ClamAV team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301, USA.
 */

#if HAVE_CONFIG_H
#include "clamav-config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <string.h>
#include <errno.h>

#include "output.h"
#include "optparser.h"
#include "execute.h"

#if defined(C_OS2)
#include <process.h>
#define _P_NOWAIT P_NOWAIT
#endif

#define MAX_CHILDREN 5

int g_active_children;

void execute(const char *type, const char *text, int bDaemonized)
{
    int ret;

    if (!bDaemonized) {
        if (sscanf(text, "EXIT_%d", &ret) == 1) {
            logg("*%s: EXIT_%d\n", type, ret);
            exit(ret);
        }
        if (system(text) == -1)
            logg("%s: system(%s) failed\n", type, text);

        return;
    }

#if defined(_WIN32) || defined(C_OS2)
    if (system(text) == -1) {
        logg("^%s: couldn't execute \"%s\".\n", type, text);
        return;
    }
#else
    if (g_active_children < MAX_CHILDREN) {
        pid_t pid;
        switch (pid = fork()) {
            case 0:
                if (-1 == system(text)) {
                    logg("^%s: couldn't execute \"%s\".\n", type, text);
                }
                exit(0);
            case -1:
                logg("^%s::fork() failed, %s.\n", type, strerror(errno));
                break;
            default:
                g_active_children++;
        }
    } else {
        logg("^%s: already %d processes active.\n", type, g_active_children);
    }
#endif
}
