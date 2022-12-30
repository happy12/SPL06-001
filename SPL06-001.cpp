#include "SPL06-001.h"

/*!
 *  @file SPL06-001.cpp
 *
 *  This is a library for the SPL06-001 pressure sensor
 *
 *  Designed to work with the open-source Adafruit Sensor library.
 *
 *  This sensors use I2C to communicate, 2 pins are required to interface.
 *
 *  Mathieu Fregeau 12/29/2022
 *
 *  BSD license, all text above must be included in any redistribution
 */

SPL06::SPL06(TwoWire *theWire): _wire(NULL), i2c_dev(NULL), spi_dev(NULL), temp_sensor(NULL), pressure_sensor(NULL)
{
    _wire = theWire;
    temp_sensor = new SPL06_Temp(this);
    pressure_sensor = new SPL06_Pressure(this);

}
SPL06::SPL06(int8_t cspin, SPIClass *theSPI): _wire(NULL), i2c_dev(NULL), spi_dev(NULL), temp_sensor(NULL), pressure_sensor(NULL)
{
    spi_dev = new Adafruit_SPIDevice(cspin, 1000000, SPI_BITORDER_MSBFIRST, SPI_MODE0, theSPI);
    temp_sensor = new SPL06_Temp(this);
    pressure_sensor = new SPL06_Pressure(this);
}
SPL06::SPL06(int8_t cspin, int8_t mosipin, int8_t misopin, int8_t sckpin): _wire(NULL), i2c_dev(NULL), spi_dev(NULL), temp_sensor(NULL), pressure_sensor(NULL)
{
    spi_dev = new Adafruit_SPIDevice(cspin, sckpin, misopin, mosipin);
    temp_sensor = new SPL06_Temp(this);
    pressure_sensor = new SPL06_Pressure(this);
}
SPL06::~SPL06() {
    if (spi_dev)
        delete spi_dev;
    if (i2c_dev)
        delete i2c_dev;
    if (temp_sensor)
        delete temp_sensor;
    if (pressure_sensor)
        delete pressure_sensor;
}

bool SPL06::begin(uint8_t addr, uint8_t prodid)
{
    if (spi_dev == NULL) { // I2C mode
        if (i2c_dev)
            delete i2c_dev;
        i2c_dev = new Adafruit_I2CDevice(addr, _wire);
        if (!i2c_dev->begin())
            return false;
    }
    else {  // SPI mode
        if (!spi_dev->begin())
            return false;
    }

  // check if sensor, i.e. the chip ID is correct
    //_sensorID = read8(SPL06_DEVICE_ID);
    _sensorID = readSensorID(SPL06_DEVICE_ID);
    //Serial.printf("Prod ID: [%d] Rev[%d]\n",_prodID,_revID);
    if (_sensorID != prodid)
        return false;

    readCoefficients();
    // write8(BMP280_REGISTER_CONTROL, 0x3F); /* needed? */
    setSampling();
    delay(100); //not sure this is required; delays are usually a bad idea with arduino ide..
    return true;
}

void SPL06::setSampling(sensor_mode mode,
                        sensor_sampling tempSampling,
                        sensor_sampling pressSampling,
                        sensor_rate tempRate,
                        sensor_rate pressRate)
{
    if (!_sensorID)
        return; // begin() not called yet
    _measReg.mode = mode;

    if (pressRate > RATE_X8)
        _configReg.prs_shift_en = 1;
    else
        _configReg.prs_shift_en = 0;

    if (tempRate > RATE_X8)
        _configReg.tmp_shift_en = 1;
    else
        _configReg.tmp_shift_en = 0;

    _pressReg.pm_rate = pressRate;
    _pressReg.pm_prc = pressSampling;

    _tempReg.tmp_rate = tempRate;
    _tempReg.tmp_prc = tempSampling;


    write8(SPL06_CFG_REG, _configReg.get());
    write8(SPL06_MEAS_CFG, _measReg.get());
    write8(SPL06_PSR_CFG, _pressReg.get());
    write8(SPL06_TMP_CFG, _tempReg.get());
}
void SPL06::reset(void) {
  write8(SPL06_SOFT_RESET, MODE_SOFT_RESET_CODE);
}
uint8_t SPL06::sensorID(void)
{
    return _sensorID;
}
uint8_t SPL06::getStatusFIFO(void)
{
    return read8(SPL06_FIFO_STS);
}
Adafruit_Sensor *SPL06::getTemperatureSensor(void) {
  return temp_sensor;
}
Adafruit_Sensor *SPL06::getPressureSensor(void) {
  return pressure_sensor;
}
void SPL06::write8(byte reg, byte value)
{
    byte buffer[2];
    buffer[1] = value;
    if (i2c_dev) {
        buffer[0] = reg;
        i2c_dev->write(buffer, 2);
    }
    else {
        buffer[0] = reg & ~0x80;
        spi_dev->write(buffer, 2);
    }
}

