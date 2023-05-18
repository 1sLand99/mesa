/*
 * Copyright © 2022 Imagination Technologies Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdbool.h>
#include <vulkan/vulkan.h>
#include <xf86drm.h>

#include "powervr/pvr_drm_public.h"
#include "pvr_private.h"
#include "pvr_winsys.h"
#include "vk_log.h"

#if defined(PVR_SUPPORT_SERVICES_DRIVER)
#   include "pvrsrvkm/pvr_srv_public.h"
#endif

void pvr_winsys_destroy(struct pvr_winsys *ws)
{
   ws->ops->destroy(ws);
}

VkResult pvr_winsys_create(int master_fd,
                           int render_fd,
                           const VkAllocationCallbacks *alloc,
                           struct pvr_winsys **const ws_out)
{
#if defined(PVR_SUPPORT_SERVICES_DRIVER)
   drmVersionPtr version;
   bool services_driver;

   version = drmGetVersion(render_fd);
   if (!version) {
      return vk_errorf(NULL,
                       VK_ERROR_INCOMPATIBLE_DRIVER,
                       "Failed to query kernel driver version for device.");
   }

   if (strcmp(version->name, "pvr") == 0) {
      services_driver = true;
   } else if (strcmp(version->name, "powervr") == 0) {
      services_driver = false;
   } else {
      drmFreeVersion(version);
      return vk_errorf(
         NULL,
         VK_ERROR_INCOMPATIBLE_DRIVER,
         "Device does not use any of the supported pvrsrvkm or powervr kernel driver.");
   }

   drmFreeVersion(version);

   if (services_driver)
      return pvr_srv_winsys_create(master_fd, render_fd, alloc, ws_out);
#endif

   return pvr_drm_winsys_create(master_fd, render_fd, alloc, ws_out);
}
