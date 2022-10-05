#ifndef wl_connector_h
#define wl_connector_h

#include "zc_bm_rgba.c"
#include <stdint.h>

void wl_connector_init(int w, int h, void (*render)(uint32_t, bm_rgba_t*));

#endif

#if __INCLUDE_LEVEL__ == 0

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include "xdg-shell-client-protocol.h"
#include "zc_bm_rgba.c"
#include "zc_draw.c"
#include "zc_log.c"
#include "zc_memory.c"

#define MAX_MONITOR_NAME_LEN 255

struct monitor_info
{
    int32_t physical_width;
    int32_t physical_height;
    int32_t logical_width;
    int32_t logical_height;
    double  scale;
    int     index;
    char    name[MAX_MONITOR_NAME_LEN];

    enum wl_output_subpixel subpixel;
    struct zxdg_output_v1*  xdg_output;
    struct wl_output*       wl_output;
};

struct wlc_t
{
    struct wl_display* display; // global display object

    struct wl_compositor* compositor; // active compositor
    struct wl_keyboard*   keyboard;   // active keyboard for seat
    struct wl_pointer*    pointer;    // active pointer for seat
    struct wl_seat*       seat;       // active seat

    struct wl_surface* surface;  // surface for window
    struct wl_buffer*  buffer;   // buffer for surface
    struct wl_shm*     shm;      // active shared memory buffer
    void*              shm_data; // active bufferdata

    struct zxdg_output_manager_v1* xdg_output_manager; // active xdg output manager
    struct zwlr_layer_shell_v1*    layer_shell;        // active layer shell
    struct zwlr_layer_surface_v1*  layer_surface;      // active layer surface

    // outputs

    int                  monitor_count;
    struct monitor_info* monitors[16];
    struct monitor_info* monitor;

    // window state

    int  win_width;
    int  win_height;
    bool running;

    void (*render)(uint32_t, bm_rgba_t*);
} wlc = {0};

/* ***WL OUTPUT EVENTS*** */

static void wl_connector_wl_output_handle_geometry(
    void*             data,
    struct wl_output* wl_output,
    int32_t           x,
    int32_t           y,
    int32_t           width_mm,
    int32_t           height_mm,
    int32_t           subpixel,
    const char*       make,
    const char*       model,
    int32_t           transform)
{
    struct monitor_info* monitor = data;

    zc_log_debug(
	"wl output handle geometry x %i y %i width_mm %i height_mm %i subpixel %i make %s model %s transform %i for monitor %i",
	x,
	y,
	width_mm,
	height_mm,
	subpixel,
	make,
	model,
	transform,
	monitor->index);

    monitor->subpixel = subpixel;
}

static void wl_connector_wl_output_handle_mode(
    void*             data,
    struct wl_output* wl_output,
    uint32_t          flags,
    int32_t           width,
    int32_t           height,
    int32_t           refresh)
{
    struct monitor_info* monitor = data;

    zc_log_debug(
	"wl output handle mode flags %u width %i height %i for monitor %i",
	flags,
	width,
	height,
	monitor->index);

    monitor->physical_width  = width;
    monitor->physical_height = height;
}

static void wl_connector_wl_output_handle_done(void* data, struct wl_output* wl_output)
{
    struct monitor_info* monitor = data;

    zc_log_debug("wl output handle done for monitor %i", monitor->index);
}

static void wl_connector_wl_output_handle_scale(void* data, struct wl_output* wl_output, int32_t factor)
{
    struct monitor_info* monitor = data;

    zc_log_debug("wl output handle scale %i for monitor %i", factor, monitor->index);

    monitor->scale = factor;
}

struct wl_output_listener wl_output_listener = {
    .geometry = wl_connector_wl_output_handle_geometry,
    .mode     = wl_connector_wl_output_handle_mode,
    .done     = wl_connector_wl_output_handle_done,
    .scale    = wl_connector_wl_output_handle_scale,
};

/* ***XDG OUTPUT EVENTS*** */

