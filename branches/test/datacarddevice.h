#ifndef DATACARDDEVICE_H
#define DATACARDDEVICE_H
#include <yatephone.h>
#include "ringbuffer.h"
#include "endreasons.h"


#define FRAME_SIZE 320
#define RDBUFF_MAX      1024

using namespace TelEngine;

typedef enum {
  BLT_STATE_WANT_CONTROL = 0,
  BLT_STATE_WANT_CMD = 1,
} blt_state_t;


typedef enum {
	CMD_UNKNOWN = 0,
	CMD_AT,
	CMD_AT_A,
	CMD_AT_CCWA,
	CMD_AT_CFUN,
	CMD_AT_CGMI,
	CMD_AT_CGMM,
	CMD_AT_CGMR,
	CMD_AT_CGSN,
	CMD_AT_CHUP,
	CMD_AT_CIMI,
	CMD_AT_CLIP,
	CMD_AT_CLIR,
	CMD_AT_CLVL,
	CMD_AT_CMGD,
	CMD_AT_CMGF,
	CMD_AT_CMGR,
	CMD_AT_CMGS,
	CMD_AT_CNMI,
	CMD_AT_CNUM,
	CMD_AT_COPS,
	CMD_AT_COPS_INIT,
	CMD_AT_CPIN,
	CMD_AT_CPMS,
	CMD_AT_CREG,
	CMD_AT_CREG_INIT,
	CMD_AT_CSCS,
	CMD_AT_CSQ,
	CMD_AT_CSSN,
	CMD_AT_CUSD,
	CMD_AT_CVOICE,
	CMD_AT_D,
	CMD_AT_DDSETEX,
	CMD_AT_DTMF,
	CMD_AT_E,
	CMD_AT_SMS_TEXT,
	CMD_AT_U2DIAG,
	CMD_AT_Z,
	CMD_AT_CMEE,
	CMD_AT_CSMP,
} at_cmd_t;

typedef enum {
	RES_PARSE_ERROR = -1,
	RES_UNKNOWN = 0,
	RES_BOOT,
	RES_BUSY,
	RES_CEND,
	RES_CLIP,
	RES_CMGR,
	RES_CMS_ERROR,
	RES_CMTI,
	RES_CNUM,
	RES_CONF,
	RES_CONN,
	RES_COPS,
	RES_CPIN,
	RES_CREG,
	RES_CSQ,
	RES_CSSI,
	RES_CSSU,
	RES_CUSD,
	RES_ERROR,
	RES_MODE,
	RES_NO_CARRIER,
	RES_NO_DIALTONE,
	RES_OK,
	RES_ORIG,
	RES_RING,
	RES_RSSI,
	RES_SMMEMFULL,
	RES_SMS_PROMPT,
	RES_SRVST,
} at_res_t;


class at_queue_t : public GenObject
{
public:
    at_cmd_t cmd;
    at_res_t res;

    int ptype;

    union
    {
    	void* data;
	int num;
    } param;
};


class ATCommand : public GenObject
{
public:
    ATCommand(String command, at_cmd_t cmd, at_res_t res, GenObject* obj = 0):m_command(command),m_cmd(cmd),m_res(res)
    {
    	m_ptype = 0;
    	m_param.obj = obj;
    }
    ATCommand(String command, at_cmd_t cmd, at_res_t res, int num):m_command(command),m_cmd(cmd),m_res(res)
    {
    	m_ptype = 1;
    	m_param.num = num;
    }

    ~ATCommand()
    {
	if((m_ptype == 0) && m_param.obj)
	    m_param.obj->destruct();
    }

public:
    String m_command;
    at_cmd_t m_cmd;
    at_res_t m_res;
    
    int m_ptype;
    union
    {
    	GenObject* obj;
	int num;
    } m_param;

};



class CardDevice;
class DevicesEndPoint;
class Connection;

class MonitorThread : public Thread
{
public:
    MonitorThread(CardDevice* dev);
    ~MonitorThread();
    virtual void run();
    virtual void cleanup();
private:
    CardDevice* m_device;
};


class MediaThread : public Thread
{
public:
    MediaThread(CardDevice* dev);
    ~MediaThread();
    virtual void run();
    virtual void cleanup();
private:
    CardDevice* m_device;
};

class CardDevice: public String
{
public:
    CardDevice(String name, DevicesEndPoint* ep);
    bool tryConnect();
    bool disconnect();
    
