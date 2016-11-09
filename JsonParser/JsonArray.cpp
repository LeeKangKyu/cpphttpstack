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

#include "JsonObject.h"
#include "Log.h"

CJsonArray::CJsonArray()
{
	m_cType = JSON_TYPE_ARRAY;
}

CJsonArray::~CJsonArray()
{
	Clear();
}

/**
 * @ingroup JsonParser
 * @brief JSON �迭 ���ڿ� �Ľ��Ͽ��� �ڷᱸ���� �����Ѵ�.
 * @param pszText		JSON �迭 ���ڿ�
 * @param iTextLen	JSON �迭 ���ڿ� ����
 * @returns JSON �迭 ���ڿ� �Ľ̿� �����ϸ� �Ľ��� ���̸� �����ϰ� �׷��� ������ -1 �� �����Ѵ�.
 */
int CJsonArray::Parse( const char * pszText, int iTextLen )
{
	int iPos = -1, iParseLen;
	uint8_t	cType = 0;
	std::string	strName;

	Clear();

	for( int i = 0; i < iTextLen; ++i )
	{
		if( pszText[i] == '[' )
		{
			iPos = i + 1;
			break;
		}
	}

	if( iPos == -1 ) 
	{
		CLog::Print( LOG_ERROR, "%s [ is not found", __FUNCTION__ );
		return -1;
	}

	for( int i = iPos; i < iTextLen; ++i )
	{
		if( cType == 0 )
		{
			CJsonType * pclsType = CJsonObject::GetJsonType( pszText, iTextLen, i );

			if( pclsType )
			{
				iParseLen = pclsType->Parse( pszText + i, iTextLen - i );
				if( iParseLen == -1 )
				{
					CLog::Print( LOG_ERROR, "%s json parse error", __FUNCTION__ );
					delete pclsType;
					return -1;
				}

				m_clsList.push_back( pclsType );
				cType = 0;
				i += iParseLen;
			}
		}
		else if( pszText[i] == ']' )
		{
			if( cType == 0 )
			{
				iPos = i + 1;
				break;
			}
		}
	}

	return iPos;
}

/**
 * @ingroup JsonParser
 * @brief �ڷᱸ���� JSON �迭 ���ڿ��� ��ȯ�Ѵ�.
 * @param strText JSON �迭 ���ڿ� ���� ����
 * @returns JSON �迭 ���ڿ� ���̸� �����Ѵ�.
 */
int CJsonArray::ToString( std::string & strText )
{
	JSON_LIST::iterator itJL;
	std::string strBuf;

	strBuf.append( "[" );

	for( itJL = m_clsList.begin(); itJL != m_clsList.end(); ++itJL )
	{
		if( itJL == m_clsList.begin() )
		{
			strBuf.append( " " );
		}
		else
		{
			strBuf.append( ", " );
		}

		CJsonObject::JsonToString( *itJL, strBuf );
	}

	strBuf.append( " ]" );
	strText.append( strBuf );

	return strBuf.length();
}

/**
 * @ingroup JsonParser
 * @brief �ڷᱸ���� �ʱ�ȭ��Ų��.
 */
void CJsonArray::Clear()
{
	JSON_LIST::iterator itJL;

	for( itJL = m_clsList.begin(); itJL != m_clsList.end(); ++itJL )
	{
		delete *itJL;
	}

	m_clsList.clear();
}

bool CJsonArray::SelectData( int iIndex, std::string & strValue )
{
	CJsonType * pclsType;

	if( SelectData( iIndex, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_STRING )
	{
		CLog::Print( LOG_ERROR, "%s type(%d) is not string (%s)", __FUNCTION__, iIndex, pclsType->GetTypeString() );
		return false;
	}

	strValue = ((CJsonString *)pclsType)->m_strValue;

	return true;
}

bool CJsonArray::SelectData( int iIndex, int64_t & iValue )
{
	CJsonType * pclsType;

	if( SelectData( iIndex, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_INT )
	{
		CLog::Print( LOG_ERROR, "%s type(%d) is not int (%s)", __FUNCTION__, iIndex, pclsType->GetTypeString() );
		return false;
	}

	iValue = ((CJsonInt *)pclsType)->m_iValue;

	return true;
}

bool CJsonArray::SelectData( int iIndex, bool bValue )
{
	CJsonType * pclsType;

	if( SelectData( iIndex, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_BOOL )
	{
		CLog::Print( LOG_ERROR, "%s type(%d) is not bool (%s)", __FUNCTION__, iIndex, pclsType->GetTypeString() );
		return false;
	}

	bValue = ((CJsonBool *)pclsType)->m_bValue;

	return true;
}

bool CJsonArray::SelectData( int iIndex, CJsonObject ** ppclsObject )
{
	CJsonType * pclsType;

	if( SelectData( iIndex, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_OBJECT )
	{
		CLog::Print( LOG_ERROR, "%s type(%d) is not object (%s)", __FUNCTION__, iIndex, pclsType->GetTypeString() );
		return false;
	}

	*ppclsObject = (CJsonObject *)pclsType;

	return true;
}

bool CJsonArray::SelectData( int iIndex, CJsonArray ** ppclsArray )
{
	CJsonType * pclsType;

	if( SelectData( iIndex, &pclsType ) == false ) return false;
	if( pclsType->m_cType != JSON_TYPE_ARRAY )
	{
		CLog::Print( LOG_ERROR, "%s type(%d) is not array (%s)", __FUNCTION__, iIndex, pclsType->GetTypeString() );
		return false;
	}

	*ppclsArray = (CJsonArray *)pclsType;

	return true;
}

bool CJsonArray::SelectData( int iIndex, CJsonType ** ppclsType )
{
	if( iIndex >= (int)m_clsList.size() ) 
	{
		CLog::Print( LOG_ERROR, "%s iIndex(%d) >= m_clsList.size(%d)", __FUNCTION__, iIndex, m_clsList.size() );
		return false;
	}

	JSON_LIST::iterator itJL;
	int iCount = 0;

	for( itJL = m_clsList.begin(); itJL != m_clsList.end(); ++itJL )
	{
		if( iIndex == iCount )
		{
			*ppclsType = *itJL;
			return true;
		}

		++iCount;
	}

	return false;
}