static void wl_connector_xdg_output_handle_logical_position(void* data, struct zxdg_output_v1* xdg_output, int32_t x, int32_t y)
{
    struct monitor_info* monitor = data;
    zc_log_debug("xdg output handle logical position, %i %i for monitor %i", x, y, monitor->index);
}

static void wl_connector_xdg_output_handle_logical_size(void* data, struct zxdg_output_v1* xdg_output, int32_t width, int32_t height)
{
    struct monitor_info* monitor = data;
    zc_log_debug("xdg output handle logical size, %i %i for monitor %i", width, height, monitor->index);

    monitor->logical_width  = width;
    monitor->logical_height = height;
}

static void wl_connector_xdg_output_handle_done(void* data, struct zxdg_output_v1* xdg_output)
{
    struct monitor_info* monitor = data;
    zc_log_debug("xdg output handle done, for monitor %i", monitor->index);
}

static void wl_connector_xdg_output_handle_name(void* data, struct zxdg_output_v1* xdg_output, const char* name)
{
    struct monitor_info* monitor = data;
    strncpy(monitor->name, name, MAX_MONITOR_NAME_LEN);

    zc_log_debug("xdg output handle name, %s for monitor %i", name, monitor->index);
}

static void wl_connector_xdg_output_handle_description(void* data, struct zxdg_output_v1* xdg_output, const char* description)
{
    struct monitor_info* monitor = data;
    zc_log_debug("xdg output handle description for monitor %i", description, monitor->index);
}

struct zxdg_output_v1_listener xdg_output_listener = {
    .logical_position = wl_connector_xdg_output_handle_logical_position,
    .logical_size     = wl_connector_xdg_output_handle_logical_size,
    .done             = wl_connector_xdg_output_handle_done,
    .name             = wl_connector_xdg_output_handle_name,
    .description      = wl_connector_xdg_output_handle_description,
};

/* *** POINTER EVENTS *** */

void wl_connector_pointer_handle_enter(void* data, struct wl_pointer* wl_pointer, uint serial, struct wl_surface* surface, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    zc_log_debug("pointer handle enter");
}
void wl_connector_pointer_handle_leave(void* data, struct wl_pointer* wl_pointer, uint serial, struct wl_surface* surface)
{
    zc_log_debug("pointer handle leave");
}
void wl_connector_pointer_handle_motion(void* data, struct wl_pointer* wl_pointer, uint time, wl_fixed_t surface_x, wl_fixed_t surface_y)
{
    zc_log_debug("pointer handle motion %f %f", wl_fixed_to_double(surface_x), wl_fixed_to_double(surface_y));
}
void wl_connector_pointer_handle_button(void* data, struct wl_pointer* wl_pointer, uint serial, uint time, uint button, uint state)
{
    zc_log_debug("pointer handle button");
}
void wl_connector_pointer_handle_axis(void* data, struct wl_pointer* wl_pointer, uint time, uint axis, wl_fixed_t value)
{
    zc_log_debug("pointer handle axis");
}
void wl_connector_pointer_handle_frame(void* data, struct wl_pointer* wl_pointer)
{
    zc_log_debug("pointer handle frame");
}
void wl_connector_pointer_handle_axis_source(void* data, struct wl_pointer* wl_pointer, uint axis_source)
{
    zc_log_debug("pointer handle axis source");
}
void wl_connector_pointer_handle_axis_stop(void* data, struct wl_pointer* wl_pointer, uint time, uint axis)
{
    zc_log_debug("pointer handle axis stop");
}
void wl_connector_pointer_handle_axis_discrete(void* data, struct wl_pointer* wl_pointer, uint axis, int discrete)
{
    zc_log_debug("pointer handle axis discrete");
}

struct wl_pointer_listener pointer_listener =
    {
	.enter         = wl_connector_pointer_handle_enter,
	.leave         = wl_connector_pointer_handle_leave,
	.motion        = wl_connector_pointer_handle_motion,
	.button        = wl_connector_pointer_handle_button,
	.axis          = wl_connector_pointer_handle_axis,
	.frame         = wl_connector_pointer_handle_frame,
	.axis_source   = wl_connector_pointer_handle_axis_source,
	.axis_stop     = wl_connector_pointer_handle_axis_stop,
	.axis_discrete = wl_connector_pointer_handle_axis_discrete,
};