    int dataStatus()
	{ return devStatus(m_data_fd);}
    int audioStatus()
	{ return devStatus(m_audio_fd);}

    bool getParams(NamedList* list);
    String getStatus();

private:
    bool startMonitor();
    int devStatus(int fd);

    DevicesEndPoint* m_endpoint;
    MonitorThread* m_monitor;
    MediaThread* m_media;

public:
    Mutex m_mutex;
    Connection* m_conn;

public:
    int m_audio_fd;			/* audio descriptor */
    int m_data_fd;			/* data  descriptor */


    char a_write_buf[FRAME_SIZE * 5];
    RingBuffer a_write_rb;

    String getNumber()
	{ return m_number; }

private:
    unsigned int has_sms:1;
    unsigned int has_voice:1;
    unsigned int use_ucs2_encoding:1;
    unsigned int cusd_use_7bit_encoding:1;
    unsigned int cusd_use_ucs2_decoding:1;
    int gsm_reg_status;
    int rssi;
    int linkmode;
    int linksubmode;
    String m_provider_name;
    String m_manufacturer;
    String m_model;
    String m_firmware;
    String m_imei;
    String m_imsi;
    String m_number;
    String m_location_area_code;
    String m_cell_id;

//FIXME: review all his flags. Simplify or implement it.

public:
    /* flags */
    bool m_connected;			/* do we have an connection to a device */
    unsigned int initialized:1;			/* whether a service level connection exists or not */
    unsigned int gsm_registered:1;		/* do we have an registration to a GSM */
    unsigned int outgoing:1;			/* outgoing call */
    unsigned int incoming:1;			/* incoming call */
    unsigned int needchup:1;			/* we need to send a CHUP */
    unsigned int needring:1;			/* we need to send a RING */
    unsigned int volume_synchronized:1;		/* we have synchronized the volume */
	
private:
    // TODO: Running flag. Do we need to stop every MonitorThread or set one 
    // flag on module level? Do we need syncronization?
    bool m_running;

public:
    bool isRunning() const;
    void stopRunning();


    /* Config */
    String m_audio_tty;			/* tty for audio connection */
    String m_data_tty;			/* tty for AT commands */
    int m_u2diag;
    int m_callingpres;			/* calling presentation */
    bool m_auto_delete_sms;
    bool m_reset_datacard;
    bool m_disablesms;

private:		
    blt_state_t state;
    char rd_buff[RDBUFF_MAX];
    int rd_buff_pos;

    // AT command methods.
public:

    /**
     * Read and handle data
     * @return 0 on success or -1 on error
     */    
    int handle_rd_data();

    /**
     * Process AT data
     * @return
     */        
    void processATEvents();

private:

    /**
     * Convert command to result type
     * @param command -- received command
     * @return result type
     */
    at_res_t at_read_result_classification(char* command);

    /**
     * Do response
     * @param str -- response string
     * @param at_res -- result type
     * @return 0 success or -1 parse error
     */
    int at_response(char* str, at_res_t at_res);

    /**
     * Handle ^CEND response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cend(char* str, size_t len);
    
    /**
     * Handle AT+CGMI response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cgmi(char* str, size_t len);
    
    /**
     * Handle AT+CGMM response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cgmm(char* str, size_t len);
    
    /**
     * Handle AT+CGMR response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cgmr(char* str, size_t len);
    
    /**
     * Handle AT+CGSN response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cgsn(char* str, size_t len);
    
    /**
     * Handle AT+CIMI response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cimi(char* str, size_t len);
    
    /**
     * Handle +CLIP response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_clip(char* str, size_t len);
    
    /**
     * Handle +CMGR response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cmgr(char* str, size_t len);
    
    /**
     * Handle +CMTI response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cmti(char* str, size_t len);
    
    /**
     * Handle +CNUM response Here we get our own phone number
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cnum(char* str, size_t len);
    
    /**
     * Handle ^CONN response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_conn(char* str, size_t len);
    
    /**
     * Handle +COPS response Here we get the GSM provider name
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cops(char* str, size_t len);
    
    /**
     * Handle +CPIN response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cpin(char* str, size_t len);
    
    /**
     * Handle +CREG response Here we get the GSM registration status
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_creg(char* str, size_t len);
    
    /**
     * Handle +CSQ response Here we get the signal strength and bit error rate
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_csq(char* str, size_t len);
    
    /**
     * Handle CUSD response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_cusd(char* str, size_t len);
    
    /**
     * Handle ERROR response
     * @return 0 success or -1 parse error
     */
    int at_response_error();
    
