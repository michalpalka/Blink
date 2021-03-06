/* 
   Copyright (c) Microsoft Corporation
   All rights reserved. 

   Licensed under the Apache License, Version 2.0 (the ""License""); you
   may not use this file except in compliance with the License. You may
   obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

   THIS CODE IS PROVIDED ON AN *AS IS* BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT
   LIMITATION ANY IMPLIED WARRANTIES OR CONDITIONS OF TITLE, FITNESS FOR
   A PARTICULAR PURPOSE, MERCHANTABLITY OR NON-INFRINGEMENT.

   See the Apache Version 2.0 License for specific language governing
   permissions and limitations under the License.
*/
#include <stdio.h>
#include <assert.h>
#include <limits.h>
#include <windows.h>

#include <thread_func.h>

#include "sora_threads.h"
#include "sora_thread_queues.h"


HANDLE threads[MAX_THREADS];
thread_info t_info[MAX_THREADS];


// Ideally, we might want to rewrite the threading support to use user mode threads in Win7+
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd627187(v=vs.85).aspx


// no_threads - number of threads to start
// User_Routines[no_threads] - pointers to functions to be started
// sizes[no_threads-1] - size of data structure to be held by each buffer (in bytes)
int StartThreads(ULONGLONG * ttstart, 
                 ULONGLONG * ttend, 
		 TIMESTAMPINFO *tsinfo,
                 int no_threads, PSORA_UTHREAD_PROC* User_Routines)
{
	bool init_error = false;
	for (int i=0; i<no_threads; i++)
	{
		if (User_Routines[i] != NULL)
		{
			t_info[i].threadID = i;
			t_info[i].fRunning = true;
			threads[i] = AllocStartThread ( User_Routines[i], (void *) (t_info+i) );
			if (threads[i] == NULL) init_error = true;
		}
		else
		{
			threads[i] = NULL;
		}
	}

	// Start measuring time
	*ttstart = SoraGetCPUTimestamp ( tsinfo );	
	
	if (!init_error)
	{
		bool not_finished = true;
		while (not_finished) 
		{
			not_finished = false;
			for (int i=0; i<no_threads; i++)
			{
				not_finished = not_finished || t_info[i].fRunning;
			}
			Sleep (1);
		}
	}
	else
	{
		printf("Initialization error: couldn't start a thread!\n");
		exit(1);
	}


	for (int i=0; i<no_threads; i++)
	{
		if (threads[i] != NULL)
		{
			StopFreeThread (threads[i]);
		}
	}

	// Stop measuring time
	*ttend = SoraGetCPUTimestamp ( tsinfo );

	return no_threads;
}

