/**
*****************************************************************************
**
**  File        : syscalls.c
**
**  Abstract    : System Workbench Minimal System calls file
**
** 		          For more information about which c-functions
**                need which of these lowlevel functions
**                please consult the Newlib libc-manual
**
**  Environment : System Workbench for MCU
**
**  Distribution: The file is distributed �as is,� without any warranty
**                of any kind.
**
*****************************************************************************
**
** <h2><center>&copy; COPYRIGHT(c) 2014 Ac6</center></h2>
**
** Redistribution and use in source and binary forms, with or without modification,
** are permitted provided that the following conditions are met:
**   1. Redistributions of source code must retain the above copyright notice,
**      this list of conditions and the following disclaimer.
**   2. Redistributions in binary form must reproduce the above copyright notice,
**      this list of conditions and the following disclaimer in the documentation
**      and/or other materials provided with the distribution.
**   3. Neither the name of Ac6 nor the names of its contributors
**      may be used to endorse or promote products derived from this software
**      without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
** FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
** DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
** SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
** CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
** OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
*****************************************************************************
*/

/* Includes */
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/unistd.h>
#include "board.h"

void SERIAL_PutString(const char* str, int size);
int SERIAL_GetChar(void);

char *__env[1] = {0};
char **environ = __env;
static char *heap_end = NULL;


/* Functions */
uint32_t memavail(void){
	return (SDRAM_DEVICE_ADDR + SDRAM_DEVICE_SIZE) - (uint32_t)heap_end;
}

void initialise_monitor_handles()
{

}

int __getpid(void)
{
	return 1;
}

int _kill(int pid, int sig)
{
	errno = EINVAL;
	return -1;
}

void _exit(int status)
{
	_kill(status, -1);
	while (1)
	{
	} /* Make sure we hang here */
}

int _read(int file, char *ptr, int len)
{
	return -1;
}


int _write(int file, char *data, int len)
{
	if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
	{
		errno = EBADF;
		return -1;
	}

	// 1:stdout 
	// 2:stderr
	if (file == STDOUT_FILENO || file == STDERR_FILENO)
	{		
		SERIAL_PutString(data, len);
		return len;
	}

	return -1;
}

caddr_t _sbrk(int incr)
{	
	char *prev_heap_end;

	if (heap_end == 0)
	{
		heap_end = (char *)SDRAM_DEVICE_ADDR + (SDRAM_DEVICE_SIZE >> 1);
	}

	prev_heap_end = heap_end;
	if ((uint32_t)(heap_end + incr) > (SDRAM_DEVICE_ADDR + SDRAM_DEVICE_SIZE))
	{
		//		write(1, "Heap and stack collision\n", 25);
		//		abort();
		errno = ENOMEM;
		return (caddr_t)-1;
	}

	heap_end += incr;

	return (caddr_t)prev_heap_end;
}

int _close(int file)
{
	return -1;
}

int _fstat(int file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _isatty(int file)
{
	return -1;
}

int _lseek(int file, int ptr, int dir)
{
	return -1;
}

int _open(char *path, int flags, ...)
{
	return -1;
}

int __wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int __unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int __times(struct tms *buf)
{
	return -1;
}

int __stat(char *file, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int __link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int __fork(void)
{
	errno = EAGAIN;
	return -1;
}

int __execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}
