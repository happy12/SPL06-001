#ifndef SENSOR_SPL06_001_H
#define SENSOR_SPL06_001_H
/*!
 *  FILE: SPL06-001.h
 *  AUTHOR: Mathieu Fregeau
 *  VERSION: 1.0.0
 *  DATE: 2022-12-29
 *
 *  This is a library for the SPL06-001 pressure sensor
 *  datasheet: https://datasheet.lcsc.com/lcsc/2101201914_Goertek-SPL06-001_C2684428.pdf
 *
 *  Designed to work with the open-source Adafruit Sensor library.
 *
 *  This sensors use I2C to communicate, 2 pins are required to interface.
 *
 *  BSD license, all text above must be included in any redistribution
 */
// note about register reading and bytes. Some data in a 8-bit register is stored in a subset of the 8-bits. Datasheet will say which bits.
// to read only some bits from a register, first read all the 8-bit register in a variable, and "extract" (aka filter) the desired bit positions with AND operator (aka bit mask).
// for example, register contains reg=12345678, to keep only the 567, do this:
// mydata = reg & 0B00001110;  note that the notation for literal binary starts with 0B and the following 8 values can only be 0 or 1

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_SPIDevice.h>

#define SPL06_ADDRESS                   0x77 /**< Default I2C address from datasheet */
#define SPL06_ADDRESS_ALT               0x76 /**< Alternate I2C address from datasheet */

#define SPL06_PRODID (0x10) /**< Default Product ID. */

// SUPPORTED COMMANDS
enum {
    SPL06_DEVICE_ID = 0x0D,
    SPL06_PSR_B2 = 0x00,
    SPL06_PSR_B1 = 0x01,
    SPL06_PSR_B0 = 0x02,
    SPL06_TMP_B2 = 0x03,
    SPL06_TMP_B1 = 0x04,
    SPL06_TMP_B0 = 0x05,
    SPL06_PSR_CFG = 0x06,
    SPL06_TMP_CFG = 0x07,
    SPL06_MEAS_CFG = 0x08,
    SPL06_CFG_REG = 0x09,
    SPL06_INT_STS = 0x0A,
    SPL06_FIFO_STS = 0x0B,
    SPL06_SOFT_RESET = 0x0C,
    SPL06_COEF_C0 = 0x10,
    SPL06_COEF_C0C1 = 0x11,
    SPL06_COEF_C1 = 0x12,
    SPL06_COEF_C00a = 0x13,
    SPL06_COEF_C00b = 0x14,
    SPL06_COEF_C00C10 = 0x15,
    SPL06_COEF_C10a = 0x16,
    SPL06_COEF_C10b = 0x17,
    SPL06_COEF_C01a = 0x18,
    SPL06_COEF_C01b = 0x19,
    SPL06_COEF_C11a = 0x1A,
    SPL06_COEF_C11b = 0x1B,
    SPL06_COEF_C20a = 0x1C,
    SPL06_COEF_C20b = 0x1D,
    SPL06_COEF_C21a = 0x1E,
    SPL06_COEF_C21b = 0x1F,
    SPL06_COEF_C30a = 0x20,
    SPL06_COEF_C30b = 0x21,
};
/*!
 *  Struct to hold calibration data.
 */
typedef struct {
  int16_t c0, c1, c01, c11, c20, c21, c30;
  int32_t c00, c10;
} spl06_calib_data;

class SPL06; //forward declaration

/** Sensor interface for temperature component of SPL06, derived from Adafruit sensor class */
class SPL06_Temp : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the temp sensor
      @param parent A pointer to the SPL06 class */
  SPL06_Temp(SPL06 *parent) { _theSPL06 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);
private:
  int _sensorID = 0;
  SPL06 *_theSPL06 = NULL;
};

/** Sensor interface for pressure component of SPL06, derived from Adafruit sensor class */
class SPL06_Pressure : public Adafruit_Sensor {
public:
  /** @brief Create an Adafruit_Sensor compatible object for the pressure sensor
      @param parent A pointer to the SPL06 class */
  SPL06_Pressure(SPL06 *parent) { _theSPL06 = parent; }
  bool getEvent(sensors_event_t *);
  void getSensor(sensor_t *);
private:
  int _sensorID = 0;
  SPL06 *_theSPL06 = NULL;
};


