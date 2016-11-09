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
 * @brief JSON 배열 문자열 파싱하여서 자료구조에 저장한다.
 * @param pszText		JSON 배열 문자열
 * @param iTextLen	JSON 배열 문자열 길이
 * @returns JSON 배열 문자열 파싱에 성공하면 파싱한 길이를 리턴하고 그렇지 않으면 -1 를 리턴한다.
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
 * @brief 자료구조를 JSON 배열 문자열로 변환한다.
 * @param strText JSON 배열 문자열 저장 변수
 * @returns JSON 배열 문자열 길이를 리턴한다.
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
 * @brief 자료구조를 초기화시킨다.
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 문자열 값을 검색한다.
 * @param iIndex		Element 인덱스
 * @param strValue	Element 값
 * @returns 검색에 성공하고 해당 값이 문자열 타입인 경우 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 정수 값을 검색한다.
 * @param iIndex Element 인덱스
 * @param iValue Element 값
 * @returns 검색에 성공하고 해당 값이 정수 타입인 경우 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 boolean 값을 검색한다.
 * @param iIndex Element 인덱스
 * @param bValue Element 값
 * @returns 검색에 성공하고 해당 값이 boolean 타입인 경우 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 object 값을 검색한다.
 * @param iIndex			Element 인덱스
 * @param ppclsObject Element 값
 * @returns 검색에 성공하고 해당 값이 object 타입인 경우 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 배열 값을 검색한다.
 * @param iIndex			Element 인덱스
 * @param ppclsArray	Element 값
 * @returns 검색에 성공하고 해당 값이 배열 타입인 경우 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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

/**
 * @ingroup JsonParser
 * @brief 배열 자료구조에서 Element 인덱스에 해당하는 값을 검색한다.
 * @param iIndex		Element 인덱스
 * @param ppclsType Element 값
 * @returns 검색에 성공하면 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 */
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
