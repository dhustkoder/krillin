#include <windows.h>
#include <ViGEm/Client.h>

#include "krl.h"
#include "actors.h"


static struct vigem_pad {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
	XUSB_REPORT report;
} vigem_pad;


static void re2_walk(actor_t* a, const char* line, size_t len)
{
	vigem_pad.report.wButtons |= XUSB_GAMEPAD_DPAD_UP;
}



void virtual_pad_init(void)
{
	vigem_pad.client = vigem_alloc();
	VIGEM_ERROR err = vigem_connect(vigem_pad.client);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_connect error = %X\n", err);
		goto Lfail;
	}

	vigem_pad.target = vigem_target_x360_alloc();
	err = vigem_target_add(vigem_pad.client, vigem_pad.target);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_target_add error = %X\n", err);
		goto Lfail;
	}

	err = vigem_target_x360_register_notification(
		vigem_pad.client,
		vigem_pad.target,
		&vigem_pad_notification,
		NULL
	);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_target_x360_register_notification error = %X\n", err);
		goto Lfail;
	}

	XUSB_REPORT_INIT(&vigem_pad.report);

Lfail:
	assert(false && "ERROR");
}


void virtual_pad_term(void)
{
	vigem_target_x360_unregister_notification(vigem_pad.target);
	vigem_target_remove(vigem_pad.client, vigem_pad.target);
	vigem_target_free(vigem_pad.target);
	vigem_free(vigem_pad.client);
}

void virtual_pad_update(void)
{
	VIGEM_ERROR ret = vigem_target_x360_update(
		vigem_pad.client,
		vigem_pad.target,
		vigem_pad.report
	);

	if (ret != VIGEM_ERROR_NONE)
		printf("Error on vigem target update: %X\n", ret);
}


const char* re2_cmd_strings[] = {
	"walk"
};

const cmd_handler_t re2_cmd_handlers[] = {
	CMD_HANDLER_FN(re2_walk)
};





