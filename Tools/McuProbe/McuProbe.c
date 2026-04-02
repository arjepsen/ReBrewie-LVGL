// Tools/McuProbe/mcu_probe.c
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

// =========================== PROTOTYPES =================================
static uint64_t now_ms_monotonic();
static speed_t baud_to_speed(int baud);
static int configure_serial(int fd, int baud);
static void usage(const char *argv0);
static int count_fields_tabs(const char *s);

// =========================== PROTOTYPES =================================
// ============================== MAIN ====================================
int main(int argc, char **argv)
{
    const char *device = "/dev/ttyS1";
    int baud = 115200;
    const char *log_path = NULL;
    bool print_stats = true;

    static struct option long_opts[] = {
        {"device", required_argument, 0, 'd'},
        {"baud", required_argument, 0, 'b'},
        {"log", required_argument, 0, 'l'},
        {"no-stats", no_argument, 0, 'n'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}};

    int c;
    while ((c = getopt_long(argc, argv, "d:b:l:nh", long_opts, NULL)) != -1)
    {
        switch (c)
        {
        case 'd':
            device = optarg;
            break;
        case 'b':
            baud = atoi(optarg);
            break;
        case 'l':
            log_path = optarg;
            break;
        case 'n':
            print_stats = false;
            break;
        case 'h':
            usage(argv[0]);
            return 0;
        default:
            usage(argv[0]);
            return 2;
        }
    }

    int fd = open(device, O_RDONLY | O_NOCTTY);
    if (fd < 0)
    {
        fprintf(stderr, "ERROR: open(%s) failed: %s\n", device, strerror(errno));
        return 1;
    }

    if (configure_serial(fd, baud) != 0)
    {
        fprintf(stderr, "ERROR: configure_serial(%s, %d) failed: %s\n", device, baud, strerror(errno));
        close(fd);
        return 1;
    }

    FILE *logf = NULL;
    if (log_path)
    {
        logf = fopen(log_path, "a");
        if (!logf)
        {
            fprintf(stderr, "ERROR: fopen(%s) failed: %s\n", log_path, strerror(errno));
            close(fd);
            return 1;
        }
        setvbuf(logf, NULL, _IOLBF, 0);
    }

    setvbuf(stdout, NULL, _IOLBF, 0);

    char line[1024];
    size_t idx = 0;

    while (1)
    {
        unsigned char buf[256];
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0)
        {
            if (errno == EAGAIN || errno == EINTR)
                continue;
            fprintf(stderr, "ERROR: read() failed: %s\n", strerror(errno));
            break;
        }
        if (r == 0)
            continue;

        for (ssize_t i = 0; i < r; i++)
        {
            unsigned char ch = buf[i];

            if (ch == '\r')
                continue; // normalize CRLF
            if (ch == '\n')
            {
                line[idx] = '\0';
                uint64_t tms = now_ms_monotonic();
                int fields = print_stats ? count_fields_tabs(line) : 0;

                if (print_stats)
                {
                    printf("%" PRIu64 "\t%s\t[len=%zu fields=%d]\n", tms, line, idx, fields);
                    if (logf)
                        fprintf(logf, "%" PRIu64 "\t%s\t[len=%zu fields=%d]\n", tms, line, idx, fields);
                }
                else
                {
                    printf("%" PRIu64 "\t%s\n", tms, line);
                    if (logf)
                        fprintf(logf, "%" PRIu64 "\t%s\n", tms, line);
                }

                idx = 0;
            }
            else
            {
                if (idx < sizeof(line) - 1)
                {
                    line[idx++] = (char)ch;
                }
                else
                {
                    // line too long; flush what we have as a truncated line
                    line[sizeof(line) - 1] = '\0';
                    uint64_t tms = now_ms_monotonic();
                    printf("%" PRIu64 "\t%s\t[len=%zu TRUNC]\n", tms, line, idx);
                    if (logf)
                        fprintf(logf, "%" PRIu64 "\t%s\t[len=%zu TRUNC]\n", tms, line, idx);
                    idx = 0;
                }
            }
        }
    }

    if (logf)
        fclose(logf);
    close(fd);
    return 0;
}

// ======================== IMPLEMENTATIONS ====================================

static uint64_t now_ms_monotonic()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)ts.tv_nsec / 1000000ULL;
}

static speed_t baud_to_speed(int baud)
{
    switch (baud)
    {
    case 9600:
        return B9600;
    case 19200:
        return B19200;
    case 38400:
        return B38400;
    case 57600:
        return B57600;
    case 115200:
        return B115200;
    case 230400:
        return B230400;
    default:
        return 0;
    }
}

static int configure_serial(int fd, int baud)
{
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0)
        return -1;

    // Raw-ish (cfmakeraw equivalent)
    tio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tio.c_oflag &= ~OPOST;
    tio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    // 8N1 + local + enable receiver
    tio.c_cflag &= ~(CSIZE | PARENB | CSTOPB);
    tio.c_cflag |= (CS8 | CLOCAL | CREAD);

    // No flow control
#ifdef CRTSCTS
    tio.c_cflag &= ~CRTSCTS;
#endif
    tio.c_iflag &= ~(IXON | IXOFF | IXANY);

    // Non-blocking-ish read with timeout
    tio.c_cc[VMIN]  = 0;
    tio.c_cc[VTIME] = 2; // 200ms

    speed_t spd = baud_to_speed(baud);
    if (!spd) { errno = EINVAL; return -1; }

    cfsetispeed(&tio, spd);
    cfsetospeed(&tio, spd);

    if (tcsetattr(fd, TCSANOW, &tio) != 0)
        return -1;

    tcflush(fd, TCIFLUSH);
    return 0;
}

static void usage(const char *argv0)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "Options:\n"
            "  -d, --device <path>   Serial device (default: /dev/ttyS1)\n"
            "  -b, --baud <rate>     Baud rate (default: 115200)\n"
            "  -l, --log <file>      Also append to log file\n"
            "  -n, --no-stats        Do not print stats (len/fields)\n"
            "  -h, --help            Show help\n"
            "\n"
            "Output format:\n"
            "  <monotonic_ms>\\t<line>\\t[len=<n> fields=<m>]\n",
            argv0);
}

static int count_fields_tabs(const char *s)
{
    // Count tab-separated fields; empty line => 0
    if (!s || !*s)
        return 0;
    int fields = 1;
    for (const char *p = s; *p; ++p)
    {
        if (*p == '\t')
            fields++;
    }
    return fields;
}