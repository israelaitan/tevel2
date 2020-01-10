
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_

//	---general
#define FILENAME_WOD_TLM				"wod"

//	---eps
#define	FILENAME_EPS_RAW_MB_TLM			"eRwMB"
#define FILENAME_EPS_ENG_MB_TLM			"eEgMB"
#define FILENAME_EPS_RAW_CDB_TLM		"eRwCdb"
#define FILENAME_EPS_ENG_CDB_TLM		"eE_Cdb"
#define	FILENAME_SOLAR_PANELS_TLM		"slrPnl"

//	---trxvu
#define FILENAME_TX_TLM					"tx"
#define FILENAME_TX_REVC				"txRevc"
#define FILENAME_RX_TLM					"rx"
#define FILENAME_RX_REVC				"rxRevC"
#define FILENAME_RX_FRAME				"rxFrame"
#define FILENAME_ANTENNA_TLM			"ant"

typedef enum {
	tlm_wod,
	tlm_eps_raw_mb,
	tlm_eps_eng_mb,
	tlm_eps_raw_cdb,
	tlm_eps_eng_cdb,
	tlm_solar,
	tlm_tx,
	tlm_tx_revc,
	tlm_rx,
	tlm_rx_revc,
	tlm_rx_frame,
	tlm_antenna,

}tlm_type;
#endif /* TELEMETRYFILES_H_ */
