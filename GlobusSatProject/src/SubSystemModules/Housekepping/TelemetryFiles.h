
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

#define END_FILE_NAME_TX_REVC			"txr"
#define END_FILE_NAME_TX				"tx"
#define END_FILE_NAME_RX				"rx"
#define END_FILE_NAME_RX_REVC           "rxr"
#define END_FILE_NAME_RX_FRAME 			"rxf"
#define END_FILE_NAME_ANTENNA			"ant"
#define END_FILENAME_WOD_TLM			"wod"
#define	END_FILENAME_EPS_RAW_MB_TLM		"erm"
#define END_FILENAME_EPS_ENG_MB_TLM		"eem"
#define END_FILENAME_EPS_RAW_CDB_TLM	"erc"
#define END_FILENAME_EPS_ENG_CDB_TLM	"eec"
#define	END_FILENAME_SOLAR_PANELS_TLM	"slr"
#define	END_FILENAME_LOGS				"log"

typedef enum {
	tlm_eps = 0,
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
    tlm_log 
}tlm_type_t;
#endif /* TELEMETRYFILES_H_ */
