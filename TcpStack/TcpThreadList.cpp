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

#include "SipPlatformDefine.h"
#include "TcpStack.h"
#include "ServerUtility.h"
#include "TimeUtility.h"
#include "Log.h"
#include "MemoryDebug.h"

THREAD_API TcpThread( LPVOID lpParameter );

CTcpThreadInfo::CTcpThreadInfo() : m_iIndex(0), m_hSend(INVALID_SOCKET), m_hRecv(INVALID_SOCKET), m_bStop(false)
{

}

CTcpThreadInfo::~CTcpThreadInfo()
{
	Close();
}

/**
 * @ingroup TcpStack
 * @brief ������ �����Ѵ�.
 */
void CTcpThreadInfo::Close()
{
	if( m_hSend != INVALID_SOCKET )
	{
		closesocket( m_hSend );
		m_hSend = INVALID_SOCKET;
	}

	if( m_hRecv != INVALID_SOCKET )
	{
		closesocket( m_hRecv );
		m_hRecv = INVALID_SOCKET;
	}
}

CTcpThreadList::CTcpThreadList() : m_pclsStack(NULL)
{
}

CTcpThreadList::~CTcpThreadList()
{
}

/**
 * @ingroup TcpStack
 * @brief ������ ����Ʈ�� �����Ѵ�.
 * @param pclsStack	CTcpStack ��ü
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpThreadList::Create( CTcpStack * pclsStack )
{
	// pipe ������ �����ؾ� �ϹǷ� 1�� �߰��Ѵ�.
	m_iMaxSocketPerThread = pclsStack->m_clsSetup.m_iMaxSocketPerThread + 1;
	m_pclsStack = pclsStack;

	for( int i = 0; i < pclsStack->m_clsSetup.m_iThreadInitCount; ++i )
	{
		if( AddThread() == false )
		{
			return false;
		}
	}

	return true;
}

/**
 * @ingroup TcpStack
 * @brief ������ ����Ʈ�� �����Ѵ�.
 */
void CTcpThreadList::Destroy()
{
	THREAD_LIST::iterator	it;

	for( it = m_clsList.begin(); it != m_clsList.end(); ++it )
	{
		(*it)->m_bStop = true;
	}

	for( int i = 0; i < 100; ++i )
	{
		MiliSleep( 20 );

		bool bAllStop = true;

		for( it = m_clsList.begin(); it != m_clsList.end(); ++it )
		{
			if( (*it)->m_bStop )
			{
				bAllStop = false;
				break;
			}
		}

		if( bAllStop ) break;
	}

	for( it = m_clsList.begin(); it != m_clsList.end(); ++it )
	{
		(*it)->Close();
		delete (*it);
	}
}

/**
 * @ingroup TcpStack
 * @brief �����忡 ����� �����Ѵ�.
 * @param pszData				���
 * @param iDataLen			pszData ����
 * @param iThreadIndex	����� ������ ������ �ε���
 *											-1 �� �Է��ϸ� �ִ� ���� ������ ���� ���� �����忡 ����� ���۵ȴ�.
 * @param piThreadIndex	���� ���۵� ������ �ε����� �����ϴ� ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpThreadList::SendCommand( const char * pszData, int iDataLen, int iThreadIndex, int * piThreadIndex )
{
	bool	bRes = false, bFound = false;

	if( iThreadIndex < -1 )
	{
		return false;
	}

	if( iThreadIndex >= (int)m_clsList.size() )
	{
		return false;
	}

	m_clsMutex.acquire();
	if( iThreadIndex == -1 )
	{
		THREAD_LIST::iterator	it;
		int iMinCount = 2000000000;

		// ������ �ּ� ����ϴ� �����带 �˻��Ѵ�.
		for( it = m_clsList.begin(), iThreadIndex = 0; it != m_clsList.end(); ++it, ++iThreadIndex )
		{
			if( iMinCount > (*it)->m_clsSessionList.m_iPoolFdCount )
			{
				iMinCount = (*it)->m_clsSessionList.m_iPoolFdCount;
				if( iMinCount == 0 ) break;
			}
		}

		if( iMinCount < m_iMaxSocketPerThread )
		{
			for( it = m_clsList.begin(), iThreadIndex = 0; it != m_clsList.end(); ++it, ++iThreadIndex )
			{
				if( iMinCount == (*it)->m_clsSessionList.m_iPoolFdCount )
				{
					bRes = _SendCommand( (*it)->m_hSend, pszData, iDataLen );
					if( piThreadIndex ) *piThreadIndex = iThreadIndex;
					bFound = true;
					break;
				}
			}
		}

		if( bFound == false )
		{
			if( AddThread() )
			{
				bRes = _SendCommand( m_clsList[m_clsList.size()-1]->m_hSend, pszData, iDataLen );
				if( piThreadIndex ) *piThreadIndex = iThreadIndex;
			}
		}
	}
	else
	{
		bRes = _SendCommand( m_clsList[iThreadIndex]->m_hSend, pszData, iDataLen );
		if( piThreadIndex ) *piThreadIndex = iThreadIndex;
	}
	m_clsMutex.release();

	return bRes;
}

/**
 * @ingroup TcpStack
 * @brief ��� �����忡 ����� �����Ѵ�.
 * @param pszData		���
 * @param iDataLen	pszData ����
 */
