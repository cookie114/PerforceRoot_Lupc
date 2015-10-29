#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <sched.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <sys/select.h>


#include "hal_arch.h"


#define UIO_MAX_NAME_SIZE			1024
#define UIO_MAX_PATH_SIZE			64
#define UIO_MAX_IDX					16
#define UIO_MAX_MAP					8
#define UIO_SEARCH_PATH_ROOT		"/sys/class/uio/"
#define UIO_NORDEV_UIO_NAME			"uio"
#define UIO_NORDEV_MAP_NAME			"map"
#define UIO_MAP_VADDR_BASE			0x40000000
#define UIO_MAX_INT					1

static struct uio_int {
	int				count;
	int				max_fd;
	fd_set			fds;
} uio_int;

struct uio_device_map {
	unsigned int				addr;
	unsigned int				size;
};

struct uio_device_info {
	int						fd;
	char					*uio_name;		/* UIO device name, uio0, uio1, ... */
	char					*dev_name;		/* UIO info name */
	int						map_pt;
	struct uio_device_map	maps[UIO_MAX_MAP];
};

struct uio_device_list {
	int  					str_pt;
	char 					buffer[UIO_MAX_NAME_SIZE];
	int  					dev_pt;
	struct uio_device_info	device[UIO_MAX_IDX];
} hal_uio_devices;

/* Int structure */
struct lsr_node {
	void					(*routine)(void *param);
	void					*param;
};

struct uio_info{
	int fd;
	guint addr[3];
	int finish_flag;
};

struct lsr_node lsr_pool[UIO_MAX_INT];
static pthread_t dec_thread=0;
struct uio_info info;

void hal_exception_clr_status(int idx)
{
	int ien = 1;
	guint hal_int_regbase  =  0x5F080000;

	HAL_PUT_UINT32((guint *)(hal_int_regbase + 0x04), HAL_GET_UINT32((guint *)(hal_int_regbase + 0x04)));
	write(idx, &ien, sizeof(int));		/* Enable interrupt */
}


void hal_exception_set_mask(int idx)
{
	int ien = 1;

	FD_SET(idx, &uio_int.fds);
	printf("set max_fd %x, %x\n", idx, uio_int.max_fd);
	if (idx > uio_int.max_fd) {
		uio_int.max_fd = idx;
		printf("set max_fd new value, %d\n", uio_int.max_fd);
	}
	write(idx, &ien, sizeof(int));		/* Enable this source */
}

void hal_exception_clr_mask(int idx)
{
	int ien = 0;

	write(idx, &ien, sizeof(int));		/* Disable this source */
	FD_CLR(idx, &uio_int.fds);
}


void hal_int_dispatch_lsr(int num)
{
	register struct lsr_node *np;

	np = &(lsr_pool[0]);
	{
		if (np->routine) {
			np->routine(np->param);
		}
	}
}

static void *uio_int_thread(void *arg)
{
	//register struct osal_lsr_node *np;
	struct timeval timeout;
	fd_set fds;
	int i;

	while (1) {
		//printf("check uio int, max_fd %x\n", uio_int.max_fd);
		if (info.finish_flag == 1)
			break;
		
		memcpy(&fds, &uio_int.fds, sizeof(fd_set));
		/* Set timeout every time */
		timeout.tv_sec  = 20 / 1000;
		timeout.tv_usec = 20 * 1000;
		switch (select(uio_int.max_fd + 1, &fds, NULL, NULL, &timeout)) {
		case -1:
			printf("select device file fail!\n");
			exit(1);
			break;
		case 0:
			//printf("time out\n");
			//uio_int_dispatch(osal_config.timer_irq_index);
			hal_int_dispatch_lsr(0);
			break;
		default:
			//printf("int occured\n");
			for (i = 0; i <= uio_int.max_fd; i++) {
				if (FD_ISSET(i, &fds)) {
					int count;
					read(i, &count, sizeof(int));
					//uio_int_dispatch(i);
					hal_int_dispatch_lsr(i);
					//gst_boda_dec_loop(i);
					hal_exception_clr_status(i);
				}
			}
			break;
		}
	}

	pthread_exit((void*)1);
}