    /**
     * Handle ^MODE response Here we get the link mode (GSM, UMTS, EDGE...).
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_mode(char* str, size_t len);
    
    /**
     * Handle NO CARRIER response
     * @return 0 success or -1 parse error
     */
    int at_response_no_carrier();
    
    /**
     * Handle NO DIALTONE response
     * @return 0 success or -1 parse error
     */
    int at_response_no_dialtone();
    
    /**
     * Handle OK response
     * @return 0 success or -1 parse error
     */
    int at_response_ok();
    
    /**
     * Handle ^ORIG response
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_orig(char* str, size_t len);
    
    /**
     * Handle RING response
     * @return 0 success or -1 parse error
     */
    int at_response_ring();
    
    /**
     * Handle ^RSSI response Here we get the signal strength.
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_rssi(char* str, size_t len);
    
    /**
     * Handle ^SMMEMFULL response This event notifies us, that the sms storage is full
     * @return 0 success or -1 parse error
     */
    int at_response_smmemfull();
    
    /**
     * Send an SMS message from the queue.
     * @return 0 success or -1 parse error
     */
    int at_response_sms_prompt();
    
    /**
     * Handle BUSY response
     * @return 0 success or -1 parse error
     */
    int at_response_busy();

    /**
     * Handle PDU for incoming SMS.
     * @param str -- string containing response (null terminated)
     * @param len -- string lenght
     * @return 0 success or -1 parse error
     */
    int at_response_pdu(char* str, size_t len);

public:

    /**
     * Get the string representation of the given AT command
     * @param res -- the response to process
     * @return a string describing the given response
     */
    const char* at_cmd2str(at_cmd_t cmd);
    
    /**
     * Get the string representation of the given AT response
     * @param res -- the response to process
     * @return a string describing the given response
     */
    const char* at_res2str(at_res_t res);

private:

    /**
     * Parse a CLIP event
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @note str will be modified when the CID string is parsed
     * @return NULL on error (parse error) or a pointer to the caller id inforamtion in str on success
     */
    char* at_parse_clip(char* str, size_t len);
    
    /**
     * Parse a CMGR message
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @param stat -- 
     * @param pdulen -- 
     * @return 0 success or -1 parse error
     */
    int at_parse_cmgr(char* str, size_t len, int* stat, int* pdulen);

    /**
     * Parse a CMTI notification
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @note str will be modified when the CMTI message is parsed
     * @return -1 on error (parse error) or the index of the new sms message
     */
    int at_parse_cmti(char* str, size_t len);

    /**
     * Parse a CNUM response
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @note str will be modified when the CNUM message is parsed
     * @return NULL on error (parse error) or a pointer to the subscriber number
     */
    char* at_parse_cnum(char* str, size_t len);
    
    /**
     * Parse a COPS response
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @note str will be modified when the COPS message is parsed
     * @return NULL on error (parse error) or a pointer to the provider name
     */
    char* at_parse_cops(char* str, size_t len);
    
    /**
     * Parse a CREG response
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @param gsm_reg -- a pointer to a int
     * @param gsm_reg_status -- a pointer to a int
     * @param lac -- a pointer to a char pointer which will store the location area code in hex format
     * @param ci  -- a pointer to a char pointer which will store the cell id in hex format
     * @note str will be modified when the CREG message is parsed
     * @return 0 success or -1 parse error
     */
    int at_parse_creg(char* str, size_t len, int* gsm_reg, int* gsm_reg_status, char** lac, char** ci);
    
    /**
     * Parse a CPIN notification
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @return  4 if PUK2 required
     * @return  3 if PIN2 required
     * @return  2 if PUK required
     * @return  1 if PIN required
     * @return  0 if no PIN required
     * @return -1 on error (parse error) or card lock
     */
    int at_parse_cpin(char* str, size_t len);
    
    /**
     * Parse +CSQ response
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @param rssi
     * @return 0 success or -1 parse error
     */
    int at_parse_csq(char* str, size_t len, int* rssi);

    /**
     * Parse a CUSD answer
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @note str will be modified when the CUSD string is parsed
     * @return 0 success or -1 parse error
     */
    int at_parse_cusd(char* str, size_t len, char** cusd, unsigned char* dcs);
    
    /**
     * Parse a ^MODE notification (link mode)
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @param mode
     * @param submode
     * @return -1 on error (parse error) or the the link mode value
     */
    int at_parse_mode(char* str, size_t len, int* mode, int* submode);
    
