#include "libpsa/header/psa/psa_receive.h"

#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

//#define DEBUG
//#define DEBUG_SET
#define MSP_WAIT_TIME 3500

#define MSP_SET_WAIT_TIME 3500

static volatile int valid_instance = 0;
static struct psa_instance global_instance;

int open_serial(int *fd, char const *const port)
{

    *fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);

    if (*fd < 0)
    {
        perror("open_serial -> open");
        return 1;
    }

    struct termios tty;
    memset(&tty, 0, sizeof(struct termios));

    if (tcgetattr(*fd, &tty) != 0)
    {
        perror("open_serial -> tcgetattr");
        close(*fd);
        return 2;
    }

    // 8N1

    tty.c_cflag &= ~PARENB;
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;

    tty.c_cflag |= CREAD | CLOCAL;

    tty.c_cc[VTIME] = 10;
    tty.c_cc[VMIN] = 1;

    cfsetspeed(&tty, B500000);

    cfmakeraw(&tty);

    if (tcsetattr(*fd, TCSANOW, &tty) != 0)
    {
        perror("open_serial -> tcsetattr TCSANOW");
        close(*fd);
        return 3;
    }

    if (tcsetattr(*fd, TCSAFLUSH, &tty) != 0)
    {
        perror("open_serial -> tcsetattr TCSAFLUSH");
        close(*fd);
        return 4;
    }

    tcflush(*fd, TCIOFLUSH);

    return 0;
}

static enum psa_status psa_send_get_command(
    PSA_IN uint16_t const ident,
    void *const buffer,
    int const buffer_len,
    int *const data_len)
{
    struct psa_instance *instance = &global_instance;

    if (instance == NULL || buffer == NULL || data_len == NULL)
    {
        // fprintf(stderr, "ERROR: psa_send_get_command() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    tcflush(instance->fd, TCIOFLUSH);

    struct psa_header *header;

    uint8_t input_buffer[100];

    uint8_t send_buffer[10];

    uint8_t crc = 0;

    memset(send_buffer, 0, sizeof(send_buffer));

    header = (struct psa_header *)send_buffer;

    header->start = 0x69;
    header->direction = '<';
    header->ident = (uint16_t)ident;
    header->size = 0;

    write(instance->fd, send_buffer, sizeof(*header));

    usleep(MSP_SET_WAIT_TIME);
    header = (struct psa_header *)input_buffer;

    uint8_t *data = (uint8_t *)(input_buffer + sizeof(*header));

    int data_read = 0;
    // lseek(instance->fd, 0, SEEK_SET);

    data_read = read(instance->fd, input_buffer, sizeof(input_buffer));

    if (data_read <= 0)
    {
        if (instance->verbose)
        {
            perror("psa_send_get_command() -> read");
        }
        return PSA_READ_ERROR;
    }

    if (header->size > buffer_len)
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_get_command() %x Receive bigger than buffer\n", ident);

        return PSA_GENERIC_ERROR;
    }

#ifdef DEBUG

    for (int i = 0; i < sizeof(*header); ++i)
    {
        printf("%02x ", input_buffer[i]);
    }
    printf("| ");
    for (int i = 0; i < header->size; ++i)
    {
        printf("%02x ", data[i]);
    }
    printf("| %02x ", data[header->size]); // CRC

    printf("\n");
    fflush(stdout);
#endif

    if (header->direction != '>' || header->ident != ident)
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_get_command() Receive Wrong packet %x\n", header->ident);

        return PSA_WRONG_PACKET;
    }

    for (unsigned long i = 2; i < header->size + sizeof(*header); ++i)
    {
        crc ^= input_buffer[i];
    }

    if (crc != data[header->size])
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_get_command() wrong crc for command %x\n", header->ident);
        return PSA_CRC_ERROR;
    }

    memcpy(buffer, data, header->size);

    *data_len = header->size;

    return PSA_OK;
}

