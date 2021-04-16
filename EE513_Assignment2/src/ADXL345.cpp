#include "ADXL345.h"
#include <iostream>
#include <unistd.h>
#include <math.h>
#include <stdio.h>

using namespace std;
//From Table 19. of the ADXL345 Data sheet
#define DEVID          0x00   //Device ID
#define THRESH_TAP     0x1D   //Tap Threshold
#define OFSX           0x1E   //X-axis Offset
#define OFSY           0x1F   //Y-axis Offset
#define OFSZ           0x20   //Z-axis Offset
#define DUR            0x21   //Tap duration
#define LATENT         0x22   //Tap latency
#define WINDOW         0x23   //Tap window
#define THRESH_ACT     0x24   //Activity threshold
#define THRESH_INACT   0x25   //Threshold inactivity
#define TIME_INACT     0x26   //Inactivity time
#define ACT_INACT_CTL  0x27   //Axis enable control for activity and inactivity detection
#define THRESH_FF      0x28   //Free-fall threshold
#define TIME_FF        0x29   //Free-fall time
#define TAP_AXES       0x2A   //Axis control for single tap/double tap
#define ACT_TAP_STATUS 0x2B   //Source of single tap/double tap
#define BW_RATE        0x2C   //Data rate and power mode control
#define POWER_CTL      0x2D   //Power-saving features control
#define INT_ENABLE     0x2E   //Interrupt enable control
#define INT_MAP        0x2F   //Interrupt mapping control
#define INT_SOURCE     0x30   //Source of interrupts
#define DATA_FORMAT    0x31   //Data format control
#define DATAX0         0x32   //X-axis Data 0
#define DATAX1         0x33   //X-axis Data 1
#define DATAY0         0x34   //Y-axis Data 0
#define DATAY1         0x35   //Y-axis Data 1
#define DATAZ0         0x36   //Z-axis Data 0
#define DATAZ1         0x37   //Z-axis Data 1
#define FIFO_CTL       0x38   //FIFO control
#define FIFO_STATUS    0x39   //FIFO status

//my added methods

//coversion from binary to decimal and vice versa
int bcdToDec(char b) { return (b/16)*10 + (b%16); }

int DecTObcd(char b){ return ((b / 10) * 16) + (b % 10); }


int ADXL345::readAllADXL345Data(int& x,int& y ,int& z){
	unsigned int readAddr[1];

	//x data
	readAddr[0] = DATAX0;
	cout << "Reading X data from ADXL345" << endl;
	int x0 = readRegister(readAddr[0]);
    if (x0 < 0) {
        perror("Failed to read X data0\n");
    }
    readAddr[0] = DATAX1;
	cout << "Reading X data from ADXL345" << endl;
    int x1 = readRegister(readAddr[1]);
    if (x1 < 0) {
        perror("Failed to read X data1\n");
    }
    x = bcdToDec((x1<<8)|(x0));

    //y data
	readAddr[0] = DATAY0;
	cout << "Reading Y data from ADXL345" << endl;
	int y0 = readRegister(readAddr[0]);
    if (y0 < 0) {
        perror("Failed to read Y data0\n");
    }
    readAddr[0] = DATAY1;
	cout << "Reading Y data from ADXL345" << endl;
    int y1 = readRegister(readAddr[1]);
    if (y1 < 0) {
        perror("Failed to read Y data1\n");
    }
    y = bcdToDec((y1<<8)|(y0));

    //z data
	readAddr[0] = DATAZ0;
	cout << "Reading Z data from ADXL345" << endl;
	int z0 = readRegister(readAddr[0]);
    if (z0 < 0) {
        perror("Failed to read Z data0\n");
    }
    readAddr[0] = DATAZ1;
	cout << "Reading Z data from ADXL345" << endl;
    int z1 = readRegister(readAddr[1]);
    if (z1 < 0) {
        perror("Failed to read Z data1\n");
    }
    z = bcdToDec((z1<<8)|(z0));


    return x,y,z;
}

//methods from original file
/**
 * Method to combine two 8-bit registers into a single short, which is 16-bits on the Raspberry Pi. It shifts
 * the MSB 8-bits to the left and then ORs the result with the LSB.
 * @param msb an unsigned character that contains the most significant byte
 * @param lsb an unsigned character that contains the least significant byte
 */
short ADXL345::combineRegisters(unsigned char msb, unsigned char lsb){
   //shift the MSB left by 8 bits and OR with LSB
   return ((short)msb<<8)|(short)lsb;
}

/**
 * Method to calculate the pitch and roll state values. This calculation takes account of the scaling
 * factors due to the resolution and gravity range to determine gravity weighted values that are used
 * to calculate the angular pitch and roll values in degrees.
 */
