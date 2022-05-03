#include "metronome.h"

int main(int argc, char *argv[])
{
	if (argc != 4)
	{
		printf("Need 4 args: metronome beats-per-minute time-signature-top time-signature-bottom\n");
		exit(EXIT_FAILURE);
	}

	init_metronome_settings(argv);

	dispatch_t *dispatch;
	resmgr_io_funcs_t io_funcs;
	resmgr_connect_funcs_t connect_funcs;
	//iofunc_attr_t io_func_attr;
	dispatch_context_t *dispatch_context;
	pthread_t metronome_tid;
	int id;

	iofunc_funcs_t ocb_funcs = { _IOFUNC_NFUNCS, ocb_calloc, ocb_free };

	iofunc_mount_t mount = { 0, 0, 0, 0, &ocb_funcs };

	if (!(dispatch = dispatch_create()))
	{
		perror("dispatch_create");
		exit(EXIT_FAILURE);
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs, _RESMGR_IO_NFUNCS, &io_funcs);
	io_funcs.read = io_read;
	io_funcs.write = io_write;
	connect_funcs.open = io_open;

	for (int i = 0; i < NUM_DEVICES; i++)
	{
		iofunc_attr_init(&my_attrs[i].iofunc_attr, S_IFCHR | 0666, NULL, NULL);
		my_attrs[i].device = i;
		my_attrs[i].iofunc_attr.mount = &mount;

		id = resmgr_attach(dispatch, NULL, device_names[i], _FTYPE_ANY, 0, &connect_funcs, &io_funcs,
				&my_attrs[i]);
		if (id == CONNECTION_ERROR)
		{
			perror("resmgr_attach");
			return EXIT_FAILURE;
		}
	}

	if (!(dispatch_context = dispatch_context_alloc(dispatch)))
	{
		perror("dispatch_context_alloc");
		return EXIT_FAILURE;
	}

	if (pthread_create(&metronome_tid, NULL, run_metronome_thread, NULL) != EXIT_SUCCESS)
	{
		printf("Error creating thread");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		if (!(dispatch_context = dispatch_block(dispatch_context)))
		{
			perror("dispatch_block");
			return EXIT_FAILURE;
		}

		dispatch_handler(dispatch_context);
	}

	pthread_join(metronome_tid, NULL);

	return EXIT_SUCCESS;
}

void init_metronome_settings(char *argv[])
{
	int bpm = atoi(argv[1]);
	if (bpm <= 0)
	{
		printf("\'%s\' is invalid", argv[1]);
		exit(EXIT_FAILURE);
	}

	int ts_top = atoi(argv[2]);
	if (ts_top <= 0)
	{
		printf("\'%s\' is invalid", argv[2]);
		exit(EXIT_FAILURE);
	}

	int ts_bottom = atoi(argv[3]);
	if (ts_bottom <= 0)
	{
		printf("\'%s\' is invalid", argv[3]);
		exit(EXIT_FAILURE);
	}

	set_current_settings(bpm, ts_top, ts_bottom);
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, IOFUNC_OCB_T *ocb)
{
	if (ocb->ocb.attr->device == METRONOME_DEVICE)
		build_metronome_msg(ocb->internal_message);
	else
		build_metronome_help_msg(ocb->internal_message);

	int num_bytes = strlen(ocb->internal_message);

	if (ocb->ocb.offset == num_bytes)
		return 0;

	num_bytes = min(num_bytes, msg->i.nbytes);

	_IO_SET_READ_NBYTES(ctp, num_bytes);

	SETIOV(ctp->iov, ocb->internal_message, num_bytes);

	ocb->ocb.offset += num_bytes;

	if (num_bytes > 0)
		ocb->ocb.attr->iofunc_attr.flags |= IOFUNC_ATTR_ATIME;

	return _RESMGR_NPARTS(1);
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
	char *inc_msg;
	int num_bytes = _IO_WRITE_GET_NBYTES(msg);

	if (num_bytes > MAX_MESSAGE_SIZE)
		return EBADMSG;

	_IO_SET_WRITE_NBYTES(ctp, num_bytes);

	inc_msg = (char *) malloc(num_bytes + 1);
	if (!inc_msg)
	{
		free(inc_msg);
		return ENOMEM;
	}

	resmgr_msgread(ctp, inc_msg, num_bytes, sizeof(msg->i));
	inc_msg[num_bytes] = '\0';

	if (ocb->ocb.attr->device == METRONOME_HELP_DEVICE)
		fprintf(stderr, "Error: Cannot write to this device");
	else
		handle_incoming_msg(inc_msg);

	free(inc_msg);

	if (num_bytes > 0)
		ocb->ocb.attr->iofunc_attr.flags |= IOFUNC_ATTR_MTIME | IOFUNC_ATTR_CTIME;

	return (_RESMGR_NPARTS(0));
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
	if ((metronome_coid = name_open(device_names[0], 0)) == CONNECTION_ERROR)
	{
		perror("name_open");
		return EXIT_FAILURE;
	}

	return (iofunc_open_default(ctp, msg, &handle->iofunc_attr, extra));
}

