/* 
   Copyright (C) 2009 - 2010
   
   Artem Makhutov <artem@makhutov.org>
   http://www.makhutov.org
   
   Dmitry Vagin <dmitry2004@yandex.ru>
*/

#include "datacarddevice.h"
#include <stdlib.h>
#include <poll.h>


int CardDevice::handle_rd_data()
{
    char c;
    int ret;

    while ((ret = read(m_data_fd, &c, 1)) == 1) 
    {
	switch (state) 
	{
	    case BLT_STATE_WANT_CONTROL:
		if (c != '\r' && c != '\n')
		{
		    state = BLT_STATE_WANT_CMD;
		    rd_buff[rd_buff_pos++] = c;
		} 
		else
		    return 0;
		break;
	    case BLT_STATE_WANT_CMD:
		if (c == '\r' || c == '\n')
		{
		    state = BLT_STATE_WANT_CONTROL;
		    Debug(DebugAll,"[%s] : [%s]\n",c_str(), rd_buff);

		    int res = at_response(rd_buff,at_read_result_classification(rd_buff));
    
		    rd_buff_pos = 0;
		    memset(rd_buff, 0, RDBUFF_MAX);
		    return res;
		}
		else 
		{
		    if (rd_buff_pos >= RDBUFF_MAX) 
		    {
			Debug(DebugAll,"Device %s: Buffer exceeded\n", c_str());
			return -1;
		    }
		    rd_buff[rd_buff_pos++] = c;
		}
		break;
    	    default:
        	Debug(DebugAll,"Device %s: Unknown device state %d\n", c_str(), state);
        	return -1;
	}
    }
    if(ret < 0)
	return ret;
    return 0;
}

void CardDevice::processATEvents()
{
    at_queue_t*	e;
    struct pollfd fds;

    m_mutex.lock();
    m_commandQueue.clear();
    m_commandQueue.append(new ATCommand("AT", CMD_AT, RES_OK));
    
    m_mutex.unlock();

    // Main loop
    while (isRunning())
    {
        m_mutex.lock();
        if (dataStatus() || audioStatus())
        {
            Debug(DebugAll, "Lost connection to Datacard %s", c_str());
            disconnect();
            m_mutex.unlock();
            return;
        }
        m_mutex.unlock();

	fds.fd = m_data_fd;
	fds.events = POLLIN;
	fds.revents = 0;
	
	if((m_commandQueue.count() > 0) && !m_lastcmd)
	    fds.events |= POLLOUT;
	    

	int res = poll(&fds, 1, 1000);
	if (res < 0) {
    	    if (errno == EINTR)
        	continue;
	    m_mutex.lock();
	    disconnect();
            m_mutex.unlock();
	    return;
	}
	else if(res == 0)
	{
	    m_mutex.lock();
            if (!initialized)
            {
                Debug(DebugAll, "[%s] timeout waiting for data, disconnecting", c_str());

		if ((e = at_fifo_queue_head()))
		{
                    Debug(DebugAll, "[%s] timeout while waiting '%s' in response to '%s'", c_str(), at_res2str (e->res), at_cmd2str (e->cmd));
                }
                Debug(DebugAll, "Error initializing Datacard %s", c_str());
                disconnect();
                m_mutex.unlock();
                return;
            }
            else
            {
                m_mutex.unlock();
                continue;
            }
	}
	if((fds.revents & POLLIN))
	{
    	    //incoming data
    	    m_mutex.lock();
	    if (handle_rd_data())
	    {
        	disconnect();
        	m_mutex.unlock();
        	return;
	    }
    	    m_mutex.unlock();
	}	
	else if (fds.revents & POLLOUT)
	{
	    m_mutex.lock();
	    if(!m_lastcmd)
	    {
	    
		ATCommand* cmd = static_cast<ATCommand*>(m_commandQueue.get());
	    
		if(cmd)
		{
		    at_write_full((char*)cmd->m_command.safe(),cmd->m_command.length());
		    m_commandQueue.remove(cmd, false);
		    m_lastcmd = cmd;
		}
	    }
    	    m_mutex.unlock();
	}
	else if (fds.revents)// & (POLLRDHUP|POLLERR|POLLHUP|POLLNVAL|POLLPRI))
	{
    	    //exeption
    	    m_mutex.lock();
	    disconnect();
            m_mutex.unlock();
    	    return;
	}
    } // End of Main loop
}

