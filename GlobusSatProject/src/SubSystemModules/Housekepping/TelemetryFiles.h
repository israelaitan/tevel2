
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_

//	---general
#define FILENAME_WOD_TLM				"wod"

//	---eps
#define	FILENAME_EPS_RAW_MB_TLM			"eRwMB"
#define FILENAME_EPS_ENG_MB_TLM			"eEgMB"
#define FILENAME_EPS_AVG_MB_TLM			"eAvgMB"

//	---trxvu
#define FILENAME_TX_TLM					"tx"
#define FILENAME_RX_TLM					"rx"
#define FILENAME_RX_FRAME				"rxFrame"
#define FILENAME_ANTENNA_SIDE_A_TLM		"antA"
#define FILENAME_ANTENNA_SIDE_B_TLM		"antB"
#define FILENAME_SOLAR_TLM				"solar"

//	---log
#define FILENAME_LOG_TLM			    "log"

// ---- payload
#define FILENAME_PIC32_TLM				"pic32"
#define FILENAME_RADFET_TLM				"radfet"
typedef enum {
	tlm_eps_raw_mb,
	tlm_eps_eng_mb,
	tlm_eps_avg_mb,
	tlm_tx,
	tlm_rx,
	tlm_antA,
	tlm_antB,
	tlm_log,
	tlm_wod,
	tlm_pic32,
	tlm_radfet,
	tlm_solar
}tlm_type;
#endif /* TELEMETRYFILES_H_ */
