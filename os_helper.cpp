#include "os_helper.h"

#include <fcntl.h>
#include <unistd.h>
#include <string.h>

os_helper::os_helper(QObject *parent)
    : QObject{parent}
{
    cpu_temp_file = open("/sys/class/thermal/thermal_zone0/hwmon0/temp1_input", O_RDONLY);
    lcd_bright_file = open("/sys/class/backlight/rpi_backlight/brightness", O_RDWR);
    lcd_backlight_file = open("/sys/class/backlight/rpi_backlight/bl_power", O_RDWR);
}

int os_helper::get_cpu_temp()
{
    int cputemp = 0;

    char temp[10];

    memset(temp, 0, 10);

    size_t bytes_read = 0;

    bytes_read = read(cpu_temp_file, temp, 10);

    lseek(cpu_temp_file, 0, SEEK_SET);

    if(bytes_read <= 0)
    {
        return 0;
    }

    sscanf(temp, "%d", &cputemp);

    return cputemp / 1000;
}

int os_helper::get_lcd_brightness()
{
    int brightness = 0;

    char temp[10];

    memset(temp, 0, 10);

    size_t bytes_read = 0;

    bytes_read = read(lcd_bright_file, temp, 10);

    lseek(lcd_bright_file, 0, SEEK_SET);

    if(bytes_read <= 0)
    {
        return -1;
    }

    sscanf(temp, "%d", &brightness);

    return brightness;
}

int os_helper::set_lcd_brightness(int brightness)
{
    if(brightness < 0 || brightness > 255)
    {
        fprintf(stderr, "Invalid brightness level %d\n", brightness);
        return -1;
    }


    char temp[10];
    sprintf(temp, "%d", brightness);

    size_t bytes_read = 0;

    bytes_read = write(lcd_bright_file, temp, strlen(temp));

    lseek(lcd_bright_file, 0, SEEK_SET);


    if(bytes_read <= 0)
    {
        return -1;
    }

    return 0;
}

int os_helper::get_lcd_backlight()
{
    int backlight = 0;

    char temp[10];

    memset(temp, 0, 10);

    size_t bytes_read = 0;

    bytes_read = read(lcd_backlight_file, temp, 10);

    lseek(lcd_backlight_file, 0, SEEK_SET);

    if(bytes_read <= 0)
    {
        return -1;
    }

    sscanf(temp, "%d", &backlight);

    return backlight;

}

int os_helper::set_lcd_backlight(bool on)
{
    char temp[10];
    if(on)
    {
        sprintf(temp, "%d", 0);
    }
    else{
        sprintf(temp, "%d", 1);
    }

    size_t bytes_read = 0;

    bytes_read = write(lcd_bright_file, temp, strlen(temp));

    lseek(lcd_bright_file, 0, SEEK_SET);

    if(bytes_read <= 0)
    {
        return -1;
    }

    return 0;
}

os_helper::~os_helper()
{
    close(cpu_temp_file);
    close(lcd_bright_file);
    close(lcd_backlight_file);
}
