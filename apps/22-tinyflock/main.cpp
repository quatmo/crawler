#include "flock.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <vector>
#include <thread>

#include <GLFW/glfw3.h>
#include <omp.h>

#include "render.h"

#include "events.h"
#include "configuration.h"

#define WINDOW_TITLE "tinyflock"

#define FPS_BUFFER_SIZE 5


void init_gl(int width, int height)
{
	glViewport(0, 0, width, height);
	glClearColor(255.0f, 255.0f, 255.0f, 0.0f);
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	glLineWidth(1.0f);

	glShadeModel(GL_SMOOTH);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glOrtho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

	glMatrixMode(GL_MODELVIEW);
}

int print_help()
{
	printf("\n(C) 2011 by Joseph A. Kogut (joseph.kogut@gmail.com)\n"
		"This software is distributed under the MIT license,\n"
		"with no warranty, express or implied. Run this software\n"
		"at your own risk.\n\n"

		"-h | --help\t\tPrint this help message.\n\n"

		"Video configuration\n"
		"------------------------------------------------------------\n"

		"--height [number]\n"
		"\tSpecify screen height in pixels.\n\n"

		"--width [number]\n"
		"\tSpecify screen width in pixels.\n\n"

		"--depth [number]\n"
		"\tSpecify screen depth in bits.\n\n"

		"--fps [number]\n"
		"\tLimit the framerate to the number specified\n\n"

		"--draw-anchor\n"
		"\tDisplay a visual anchor to prevent motion sickness\n\n"

		"Input Configuration\n"
		"------------------------------------------------------------\n"
		"-ir | --influence-radius [pixels]\n\tSpecify the maximum distance from the cursor that"
		"\n\twill influence boids.\n\n"

		"Flock configuration\n"
		"------------------------------------------------------------\n"
		"-fc | --flock-count\n\tSpecify the number of boids to create.\n\n"
		"-fs | --flock-separation\n\tSpecify a minimum distance to keep from neighbors.\n\n"
		"-fv | --flock-velocity\n\tSpecify a maximum velocity a boid can travel.\n\n"
		"-fn | --flock-neighborhood\n\tSpecify the size of the neighborhood a boid can see.\n\n"

		"Misc.\n"
		"------------------------------------------------------------\n"
		"-t | --num-threads\n\tSpecify the number of worker threads used to\n"
		"\tcalculate boid movement.\n"
	);

	return 0;
}