void CTcpThreadList::SendCommandAll( const char * pszData, int iDataLen )
{
	THREAD_LIST::iterator	it;

	m_clsMutex.acquire();
	for( it = m_clsList.begin(); it != m_clsList.end(); ++it )
	{
		_SendCommand( (*it)->m_hSend, pszData, iDataLen );
	}
	m_clsMutex.release();
}

/**
 * @ingroup TcpStack
 * @brief ����� �����Ѵ�.
 * @param hSocket		pipe ���� �ڵ�
 * @param pszData		��� ���� ����
 * @param iDataSize pszData ���� ũ��
 * @returns ������ ���̸� �����Ѵ�.
 */
int CTcpThreadList::RecvCommand( Socket hSocket, char * pszData, int iDataSize )
{
	int	n;

#ifdef WIN32
	n = recv( hSocket, pszData, iDataSize, 0 );
#else
	n = read( hSocket, pszData, iDataSize );
#endif
	
	return n;
}

/**
 * @ingroup TcpStack
 * @brief TCP ��Ŷ�� �����Ѵ�.
 * @param iThreadIndex	TCP ������ ��ȣ
 * @param iSessionIndex TCP ���� ��ȣ
 * @param pszPacket			��Ŷ
 * @param iPacketLen		��Ŷ ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpThreadList::Send( int iThreadIndex, int iSessionIndex, const char * pszPacket, int iPacketLen )
{
	return m_clsList[iThreadIndex]->m_clsSessionList.Send( iSessionIndex, pszPacket, iPacketLen );
}

/**
 * @ingroup TcpStack
 * @brief TCP ������ ��ȣ�� ���� ������ �����´�.
 * @param iThreadIndex		TCP ������ ��ȣ
 * @param ppclsThreadInfo TCP ������ ����
 * @returns true �� �����Ѵ�.
 */
bool CTcpThreadList::Select( int iThreadIndex, CTcpThreadInfo ** ppclsThreadInfo )
{
	*ppclsThreadInfo = m_clsList[iThreadIndex];

	return true;
}

/**
 * @ingroup TcpStack
 * @brief ������ ������ �ϳ��� ���ڿ��� �����Ѵ�.
 * @param strBuf ������ ������ ������ ���ڿ� ����
 */
void CTcpThreadList::GetString( CMonitorString & strBuf )
{
	THREAD_LIST::iterator	it;

	strBuf.Clear();

	for( it = m_clsList.begin(); it != m_clsList.end(); ++it )
	{
		strBuf.AddRow( (*it)->m_clsSessionList.m_iPoolFdCount );
	}
}

/**
 * @ingroup TcpStack
 * @brief �����带 �߰��Ѵ�.
 * @returns �����ϸ� true �� �����ϰ� �����ϸ� false �� �����Ѵ�.
 */
bool CTcpThreadList::AddThread()
{
	if( (int)m_clsList.size() >= m_pclsStack->m_clsSetup.m_iThreadMaxCount )
	{
		CLog::Print( LOG_ERROR, "%s thread count(%d) >= max thread count(%d)", __FUNCTION__, (int)m_clsList.size(), m_pclsStack->m_clsSetup.m_iThreadMaxCount );
		return false;
	}

	Socket	arrSocket[2];
	CTcpThreadInfo * pclsTcpThreadInfo = new CTcpThreadInfo();
	if( pclsTcpThreadInfo == NULL )
	{
		CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
		return false;
	}

	pclsTcpThreadInfo->m_iIndex = m_clsList.size();

	if( pclsTcpThreadInfo->m_clsSessionList.Init( pclsTcpThreadInfo->m_iIndex, m_iMaxSocketPerThread ) == false )
	{
		CLog::Print( LOG_ERROR, "%s m_clsSessionList.Init error", __FUNCTION__ );
		delete pclsTcpThreadInfo;
		return false;
	}

	if( pipe( arrSocket ) == -1 )
	{
		CLog::Print( LOG_ERROR, "%s pipe error", __FUNCTION__ );
		delete pclsTcpThreadInfo;
		return false;
	}

	pclsTcpThreadInfo->m_hRecv = arrSocket[0];
	pclsTcpThreadInfo->m_hSend = arrSocket[1];
	pclsTcpThreadInfo->m_clsSessionList.Insert( pclsTcpThreadInfo->m_hRecv );
	pclsTcpThreadInfo->m_pclsStack = m_pclsStack;

	m_clsList.push_back( pclsTcpThreadInfo );

	bool bRes = StartThread( "TcpThread", TcpThread, pclsTcpThreadInfo );
	if( bRes == false )
	{
		delete pclsTcpThreadInfo;
	}

	return bRes;
}

/**
 * @ingroup TcpStack
 * @brief �����忡 ����� �����Ѵ�.
 * @param hSocket		pipe ���� �ڵ�
 * @param pszData		���
 * @param iDataLen	pszData ����
 * @returns �����ϸ� true �� �����ϰ� �׷��� ������ false �� �����Ѵ�.
 */
bool CTcpThreadList::_SendCommand( Socket hSocket, const char * pszData, int iDataLen )
{
	int	n;

#ifdef WIN32
	n = send( hSocket, pszData, iDataLen, 0 );
#else
	n = write( hSocket, pszData, iDataLen );
#endif
	if( n != iDataLen )
	{
		return false;
	}

	return true;
}
