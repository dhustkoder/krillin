#include <windows.h>
#include <ViGEm/Client.h>
#include "krl.h"

typedef enum {
	BTN_UP,
	BTN_DOWN,
	BTN_LEFT,
	BTN_RIGHT,
	BTN_START,
	BTN_BACK,
	BTN_A,
	BTN_B,
	BTN_X,
	BTN_Y,
	BTN_LB,
	BTN_RB,
	BTN_LT,
	BTN_RT
} btn_t;



static struct vigem_pad {
	PVIGEM_CLIENT client;
	PVIGEM_TARGET target;
	XUSB_REPORT report;
} vigem_pad;


typedef struct botinput {
	uint16_t wbtn;
	timer_t timer;
	timer_t ms;
} botinput_t;


static botinput_t botinputs[] = {
	[BTN_UP]    = {XUSB_GAMEPAD_DPAD_UP},
	[BTN_DOWN]  = {XUSB_GAMEPAD_DPAD_DOWN},
	[BTN_LEFT]  = {XUSB_GAMEPAD_DPAD_LEFT}, 
	[BTN_RIGHT] = {XUSB_GAMEPAD_DPAD_RIGHT},
	[BTN_START] = {XUSB_GAMEPAD_START},
	[BTN_BACK]  = {XUSB_GAMEPAD_BACK},
	[BTN_A]     = { XUSB_GAMEPAD_A },
	[BTN_B]     = { XUSB_GAMEPAD_B },
	[BTN_X]     = { XUSB_GAMEPAD_X },
	[BTN_Y]     = { XUSB_GAMEPAD_Y },
	[BTN_LB]    = { XUSB_GAMEPAD_LEFT_SHOULDER },
	[BTN_RB]    = { XUSB_GAMEPAD_RIGHT_SHOULDER },
	[BTN_LT]    = { XUSB_GAMEPAD_LEFT_THUMB },
	[BTN_RT]    = { XUSB_GAMEPAD_RIGHT_THUMB }
};

static void botinput_press(int idx, timer_t ms)
{
	botinputs[idx].ms = ms;
	botinputs[idx].timer = get_timer();
}


void vigem_pad_feedback_clbk(
	PVIGEM_CLIENT client,
	PVIGEM_TARGET target,
	UCHAR large_motor,
	UCHAR small_motor,
	UCHAR led_number,
	LPVOID udata
)
{
	__pragma(warning(suppress:4100)) client;
	__pragma(warning(suppress:4100)) target;
	__pragma(warning(suppress:4100)) large_motor;
	__pragma(warning(suppress:4100)) small_motor;
	__pragma(warning(suppress:4100)) led_number;
	__pragma(warning(suppress:4100)) udata;

	printf(
		"X360 NOTIFICATION CALLED: \n"
		"%.2X large_motor\n"
		"%.2X small_motor\n",
		(unsigned)large_motor,
		(unsigned)small_motor
	);
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
		&vigem_pad_feedback_clbk,
		NULL
	);

	if (err != VIGEM_ERROR_NONE) {
		printf("vigem_target_x360_register_notification error = %X\n", err);
		goto Lfail;
	}

	XUSB_REPORT_INIT(&vigem_pad.report);

	return;
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
	timer_t now = get_timer();
	vigem_pad.report.wButtons = 0x00;
	for (size_t i = 0; i < STATIC_ARRAY_COUNT(botinputs); ++i) {
		if ((now - botinputs[i].timer) <= botinputs[i].ms)
			vigem_pad.report.wButtons |= botinputs[i].wbtn;
	}

	VIGEM_ERROR ret = vigem_target_x360_update(
		vigem_pad.client,
		vigem_pad.target,
		vigem_pad.report
	);

	if (ret != VIGEM_ERROR_NONE)
		printf("Error on vigem target update: %X\n", ret);
}

static void vpad_cmd_handler(btn_t btn, cmd_info_t* cmd)
{
	timer_t ms = 16;

	if (cmd->args_len > 1) {
		ms = atoi(cmd->args);
		ms = clamp(ms, 16, 5000);
	}

	botinput_press(btn, 16);
}

static void up_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_UP, cmd); }
static void down_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_DOWN, cmd); }
static void left_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_LEFT, cmd); }
static void right_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_RIGHT, cmd); }
static void start_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_START, cmd); }
static void select_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_BACK, cmd); }
static void a_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_A, cmd); }
static void b_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_B, cmd); }
static void x_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_X, cmd); }
static void y_handler(cmd_info_t* cmd) { vpad_cmd_handler(BTN_Y, cmd); }

const cmd_handler_t vpad_default_handlers[VPAD_DEFAULT_HANDLERS_COUNT] = {
	CMD_HANDLER_FN("up", up_handler),
	CMD_HANDLER_FN("down", down_handler),
	CMD_HANDLER_FN("left", left_handler),
	CMD_HANDLER_FN("right", right_handler),
	CMD_HANDLER_FN("start", start_handler),
	CMD_HANDLER_FN("a", a_handler),
	CMD_HANDLER_FN("b", b_handler),
	CMD_HANDLER_FN("x", x_handler),
	CMD_HANDLER_FN("y", y_handler)
};