/* ***SEAT EVENTS*** */

static void
wl_connector_seat_handle_capabilities(void* data, struct wl_seat* wl_seat, enum wl_seat_capability caps)
{
    zc_log_debug("seat handle capabilities %i", caps);
    if (caps & WL_SEAT_CAPABILITY_KEYBOARD)
    {
	wlc.keyboard = wl_seat_get_keyboard(wl_seat);
	zc_log_debug("added keyboard");
    }
    if (caps & WL_SEAT_CAPABILITY_POINTER)
    {
	wlc.pointer = wl_seat_get_pointer(wl_seat);
	wl_pointer_add_listener(wlc.pointer, &pointer_listener, NULL);
	zc_log_debug("added pointer");
    }
}

static void wl_connector_seat_handle_name(void* data, struct wl_seat* wl_seat, const char* name)
{
    zc_log_debug("seat handle name %s", name);
}

const struct wl_seat_listener seat_listener = {
    .capabilities = wl_connector_seat_handle_capabilities,
    .name         = wl_connector_seat_handle_name,
};

/* ***GLOBAL EVENTS*** */

static void wl_connector_handle_global(
    void*               data,
    struct wl_registry* registry,
    uint32_t            name,
    const char*         interface,
    uint32_t            version)
{
    zc_log_debug("handle global, interface : %s, version %u", interface, version);

    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
	wlc.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
	zc_log_debug("compositor stored");
    }
    else if (strcmp(interface, wl_seat_interface.name) == 0)
    {
	wlc.seat = wl_registry_bind(registry, name, &wl_seat_interface, 4);
	zc_log_debug("seat stored");
	wl_seat_add_listener(wlc.seat, &seat_listener, NULL);
	zc_log_debug("seat listener added");
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
	wlc.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
	zc_log_debug("shm stored");
    }
    else if (strcmp(interface, wl_output_interface.name) == 0)
    {
	if (wlc.monitor_count >= 16) return;

	struct monitor_info* monitor = malloc(sizeof(struct monitor_info));
	memset(monitor->name, 0, MAX_MONITOR_NAME_LEN);
	monitor->wl_output = wl_registry_bind(registry, name, &wl_output_interface, 2);
	monitor->index     = wlc.monitor_count;

	// get wl_output events
	wl_output_add_listener(monitor->wl_output, &wl_output_listener, monitor);
	zc_log_debug("wl_output listener added");

	// set up output if it comes after xdg_output_manager_init
	if (wlc.xdg_output_manager != NULL)
	{
	    monitor->xdg_output = zxdg_output_manager_v1_get_xdg_output(wlc.xdg_output_manager, monitor->wl_output);
	    zxdg_output_v1_add_listener(monitor->xdg_output, &xdg_output_listener, monitor);
	    zc_log_debug("xdg_output listener added");
	}

	wlc.monitors[wlc.monitor_count++] = monitor;
	zc_log_debug("output stored at index : %u", wlc.monitor_count - 1);
    }
    else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
    {
	wlc.layer_shell = wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
	zc_log_debug("layer shell stored");
    }
    else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0)
    {
	wlc.xdg_output_manager = wl_registry_bind(registry, name, &zxdg_output_manager_v1_interface, 2);
	zc_log_debug("xdg output manager stored");

	// set up outputs if event comes after interface setup
	for (int index = 0; index < wlc.monitor_count; index++)
	{
	    wlc.monitors[index]->xdg_output = zxdg_output_manager_v1_get_xdg_output(wlc.xdg_output_manager, wlc.monitors[index]->wl_output);
	    zxdg_output_v1_add_listener(wlc.monitors[index]->xdg_output, &xdg_output_listener, wlc.monitors[index]);
	    zc_log_debug("xdg_output listener added");
	}
    }
}

static void wl_connector_handle_global_remove(void* data, struct wl_registry* registry, uint32_t name)
{
    zc_log_debug("handle global remove");
}