static enum psa_status psa_send_set_command(
    PSA_IN uint16_t const ident,
    uint8_t *const data_to_send,
    int const data_len)
{
    struct psa_instance *instance = &global_instance;

    if (instance == NULL || data_to_send == NULL)
    {
        // fprintf(stderr, "ERROR: psa_send_get_command() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    tcflush(instance->fd, TCIOFLUSH);

    struct psa_header *header;

    uint8_t response_buffer[100];

    uint8_t send_buffer[100];

    uint8_t crc = 0;

    memset(send_buffer, 0, sizeof(send_buffer));

    header = (struct psa_header *)send_buffer;

    header->start = 0x69;
    header->direction = '<';
    header->ident = (uint16_t)ident;
    header->size = data_len;

    uint8_t *data_ptr = (uint8_t *)(send_buffer + sizeof(*header));

    memcpy(data_ptr, data_to_send, data_len);

    for (unsigned long i = 2; i < header->size + sizeof(*header); ++i)
    {
        crc ^= send_buffer[i];
    }

    data_ptr[data_len] = crc;

    uint8_t packet_size = sizeof(*header) + data_len + 1;

#ifdef DEBUG_SET
    printf("Sent: ");
    for (int i = 0; i < sizeof(*header); ++i)
    {
        printf("%02x ", send_buffer[i]);
    }
    printf("| ");
    for (int i = 0; i < header->size; ++i)
    {
        printf("%02x ", data_ptr[i]);
    }
    printf("| %02x ", data_ptr[header->size]); // CRC

    printf("\n");
    fflush(stdout);
#endif

    write(instance->fd, send_buffer, packet_size);

    usleep(MSP_WAIT_TIME);
    header = (struct psa_header *)response_buffer;

    uint8_t *data = (uint8_t *)(response_buffer + sizeof(*header));

    int data_read = 0;
    // lseek(instance->fd, 0, SEEK_SET);

    int response_size = 1;

    data_read = read(instance->fd, response_buffer, sizeof(*header) + response_size + 1);

    if (data_read <= 0)
    {
        if (instance->verbose)
        {
            perror("psa_send_set_command() -> read");
        }
        return PSA_READ_ERROR;
    }

    if (header->size > response_size)
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_set_command() Receive bigger than buffer\n");

        return PSA_GENERIC_ERROR;
    }

#ifdef DEBUG_SET
    printf("Received ");
    for (int i = 0; i < sizeof(*header); ++i)
    {
        printf("%02x ", response_buffer[i]);
    }
    printf("| ");
    for (int i = 0; i < header->size; ++i)
    {
        printf("%02x ", data[i]);
    }
    printf("| %02x ", data[header->size]); // CRC

    printf("\n");
    fflush(stdout);
#endif

    if (header->direction != '>' || header->ident != ident)
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_set_command() Receive Wrong packet %x\n", header->ident);

        return PSA_WRONG_PACKET;
    }

    crc = 0;

    for (unsigned long i = 2; i < header->size + sizeof(*header); ++i)
    {
        crc ^= response_buffer[i];
    }

    if (crc != data[header->size])
    {
        if (instance->verbose)
            fprintf(stderr, "psa_send_set_command() wrong crc for command %x\n", header->ident);
        return PSA_CRC_ERROR;
    }

    uint8_t response_status = data[0];
    switch (response_status)
    {
    case PSA_MSP_OK:
        return PSA_OK;
        break;
    case PSA_MSP_CRC_ERROR:
        return PSA_RESPONSE_CRC_ERROR;
        break;
    case PSA_MSP_INVALID_DATA_SIZE:
        return PSA_RESPONSE_PACKET_SIZE;
        break;
    case PSA_MSP_UNKNOWN_IDENT:
        return PSA_RESPONSE_WRONG_PACKET;
        break;
    case PSA_MSP_UNKNOWN_ERROR:
    default:
        return PSA_GENERIC_ERROR;
        break;
    }

    return PSA_OK;
}

enum psa_status psa_init(
    PSA_IN char const *const port)
{
    struct psa_instance *instance = &global_instance;
    if (instance == NULL || port == NULL)
    {
        if(instance->verbose)
            fprintf(stderr, "ERROR: psa_init() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    int status = open_serial(&(instance->fd), port);
    instance->verbose = 0;
    if (status)
    {
        fprintf(stderr, "ERROR: An erro occured after opening serial: %d\n", status);
        return PSA_GENERIC_ERROR;
    }

    valid_instance++;

    return PSA_OK;
}

enum psa_status psa_close()
{
    struct psa_instance *instance = &global_instance;
    if (valid_instance == 0)
    {
        if(instance->verbose)
            fprintf(stderr, "No instance\n");
        return PSA_NO_INSTANCE;
    }

    fprintf(stdout, "Closing psa instance no. %d\n", valid_instance);
    fflush(stdout);
    if (instance == NULL)
    {
        return PSA_NULL_ARG;
    }

    if(valid_instance == 1)
    {
        close(instance->fd);
        memset(instance, 0, sizeof(*instance));
    }

    valid_instance--;

    return PSA_OK;
}

enum psa_status psa_get_engine_data(
    PSA_OUT struct psa_engine_data *const engine_data)
{
    if (engine_data == NULL || valid_instance == 0)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_engine_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;

    status = psa_send_get_command(PSA_IDENT_ENGINE, engine_data, sizeof(*engine_data), &data_len);

    return status;
}

enum psa_status psa_get_radio_data(
    PSA_OUT struct psa_radio_data *const radio_data)
{
    if (radio_data == NULL || valid_instance == 0)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_radio_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_RADIO, radio_data, sizeof(*radio_data), &data_len);

    return status;
}

enum psa_status psa_get_radio_preset_data(
    PSA_OUT struct psa_preset_data *const preset_data)
{
    if (preset_data == NULL || valid_instance == 0)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_radio_preset_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_RADIO_PRESETS, preset_data, sizeof(*preset_data), &data_len);

    return status;
}