uint8_t SPL06::read8(byte reg)
{
    uint8_t buffer[1];
    if (i2c_dev) {
        buffer[0] = uint8_t(reg);
        i2c_dev->write_then_read(buffer, 1, buffer, 1);
    }
    else {
        buffer[0] = uint8_t(reg | 0x80);
        spi_dev->write_then_read(buffer, 1, buffer, 1);
    }
    return buffer[0];
}

void SPL06::readCoefficients()
{
  _spl06_calib.c0 = get_c0();
  _spl06_calib.c1 = get_c1();
  _spl06_calib.c01 = get_coef(SPL06_COEF_C01a, SPL06_COEF_C01b);
  _spl06_calib.c11 = get_coef(SPL06_COEF_C11a, SPL06_COEF_C11b);
  _spl06_calib.c20 = get_coef(SPL06_COEF_C20a, SPL06_COEF_C20b);
  _spl06_calib.c21 = get_coef(SPL06_COEF_C21a, SPL06_COEF_C21b);
  _spl06_calib.c30 = get_coef(SPL06_COEF_C30a, SPL06_COEF_C30b);
  _spl06_calib.c00 = get_c00();
  _spl06_calib.c10 = get_c10();
}
int8_t SPL06::readSensorID(byte reg)
{
    int16_t tmp = read8(reg);

    //datasheet page 27
    _prodID = tmp & 0B11110000; //only read the first 4 bits using bit mask
    _revID  = tmp & 0B00001111; //only read the last 4 bits using bit mask

    return _prodID;
}
int16_t SPL06::get_c0(void)
{
    //datasheet table 10 page 28
    uint8_t tmp_MSB = read8(SPL06_COEF_C0);
    uint8_t tmp_LSB = read8(SPL06_COEF_C0C1);//then use the bit7 to bit4 only

    tmp_LSB = tmp_LSB >> 4;
    int16_t tmp = (tmp_MSB << 4) | tmp_LSB;

    if(tmp & (1 << 11)) // Check for 2's complement negative number
        tmp = tmp | 0XF000; // Set left bits to one for 2's complement conversion of negative number

    return tmp;
}
int16_t SPL06::get_c1(void)
{
    //datasheet table 10 page 28
    uint8_t tmp_MSB = read8(SPL06_COEF_C0C1);
    uint8_t tmp_LSB = read8(SPL06_COEF_C1);

    tmp_MSB = tmp_MSB & 0XF;

    int16_t tmp = (tmp_MSB << 8) | tmp_LSB;

    if(tmp & (1 << 11)) // Check for 2's complement negative number
        tmp = tmp | 0XF000; // Set left bits to one for 2's complement conversion of negative number

    return tmp;
}
int32_t SPL06::get_c00(void)
{
    //datasheet table 10 page 28
    uint8_t tmp_MSB = read8(SPL06_COEF_C00a);
    uint8_t tmp_LSB = read8(SPL06_COEF_C00b);
    uint8_t tmp_XLSB = read8(SPL06_COEF_C00C10);

    tmp_XLSB = tmp_XLSB >> 4; //only keep the first 4 bits

    int32_t tmp = (tmp_MSB << 8) | tmp_LSB;
    tmp = (tmp << 4) | tmp_XLSB;

    tmp = (uint32_t)tmp_MSB << 12 | (uint32_t)tmp_LSB << 4 | (uint32_t)tmp_XLSB >> 4;

    if(tmp & (1 << 19))
        tmp = tmp | 0XFFF00000; // Set left bits to one for 2's complement conversion of negative number

    return tmp;
}
int32_t SPL06::get_c10(void)
{
    //datasheet table 10 page 28
    uint8_t tmp_MSB = read8(SPL06_COEF_C00C10);
    uint8_t tmp_LSB = read8(SPL06_COEF_C10a);
    uint8_t tmp_XLSB = read8(SPL06_COEF_C10b);

    tmp_MSB = tmp_MSB & 0B00001111; //only read the last 4 bits using a bit mask

    int32_t tmp = (tmp_MSB << 4) | tmp_LSB;
    tmp = (tmp << 8) | tmp_XLSB;

    tmp = (uint32_t)tmp_MSB << 16 | (uint32_t)tmp_LSB << 8 | (uint32_t)tmp_XLSB;

    if(tmp & (1 << 19))
        tmp = tmp | 0XFFF00000; // Set left bits to one for 2's complement conversion of negative number

    return tmp;
}
int16_t SPL06::get_coef(unsigned int line1, unsigned int line2)
{
    //datasheet table 10 page 28
    uint8_t tmp_MSB = read8(line1);
    uint8_t tmp_LSB = read8(line2);
    return (tmp_MSB << 8) | tmp_LSB;
}
//scaling factor depends on sampling rate that it is set at
double SPL06::get_scale_factor(const uint8_t _regToRead, byte _bitToKeep)
{
    //scale factor datasheet table 4 page 13
    uint8_t tmp_Byte = read8(_regToRead);
    tmp_Byte = tmp_Byte & _bitToKeep; //filter out which bits to keep using provided bit mask
    switch (tmp_Byte){
    case 0B000:
        return 524288.0d;
        break;
    case 0B001:
        return 1572864.0d;
        break;
    case 0B010:
        return 3670016.0d;
        break;
    case 0B011:
        return 7864320.0d;
        break;
    case 0B100:
        return 253952.0d;
        break;
    case 0B101:
        return 516096.0d;
        break;
    case 0B110:
        return 1040384.0d;
        break;
    case 0B111:
        return 2088960.0d;
        break;
    default:
        return 0.0;
        break;
    }//switch
    return 0.0;//this should result in a "NAN" value, which should bring the attention to the user
}
int32_t SPL06::get_traw()
{
    //raw temperature from the 24bit register; datasheet table 7 page 17
    if (!_sensorID)
        return int32_t(NAN); // begin() not called yet

    uint8_t tmp_MSB = read8(SPL06_TMP_B2); // MSB
    uint8_t tmp_LSB = read8(SPL06_TMP_B1); // LSB
    uint8_t tmp_XLSB = read8(SPL06_TMP_B0); // XLSB

    int32_t tmp = (tmp_MSB << 8) | tmp_LSB;
    tmp = (tmp << 8) | tmp_XLSB;

    if(tmp & (1 << 23))
        tmp = tmp | 0XFF000000; // Set left bits to one for 2's complement conversion of negative number
    return tmp;
}
double SPL06::get_traw_sc()
{
    //scaled raw temperature, datasheet page 13
    double k = get_scale_factor(SPL06_TMP_CFG, 0B00000111); //temperature keep last 3 bit using bit mask
	return ( double( get_traw() )/k );
}
//value in Celcius
double SPL06::readTemperature()
{
    //compensated temperature, datasheet page 13
    if (!_sensorID)
        return NAN; // begin() not called yet
	return ( (double(_spl06_calib.c0) * 0.5d) + (double(_spl06_calib.c1) * get_traw_sc()) );
}
//value in Fahrenheit
double SPL06::readTemperatureF()
{
    //compensated temperature, datasheet page 13; transformed to Fahrenheit
    if (!_sensorID)
        return NAN; // begin() not called yet
	return (((double(_spl06_calib.c0) * 0.5d) + (double(_spl06_calib.c1) * get_traw_sc())) * 9.0d/5.0d) + 32.0d;
}
int32_t SPL06::get_praw()
{
    //raw pressure from the 24bit register; datasheet table 7 page 17
    if (!_sensorID)
        return int32_t(NAN); // begin() not called yet

    uint8_t tmp_MSB = read8(SPL06_PSR_B2); // MSB
    uint8_t tmp_LSB = read8(SPL06_PSR_B1); // LSB
    uint8_t tmp_XLSB = read8(SPL06_PSR_B0); // XLSB

    int32_t tmp = (tmp_MSB << 8) | tmp_LSB;
    tmp = (tmp << 8) | tmp_XLSB;

    if(tmp & (1 << 23))
        tmp = tmp | 0XFF000000; // Set left bits to one for 2's complement conversion of negative number
    return tmp;
}
double SPL06::get_praw_sc()
{
    //scaled raw pressure, datasheet page 12
    double k = get_scale_factor(SPL06_PSR_CFG, 0B00001111); //pressure keep last 4 bit using bit mask
	return ( double(get_praw())/k );
}
//in Pascal
double SPL06::readPressure()
{
    //compensated pressure, datasheet page 12
	int32_t c00 = _spl06_calib.c00;//notice the int type is 32
	int32_t c10 = _spl06_calib.c10;//notice the int type is 32
	int16_t c01 = _spl06_calib.c01;
	int16_t c11 = _spl06_calib.c11;
	int16_t c20 = _spl06_calib.c20;
	int16_t c21 = _spl06_calib.c21;
	int16_t c30 = _spl06_calib.c30;
	double traw_sc = get_traw_sc();
	double praw_sc = get_praw_sc();
	return ( double(c00) + praw_sc * (double(c10) + praw_sc * (double(c20) + praw_sc * double(c30))) + traw_sc * double(c01) + traw_sc * praw_sc * ( double(c11) + praw_sc * double(c21)) );
}
//unit is atmosphere
double SPL06::readPressureATM()
{
    return (readPressure()/(101325.0d));
}
//unit is kiloPascal
double SPL06::readPressureKPa()
{
    return (readPressure()/(1000.0d));
}
//unit is milli-bar
double SPL06::readPressureMBar()
{
    return (readPressure()/(100.0d));
}
//unit is PSI
double SPL06::readPressurePSI()
{
	return ((readPressure()*0.0254d*0.0254d)/(0.45359237d * 9.80665d));
}
//mm of mercury
double SPL06::readPressureMMHg()
{
    // 1 mmHg is equal exactly to 133.322387415 pascal
    return (readPressure()/133.322387415d);
}
//in of mercury
double SPL06::readPressureINHg()
{
    // 1 mmHg is equal exactly to 133.322387415 pascal
    return (readPressure()/(25.4d*133.322387415d));
}
//estimated altitude in meters
//reference: https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
float SPL06::readPressureAltitudeMeter(float altimeterSetting_mbar)
{
    double pressure_mbar = readPressureMBar();
    //0.190284 = 1.0/5.255
    return (0.3048d*(1.0d-pow((pressure_mbar/altimeterSetting_mbar),(1.0d/5.255d) ))*145366.45d);//correct the formula for meters using 0.3048
}
//estimated altitude in feet
//reference: https://www.weather.gov/media/epz/wxcalc/pressureAltitude.pdf
float SPL06::readPressureAltitudeFeet(float altimeterSetting_inHg)
{
    double pressure_inHg = readPressureINHg();
    return ((1.0d-pow((pressure_inHg/altimeterSetting_inHg),(1.0d/5.255d) ))*145366.45d);//do not correct the formula for meters
}


