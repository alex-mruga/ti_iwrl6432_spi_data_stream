#ifndef FACTORY_CAL_H
#define FACTORY_CAL_H


/*!
 * @brief
 * Structure holds calibration save configuration used during sensor open.
 *
 * @details
 *  The structure holds calibration save configuration.
 */
typedef struct Mmw_calibData_t
{
    /*! @brief      Magic word for calibration data */
    uint32_t 	    magic;

    /*! @brief      RX TX Calibration data */
    T_RL_API_FECSS_RXTX_CAL_DATA  calibData;
} Mmw_calibData;


int32_t restoreFactoryCal(void);

#endif //FACTORY_CAL_H