class SPL06 {

public:
  /** Oversampling rate for the sensor. */
  enum sensor_sampling {
    SAMPLING_NONE = 0x00, /** No over-sampling. */
    SAMPLING_X2   = 0x01, /** 2x over-sampling. */
    SAMPLING_X4   = 0x02, /** 4x over-sampling. */
    SAMPLING_X8   = 0x03, /** 8x over-sampling. */
    SAMPLING_X16  = 0x04, /** 16x over-sampling. */
    SAMPLING_X32  = 0x05, /** 32x over-sampling. */
    SAMPLING_X64  = 0x06, /** 64x over-sampling. */
    SAMPLING_X128 = 0x07 /** 128x over-sampling. */
  };

  /** Operating mode for the sensor. */
  enum sensor_mode {
    MODE_STANDBY = 0x00, /** Standby mode. */
    MODE_FORCED_P = 0x01, /** Forced mode for Pressure. */
    MODE_FORCED_T = 0x02, /** Forced mode for Temperature. */
    MODE_BACKGND_P = 0x05, /** Background mode for Pressure. */
    MODE_BACKGND_T = 0x06, /** Background mode for Temperature. */
    MODE_BACKGND_BOTH = 0x07, /** Background mode for Both. */
    MODE_SOFT_RESET_CODE = 0x09 /** Software reset. */
  };

  /** measurement rate for sensor data. Applicable for background mode only*/
  enum sensor_rate {
    RATE_X1 = 0x00, /** 1 measurement per second. */
    RATE_X2 = 0x01, /** 2 measurement per second. */
    RATE_X4 = 0x02, /** 4 measurement per second. */
    RATE_X8 = 0x03, /** 8 measurement per second. */
    RATE_X16 = 0x04, /** 16 measurement per second. */
    RATE_X32 = 0x05, /** 32 measurement per second. */
    RATE_X64 = 0x06, /** 64 measurement per second. */
    RATE_X128 = 0x07 /** 128 measurement per second. */
  };

public:
    SPL06(TwoWire *theWire = &Wire);
    SPL06(int8_t cspin, SPIClass *theSPI = &SPI);
    SPL06(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin);
    ~SPL06();

    bool begin(uint8_t addr = SPL06_ADDRESS, uint8_t prodid = SPL06_PRODID);
    void reset(void);
    uint8_t getStatusFIFO(void);
    uint8_t sensorID(void);

    double readTemperature();//Celcius
    double readTemperatureF();//Fahrenheit
    double readPressure();//Pascal
    double readPressureKPa();//kiloPascal
    double readPressureATM();//atmosphere
    double readPressureMBar();//millibar
    double readPressurePSI();//pound per square inch
    double readPressureMMHg();//mm of mercury
    double readPressureINHg();//in of mercury
    float readPressureAltitudeMeter(float altimeterSetting_mbar=1013.25f);// in meters, correction is in millibar. Used in aviation. Default correction uses standard day sea level
    float readPressureAltitudeFeet(float altimeterSetting_inHg=29.921255f);// in feet, correction is in inches of mercury. Used in aviation. Default correction uses standard day sea level

    Adafruit_Sensor *getTemperatureSensor(void);
    Adafruit_Sensor *getPressureSensor(void);

    void setSampling(sensor_mode mode = MODE_BACKGND_BOTH,
                     sensor_sampling tempSampling = SAMPLING_X16,
                     sensor_sampling pressSampling = SAMPLING_X16,
                     sensor_rate tempRate = RATE_X16,
                     sensor_rate pressRate = RATE_X16);



protected:
    TwoWire *_wire;                     /**< Wire object */
    Adafruit_I2CDevice *i2c_dev = NULL; ///< Pointer to I2C bus interface
    Adafruit_SPIDevice *spi_dev = NULL; ///< Pointer to SPI bus interface
    SPL06_Temp *temp_sensor = NULL;
    SPL06_Pressure *pressure_sensor = NULL;

