/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// A simple demo using dispmanx to display an overlay

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

#include "bcm_host.h"
#include <linux/ioctl.h>

#define MAJOR_NUM 100
#define IOCTL_MBOX_PROPERTY _IOWR(MAJOR_NUM, 0, char *)
#define DEVICE_FILE_NAME "/dev/vcio"
int mboxfd = 0;

typedef struct vc_memory {
    __u32 base ;
    __u32 size ;
} vc_memory_t;
#define IOCTL_VCMSG_GET_VC_MEMORY _IOWR(MAJOR_NUM, 1,struct vc_memory)
static int mbox_property()
{
   if (mboxfd < -1) return -1;
   struct vc_memory vcmem;
   int ret_val = ioctl(mboxfd, IOCTL_VCMSG_GET_VC_MEMORY, &vcmem);

   if (ret_val < 0) {
      printf("ioctl_set_msg failed:%d\n", ret_val);
   }
    printf("success base 0x%x, size 0x%x %d\n",vcmem.base,vcmem.size,vcmem.size);
   return ret_val;
}


/*
 * Set the message of the device driver
 */
#define IOCTL_MBOX_PROPERTY _IOWR(MAJOR_NUM, 0, char *)


int main(void)
{
    mboxfd = open(DEVICE_FILE_NAME, 0);

    bcm_host_init();

    mbox_property();
    close(mboxfd);
    return 0;
}