int parse_arguments(int argc, char** argv, configuration* config)
{
	config->num_threads = NUM_THREADS;

	config->video.screen_width = SCREEN_WIDTH;
	config->video.screen_height = SCREEN_HEIGHT;
	config->video.screen_depth = SCREEN_DEPTH;
	config->video.frames_per_second = FPS;

	config->flock.size = NUM_BOIDS;
	config->flock.max_velocity = MAX_BOID_VELOCITY;
	config->flock.min_separation = MIN_BOID_SEPARATION;
	config->flock.max_steering_force = MAX_BOID_STEERING_FORCE;
	config->flock.neighborhood_radius = NEIGHBORHOOD_RADIUS;

	// Parse arguments
	for(int i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
			return print_help();
		else if(strcmp(argv[i], "--width") == 0)
			config->video.screen_width = atoi(argv[++i]);
		else if(strcmp(argv[i], "--height") == 0)
			config->video.screen_height = atoi(argv[++i]);
		else if(strcmp(argv[i], "--depth") == 0)
			config->video.screen_depth = atoi(argv[++i]);
		else if(strcmp(argv[i], "--fps") == 0)
			config->video.frames_per_second = atoi(argv[++i]);
		else if(strcmp(argv[i], "-ir") == 0 || strcmp(argv[i], "--influence-radius") == 0)
			config->input.influence_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fc") == 0 || strcmp(argv[i], "--flock-count") == 0)
			config->flock.size = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fs") == 0 || strcmp(argv[i], "--flock-separation") == 0)
			config->flock.min_separation = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fv") == 0 || strcmp(argv[i], "--flock-velocity") == 0)
			config->flock.max_velocity = atoi(argv[++i]);
		else if(strcmp(argv[i], "-fn") == 0 || strcmp(argv[i], "--flock-neighborhood") == 0)
			config->flock.neighborhood_radius = atoi(argv[++i]);
		else if(strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--num-threads") == 0)
			config->num_threads = atoi(argv[++i]);
	}

	// We want the influence radius to scale with the screen real estate
	config->input.influence_radius = sqrt(config->video.screen_height * config->video.screen_width) / 5;

	return 1;
}

/* Here we take the time taken to render the newest frame and average it with the last N frames */
long avg_fps(long frame_time_nsec)
{
	long frames_per_second = 1000000000 / frame_time_nsec;

	static long fps_buffer[FPS_BUFFER_SIZE];
	for(int i = FPS_BUFFER_SIZE - 1; i != 0; i--) fps_buffer[i] = fps_buffer[i - 1];
	fps_buffer[0] = frames_per_second;

	long fps_avg = 0;
	for(int i = 0; i < FPS_BUFFER_SIZE; i++) fps_avg += fps_buffer[i];
	return fps_avg / FPS_BUFFER_SIZE;
}

void print_time_stats(long fps, long tps)
{
	printf("\rFrames Per Second: %ld, Ticks Per Second: %ld        ", fps, tps);
	fflush(stdout);
}


int run = 1;
vec2_t cursor_pos;
int cursor_interaction;
flock* flock_ptr;
configuration* config;

int main(int argc, char** argv)
{
	// Create a configuration object, and set the values to the defaults
	config = (configuration*)calloc(1, sizeof(configuration));
	if(!parse_arguments(argc, argv, config)) return 0;

	srand(time(NULL));

	glfwInit();

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
//	glfwWindowHint(GLFW_SAMPLES, 8);

	GLFWwindow* window = glfwCreateWindow(config->video.screen_width, config->video.screen_height, WINDOW_TITLE, NULL, NULL);
	if(!window) printf("Unable to set video mode.\n");

	// Register callbacks
	glfwSetCursorPosCallback(window, callback_cursormov);
	glfwSetMouseButtonCallback(window, callback_mousebtn);
	glfwSetKeyCallback(window, callback_keyboard);
	glfwSetWindowSizeCallback(window, callback_windowresize);
	glfwSetWindowCloseCallback(window, callback_wclose);

	vec2_zero(cursor_pos);
	cursor_interaction = 0;

	glfwMakeContextCurrent(window);

	init_gl(config->video.screen_width, config->video.screen_height);
	printf("Using OpenGL Version: %i.%i\n", glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR),
						glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR));

	// Create our flock
	flock_ptr = flock_create(config);

	long* tps = (long *)calloc(sizeof(long), config->num_threads);

// DISPATCH //
	std::vector<std::thread> workers;
	flock_update_worker_args* worker_args = (flock_update_worker_args*)calloc(sizeof(flock_update_worker_args), config->num_threads);

	for(int i = 0; i < config->num_threads; i++) {
			auto &args = worker_args[i];
			args.run = &run;
			args.thread_id = i;
			args.ticks = tps;
			args.f = flock_ptr;
			args.config = config;
			args.cursor_pos = &cursor_pos;
			args.cursor_interaction = &cursor_interaction;
			workers.emplace_back( flock_update_worker_thread, (void*)&worker_args[i] );
	}

//////////////

	double curr_time, new_time;
	long frame_time_nsec;

        curr_time = omp_get_wtime();

	while(run && !glfwWindowShouldClose(window))
	{
		flock_render(window, flock_ptr, config);

		new_time = omp_get_wtime();
		frame_time_nsec = 1 + (long)((new_time - curr_time) * 1000000000 );
		curr_time = new_time;

		long avg_tps = 0; for(int i = 0; i < config->num_threads; i++) avg_tps += tps[i];
		avg_tps /= config->num_threads;

		print_time_stats(avg_fps(frame_time_nsec), avg_tps);

		glfwPollEvents();
	}

        for(int i = 0; i < config->num_threads; i++)
                if(workers[i].joinable()) workers[i].join();

	glfwDestroyWindow(window);

	free(config);
	free(tps);
        free(worker_args);
 	flock_destroy(flock_ptr);

	glfwTerminate();
	return 0;
}