/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the BMP280's temperature sensor
*/
/**************************************************************************/
void SPL06_Temp::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "SPL06-001", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  sensor->min_delay = 0;
  sensor->min_value = -40.0; /* Temperature range -40 ~ +85 C  */
  sensor->max_value = +85.0;
  sensor->resolution = 0.5; /*  SPL06-001 is indeed not that accurate, but hey it's a cheaper option */
}

/**************************************************************************/
/*!
    @brief  Gets the temperature as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool SPL06_Temp::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_AMBIENT_TEMPERATURE;
  event->timestamp = millis();
  event->temperature = _theSPL06->readTemperature();
  return true;
}

/**************************************************************************/
/*!
    @brief  Gets the sensor_t data for the BMP280's pressure sensor
*/
/**************************************************************************/
void SPL06_Pressure::getSensor(sensor_t *sensor) {
  /* Clear the sensor_t object */
  memset(sensor, 0, sizeof(sensor_t));

  /* Insert the sensor name in the fixed length char array */
  strncpy(sensor->name, "SPL06-001", sizeof(sensor->name) - 1);
  sensor->name[sizeof(sensor->name) - 1] = 0;
  sensor->version = 1;
  sensor->sensor_id = _sensorID;
  sensor->type = SENSOR_TYPE_PRESSURE;
  sensor->min_delay = 0;
  sensor->min_value = 300.0; /* 300 ~ 1100 hPa  */
  sensor->max_value = 1100.0;
  sensor->resolution = 1.0; /* 0.06 hPa relative */
}

/**************************************************************************/
/*!
    @brief  Gets the pressure as a standard sensor event
    @param  event Sensor event object that will be populated
    @returns True
*/
/**************************************************************************/
bool SPL06_Pressure::getEvent(sensors_event_t *event) {
  /* Clear the event */
  memset(event, 0, sizeof(sensors_event_t));

  event->version = sizeof(sensors_event_t);
  event->sensor_id = _sensorID;
  event->type = SENSOR_TYPE_PRESSURE;
  event->timestamp = millis();
  event->pressure = _theSPL06->readPressure() / 100.0; // convert Pa to hPa
  return true;
}
