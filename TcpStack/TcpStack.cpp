/* 
 * Copyright (C) 2012 Yee Young Han <websearch@naver.com> (http://blog.naver.com/websearch)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
 */

#include "TcpStack.h"
#include "ServerUtility.h"
#include "Log.h"
#include "MemoryDebug.h"

THREAD_API TcpListenThread( LPVOID lpParameter );

CTcpStack::CTcpStack() : m_pclsCallBack(NULL), m_hTcpListenFd(INVALID_SOCKET), m_bStop(false)
{
	InitNetwork();
}

CTcpStack::~CTcpStack()
{
}

/**
 * @ingroup TcpStack
 * @brief TCP stack �� �����Ѵ�.
 * @param pclsSetup			TCP stack ����
 * @param pclsCallBack	TCP stack callback
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpStack::Start( CTcpStackSetup * pclsSetup, ITcpStackCallBack * pclsCallBack )
{
	if( pclsSetup == NULL || pclsCallBack == NULL ) return false;

	m_clsSetup = *pclsSetup;
	m_pclsCallBack = pclsCallBack;
	m_clsClientMap.Create( this );

	if( pclsSetup->m_iListenPort > 0 )
	{
		if( pclsSetup->m_bUseTls )
		{
			if( SSLServerStart( pclsSetup->m_strCertFile.c_str() ) == false )
			{
				CLog::Print( LOG_ERROR, "SSLServerStart error" );
				return false;
			}
		}

		if( pclsSetup->m_strListenIp.empty() )
		{
			m_hTcpListenFd = TcpListen( pclsSetup->m_iListenPort, 255 );
		}
		else
		{
			m_hTcpListenFd = TcpListen( pclsSetup->m_iListenPort, 255, pclsSetup->m_strListenIp.c_str() );
		}

		if( m_hTcpListenFd == INVALID_SOCKET )
		{
			CLog::Print( LOG_ERROR, "TcpListen(%d) error(%d)", pclsSetup->m_iListenPort, GetError() );
			return false;
		}

		if( StartThread( "TcpListenThread", TcpListenThread, this ) == false )
		{
			Stop();
			return false;
		}
	}
	else if( pclsSetup->m_bUseTls )
	{
		SSLClientStart();
	}

	if( m_clsThreadList.Create( this ) == false )
	{
		return false;
	}

	return true;
}

/**
 * @ingroup TcpStack
 * @brief TCP stack �� �����Ѵ�.
 * @returns true �� �����Ѵ�.
 */
bool CTcpStack::Stop( )
{
	m_bStop = true;

	if( m_clsSetup.m_bUseTls )
	{
		if( m_clsSetup.m_iListenPort > 0 )
		{
			SSLServerStop();
		}
		else
		{
			SSLClientStop();
		}
	}

	m_clsThreadList.Destroy();

	if( m_hTcpListenFd != INVALID_SOCKET )
	{
		closesocket( m_hTcpListenFd );
		m_hTcpListenFd = INVALID_SOCKET;
	}

	return true;
}

/**
 * @ingroup TcpStack
 * @brief Ư�� ���ǿ� TCP ��Ŷ�� �����Ѵ�.
 * @param pszIp				IP �ּ�
 * @param iPort				��Ʈ ��ȣ
 * @param pszPacket		��Ŷ
 * @param iPacketLen	��Ŷ ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpStack::Send( const char * pszIp, int iPort, const char * pszPacket, int iPacketLen )
{
	return m_clsClientMap.Send( pszIp, iPort, pszPacket, iPacketLen );
}

/**
 * @ingroup TcpStack
 * @brief Ư�� ���ǿ� TCP ��Ŷ�� �����Ѵ�.
 * @param iThreadIndex	TCP ������ ��ȣ
 * @param iSessionIndex TCP ���� ��ȣ
 * @param pszPacket			��Ŷ
 * @param iPacketLen		��Ŷ ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpStack::Send( int iThreadIndex, int iSessionIndex, const char * pszPacket, int iPacketLen )
{
	return m_clsThreadList.Send( iThreadIndex, iSessionIndex, pszPacket, iPacketLen );
}

/**
 * @ingroup TcpStack
 * @brief ��� ���ǿ� TCP ��Ŷ�� �����Ѵ�.
 * @param pszPacket			��Ŷ
 * @param iPacketLen		��Ŷ ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpStack::SendAll( const char * pszPacket, int iPacketLen )
{
	return m_clsThreadList.SendAll( pszPacket, iPacketLen, m_pclsCallBack );
}
