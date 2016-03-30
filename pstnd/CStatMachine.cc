/**
 * @file CStatMachine.cc
 * @author shujie.li
 * @version 1.0
 * @brief Cubic Stat Machine, utility tool
 * @detail Cubic Stat Machine, utility tool
 */
#ifndef _CSTATMACHINE_CC_
#define _CSTATMACHINE_CC_ 1
#include <iostream>
#include <map>
#include "CLock.cc"

using namespace std;

typedef int TStat;
typedef int TEvt;
static const TStat STAT_BEGIN = 1;
static const TStat STAT_END = 0;
static const TStat STAT_ERR = -1;
static const TEvt  EVT_DEFAULT = 0;


class IAction{
public:
	virtual bool onStatChange(TStat stat_from, TEvt evt, TStat stat_to, void* data) = 0;
    virtual ~IAction() {};
};


class CStatMachine
{
private:
    typedef struct Target{
        TStat     t_stat;
        IAction*  p_act;
    } TTarget;
    map< TStat, map<TEvt, TTarget> > m_bridge_tab;
	TStat m_curr_stat;
    CLock m_lock;

public:
	typedef enum ErrCode {
		ERR_NO_ERROR = 0,
		ERR_CONFLICT = -1,
		ERR_NOT_FOUND = -2,
		ERR_ACTION_FAIL = -3,
	} EErrCode;

	CStatMachine()
		: m_bridge_tab()
		, m_curr_stat(STAT_BEGIN)
		, m_lock()
	{};

	inline void reset()
	{
		m_curr_stat = STAT_BEGIN;
	};

	inline TStat current()
	{
		return m_curr_stat;
	};

	EErrCode insertAction( TStat from, TEvt evt, TStat to, IAction* act )
	{
        Target target = {to, act};
        map<TEvt, TTarget> cfg;

        if( m_bridge_tab.find(from) != m_bridge_tab.end() )
        {
            cfg = m_bridge_tab[from];
        }

        if( cfg.find(evt) != cfg.end() )
        {
            return ERR_CONFLICT;
        }

        cfg[evt] = target;
        m_bridge_tab[from] = cfg;
        
		return ERR_NO_ERROR;
	};

	EErrCode input( TEvt evt, void* data = NULL )
	{
	    m_lock.lock();
        if( m_bridge_tab.find(m_curr_stat) == m_bridge_tab.end() )
        {
       	    m_lock.unlock();
            return ERR_NOT_FOUND;
        }

        map<TEvt, TTarget> cfg = m_bridge_tab[m_curr_stat];
        if( cfg.find(evt) == cfg.end() )
        {
            if( cfg.find(EVT_DEFAULT) == cfg.end() )
	        {
	            m_lock.unlock();
	            return ERR_NOT_FOUND;
	        }
	        evt = EVT_DEFAULT;
        }

        Target target = cfg[evt];
        if( target.p_act && !target.p_act->onStatChange(m_curr_stat, evt, target.t_stat, data ) )
        {
            m_lock.unlock();
            return ERR_ACTION_FAIL;
        }
        m_curr_stat = target.t_stat;
        m_lock.unlock();
        return ERR_NO_ERROR;
	};
};
#endif  //_CSTATMACHINE_CC_