static const struct wl_registry_listener registry_listener =
    {.global        = wl_connector_handle_global,
     .global_remove = wl_connector_handle_global_remove};

int32_t round_to_int(double val)
{
    return (int32_t) (val + 0.5);
}
int wl_connector_shm_create()
{
    int  shmid = -1;
    char shm_name[NAME_MAX];
    for (int i = 0; i < UCHAR_MAX; ++i)
    {
	if (snprintf(shm_name, NAME_MAX, "/wcp-%d", i) >= NAME_MAX)
	{
	    break;
	}
	shmid = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0600);
	if (shmid > 0 || errno != EEXIST)
	{
	    break;
	}
    }

    if (shmid < 0)
    {
	zc_log_debug("shm_open() failed: %s", strerror(errno));
	return -1;
    }

    if (shm_unlink(shm_name) != 0)
    {
	zc_log_debug("shm_unlink() failed: %s", strerror(errno));
	return -1;
    }

    return shmid;
}

void* wl_connector_shm_alloc(const int shmid, const size_t size)
{
    if (ftruncate(shmid, size) != 0)
    {
	zc_log_debug("ftruncate() failed: %s", strerror(errno));
	return NULL;
    }

    void* buffer = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, shmid, 0);
    if (buffer == MAP_FAILED)
    {
	zc_log_debug("mmap() failed: %s", strerror(errno));
	return NULL;
    }

    return buffer;
}

static void wl_connector_buffer_release(void* data, struct wl_buffer* wl_buffer)
{
    zc_log_debug("buffer release");
}

static const struct wl_buffer_listener buffer_listener = {
    .release = wl_connector_buffer_release};

struct wl_buffer* wl_connector_create_buffer()
{
    zc_log_debug("create buffer");

    double factor = wlc.monitor->scale / ((double) wlc.monitor->physical_width / wlc.monitor->logical_width);

    int32_t width  = round_to_int(wlc.monitor->physical_width * factor);
    int32_t height = round_to_int(wlc.win_height * wlc.monitor->scale);

    int stride = width * 4;
    int size   = stride * height;

    zc_log_debug("factor : %f, buffer width %i height %i size %i", factor, width, height, size);

    int fd = wl_connector_shm_create();
    if (fd < 0)
    {
	zc_log_error("creating a buffer file for %d B failed: %m", size);
	return NULL;
    }
    zc_log_debug("shm file created");

    wlc.shm_data = wl_connector_shm_alloc(fd, size);

    if (wlc.shm_data == MAP_FAILED)
    {
	zc_log_error("mmap failed: %m");
	close(fd);
	return NULL;
    }

    zc_log_debug("shm data created");

    struct wl_shm_pool* pool   = wl_shm_create_pool(wlc.shm, fd, size);
    struct wl_buffer*   buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);

    wl_shm_pool_destroy(pool);

    wl_buffer_add_listener(buffer, &buffer_listener, NULL);
    zc_log_debug("buffer listener added");

    return buffer;
}

void wl_connector_draw()
{
    zc_log_debug("draw");
    double factor = wlc.monitor->scale / ((double) wlc.monitor->physical_width / wlc.monitor->logical_width);

    int32_t width  = round_to_int(wlc.monitor->physical_width * factor);
    int32_t height = wlc.win_height * wlc.monitor->scale;

    uint8_t*   argb   = wlc.shm_data;
    bm_rgba_t* bitmap = bm_rgba_new(width, height);

    gfx_rect(bitmap, 0, 0, width, height, 0x000000FF, 0);

    (*wlc.render)(0, bitmap);

    for (int i = 0; i < bitmap->size; i += 4)
    {
	argb[i]     = bitmap->data[i + 2];
	argb[i + 1] = bitmap->data[i + 1];
	argb[i + 2] = bitmap->data[i];
	argb[i + 3] = bitmap->data[i + 3];
    }

    wl_surface_attach(wlc.surface, wlc.buffer, 0, 0);
    /* zwlr_layer_surface_v1_set_keyboard_interactivity(wlc.layer_surface, true); */
    wl_surface_damage(wlc.surface, 0, 0, wlc.monitor->logical_width, wlc.win_height);
    wl_surface_commit(wlc.surface);
}