    /**
     * Parse a ^RSSI notification
     * @param str -- string to parse (null terminated)
     * @param len -- string lenght
     * @return -1 on error (parse error) or the rssi value
     */
    int at_parse_rssi(char* str, size_t len);

    /**
     * Write to data socket
     * This function will write count characters from buf. It will always write
     * count chars unless it encounters an error.
     * @param buf -- buffer to write
     * @param count -- number of bytes to write
     * @return 0 success or -1 on error
     */
    int at_write_full(char* buf, size_t count);

    /**
     * Formatted output to data socket
     * @param fmt -- format string
     * @param ... - params according to format string
     * @return 0 success or -1 on error
     */    
    int send_atcmd(const char* fmt, ...);

public:

    /**
     * Send the AT command
     */
    int at_send_at();
    
    /**
     * Send ATA command
     */
    int at_send_ata();
    
    /**
     * Send ATD command
     * @param number
     */
    int at_send_atd(const char* number);
    
    /**
     * Send the ATE0 command
     */
    int at_send_ate0();
    
    /**
     * Send the ATZ command
     */
    int at_send_atz();
    
    /**
     * Send the AT+CGMI command
     */
    int at_send_cgmi();
    
    /**
     * Send the AT+CGMM command
     */
    int at_send_cgmm();
    
    /**
     * Send the AT+CGMR command
     */
    int at_send_cgmr();
    
    /**
     * Send the AT+CGSN command
     */
    int at_send_cgsn();
    
    /**
     * Send AT+CHUP command
     */
    int at_send_chup();
    
    /**
     * Send AT+CIMI command
     */
    int at_send_cimi();
    
    /**
     * Enable or disable calling line identification
     * @param status -- enable or disable calling line identification (should be 1 or 0)
     */
    int at_send_clip(int status);
    
    /**
     * Send the AT+CLIR command
     * @param mode -- the CLIR mode
     */
    int at_send_clir(int mode);

    /**
     * Send AT+CLVL command
     * @param volume -- level to send
     */
    int at_send_clvl(int level);

    /**
     * Delete an SMS message
     * @param index -- the location of the requested message
     */
    int at_send_cmgd(int index);
    
    /**
     * Set the SMS mode
     * @param mode -- the sms mode (0 = PDU, 1 = Text)
     */
    int at_send_cmgf(int mode);

    /**
     * Read an SMS message
     * @param index -- the location of the requested message
     */
    int at_send_cmgr(int index);
    
    /**
     * Start sending an SMS message
     * @param len -- PDU length
     */
    int at_send_cmgs(const int len);

    /**
     * Setup SMS new message indication
     */
    int at_send_cnmi();
    
    /**
     * Send the AT+CNUM command
     */
    int at_send_cnum();
    
    /**
     * Send the AT+COPS? command
     */
    int at_send_cops();
    
    /**
     * Send the AT+COPS= command
     * @param mode
     * @param format
     */
    int at_send_cops_init(int mode, int format);
    
    /**
     * Send AT+CPIN? to ask the datacard if a pin code is required
     */
    int at_send_cpin_test();
    
    /**
     * Send the AT+CREG? command
     */
    int at_send_creg();
    
    /**
     * Send the AT+CREG=n command
     * @param level -- verbose level of CREG
     */
    int at_send_creg_init(int level);
    
    /**
     * Send AT+CSQ.
     */
    int at_send_csq();
    
    /**
     * Manage Supplementary Service Notification.
     * @param cssi the value to send (0 = disabled, 1 = enabled)
     * @param cssu the value to send (0 = disabled, 1 = enabled)
     */
    int at_send_cssn(int cssi, int cssu);

    /**
     * Send AT+CUSD.
     * @param code the CUSD code to send
     */
    int at_send_cusd(const char* code);
    
    /**
     * Check device for audio capabilities
     */
    int at_send_cvoice_test();
    
    /**
     * Enable transmitting of audio to the debug port (tty)
     */
    int at_send_ddsetex();
    
    /**
     * Send a DTMF command
     * @param digit -- the dtmf digit to send
     */
    int at_send_dtmf(char digit);
    
    /**
     * Send the SMS PDU message
     * @param pdu -- SMS PDU
     */
    int at_send_sms_text(const char* pdu);
    
    /**
     * Set the U2DIAG mode
     * @param mode -- the U2DIAG mode (0 = Only modem functions)
     */
    int at_send_u2diag(int mode);
    
