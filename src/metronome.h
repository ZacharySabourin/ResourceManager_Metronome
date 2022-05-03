
#ifndef METRONOME_H_
#define METRONOME_H_

struct metronome_attr_s;
#define IOFUNC_ATTR_T struct metronome_attr_s

struct metronome_ocb_s;
#define IOFUNC_OCB_T struct metronome_ocb_s

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <errno.h>

#include <sys/siginfo.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#define NUM_DEVICES 2
#define METRONOME_DEVICE 0
#define METRONOME_HELP_DEVICE 1

#define CONNECTION_ERROR (-1)
#define MAX_MESSAGE_SIZE 512

//Pulse codes
#define PULSE_CODE_TIMER 0
#define PULSE_CODE_PAUSE 1
#define PULSE_CODE_QUIT 2
#define PULSE_CODE_SET 3
#define PULSE_CODE_START 4
#define PULSE_CODE_STOP 5

typedef struct metronome_attr_s
{
	iofunc_attr_t iofunc_attr;
	int device;
} met_attr_t;

typedef struct metronome_ocb_s
{
	iofunc_ocb_t ocb;
	char internal_message[MAX_MESSAGE_SIZE];
} met_ocb_t;

typedef struct time_signature
{
	int ts_top;
	int ts_bottom;
	int num_intervals;
	char *pattern;
} time_sig_t;

typedef struct metronome_settings
{
	int bpm;
	time_sig_t current_signature;
} settings_t;

static settings_t current_settings;

static met_attr_t my_attrs[NUM_DEVICES];
static const char const *device_names[NUM_DEVICES] = { "./dev/local/metronome", "./dev/local/metronome-help", };

static int metronome_coid;

static const char const *PAUSE_COMMAND = "pause";
static const char const *QUIT_COMMAND = "quit";
static const char const *SET_COMMAND = "set";
static const char const *START_COMMAND = "start";
static const char const *STOP_COMMAND = "stop";

static const time_sig_t signatures[] = { { 2, 4, 4, "|1&2&" }, { 3, 4, 6, "|1&2&3&" },
		{ 4, 4, 8, "|1&2&3&4&" }, { 5, 4, 10, "|1&2&3&4-5-" }, { 3, 8, 6, "|1-2-3-" }, { 6, 8, 6, "|1&a2&a" },
		{ 9, 8, 9, "|1&a2&a3&a" }, { 12, 8, 12, "|1&a2&a3&a4&a" } };

//ResMGR functions
void init_metronome_settings(char *argv[]);
int io_read(resmgr_context_t *ctp, io_read_t *msg, IOFUNC_OCB_T *ocb);
int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb);
int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra);
IOFUNC_OCB_T* ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *attr);
void ocb_free(IOFUNC_OCB_T *tocb);
void build_metronome_msg(char *buffer);
void build_metronome_help_msg(char *buffer);
void handle_incoming_msg(char *inc_msg);
void handle_pause_command(char *inc_msg);
void handle_set_command(char *inc_msg);
void send_pulse_to_metronome(int pulse_code, int value);
void set_current_settings(int bpm, int ts_top, int ts_bottom);

//Metronome functions
void* run_metronome_thread(void *args);
void display_beat();
void timer_init(int chid, timer_t *timer_id);
struct itimerspec get_current_timer_spec();
void set_timer(struct itimerspec timer_spec, timer_t *timer_id);

#endif