/* Layer surface listener */

static void wl_connector_layer_surface_configure(void* data, struct zwlr_layer_surface_v1* surface, uint32_t serial, uint32_t width, uint32_t height)
{
    zc_log_debug("layer surface configure serial %u width %i height %i", serial, width, height);

    zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void wl_connector_layer_surface_closed(void* _data, struct zwlr_layer_surface_v1* surface)
{
    zc_log_debug("layer surface configure");
}

struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = wl_connector_layer_surface_configure,
    .closed    = wl_connector_layer_surface_closed,
};

void wl_connector_init(int w, int h, void (*render)(uint32_t, bm_rgba_t*))
{
    zc_log_debug("init %i %i", w, h);

    wlc.win_width  = w;
    wlc.win_height = h;
    wlc.render     = render;

    wlc.display = wl_display_connect(NULL);
    if (wlc.display)
    {
	zc_log_debug("display connected");

	struct wl_registry* registry = wl_display_get_registry(wlc.display);
	wl_registry_add_listener(registry, &registry_listener, NULL);

	zc_log_debug("registry added");

	// first roundtrip triggers global events
	wl_display_roundtrip(wlc.display);

	// second roundtrip triggers events attached in global events
	wl_display_roundtrip(wlc.display);

	if (wlc.compositor)
	{
	    wlc.surface = wl_compositor_create_surface(wlc.compositor);

	    zc_log_debug("surface created");

	    wlc.monitor = wlc.monitors[0];

	    zc_log_debug("monitor selected");

	    wlc.buffer = wl_connector_create_buffer();

	    zc_log_debug("buffer created and drawn");

	    if (!wlc.layer_shell) zc_log_debug("Compositor does not implement wlr-layer-shell protocol.");

	    wlc.layer_surface = zwlr_layer_shell_v1_get_layer_surface(
		wlc.layer_shell,
		wlc.surface,
		wlc.monitor->wl_output,
		ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
		"wcp");

	    zc_log_debug("layer surface crated");

	    zwlr_layer_surface_v1_set_size(
		wlc.layer_surface,
		wlc.win_width,
		wlc.win_height);

	    zwlr_layer_surface_v1_set_anchor(
		wlc.layer_surface,
		ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT | ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP);

	    zc_log_debug("anchor and size set");

	    zwlr_layer_surface_v1_add_listener(wlc.layer_surface, &layer_surface_listener, NULL);

	    zc_log_debug("layer surface listener added");

	    wl_surface_set_buffer_scale(wlc.surface, wlc.monitor->scale);

	    zc_log_debug("scaling set");

	    wl_surface_commit(wlc.surface);

	    wl_display_roundtrip(wlc.display);

	    /* zwlr_layer_surface_v1_set_keyboard_interactivity(panel->surface.layer_surface, true); */

	    wl_surface_attach(wlc.surface, wlc.buffer, 0, 0);
	    wl_surface_commit(wlc.surface);

	    // first draw

	    wl_connector_draw();

	    struct pollfd fds[] = {
		{wl_display_get_fd(wlc.display), POLLIN}};

	    const int nfds = sizeof(fds) / sizeof(*fds);

	    wl_display_flush(wlc.display);

	    wlc.running = true;

	    while (wlc.running)
	    {
		if (wl_display_flush(wlc.display) < 0)
		{
		    if (errno == EAGAIN)
			continue;
		    break;
		}

		if (poll(fds, nfds, -1) < 0)
		{
		    if (errno == EAGAIN)
			continue;
		    break;
		}

		if (fds[0].revents & POLLIN)
		{
		    if (wl_display_dispatch(wlc.display) < 0)
		    {
			wlc.running = false;
		    }
		}
	    }

	    /* dmenu_close called */
	    wl_display_disconnect(wlc.display);
	}
	else zc_log_debug("compositor not received");
    }
    else zc_log_debug("cannot open display");
}

#endif