void ADXL345::calculatePitchAndRoll(){
	float gravity_range;
	switch(ADXL345::range){
		case ADXL345::PLUSMINUS_16_G: gravity_range=32.0f; break;
		case ADXL345::PLUSMINUS_8_G: gravity_range=16.0f; break;
		case ADXL345::PLUSMINUS_4_G: gravity_range=8.0f; break;
		default: gravity_range=4.0f; break;
	}
    float resolution = 1024.0f;
    if (this->resolution==ADXL345::HIGH) resolution = 8192.0f; //13-bit resolution
    float factor = gravity_range/resolution;

    float accXg = this->accelerationX * factor;
    float accYg = this->accelerationY * factor;
    float accZg = this->accelerationZ * factor;
	float accXSquared = accXg * accXg ;
	float accYSquared = accYg * accYg ;
	float accZSquared = accZg * accZg ;
	this->pitch = 180 * atan(accXg/sqrt(accYSquared + accZSquared))/M_PI;
	this->roll = 180 * atan(accYg/sqrt(accXSquared + accZSquared))/M_PI;
}

/**
 * Method used to update the DATA_FORMAT register and any other registers that might be added
 * in the future.
 * @return 0 if the register is updated successfully
 */
int ADXL345::updateRegisters(){
   //update the DATA_FORMAT register
   char data_format = 0x00;  //+/- 2g with normal resolution
   //Full_resolution is the 3rd LSB
   data_format = data_format|((this->resolution)<<3);
   data_format = data_format|this->range; // 1st and 2nd LSB therefore no shift
   return this->writeRegister(DATA_FORMAT, data_format);
}

/**
 * The constructor for the ADXL345 accelerometer object. It passes the bus number and the
 * device address (with is 0x53 by default) to the constructor of I2CDevice. All of the states
 * are initialized and the registers are updated.
 * @param I2CBus The bus number that the ADXL345 device is on - typically 0 or 1
 * @param I2CAddress The address of the ADLX345 device (default 0x53, but can be altered)
 */
ADXL345::ADXL345(unsigned int I2CBus, unsigned int I2CAddress):
	I2CDevice(I2CBus, I2CAddress){   // this member initialisation list calls the parent constructor
	this->I2CAddress = I2CAddress;
	this->I2CBus = I2CBus;
	this->accelerationX = 0;
	this->accelerationY = 0;
	this->accelerationZ = 0;
	this->pitch = 0.0f;
	this->roll = 0.0f;
	this->registers = NULL;
	this->range = ADXL345::PLUSMINUS_16_G;
	this->resolution = ADXL345::HIGH;
	this->writeRegister(POWER_CTL, 0x08);
	this->updateRegisters();
}

/**
 * Read the sensor state. This method checks that the device is being correctly
 * read by using the device ID of the ADXL345 sensor. It will read in the accelerometer registers
 * and pass them to the combineRegisters() method to be processed.
 * @return 0 if the registers are successfully read and -1 if the device ID is incorrect.
 */
int ADXL345::readSensorState(){
	this->registers = this->readRegisters(BUFFER_SIZE, 0x00);
	if(*this->registers!=0xe5){
		perror("ADXL345: Failure Condition - Sensor ID not Verified");
		return -1;
	}
	this->accelerationX = this->combineRegisters(*(registers+DATAX1), *(registers+DATAX0));
	this->accelerationY = this->combineRegisters(*(registers+DATAY1), *(registers+DATAY0));
	this->accelerationZ = this->combineRegisters(*(registers+DATAZ1), *(registers+DATAZ0));
	this->resolution = (ADXL345::RESOLUTION) (((*(registers+DATA_FORMAT))&0x08)>>3);
	this->range = (ADXL345::RANGE) ((*(registers+DATA_FORMAT))&0x03);
	this->calculatePitchAndRoll();
	return 0;
}

/**
 * Set the ADXL345 gravity range according to the RANGE enumeration
 * @param range One of the four possible gravity ranges defined by the RANGE enumeration
 */
void ADXL345::setRange(ADXL345::RANGE range) {
	this->range = range;
	updateRegisters();
}

/**
 * Set the ADXL345 resolution according to the RESOLUTION enumeration
 * @param resolution either HIGH or NORMAL resolution. HIGH resolution is only available if the range is set to +/- 16g
 */
void ADXL345::setResolution(ADXL345::RESOLUTION resolution) {
	this->resolution = resolution;
	updateRegisters();
}

/**
 * Useful debug method to display the pitch and roll values in degrees on a single standard output line
 * @param iterations The number of 0.1s iterations to take place.
 */
void ADXL345::displayPitchAndRoll(int iterations){
	int count = 0;
	while(count < iterations){
	      cout << "Pitch:"<< this->getPitch() << " Roll:" << this->getRoll() << "     \r"<<flush;
	      usleep(100000);
	      this->readSensorState();
	      count++;
	}
}

ADXL345::~ADXL345() {}