    /**
     * Send the AT+CCWA command (disable)
     */
    int at_send_ccwa_disable();
    
    /**
     * Send the AT+CFUN command (Operation Mode Setting)
     * @param fun
     * @param rst
     */
    int at_send_cfun(int fun, int rst);
    
    /**
     * Set error reporing verbosity level
     * @param level -- the verbosity level
     */
    int at_send_cmee(int level);
    
    /**
     * Set storage location for incoming SMS
     */
    int at_send_cpms();
    
public:

    /**
     * Add an item to the back of the queue
     * @param cmd the command that was sent to generate the response
     * @param res the expected response
     */
    int	at_fifo_queue_add(at_cmd_t cmd, at_res_t res);
    
    /**
     * Add an item to the back of the queue with pointer data
     * @param cmd -- the command that was sent to generate the response
     * @param res -- the expected response
     * @param data -- pointer data associated with this entry, it will be freed when the message is freed
     */
    int	at_fifo_queue_add_ptr(at_cmd_t cmd, at_res_t res, void* data);
    
    /**
     * Add an item to the back of the queue with pointer data
     * @param cmd -- the command that was sent to generate the response
     * @param res -- the expected response
     * @param num -- numeric data
     */
    int	at_fifo_queue_add_num(at_cmd_t cmd, at_res_t res, int num);
    
    /**
     * Remove an item from the front of the queue, and free it
     */
    void at_fifo_queue_rem();
    
    /**
     * Remove all itmes from the queue and free them
     */
    void at_fifo_queue_flush();
    
    /**
     * Get the head of a queue
     * @return a pointer to the head of the given queue
     */
    at_queue_t* at_fifo_queue_head();

private:
    ObjList m_atQueue;



    ssize_t convert_string(const char* in, size_t in_length, char* out, size_t out_size, char* from, char* to);
    ssize_t hexstr_to_ucs2char(const char* in, size_t in_length, char* out, size_t out_size);
    ssize_t ucs2char_to_hexstr(const char* in, size_t in_length, char* out, size_t out_size);
    ssize_t hexstr_ucs2_to_utf8(const char* in, size_t in_length, char* out, size_t out_size);
    ssize_t utf8_to_hexstr_ucs2(const char* in, size_t in_length, char* out, size_t out_size);
    ssize_t char_to_hexstr_7bit(const char* in, size_t in_length, char* out, size_t out_size);
    ssize_t hexstr_7bit_to_char(const char* in, size_t in_length, char* out, size_t out_size);

public:
// SMS and USSD
    bool sendSMS(const String &called, const String &sms);
    bool sendUSSD(const String &ussd);   

    void forwardAudio(char* data, int len);
    int sendAudio(char* data, int len);
    
    bool newCall(const String &called, void* usrData);
    
private:
    bool receiveSMS(const char* pdustr, size_t len);
    bool incomingCall(const String &caller);
    bool Hangup(int error);
    int getReason(int end_status, int cc_cause);
    bool m_incoming_pdu;

    ATCommand* m_lastcmd;
public:
    ObjList m_commandQueue;
    
};


class Connection
{
public:
    Connection(CardDevice* dev);
    virtual bool onIncoming(const String &caller);
    virtual bool onProgress();
    virtual bool onAnswered();
    virtual bool onHangup(int reason);
    
    bool sendAnswer();
    bool sendHangup();

    bool sendDTMF(char digit);
    
    virtual void forwardAudio(char* data, int len);
    int sendAudio(char* data, int len);
    
protected:
    CardDevice* m_dev;
};

class DevicesEndPoint : public Thread
{
public:
    
    DevicesEndPoint(int interval);
    virtual ~DevicesEndPoint();
    
    
    virtual void run();
    virtual void cleanup();
    
    virtual void onReceiveUSSD(CardDevice* dev, String ussd);
    virtual void onReceiveSMS(CardDevice* dev, String caller, String sms);
    
    bool sendSMS(CardDevice* dev, const String &called, const String &sms);
    bool sendUSSD(CardDevice* dev, const String &ussd);
    
    CardDevice* appendDevice(String name, NamedList* data);
    CardDevice* findDevice(const String &name);
    void cleanDevices();
    
    virtual Connection* createConnection(CardDevice* dev, void* usrData = 0);
    bool MakeCall(CardDevice* dev, const String &called, void* usrData);

private:
    Mutex m_mutex;
    ObjList m_devices;
    int m_interval;
    bool m_run;
};

#endif

/* vi: set ts=8 sw=4 sts=4 noet: */