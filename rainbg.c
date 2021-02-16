/*
 * 	$FishBone Corp. rainbg.c,v 1.0 2021/02/16 16:09 FishBone.EarthKhan$	*/
/*
 * This redistribution is originally from binary executable "rain v1.17" of bsdgames
 * ubuntu package.
 * The original binary and source code has the copyright of University of
 * California, and the copyright infomation is down below.
 * The author of this redistribution has no legal knowledge. Anyone who uses this 
 * redistribution with any mean, including, but not limited to executing, modifying, 
 * redistributing, should follow the original license and copyright.
 */


//	START OF ORIGINAL COPYRIGHT INFOMATION
//
/*	$NetBSD: rain.c,v 1.17 2004/05/02 21:31:23 christos Exp $	*/

/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
//
//	END OF THE ORIGINAL COPYRIGHT INFOMATION

#include <sys/cdefs.h>
#ifndef lint
//	###	BELOW COPYRIGHT IS COMMENTED OUT DUE TO COMPILE ERROR
//__COPYRIGHT("@(#) Copyright (c) 1980, 1993\n\
	The Regents of the University of California.  All rights reserved.\n");
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)rain.c	8.1 (Berkeley) 5/31/93";
#else
//	###	SO IS THIS.
//__RCSID("$NetBSD: rain.c,v 1.17 2004/05/02 21:31:23 christos Exp $");
#endif
#endif /* not lint */

/*
 * rain 11/3/1980 EPS/CITHEP
 * cc rain.c -o rain -O -ltermlib
 */

#include <sys/types.h>
#include <curses.h>
#include <err.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <fstream>
#include <string>
#include <list>
#include <cstring>
#include <iostream>

using namespace std;

static volatile sig_atomic_t sig_caught = 0;
long cols, lines;
int bgheight = -1;
int dbg_cnt = 0;
string *bgp;

int main(int, char **);
void bgwrt(int, int, string);
static void onsig(int);


int
main(int argc, char **argv)
{
	int x, y, j;
	unsigned int delay = 100000;
	unsigned long val = 0;
	int ch;
	char *ep;
	int xpos[5], ypos[5];
	const string path = (string)getenv("HOME") + "/.rainbg";

	/* Revoke setgid privileges */
	setregid(getgid(), getgid());

	while ((ch = getopt(argc, argv, "d:")) != -1)
		switch (ch) {
		case 'd':
			val = strtoul(optarg, &ep, 0);
			if (ep == optarg || *ep)
				errx(1, "Invalid delay `%s'", optarg);
			if (errno == ERANGE && val == ULONG_MAX)
				err(1, "Invalid delay `%s'", optarg);
			if (val >= 1000)
				errx(1, "Invalid delay `%s' (1-999)", optarg);
			delay = (unsigned int)val * 1000;  /* ms -> us */
			break;
		default:
			(void)fprintf(stderr, "Usage: %s [-d delay]\n",
			    "rainbg");
			return 1;
		}

	initscr();
	ifstream read(path);
	string line;
	char spaces[COLS] = {' '};
	memset(spaces, ' ', sizeof(spaces));
	list<string> bgl;
	while(read.good() && getline(read, line)){
		bgheight++;
		line = (line + spaces).substr(0, COLS);
		bgl.push_front(line);
	}
	string bg[bgheight+1];
	for(int i = 0; i <= bgheight; i++){
		bg[i] = bgl.front();
		mvaddstr(LINES-1-i, 0, bg[i].c_str());
		bgl.pop_front();
	}
	bgp = bg;

	cols = COLS - 4;
	lines = LINES - 4;
	
	(void)signal(SIGHUP, onsig);
	(void)signal(SIGINT, onsig);
	(void)signal(SIGTERM, onsig);

	curs_set(0);
	for (j = 4; j >= 0; --j) {
		xpos[j] = random() % cols + 2;
		ypos[j] = random() % lines + 2;
	}
	for (j = 0;;) {
		if (sig_caught) {
			endwin();
			exit(0);
		}
		x = random() % cols + 2;
		y = random() % lines + 2;
		mvaddch(y, x, '.');
		mvaddch(ypos[j], xpos[j], 'o');
		if (!j--)
			j = 4;
		mvaddch(ypos[j], xpos[j], 'O');
		if (!j--)
			j = 4;
		mvaddch(ypos[j] - 1, xpos[j], '-');
		mvaddstr(ypos[j], xpos[j] - 1, "|.|");
		mvaddch(ypos[j] + 1, xpos[j], '-');
		if (!j--)
			j = 4;
		mvaddch(ypos[j] - 2, xpos[j], '-');
		mvaddstr(ypos[j] - 1, xpos[j] - 1, "/ \\");
		mvaddstr(ypos[j], xpos[j] - 2, "| O |");
		mvaddstr(ypos[j] + 1, xpos[j] - 1, "\\ /");
		mvaddch(ypos[j] + 2, xpos[j], '-');
		if (!j--)
			j = 4;
		bgwrt(ypos[j] - 2, xpos[j], " ");
		bgwrt(ypos[j] - 1, xpos[j] - 1, "   ");
		bgwrt(ypos[j], xpos[j] - 2, "     ");
		bgwrt(ypos[j] + 1, xpos[j] - 1, "   ");
		bgwrt(ypos[j] + 2, xpos[j], " ");
		xpos[j] = x;
		ypos[j] = y;
		refresh();
		if (delay)
			usleep(delay);
		else
			tcdrain(STDOUT_FILENO);
	}
}

void bgwrt(int ypos, int xpos, string space){
	//cout<<++dbg_cnt<<"brwrt "<<ypos<<", "<<xpos<<" "<<endl;
	if(ypos>=(LINES-1-bgheight)){
		mvaddstr(ypos, xpos, bgp[LINES-1-ypos].substr(xpos, xpos+space.length()).c_str());
		//cout<<"T ";
	}else{
		mvaddstr(ypos, xpos, space.c_str());
		//cout<<"F ";
	}
	//cout<<"completed"<<endl;
}

static void
onsig(int dummy __attribute__((__unused__)))
{
	sig_caught = 1;
}
