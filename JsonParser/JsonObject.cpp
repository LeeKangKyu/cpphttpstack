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
#include "JsonNumber.h"
#include "Log.h"

CJsonObject::CJsonObject()
{
	m_cType = JSON_TYPE_OBJECT;
}

CJsonObject::~CJsonObject()
{
	Clear();
}

/**
 * @ingroup JsonParser
 * @brief JSON object ���ڿ� �Ľ��Ͽ��� �ڷᱸ���� �����Ѵ�.
 * @param pszText		JSON object ���ڿ�
 * @param iTextLen	JSON object ���ڿ� ����
 * @returns JSON object ���ڿ� �Ľ̿� �����ϸ� �Ľ��� ���̸� �����ϰ� �׷��� ������ -1 �� �����Ѵ�.
 */
int CJsonObject::Parse( const char * pszText, int iTextLen )
{
	int iPos = -1, iParseLen;
	uint8_t	cType = 0;
	std::string	strName;

	for( int i = 0; i < iTextLen; ++i )
	{
		if( pszText[i] == '{' )
		{
			iPos = i + 1;
			break;
		}
	}

	if( iPos == -1 ) 
	{
		CLog::Print( LOG_ERROR, "%s { is not found", __FUNCTION__ );
		return -1;
	}

	for( int i = iPos; i < iTextLen; ++i )
	{
		if( cType == 3 )
		{
			CJsonType * pclsType = NULL;

			if( pszText[i] == '"' )
			{
				pclsType = new CJsonString();
				if( pclsType == NULL )
				{
					CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
					return -1;
				}
			}
			else if( pszText[i] == '{' )
			{
				pclsType = new CJsonObject();
				if( pclsType == NULL )
				{
					CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
					return -1;
				}
			}
			else if( pszText[i] == '[' )
			{

			}
			else if( isdigit(pszText[i]) )
			{
				CJsonNumber clsNumber;

				iParseLen = clsNumber.Parse( pszText + i, iTextLen - i );
				if( iParseLen == -1 )
				{
					CLog::Print( LOG_ERROR, "%s json number parse error", __FUNCTION__ );
					return -1;
				}

				if( clsNumber.IsDouble() )
				{
					pclsType = new CJsonNumber();
					if( pclsType == NULL )
					{
						CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
						return -1;
					}
				}
				else
				{
					pclsType = new CJsonInt();
					if( pclsType == NULL )
					{
						CLog::Print( LOG_ERROR, "%s new error", __FUNCTION__ );
						return -1;
					}
				}
			}

			if( pclsType )
			{
				iParseLen = pclsType->Parse( pszText + i, iTextLen - i );
				if( iParseLen == -1 )
				{
					CLog::Print( LOG_ERROR, "%s json string parse error", __FUNCTION__ );
					delete pclsType;
					return -1;
				}

				m_clsMap.insert( JSON_OBJECT_MAP::value_type( strName, pclsType ) );
				cType = 0;
				i += iParseLen;
			}
		}
		else if( pszText[i] == '"' )
		{
			if( cType == 0 )
			{
				iPos = i + 1;
				++cType;
			}
			else if( cType == 1 )
			{
				strName.clear();
				strName.append( pszText + iPos, i - iPos );
				++cType;
			}
		}
		else if( pszText[i] == ':' )
		{
			if( cType == 2 )
			{
				++cType;
			}
		}
		else if( pszText[i] == '}' )
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
 * @brief �ڷᱸ���� JSON object ���ڿ��� ��ȯ�Ѵ�.
 * @param strText JSON object ���ڿ� ���� ����
 * @returns JSON object ���ڿ� ���̸� �����Ѵ�.
 */
int CJsonObject::ToString( std::string & strText )
{
	JSON_OBJECT_MAP::iterator itMap;

	strText.append( "{" );

	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		if( itMap != m_clsMap.begin() )
		{
			strText.append( "," );
		}

		strText.append( " \"" );
		strText.append( itMap->first );
		strText.append( "\" : " );

		switch( itMap->second->m_cType )
		{
		case JSON_TYPE_STRING:
			((CJsonString *)itMap->second)->ToString( strText );
			break;
		case JSON_TYPE_NUMBER:
			((CJsonNumber *)itMap->second)->ToString( strText );
			break;
		case JSON_TYPE_INT:
			((CJsonInt *)itMap->second)->ToString( strText );
			break;
		case JSON_TYPE_OBJECT:
			((CJsonObject *)itMap->second)->ToString( strText );
			break;
		}
	}

	strText.append( " }" );

	return strText.length();
}

/**
 * @ingroup JsonParser
 * @brief �ڷᱸ���� �ʱ�ȭ��Ų��.
 */
void CJsonObject::Clear()
{
	JSON_OBJECT_MAP::iterator itMap;

	for( itMap = m_clsMap.begin(); itMap != m_clsMap.end(); ++itMap )
	{
		delete itMap->second;
	}

	m_clsMap.clear();
}
