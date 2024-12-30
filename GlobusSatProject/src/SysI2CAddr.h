/*
 * @file SysI2CAddr.h
 * @brief system I2C addresses
 */

#ifndef SYSI2CADDR_H_
#define SYSI2CADDR_H_



#define ISIS_TRXVU_I2C_BUS_INDEX 0
#define ANTS_SIDE_A_BUS_INDEX 0
#define ANTS_SIDE_B_BUS_INDEX 1
#define EPS_I2C_BUS_INDEX 0


#define EPS_I2C_ADDR			 0x20
#define ANTS_I2C_SIDE_A_ADDR	 0x31
#define ANTS_I2C_SIDE_B_ADDR	 0x32
#define PAYLOAD_I2C_ADDR		 0x55
#define I2C_TRXVU_RC_ADDR 		 0x60
#define I2C_TRXVU_TC_ADDR		 0x61

#define TRXVU_RX_FREQ 145970
#define TRXVU_TX_FREQ 436400

#endif /* SYSI2CADDR_H_ */