    /** Encapsulates the config register */
    struct config {
        /** Initialize to power-on-reset state: int_HL, INT_SEL, TMP_SHIFT_EN, PRS_SHIFT_EN, FIFO_EN */
        config() : int_hl(1), int_fifo(0), int_prs(0), int_tmp(0), tmp_shift_en(0), prs_shift_en(0), fifo_en(0), spi3w_en(0) {}
        unsigned int int_hl : 1;/** `1` here is the width of the register field, not the value. Interrupt. 0=Active Low, 1=Active High.  */
        unsigned int int_fifo : 1;/** Generate interrupt when the FIFO is full. 1=Enable */
        unsigned int int_prs  : 1;/** Generate interrupt when a pressure measurement is ready. 1=Enable */
        unsigned int int_tmp  : 1;/** Generate interrupt when a temperature measurement is ready. 1=Enable */
        unsigned int tmp_shift_en : 1;/** Temperature result bit-shift. Must be set to '1' when the oversampling rate is >8 times */
        unsigned int prs_shift_en : 1;/** Pressure result bit-shift.  Must be set to '1' when the oversampling rate is >8 times */
        unsigned int fifo_en : 1;/** Enable the FIFO */
        unsigned int spi3w_en : 1;/** Enables 3-wire SPI */
        /** Used to retrieve the assembled config register's byte value. */
        unsigned int get() { return (int_hl << 7) | (int_fifo << 6) | (int_prs << 5) | (int_tmp << 4) | (tmp_shift_en << 3) | (prs_shift_en << 2) | (fifo_en << 1) | spi3w_en; }
    };

    /** Encapsulates the meas_cfg register */
    struct meas_cfg {
        /** Initialize to power-on-reset state */
        meas_cfg() : coef_rdy(0), sensor_rdy(0), tmp_rdy(0), prs_rdy(0), none(0), mode(MODE_STANDBY) {}
        unsigned int coef_rdy : 1; /** Coefficient ready (read-only). 1=ready */
        unsigned int sensor_rdy : 1; /** Sensor ready (read-only). 1=ready */
        unsigned int tmp_rdy : 1; /** Temperature ready (read-only). 1=ready */
        unsigned int prs_rdy : 1; /** Pressure ready (read-only). 1=ready */
        unsigned int none : 1; /** Unused, do not set */
        unsigned int mode : 3; /** Measurement Mode */
        /** Used to retrieve the assembled ctrl_meas register's byte value. */
        unsigned int get() { return (coef_rdy << 7) | (sensor_rdy << 6) | (tmp_rdy << 5) | (prs_rdy << 4) | mode; }
    };

    /** Encapsulates the prs_cfg register */
    struct prs_cfg {
        /** Initialize to power-on-reset state */
        prs_cfg() : none(0), pm_rate(RATE_X1), pm_prc(SAMPLING_NONE) {}
        unsigned int none : 1; /** Unused, do not set */
        unsigned int pm_rate : 3; /** Pressure measurement rate. */
        unsigned int pm_prc : 4; /** Pressure Resolution/Oversampling. */
        /** Used to retrieve the assembled ctrl_meas register's byte value. */
        unsigned int get() { return (pm_rate << 4) | pm_prc; }
    };

    /** Encapsulates the tmp_cfg register */
    struct tmp_cfg {
        /** Initialize to power-on-reset state */
        tmp_cfg() : tmp_ext(1), tmp_rate(RATE_X1), none(0), tmp_prc(SAMPLING_NONE) {}
        unsigned int tmp_ext : 1; /** Temperature external setting, datasheet (p. 22) specifies to use external, which has a value 1. */
        unsigned int tmp_rate : 3; /** Temperature measurement rate. */
        unsigned int none : 1; /** Unused, do not set */
        unsigned int tmp_prc : 3; /** Temperature Resolution/Oversampling. */
        /** Used to retrieve the assembled ctrl_meas register's byte value. */
        unsigned int get() { return (tmp_ext << 7) | (tmp_rate << 4) | tmp_prc; }
    };

    void readCoefficients(void);
    int16_t get_c0(void);
    int16_t get_c1(void);
    int32_t get_c00(void);
    int32_t get_c10(void);
    int16_t get_coef(unsigned int line1, unsigned int line2);
    double get_scale_factor(const uint8_t _regToRead, byte _bitToKeep);

    int32_t get_traw();
    double get_traw_sc();

    int32_t get_praw();
    double get_praw_sc();


    void write8(byte reg, byte value);
    uint8_t  read8(byte reg);

    int8_t _sensorID = 0;
    int8_t _prodID = 0;
    int8_t _revID = 0;
    int8_t readSensorID(byte reg);

    spl06_calib_data _spl06_calib;
    config _configReg;
    meas_cfg _measReg;
    prs_cfg _pressReg;
    tmp_cfg _tempReg;

};

#endif