enum psa_status psa_get_headunit_data(
    PSA_OUT struct psa_headunit_data *const headunit_data)
{
    if (headunit_data == NULL || valid_instance == 0)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_headunit_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_HEADUNIT, headunit_data, sizeof(*headunit_data), &data_len);

    return status;
}

enum psa_status psa_get_trip_data(
    PSA_OUT struct psa_trip_data *const trip_data)
{
    if (trip_data == NULL || valid_instance == 0)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_trip_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_TRIP, trip_data, sizeof(*trip_data), &data_len);

    return status;
}

enum psa_status psa_get_vin(
    PSA_OUT struct psa_vin_data *const vin_data)
{
    if (valid_instance == 0 || vin_data == NULL)
    {
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_VIN, vin_data->vin, 17, &data_len);

    return status;
}

enum psa_status psa_get_time(
    PSA_OUT struct psa_time_data *const time_data)
{
    if (valid_instance == 0 || time_data == NULL)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_time() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    int data_len = 0;
    return psa_send_get_command(PSA_IDENT_TIME, time_data, sizeof(*time_data), &data_len);
}

enum psa_status psa_get_door_data(
    PSA_OUT struct psa_door_data *const door_data)
{
    if (valid_instance == 0 || door_data == NULL)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_door_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    enum psa_status status = PSA_OK;

    int data_len = 0;
    status = psa_send_get_command(PSA_IDENT_DOORS, door_data, sizeof(*door_data), &data_len);

    return status;
}

enum psa_status psa_get_dash_data(
    PSA_OUT struct psa_dash_data *const dash_data)
{
    if (valid_instance == 0 || dash_data == NULL)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_dash_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    int data_len = 0;
    return psa_send_get_command(PSA_IDENT_DASHBOARD, dash_data, sizeof(*dash_data), &data_len);
}

enum psa_status psa_get_car_status(
    PSA_OUT struct psa_status_data *const status_data)
{
    if (valid_instance == 0 || status_data == NULL)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_car_status() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    int data_len = 0;
    return psa_send_get_command(PSA_IDENT_CAR_STATUS, status_data, sizeof(*status_data), &data_len);
}

enum psa_status psa_get_cd_player_data(
    PSA_OUT struct psa_cd_player_data *const cd_data)
{
    if (valid_instance == 0 || cd_data == NULL)
    {
        if(global_instance.verbose)
            fprintf(stderr, "ERROR: psa_get_cd_player_data() -> argument is NULL\n");
        return PSA_NULL_ARG;
    }

    int data_len = 0;
    return psa_send_get_command(PSA_IDENT_CD_PLAYER, cd_data, sizeof(*cd_data), &data_len);
}

//enum psa_status psa_get_raw(
//    PSA_OUT struct psa_raw_data *const raw_data)
//{
//    if (valid_instance == 0 || raw_data == NULL)
//    {
//        fprintf(stderr, "ERROR: psa_get_raw() -> argument is NULL\n");
//        return PSA_NULL_ARG;
//    }

//    int data_len = 0;
//    return psa_send_get_command(PSA_IDENT_RAW, raw_data, sizeof(*raw_data), &data_len);
//}

enum psa_status psa_set_cd_changer_data(
    PSA_IN struct psa_cd_changer_data const *const cdc_data)
{
    if (valid_instance == 0 || cdc_data == NULL)
    {
        if (global_instance.verbose)
        {
            fprintf(stderr, "ERROR: psa_set_cd_changer_data() -> argument is NULL\n");
            return PSA_NULL_ARG;
        }
    }

    return psa_send_set_command(PSA_IDENT_SET_CD_CHANGER_DATA, (uint8_t *)cdc_data, sizeof(*cdc_data));
}