static void uio_init_interrupt()
{
	pthread_attr_t pthread_attr;
	struct sched_param sched_param;

	memset(&uio_int, 0, sizeof(uio_int));
	/* Create LSR thread */
	pthread_attr_init(&pthread_attr);
	pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_JOINABLE);/* Joinable for main thread */
	pthread_attr_setscope(&pthread_attr, PTHREAD_SCOPE_SYSTEM);			/* Real time */
	pthread_attr_getschedparam(&pthread_attr, &sched_param);
	sched_param.sched_priority = 90;//OSAL_PRI_IRQ;							/* Highest priority */
	pthread_attr_setschedparam(&pthread_attr, &sched_param);
	pthread_attr_setschedpolicy(&pthread_attr, SCHED_FIFO);				/* FIFO */
	pthread_create(&dec_thread, &pthread_attr, &uio_int_thread, NULL);
	pthread_attr_destroy(&pthread_attr);
	printf("uio_init_interrupt: PPID: 0x%08x -> PID: %d\n", pthread_self(), dec_thread);
}

static int uio_root_filter(const struct dirent *dir)
{
	if (strncmp(dir->d_name, UIO_NORDEV_UIO_NAME, strlen(UIO_NORDEV_UIO_NAME)) == 0) {
		return 1;
	}
	return 0;
}

static int uio_maps_filter(const struct dirent *dir)
{
	if (strncmp(dir->d_name, UIO_NORDEV_MAP_NAME, strlen(UIO_NORDEV_MAP_NAME)) == 0) {
		return 1;
	}
	return 0;
}

static int uio_add_name(char *path, char *uio_name)
{
	char file_name[UIO_MAX_PATH_SIZE], dev_name[UIO_MAX_PATH_SIZE];
	struct uio_device_info *device;
	char *buf;
	FILE *fp;

	printf("\t%s -> ", uio_name);

	sprintf(file_name, "%s%s%s", path, uio_name, "/name");
	if ((fp = fopen(file_name, "r")) == NULL) {
		return -1;
	}

	device = &hal_uio_devices.device[hal_uio_devices.dev_pt];
	buf    = &hal_uio_devices.buffer[hal_uio_devices.str_pt];
	device->uio_name = buf;
	strcpy(buf, uio_name);
	hal_uio_devices.str_pt += (strlen(buf) + 1);
	if ( hal_uio_devices.str_pt > UIO_MAX_NAME_SIZE )
	{
		g_assert(0);
	}

	fscanf(fp, "%s", dev_name);
	printf("%s\n", dev_name);

	buf    = &hal_uio_devices.buffer[hal_uio_devices.str_pt];
	device->dev_name = buf;
	strcpy(buf, dev_name);
	hal_uio_devices.str_pt += (strlen(buf) + 1);
	if (hal_uio_devices.str_pt > UIO_MAX_NAME_SIZE)
	{
		g_assert(0);
	}

	fclose(fp);

	return hal_uio_devices.dev_pt++;
}

static int uio_add_maps(char *path, char *map_name, int nod)
{
	char file_name[UIO_MAX_PATH_SIZE];
	struct uio_device_info *dev = &hal_uio_devices.device[nod];
	struct uio_device_map *map;
	unsigned int value;
	FILE *fp;

	GST_LOG("\t\t%s -> ", map_name);

	g_assert((dev->map_pt < UIO_MAX_MAP));
	map = &dev->maps[dev->map_pt++];
	/* Get addr */
	sprintf(file_name, "%s%s%s", path, map_name, "/addr");
	if ((fp = fopen(file_name, "r")) == NULL) {
		return -1;
	}
	fscanf(fp, "%i", &value);
	map->addr = value;
	fclose(fp);
	GST_LOG("0x%x : ", value);
	/* Get size */
	sprintf(file_name, "%s%s%s", path, map_name, "/size");
	if ((fp = fopen(file_name, "r")) == NULL) {
		return -1;
	}
	fscanf(fp, "%i", &value);
	map->size = value;
	fclose(fp);
	printf("0x%x\n", value);

	return nod;
}

static void hal_device_init(void)
{
	struct dirent **uio_dev_dir, **uio_map_dir;
	int i, j, m, n;

	memset(&hal_uio_devices, 0, sizeof(hal_uio_devices));

	if ((n = scandir(UIO_SEARCH_PATH_ROOT, &uio_dev_dir, uio_root_filter, alphasort)) > 0) {
		for (i = 0; i < n; i++) {
			char map_dir[UIO_MAX_PATH_SIZE];
			int nod;

			/* Add new UIO device nod */
			nod = uio_add_name((char *)UIO_SEARCH_PATH_ROOT, uio_dev_dir[i]->d_name);
			/* Search UIO device maps */
			sprintf(map_dir, "%s%s%s", UIO_SEARCH_PATH_ROOT, uio_dev_dir[i]->d_name, "/maps/");
			if ((m = scandir(map_dir, &uio_map_dir, uio_maps_filter, alphasort)) > 0) {
				for (j = 0; j < m; j++) {
					uio_add_maps(map_dir, uio_map_dir[j]->d_name, nod);
				}
				free(uio_map_dir);
			}
		}
		free(uio_dev_dir);
	}

	uio_init_interrupt();
	
}

