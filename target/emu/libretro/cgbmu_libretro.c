#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>

#include "libretro.h"
#include "libcgbmu.h"

// Gameboy screen resolution 160x144
#define DISPLAY_WIDTH   160
#define DISPLAY_HEIGHT  144

static cpu_t dmgcpu;

static uint32_t *frame_buf;

static struct retro_log_callback logging;
static retro_log_printf_t log_cb;
static retro_environment_t environ_cb;

static retro_video_refresh_t video_cb;
static retro_audio_sample_t audio_cb;
static retro_audio_sample_batch_t audio_batch_cb;
static retro_environment_t environ_cb;
static retro_input_poll_t input_poll_cb;
static retro_input_state_t input_state_cb;

static unsigned x_coord;
static unsigned y_coord;
static int mouse_rel_x;
static int mouse_rel_y;

static const uint8_t boot_rom[] = {
    0x31,0xFE,0xFF,0xAF,0x21,0xFF,0x9F,0x32,0xCB,0x7C,0x20,0xFB,0x21,0x26,0xFF,0x0E,
    0x11,0x3E,0x80,0x32,0xE2,0x0C,0x3E,0xF3,0xE2,0x32,0x3E,0x77,0x77,0x3E,0xFC,0xE0,
    0x47,0x11,0x04,0x01,0x21,0x10,0x80,0x1A,0xCD,0x95,0x00,0xCD,0x96,0x00,0x13,0x7B,
    0xFE,0x34,0x20,0xF3,0x11,0xD8,0x00,0x06,0x08,0x1A,0x13,0x22,0x23,0x05,0x20,0xF9,
    0x3E,0x19,0xEA,0x10,0x99,0x21,0x2F,0x99,0x0E,0x0C,0x3D,0x28,0x08,0x32,0x0D,0x20,
    0xF9,0x2E,0x0F,0x18,0xF3,0x67,0x3E,0x64,0x57,0xE0,0x42,0x3E,0x91,0xE0,0x40,0x04,
    0x1E,0x02,0x0E,0x0C,0xF0,0x44,0xFE,0x90,0x20,0xFA,0x0D,0x20,0xF7,0x1D,0x20,0xF2,
    0x0E,0x13,0x24,0x7C,0x1E,0x83,0xFE,0x62,0x28,0x06,0x1E,0xC1,0xFE,0x64,0x20,0x06,
    0x7B,0xE2,0x0C,0x3E,0x87,0xE2,0xF0,0x42,0x90,0xE0,0x42,0x15,0x20,0xD2,0x05,0x20,
    0x4F,0x16,0x20,0x18,0xCB,0x4F,0x06,0x04,0xC5,0xCB,0x11,0x17,0xC1,0xCB,0x11,0x17,
    0x05,0x20,0xF5,0x22,0x23,0x22,0x23,0xC9,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,
    0x03,0x73,0x00,0x83,0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,
    0xDC,0xCC,0x6E,0xE6,0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,
    0xDD,0xDC,0x99,0x9F,0xBB,0xB9,0x33,0x3E,0x3C,0x42,0xB9,0xA5,0xB9,0xA5,0x42,0x3C,
    0x21,0x04,0x01,0x11,0xA8,0x00,0x1A,0x13,0xBE,0x20,0xFE,0x23,0x7D,0xFE,0x34,0x20,
    0xF5,0x06,0x19,0x78,0x86,0x23,0x05,0x20,0xFB,0x86,0x20,0xFE,0x3E,0x01,0xE0,0x50,
    0x00,0xC3,0x50,0x01,0xCE,0xED,0x66,0x66,0xCC,0x0D,0x00,0x0B,0x03,0x73,0x00,0x83,
    0x00,0x0C,0x00,0x0D,0x00,0x08,0x11,0x1F,0x88,0x89,0x00,0x0E,0xDC,0xCC,0x6E,0xE6,
    0xDD,0xDD,0xD9,0x99,0xBB,0xBB,0x67,0x63,0x6E,0x0E,0xEC,0xCC,0xDD,0xDC,0x99,0x9F,
    0xBB,0xB9,0x33,0x3E,0x54,0x45,0x54,0x52,0x49,0x53,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x0B,0x89,0xB5,
    0xC3,0x8B,0x02,0xCD,0x2B,0x2A,0xF0,0x41,0xE6,0x03,0x20,0xFA,0x46,0xF0,0x41,0xE6,
    0x03,0x20,0xFA,0x7E,0xA0,0xC9,0x7B,0x86,0x27,0x22,0x7A,0x8E,0x27,0x22,0x3E,0x00,
    0x8E,0x27,0x77,0x3E,0x01,0xE0,0xE0,0xD0,0x3E,0x99,0x32,0x32,0x77,0xC9,0xF5,0xE5,
};


static void render_checkered(void)
{
    uint32_t *buf = frame_buf;
    unsigned stride = DISPLAY_WIDTH;
    uint32_t color_r = 0xff << 16;
    uint32_t color_g = 0xff << 8;
    uint32_t *line = buf;

    for (unsigned y = 0; y < DISPLAY_HEIGHT; y++, line += stride)
    {
        unsigned index_y = ((y - y_coord) >> 4) & 1;
        for (unsigned x = 0; x < DISPLAY_WIDTH; x++)
        {
            unsigned index_x = ((x - x_coord) >> 4) & 1;
            line[x] = (index_y ^ index_x) ? color_r : color_g;
        }
    }

    //for (unsigned y = mouse_rel_y - 5; y <= mouse_rel_y + 5; y++)
    //   for (unsigned x = mouse_rel_x - 5; x <= mouse_rel_x + 5; x++)
    //     buf[y * stride + x] = 0xff;

    video_cb(buf, DISPLAY_WIDTH, DISPLAY_HEIGHT, stride << 2);
}

