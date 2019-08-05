/*
 * @file SysI2CAddr.h
 * @brief system I2C addresses
 */

#ifndef SYSI2CADDR_H_
#define SYSI2CADDR_H_


/// Subsystem I2C addresses
#define ISIS_TRXVU_I2C_BUS_INDEX 0	//!< index of the trxvu I2C

#define I2C_TRXVU_TC_ADDR 0x61		//!< I2C address of the transceiver
#define I2C_TRXVU_RC_ADDR 0x60		//!< I2C address of the receiver

#define EPS_I2C_BUS_INDEX 0			//!< index of the EPS I2C
#define EPS_I2C_ADDR 0x02			//!< I2C address of the EPS

#define ANTS_I2C_ADDR_SIDE_A 0x04			//!< I2C address of the side A ants
#define ANTS_I2C_ADDR_SIDE_B 0x06			//!< I2C address of the side B ants

#endif /* SYSI2CADDR_H_ */