IOFUNC_OCB_T* ocb_calloc(resmgr_context_t *ctp, IOFUNC_ATTR_T *attr)
{
	met_ocb_t *ocb;
	ocb = calloc(1, sizeof(met_ocb_t));
	ocb->ocb.offset = 0;
	return (ocb);
}

void ocb_free(IOFUNC_OCB_T *ocb)
{
	free(ocb);
}

void build_metronome_msg(char * buffer)
{
	double measure_length = 60.0 / (double) current_settings.bpm
			* (double) current_settings.current_signature.ts_top;
	double secs_between_output = measure_length / (double) current_settings.current_signature.num_intervals;

	sprintf(buffer, "[metronome: %d beats/min, time signature %d/%d, sec-per-interval: %.2f, nanoSecs: %.0f]\n",
			current_settings.bpm, current_settings.current_signature.ts_top,
			current_settings.current_signature.ts_bottom, secs_between_output, secs_between_output * 1e+9);
}

void build_metronome_help_msg(char *buffer)
{
	if (strlen(buffer) < 5)
		sprintf(buffer,
				"\n%s\n\n  %s\n\n%s\n  %s\n    %s\n  %s\n    %s\n  %s\n    %s\n  %s\n    %s\n  %s\n    %s\n",
				"Metronome Resource Manager", "Usage: metronome <bpm> <ts-top> <ts-bottom>", "API:",
				"pause [1-9]", "-pause the metronome for 1-9 seconds", "quit", "-quit the metronome",
				"set <bpm> <ts-top> ts-bottom>", "-set the metronome to <bpm> ts-top/ts-bottom", "start",
				"-start the metronome from the stopped state", "stop",
				"-stop the metronome; use 'start' to resume");
}

void handle_incoming_msg(char *inc_msg)
{
	if (strncmp(inc_msg, PAUSE_COMMAND, 5) == 0)
		handle_pause_command(inc_msg);

	else if (strncmp(inc_msg, QUIT_COMMAND, 4) == 0)
	{
		send_pulse_to_metronome(PULSE_CODE_QUIT, 0);
		exit(EXIT_SUCCESS);
	}

	else if (strncmp(inc_msg, SET_COMMAND, 3) == 0)
		handle_set_command(inc_msg);

	else if (strncmp(inc_msg, START_COMMAND, 5) == 0)
		send_pulse_to_metronome(PULSE_CODE_START, 0);

	else if (strncmp(inc_msg, STOP_COMMAND, 4) == 0)
		send_pulse_to_metronome(PULSE_CODE_STOP, 0);

	else
		fprintf(stderr, "Error: Invalid command");
}

void handle_pause_command(char *inc_msg)
{
	char *pause_val_str;
	for (int i = 0; i < 2; i++)
		pause_val_str = strsep(&inc_msg, " ");

	int pause_val = atoi(pause_val_str);
	if (pause_val >= 1 && pause_val <= 9)
		send_pulse_to_metronome(PULSE_CODE_PAUSE, pause_val);
	else
		fprintf(stderr, "Error: Invalid pause value [1-9]");
}