int hal_device_attach(const char *name)
{
	struct uio_device_info *dev;
	char dev_path[64];
	int i;

	printf("attach %s\n", name);
	for (i = 0; i < UIO_MAX_IDX; i++) {
		dev = &hal_uio_devices.device[i];
		if (dev->dev_name == NULL) {
			continue;
		}
		if (strcmp(name, dev->dev_name) == 0) {
			break;
		}
	}
	if (i == UIO_MAX_IDX) {
		return -1;
	}
	sprintf(dev_path, "/dev/%s", dev->uio_name);
	dev->fd = open(dev_path, O_RDWR);
	printf("/dev/%s fd %x\n", dev->uio_name, dev->fd);

	return dev->fd;
}

static void hal_device_detach(int fd)
{
	if (fd >= 0) {
		close(fd);
	}
}

static void *hal_device_mmap(int fd, int mem_id, int *len)
{
	struct uio_device_info *dev;
	guchar*addr;
	int i;

	for (i = 0; i < UIO_MAX_IDX; i++) {
		dev = &hal_uio_devices.device[i];
		if (dev->fd == fd) {
			break;
		}
	}
	if (i == UIO_MAX_IDX) {
		return NULL;
	}
	addr = (guchar *)(dev->maps[mem_id].addr + UIO_MAP_VADDR_BASE);
	addr = (guchar *)(mmap(addr, dev->maps[mem_id].size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mem_id * PAGE_SIZE));
	if (len) {
		*len = dev->maps[mem_id].size;
	}
	GST_LOG("\t%s\tmap: 0x%x\n", dev->dev_name, (unsigned int)addr);
	return addr;
}
#if 1
static void hal_device_munmap(void *start)
{
	struct uio_device_info *dev;
	int i, j;

	for (i = 0; i < UIO_MAX_IDX; i++) {
		dev = &hal_uio_devices.device[i];
		for (j = 0; j < dev->map_pt; j++) {
			if (dev->maps[j].addr == (guint)start) {
				break;
			}
		}
		if (j < dev->map_pt) {
			break;
		}
	}
	if (i == UIO_MAX_IDX) {
		return;
	}
	munmap(start, dev->maps[j].size);
}
#endif

gpointer hal_vaddr_to_paddr(gpointer vaddr)
{
	return (gpointer)((guint)vaddr - UIO_MAP_VADDR_BASE);
}
gpointer hal_paddr_to_vaddr(gpointer paddr)
{
	/* Just mapping to cacheable space, use hal_cache_to_uncache() for uncache space */
	return (gpointer)((guint)paddr + UIO_MAP_VADDR_BASE);
}

void bsp_postboot_init(void)
{
	/* Build device name list */
	info.finish_flag = 0;
	
	hal_device_init();

	/* Mapping all resource from kernel */
	info.fd = hal_device_attach("soc_h2");
	info.addr[0] = (guint)hal_device_mmap(info.fd, 0, NULL);
	info.addr[1] = (guint)hal_device_mmap(info.fd, 1, NULL);
	info.addr[2] = (guint)hal_device_mmap(info.fd, 2, NULL);
}

void bsp_close(void)
{
	void *tret;
	
	hal_device_munmap((void *)info.addr[0]);
	hal_device_munmap((void *)info.addr[1]);
	hal_device_munmap((void *)info.addr[2]);	
	hal_device_detach(info.fd);
	info.finish_flag = 1;

	pthread_join(dec_thread, &tret);
}


guint HAL_GET_UINT32(volatile guint *addr)
{
	return *addr;
}

void HAL_PUT_UINT32(volatile guint *addr, guint data)
{
	*addr = data;
}

void hal_int_register(unsigned int num, void (*routine)(void *), void *param)
{
	struct lsr_node *np, *lp;
	int i;

	np = &(lsr_pool[0]);
	np->routine = routine;
	np->param = param;

	hal_exception_set_mask(num);

}



