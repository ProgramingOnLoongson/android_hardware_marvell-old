/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <errno.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cutils/properties.h>

#include <bootloader.h>
#include <fs_mgr.h>

#include "bootinfo.h"

// Open the appropriate fstab file and fallback to /fstab.device if
// that's what's being used.
static struct fstab *open_fstab(void)
{
	char propbuf[PROPERTY_VALUE_MAX];
	char fstab_name[PROPERTY_VALUE_MAX + 32];
	struct fstab *fstab;

	property_get("ro.hardware", propbuf, "");
	snprintf(fstab_name, sizeof(fstab_name), "/fstab.%s", propbuf);
	fstab = fs_mgr_read_fstab(fstab_name);

	if (fstab != NULL) {
		return fstab;
	} else {
		fstab = fs_mgr_read_fstab("/fstab.device");
		if (fstab != NULL)
			return fstab;
	}

	return NULL;
}

static int boot_info_open_misc(int flags)
{
	char *path;
	int fd;
	struct fstab *fstab;
	struct fstab_rec *record;

	fstab = open_fstab();
	if (fstab == NULL)
		return -1;

	record = fs_mgr_get_entry_for_mount_point(fstab, "/misc");
	if (record == NULL) {
		fs_mgr_free_fstab(fstab);
		return -1;
	}

	path = strdup(record->blk_device);
	fs_mgr_free_fstab(fstab);

	fd = open(path, flags);
	free(path);

	return fd;
}

// As per struct bootloader_message_ab which is defined in
// bootable/recovery/bootloader.h we can use the 32 bytes in the
// bootctrl_suffix field provided that they start with the active slot
// suffix terminated by NUL. It just so happens that BrilloBootInfo is
// laid out this way.
#define BOOTINFO_OFFSET offsetof(struct bootloader_message_ab, slot_suffix)

bool boot_info_load(BrilloBootInfo *out_info)
{
	int fd;

	memset(out_info, '\0', sizeof(BrilloBootInfo));

	fd = boot_info_open_misc(O_RDONLY);
	if (fd == -1)
		return false;

	if (lseek(fd, BOOTINFO_OFFSET, SEEK_SET) != BOOTINFO_OFFSET) {
		close(fd);
		return false;
	}

	ssize_t num_read;
	do {
		num_read = read(fd, (void*) out_info, sizeof(BrilloBootInfo));
	} while (num_read == -1 && errno == EINTR);

	close(fd);

	if (num_read != sizeof(BrilloBootInfo))
		return false;

	return true;
}

bool boot_info_save(BrilloBootInfo *info)
{
	int fd;

	fd = boot_info_open_misc(O_RDWR);
	if (fd == -1)
		return false;

	if (lseek(fd, BOOTINFO_OFFSET, SEEK_SET) != BOOTINFO_OFFSET) {
		close(fd);
		return false;
	}

	ssize_t num_written;
	do {
		num_written = write(fd, (void*) info, sizeof(BrilloBootInfo));
	} while (num_written == -1 && errno == EINTR);

	close(fd);

	if (num_written != sizeof(BrilloBootInfo))
		return false;

	return true;
}

bool boot_info_validate(BrilloBootInfo* info)
{
	if (info->magic[0] != '\0' ||
		info->magic[1] != 'B' ||
		info->magic[2] != 'C')
		return false;

	return true;
}

void boot_info_reset(BrilloBootInfo* info)
{
	memset(info, '\0', sizeof(BrilloBootInfo));
	info->magic[1] = 'B';
	info->magic[2] = 'C';

	return;
}