void handle_set_command(char *inc_msg)
{
	int nums[3];
	char *split_str;

	for (int i = 0; i < 4; i++)
	{
		split_str = strsep(&inc_msg, " ");
		if (i >= 1)
			nums[i - 1] = atoi(split_str);
	}

	if (nums[0] <= 0 || nums[1] <= 0 || nums[2] <= 0)
		fprintf(stderr, "Error: Cannot have negative values");
	else
	{
		set_current_settings(nums[0], nums[1], nums[2]);
		send_pulse_to_metronome(PULSE_CODE_SET, 0);
	}
}

void send_pulse_to_metronome(int pulse_code, int value)
{
	MsgSendPulse(metronome_coid, SchedGet(0, 0, NULL), pulse_code, value);
}

void set_current_settings(int bpm, int ts_top, int ts_bottom)
{
	current_settings.bpm = bpm;
	for (int i = 0; i < 8; i++)
		if (ts_top == signatures[i].ts_top && ts_bottom == signatures[i].ts_bottom)
			current_settings.current_signature = signatures[i];
}

void* run_metronome_thread(void *args)
{
	name_attach_t *attach;
	int rcvid;
	int is_finished = 0;
	struct _pulse inc_pulse;
	timer_t timer_id;

	if (!(attach = name_attach(NULL, device_names[0], 0)))
	{
		perror("name_attach");
		exit(EXIT_FAILURE);
	}

	timer_init(attach->chid, &timer_id);

	while (!is_finished)
	{
		rcvid = MsgReceivePulse(attach->chid, &inc_pulse, sizeof(inc_pulse), NULL);
		if (rcvid != 0)
		{
			perror("MsgReceive");
			exit(EXIT_FAILURE);
		}

		switch (inc_pulse.code)
		{
			case PULSE_CODE_TIMER:
				display_beat();
				break;
			case PULSE_CODE_PAUSE:
				set_timer((struct itimerspec ) { 0 }, &timer_id);
				delay(inc_pulse.value.sival_int * 1000);
				set_timer(get_current_timer_spec(), &timer_id);
				break;
			case PULSE_CODE_QUIT:
				is_finished = 1;
				break;
			case PULSE_CODE_SET:
			case PULSE_CODE_START:
				set_timer(get_current_timer_spec(), &timer_id);
				break;
			case PULSE_CODE_STOP:
				set_timer((struct itimerspec ) { 0 }, &timer_id);
				break;
		}
	}

	if (timer_delete(timer_id) == CONNECTION_ERROR)
	{
		perror("timer_delete");
		exit(EXIT_FAILURE);
	}

	if (name_detach(attach, 0) == CONNECTION_ERROR)
	{
		perror("name_detach");
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

void display_beat()
{
	static int position = 0;
	int end_position = current_settings.current_signature.num_intervals + 1;

	if (position == 0)
	{
		for (; position < 2; position++)
		{
			printf("%c", current_settings.current_signature.pattern[position]);
			fflush( stdout);
		}
	}
	else if (position < end_position)
	{
		printf("%c", current_settings.current_signature.pattern[position++]);
		fflush( stdout);
	}

	if (position >= end_position)
	{
		printf("\n");
		position = 0;
	}
}

void timer_init(int chid, timer_t *timer_id)
{
	struct sigevent event;
	event.sigev_notify = SIGEV_PULSE;
	event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0, chid, _NTO_SIDE_CHANNEL, 0);
	event.sigev_priority = SchedGet(0, 0, NULL);
	event.sigev_code = PULSE_CODE_TIMER;

	if (timer_create(CLOCK_REALTIME, &event, timer_id) == -1)
	{
		perror("timer_create");
		exit(EXIT_FAILURE);
	}

	set_timer(get_current_timer_spec(), timer_id);
}

struct itimerspec get_current_timer_spec()
{
	struct itimerspec timer_spec;

	double measure_length = 60.0 / (double) current_settings.bpm
			* (double) current_settings.current_signature.ts_top;
	double nsecs_between_output = measure_length / (double) current_settings.current_signature.num_intervals
			* 1e+9;

	timer_spec.it_value.tv_sec = 0;
	timer_spec.it_value.tv_nsec = 1000;
	timer_spec.it_interval.tv_sec = 0;
	timer_spec.it_interval.tv_nsec = nsecs_between_output;

	return timer_spec;
}

void set_timer(struct itimerspec timer_spec, timer_t *timer_id)
{
	timer_settime(*timer_id, 0, &timer_spec, NULL);
}
