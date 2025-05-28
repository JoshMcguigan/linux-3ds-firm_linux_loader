#include "arm.h"
#include "cache.h"
#include "cfg11.h"
#include "common.h"
#include "draw.h"
#include "fs.h"
#include "linux_config.h"

static int load_file(const char *filename, uint32_t addr)
{
	size_t size;

	Debug("Loading %s...", filename);

	if (!FileOpen(filename)) {
		DebugColor(COLOR_RED, "Loading of %s failed!", filename);
		return 0;
	}

	size = FileRead((void *)addr, ~0UL, 0);

	Debug("File %s loaded:", filename);
	Debug("    size: %d B", size);

	FileClose();

	return 1;
}

int main(int argc, char *argv[])
{
	const char *dtb_filename;

	InitScreenFbs(argc, argv);
	ClearScreenFull(true, true);
	DebugClear();

	Debug("-- FIRM Linux loader by xerpi --");
	uint16_t val = *(vu32 *)(0x10100000 + 0x1e);
	Debug("read %d", val);

	if (InitFS()) {
		Debug("Initializing SD card... success");
	} else {
		Debug("Initializing SD card... failed");
		goto error;
	}

	if (!load_file(LINUXIMAGE_FILENAME, ZIMAGE_ADDR)) {
		Debug("Failed to load " LINUXIMAGE_FILENAME);
		goto error;
	}

	if (FileExists(INITRAMFS_FILENAME)) {
		if (!load_file(INITRAMFS_FILENAME, INITRAMFS_ADDR)) {
			Debug("Failed to load " INITRAMFS_FILENAME);
			goto error;
		}
	}
	else {
		Debug("Note: initramfs file not present (" INITRAMFS_FILENAME ")");
	}

	dtb_filename = is_lgr() ? KTR_DTB_FILENAME : CTR_DTB_FILENAME;
	if (!load_file(dtb_filename, DTB_ADDR)) {
		Debug("Failed to load %s", dtb_filename);
		goto error;
	}

	if (!load_file(ARM9LINUXFW_FILENAME, ARM9LINUXFW_ADDR)) {
		Debug("Failed to load arm9linuxfw");
		goto error;
	}

	flushCaches();

error:
	DeinitFS();
	while(1);
}
