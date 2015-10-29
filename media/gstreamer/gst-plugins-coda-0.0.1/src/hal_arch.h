/* GStreamer
 * Copyright (C)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef HAL_ARCH_H
#define HAL_ARCH_H

#include <gst/gst.h>

G_BEGIN_DECLS

gpointer hal_vaddr_to_paddr(gpointer vaddr);
gpointer hal_paddr_to_vaddr(gpointer paddr);
void bsp_postboot_init(void);
void bsp_close(void);
guint HAL_GET_UINT32(volatile guint *addr);
void HAL_PUT_UINT32(volatile guint *addr, guint data);
int hal_device_attach(const char *name);
void hal_device_detach(int fd);
void hal_int_register(unsigned int num, void (*routine)(void *), void *param);


G_END_DECLS


#endif