static void update_input(void)
{
    input_poll_cb();
    if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, RETRO_DEVICE_ID_JOYPAD_UP))
    {
        /* stub */
    }
}

static void fallback_log(enum retro_log_level level, const char *fmt, ...)
{
    (void)level;
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
}

uint8_t readButtons(void)
{
    return 0;
}

void pushScanLine(cpu_t *cpu) {
    const uint32_t lcd_pal[] = { 0x9CBD10, 0x8CAD10, 0x316331, 0x103910 };

    uint8_t *pixel = cpu->screen_line;
    uint8_t *end = pixel + DISPLAY_WIDTH;

    uint32_t *buf = &frame_buf[cpu->IOLY * DISPLAY_WIDTH];

    while (pixel < end) {
        *buf++ = lcd_pal[*pixel++];
    }
}
/**
*
*/
unsigned retro_api_version(void)
{
    return RETRO_API_VERSION;
}

void retro_init()
{
    frame_buf = calloc(DISPLAY_WIDTH * DISPLAY_HEIGHT, sizeof(uint32_t));
}

void retro_deinit()
{
    free(frame_buf);
    frame_buf = NULL;
}

void retro_get_system_info(struct retro_system_info* info)
{
    memset(info, 0, sizeof(struct retro_system_info));

    info->library_name = "cgbmu";
    info->library_version = "0.0.0";
    info->need_fullpath = false;
    info->valid_extensions = "bin|gb";
}

void retro_set_environment(retro_environment_t cb)
{
    environ_cb = cb;

    bool no_content = true;
    environ_cb(RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME, &no_content);

    if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &logging))
        log_cb = logging.log;
    else
        log_cb = fallback_log;
}

void retro_get_system_av_info(struct retro_system_av_info* info)
{
    memset(info, 0, sizeof(struct retro_system_av_info));

    info->timing.fps = 60.0f;
    info->timing.sample_rate = 0.0f;
    info->geometry.base_width = DISPLAY_WIDTH;
    info->geometry.base_height = DISPLAY_HEIGHT;
    info->geometry.max_width = DISPLAY_WIDTH;
    info->geometry.max_height = DISPLAY_HEIGHT;
    info->geometry.aspect_ratio = 4.0f / 3.0f;
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
    log_cb(RETRO_LOG_INFO, "Plugging device %u into port %u.\n", device, port);
}

unsigned retro_get_region(void)
{
    return RETRO_REGION_NTSC;
}

bool retro_load_game(const struct retro_game_info* info)
{
    log_cb(RETRO_LOG_INFO, "Loading rom\n");

    enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;

    if (!environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt))
    {
        log_cb(RETRO_LOG_INFO, "XRGB8888 is not supported.\n");
        return false;
    }

    if (info && info->data)
    {
        cartridgeInit(&dmgcpu, (const uint8_t*)info->data);
    }
    else {
        cartridgeInit(&dmgcpu, (const uint8_t*)boot_rom);
        dmgcpu.PC = 0;
    }

    initCpu(&dmgcpu);

    return true;
}

void retro_unload_game(void)
{
    cartridgeDeInit(&dmgcpu);
}

bool retro_load_game_special(unsigned type, const struct retro_game_info *info, size_t num)
{
    log_cb(RETRO_LOG_INFO, "%s Not implemented\n", __func__);
    return false;

#if 0
    if (type != 0x200)
        return false;
    if (num != 2)
        return false;
    return retro_load_game(NULL);
#endif
}

void retro_run(void)
{
    //update_input();
    //render_checkered();
    //audio_cb(0, 0);

    while (1) {
        decode(&dmgcpu);
        if (video(&dmgcpu))
        {
            video_cb(frame_buf, DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_WIDTH << 2);
            break;
        }
        timer(&dmgcpu);
        serial(&dmgcpu);
        interrupts(&dmgcpu);
    }
}

void retro_reset(void)
{
    initCpu(&dmgcpu);
}

void retro_set_audio_sample(retro_audio_sample_t cb)
{
    audio_cb = cb;
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
    audio_batch_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
    input_poll_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
    input_state_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
    video_cb = cb;
}

size_t retro_serialize_size(void)
{
    int size = sizeof(cpu_t);

    log_cb(RETRO_LOG_DEBUG, "state size %d\n", size);
    
    return size;
}

bool retro_serialize(void *data_, size_t size)
{
    if (size < sizeof(cpu_t))
        return false;

    memcpy(data_, &dmgcpu, size);

    return true;
}

bool retro_unserialize(const void *data_, size_t size)
{
    if (size < sizeof(cpu_t))
        return false;

    memcpy(&dmgcpu, data_, size);
    
    return true;
}

void *retro_get_memory_data(unsigned id)
{
    (void)id;
    return NULL;
}

size_t retro_get_memory_size(unsigned id)
{
    (void)id;
    return 0;
}

void retro_cheat_reset(void)
{
}

void retro_cheat_set(unsigned index, bool enabled, const char *code)
{
    (void)index;
    (void)enabled;
    (void)code;
}
