#ifndef MIVE_PSA_RECV_H
#define MIVE_PSA_RECV_H

#include "libpsa/header/common/psa_argument_defs.h"

#include "libpsa/header/psa/psa_packet_defs.h"

enum psa_status
{
    PSA_OK = 0,

    PSA_INSTANCE_EXISTS,
    PSA_NO_INSTANCE,

    PSA_NULL_ARG,
    PSA_INVALID_ARG,

    PSA_READ_ERROR,
    PSA_WRITE_ERROR,

    PSA_WRONG_PACKET,
    PSA_CRC_ERROR,

    PSA_GENERIC_ERROR,

    PSA_RESPONSE_CRC_ERROR,
    PSA_RESPONSE_WRONG_PACKET,
    PSA_RESPONSE_PACKET_SIZE,
};

struct psa_instance
{
    int fd;
    unsigned char verbose;
};

#ifdef __cplusplus
extern "C"
{
#endif

    enum psa_status psa_init(
        PSA_IN char const *const port);

    enum psa_status psa_close();

    enum psa_status psa_get_engine_data(
        PSA_OUT struct psa_engine_data *const engine_data);

    enum psa_status psa_get_radio_data(
        PSA_OUT struct psa_radio_data *const radio_data);

    enum psa_status psa_get_radio_preset_data(
        PSA_OUT struct psa_preset_data *const preset_data);

    enum psa_status psa_get_headunit_data(
        PSA_OUT struct psa_headunit_data *const headunit_data);

    enum psa_status psa_get_trip_data(
        PSA_OUT struct psa_trip_data *const trip_data);

    enum psa_status psa_get_door_data(
        PSA_OUT struct psa_door_data *const door_data);

    enum psa_status psa_get_dash_data(
        PSA_OUT struct psa_dash_data *const dash_data);

    enum psa_status psa_get_vin(
        PSA_OUT struct psa_vin_data *const vin_data);

    enum psa_status psa_get_car_status(
        PSA_OUT struct psa_status_data *const status_data);

    enum psa_status psa_get_cd_player_data(
        PSA_OUT struct psa_cd_player_data *const cd_data);

    enum psa_status psa_get_time(
        PSA_OUT struct psa_time_data *const time_data);

    enum psa_status psa_get_raw(
        PSA_OUT struct psa_raw_data *const raw_data);

    enum psa_status psa_set_cd_changer_data(
        PSA_IN struct psa_cd_changer_data const *const cdc_data);

#ifdef __cplusplus
} // extern "C"
#endif // __cplusplus

#endif