at_res_t CardDevice::at_read_result_classification (char* command)
{
    at_res_t at_res;

    if(memcmp(command,"^BOOT:", 6) == 0)		// 5115
    {
	at_res = RES_BOOT;
    }
    else if(memcmp(command,"+CNUM:", 6) == 0)
    {
	at_res = RES_CNUM;
    }
    else if (memcmp(command,"ERROR+CNUM:", 11) == 0)
    {
	at_res = RES_CNUM;
    }
    else if (memcmp(command,"OK", 2) == 0)		// 2637
    {
	at_res = RES_OK;
    }
    else if (memcmp(command,"^RSSI:", 6) == 0)		// 880
    {
	at_res = RES_RSSI;
    }
    else if (memcmp(command,"^MODE:", 6) == 0)		// 656
    {
	at_res = RES_MODE;
    }
    else if (memcmp(command,"^CEND:", 6) == 0)		// 425
    {
	at_res = RES_CEND;
    }
    else if (memcmp(command,"+CSSI:", 6) == 0)		// 416
    {
	at_res = RES_CSSI;
    }
    else if (memcmp(command,"^ORIG:", 6) == 0)		// 408
    {
	at_res = RES_ORIG;
    }
    else if (memcmp(command,"^CONF:", 6) == 0)		// 404
    {
	at_res = RES_CONF;
    }
    else if (memcmp(command,"^CONN:", 6) == 0)		// 332
    {
	at_res = RES_CONN;
    }
    else if (memcmp(command,"+CREG:", 6) == 0)		// 56
    {
	at_res = RES_CREG;
    }
    else if (memcmp(command,"+COPS:", 6) == 0)		// 56
    {
	at_res = RES_COPS;
    }
    else if (memcmp(command,"^SRVST:", 7) == 0)	// 35
    {
	at_res = RES_SRVST;
    }
    else if (memcmp(command,"+CSQ:", 5) == 0)		// 28 init
    {
	at_res = RES_CSQ;
    }
    else if (memcmp(command,"+CPIN:", 6) == 0)		// 28 init
    {
	at_res = RES_CPIN;
    }
    else if (memcmp(command,"RING", 4) == 0)		// 15 incoming
    {
	at_res = RES_RING;
    }
    else if (memcmp(command,"+CLIP:", 6) == 0)		// 15 incoming
    {
	at_res = RES_CLIP;
    }
    else if (memcmp(command,"ERROR", 5) == 0)	// 12
    {
	at_res = RES_ERROR;
    }
    else if (memcmp(command,"+CMTI:", 6) == 0)		// 8 SMS
    {
	at_res = RES_CMTI;
    }
    else if (memcmp(command,"+CMGR:", 6) == 0)		// 8 SMS
    {
	at_res = RES_CMGR;
    }
    else if (memcmp(command,"+CSSU:", 6) == 0)		// 2
    {
	at_res = RES_CSSU;
    }
    else if (memcmp(command,"BUSY", 4) == 0)
    {
	at_res = RES_BUSY;
    }
    else if (memcmp(command,"NO DIALTONE", 11) == 0)
    {
	at_res = RES_NO_DIALTONE;
    }
    else if (memcmp(command,"NO CARRIER", 10) == 0)
    {
	at_res = RES_NO_CARRIER;
    }
    else if (memcmp(command,"COMMAND NOT SUPPORT", 19) == 0)
    {
	at_res = RES_ERROR;
    }
    else if (memcmp(command,"+CMS ERROR:", 11) == 0)
    {
	at_res = RES_CMS_ERROR;
    }
    else if (memcmp(command,"^SMMEMFULL:", 11) == 0)
    {
	at_res = RES_SMMEMFULL;
    }
    else if (memcmp(command,"> ", 2) == 0)
    {
	at_res = RES_SMS_PROMPT;
    }
    else if (memcmp(command,"+CUSD:", 6) == 0)
    {
	at_res = RES_CUSD;
    }
    else
    {
	at_res = RES_UNKNOWN;
    }
    return at_res;
}

/* vi: set ts=8 sw=4 sts=4 noet: */