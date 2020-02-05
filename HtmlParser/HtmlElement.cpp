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

#include "HtmlElement.h"
#include "StringUtility.h"
#include "Log.h"
#include "MemoryDebug.h"

CHtmlElement::CHtmlElement() : m_bNotParseUntilNameEnd(false)
{
}

CHtmlElement::~CHtmlElement()
{
}

#define HTML_ELEMENT_NULL				0
#define HTML_ELEMENT_NAME				1
#define HTML_ELEMENT_ATTR				2
#define HTML_ELEMENT_DATA				3
#define HTML_ELEMENT_NAME_END		4
#define HTML_ELEMENT_DATA_PARSE	5
#define HTML_ELEMENT_COMMENT			6
#define HTML_ELEMENT_DECLARATION	8

/**
 * @ingroup HtmlParser
 * @brief HTML 문자열을 파싱하여서 멤버 변수에 저장한다.
 * @param pszText		HTML 문자열
 * @param iTextLen	HTML 문자열 길이
 * @returns 성공하면 파싱한 HTML 문자열의 길이를 리턴하고 그렇지 않으면 -1 를 리턴한다.
 */
int CHtmlElement::Parse( const char * pszText, int iTextLen )
{
	int		iPos, iStartPos = -1, iLen;
	char	cType = HTML_ELEMENT_NULL, cTypeOld = HTML_ELEMENT_NULL;
	std::string	strName, strValue;

	m_bNotParseUntilNameEnd = false;
	Clear();

	for( iPos = 0; iPos < iTextLen; ++iPos )
	{
		if( cType == HTML_ELEMENT_COMMENT )
		{
			if( pszText[iPos] == '-' )
			{
				if( iPos + 3 < iTextLen && !strncmp( pszText + iPos + 1, "->", 2 ) )
				{
					iPos += 2;
					cType = cTypeOld;
				}
			}

			continue;
		}
		else if( cType == HTML_ELEMENT_DECLARATION )
		{
			if( pszText[iPos] == '>' )
			{
				cType = HTML_ELEMENT_NULL;
			}

			continue;
		}

		if( pszText[iPos] == ' ' || pszText[iPos] == '\t' || pszText[iPos] == '/' || pszText[iPos] == '\r' || pszText[iPos] == '\n' )
		{
			if( cType == HTML_ELEMENT_NAME )
			{
				SetName( pszText + iStartPos, iPos - iStartPos );
				cType = HTML_ELEMENT_ATTR;
				iStartPos = -1;
				strName.clear();
				strValue.clear();
			}
		}
		else if( pszText[iPos] == '<' )
		{
			if( iPos + 4 < iTextLen && !strncmp( pszText + iPos + 1, "!--", 3 ) )
			{
				cTypeOld = cType;
				cType = HTML_ELEMENT_COMMENT;
			}
			else if( iPos + 1 < iTextLen && ( pszText[iPos+1] == '?' || pszText[iPos+1] == '!' ) )
			{
				cTypeOld = cType;
				cType = HTML_ELEMENT_DECLARATION;
			}
			else if( cType == HTML_ELEMENT_NULL )
			{
				cType = HTML_ELEMENT_NAME;
				iStartPos = iPos + 1;
			}
			else if( pszText[iPos+1] == '/' )
			{
				if( cType == HTML_ELEMENT_DATA )
				{
					m_strData.append( pszText + iStartPos, iPos - iStartPos );		
				}

				iLen = (int)m_strName.length();

				if( iTextLen - ( iPos + 2 ) < (int)( iLen + 1 ) ) 
				{
					CLog::Print( LOG_ERROR, "end tag is not correct for start tag(%s)", m_strName.c_str() );
					return -1;
				}

				if( !strncmp( pszText + iPos + 2, m_strName.c_str(), iLen ) )
				{
					cType = HTML_ELEMENT_NAME_END;
					iPos += iLen + 1;
				}
				else
				{
					CLog::Print( LOG_ERROR, "end tag(%.*s) is not correct for start tag(%s)", iLen, pszText + iPos + 2, m_strName.c_str() );
					return -1;
				}
			}
			else if( m_bNotParseUntilNameEnd == false )
			{
				CHtmlElement clsElement;

				iLen = clsElement.Parse( pszText + iPos, iTextLen - iPos );
				if( iLen == -1 ) return -1;

				m_clsElementList.push_back( clsElement );

				cType = HTML_ELEMENT_DATA_PARSE;
				iPos += iLen - 1;
			}
		}
		else if( pszText[iPos] == '>' )
		{
			if( cType == HTML_ELEMENT_NAME )
			{
				SetName( pszText + iStartPos, iPos - iStartPos );
			}
			else if( cType == HTML_ELEMENT_NAME_END )
			{
				++iPos;
				break;
			}
			else if( iPos < 2 )
			{
				CLog::Print( LOG_ERROR, "iPos(%d) < 2 : pszText[iPos](%s)", iPos, pszText + iPos );
				return -1;
			}
			else if( pszText[iPos-1] == '/' )
			{
				++iPos;
				break;
			}
			
			if( cType != HTML_ELEMENT_DATA )
			{
				cType = HTML_ELEMENT_DATA;
				iStartPos = iPos + 1;
			}
		}
		else if( cType == HTML_ELEMENT_ATTR )
		{
			if( iStartPos == -1 )
			{
				iStartPos = iPos;
			}
			else if( pszText[iPos] == '=' && strName.empty() )
			{
				if( pszText[iPos+1] != '"' )
				{
					CLog::Print( LOG_ERROR, "iPos(%d+1) != '\"' : pszText[iPos](%s)", iPos, pszText + iPos );
					return -1;
				}

				strName.append( pszText + iStartPos, iPos - iStartPos );
				++iPos;
				iStartPos = iPos + 1;
			}
			else if( pszText[iPos] == '"' )
			{
				strValue.append( pszText + iStartPos, iPos - iStartPos );

				m_clsAttributeMap.insert( HTML_ATTRIBUTE_MAP::value_type( strName, strValue ) );

				strName.clear();
				strValue.clear();
				iStartPos = -1;
			}
		}
	}

	return iPos;
}

/**
 * @ingroup HtmlParser
 * @brief XML 문자열을 파싱하여서 멤버 변수에 저장한다.
 * @param strText XML 문자열
 * @returns 성공하면 파싱한 XML 문자열의 길이를 리턴하고 그렇지 않으면 -1 를 리턴한다.
 */
int CHtmlElement::Parse( std::string & strText )
{
	return Parse( strText.c_str(), (int)strText.length() );
}

/**
 * @ingroup HtmlParser
 * @brief 멤버 변수에 저장된 값을 이용하여서 XML 문자열을 생성한다.
 * @param pszText			XML 문자열을 저장할 변수
 * @param iTextSize		XML 문자열을 저장할 변수의 크기
 * @param bUseTab			TAB 문자를 사용하는가? 하위 element 시작시 TAB 문자를 넣어주고 싶으면 true 를 입력하고 그렇지 않으면 false 를 입력한다.
 * @param	iDepth			XML 하위 element 깊이. 맨 위의 XML element 는 0 이고 그 하위 element 는 1 이다.
 * @returns 생성된 XML 문자열의 길이를 리턴한다.
 */
int CHtmlElement::ToString( char * pszText, int iTextSize, bool bUseTab, int iDepth )
{
	int iLen = 0, n;

	if( bUseTab )
	{
		for( int i = 0; i < iDepth; ++i )
		{
			iLen += snprintf( pszText + iLen, iTextSize, "\t" );
		}
	}

	iLen += snprintf( pszText + iLen, iTextSize, "<%s", m_strName.c_str() );

	if( m_clsAttributeMap.empty() == false )
	{
		HTML_ATTRIBUTE_MAP::iterator	itAM;

		for( itAM = m_clsAttributeMap.begin(); itAM != m_clsAttributeMap.end(); ++itAM )
		{
			iLen += snprintf( pszText + iLen, iTextSize - iLen, " %s=\"%s\"", itAM->first.c_str(), itAM->second.c_str() );
		}
	}

	if( m_strData.empty() == false )
	{
		iLen += snprintf( pszText + iLen, iTextSize - iLen, ">%s</%s>\n", m_strData.c_str(), m_strName.c_str() );
	}
	else if( m_clsElementList.empty() == false )
	{
		HTML_ELEMENT_LIST::iterator	itEL;

		iLen += snprintf( pszText + iLen, iTextSize - iLen, ">\n" );

		for( itEL = m_clsElementList.begin(); itEL != m_clsElementList.end(); ++itEL )
		{
			n = itEL->ToString( pszText + iLen, iTextSize - iLen, bUseTab, iDepth + 1 );
			if( n == -1 ) return -1;
			iLen += n;
		}

		if( bUseTab )
		{
			for( int i = 0; i < iDepth; ++i )
			{
				iLen += snprintf( pszText + iLen, iTextSize, "\t" );
			}
		}

		iLen += snprintf( pszText + iLen, iTextSize - iLen, "</%s>\n", m_strName.c_str() );
	}
	else
	{
		iLen += snprintf( pszText + iLen, iTextSize - iLen, "/>\n" );
	}

	return iLen;
}

/**
 * @ingroup HtmlParser
 * @brief 멤버 변수에 저장된 값을 이용하여서 XML 문자열을 생성한다.
 * @param strText XML 문자열을 저장할 변수
 * @param bUseTab			TAB 문자를 사용하는가? 하위 element 시작시 TAB 문자를 넣어주고 싶으면 true 를 입력하고 그렇지 않으면 false 를 입력한다.
 * @param	iDepth			XML 하위 element 깊이. 맨 위의 XML element 는 0 이고 그 하위 element 는 1 이다.
 */
void CHtmlElement::ToString( std::string & strText, bool bUseTab, int iDepth )
{
	if( bUseTab )
	{
		for( int i = 0; i < iDepth; ++i )
		{
			strText.append( "\t" );
		}
	}

	strText.append( "<" );
	strText.append( m_strName );

	if( m_clsAttributeMap.empty() == false )
	{
		HTML_ATTRIBUTE_MAP::iterator	itAM;

		for( itAM = m_clsAttributeMap.begin(); itAM != m_clsAttributeMap.end(); ++itAM )
		{
			strText.append( " " );
			strText.append( itAM->first );
			strText.append( "=\"" );
			strText.append( itAM->second );
			strText.append( "\"" );
		}
	}

	if( m_strData.empty() == false )
	{
		strText.append( ">" );
		strText.append( m_strData );
		strText.append( "</" );
		strText.append( m_strName );
		strText.append( ">\n" );
	}
	else if( m_clsElementList.empty() == false )
	{
		HTML_ELEMENT_LIST::iterator	itEL;

		strText.append( ">\n" );

		for( itEL = m_clsElementList.begin(); itEL != m_clsElementList.end(); ++itEL )
		{
			itEL->ToString( strText, bUseTab, iDepth + 1 );
		}

		if( bUseTab )
		{
			for( int i = 0; i < iDepth; ++i )
			{
				strText.append( "\t" );
			}
		}

		strText.append( "</" );
		strText.append( m_strName );
		strText.append( ">\n" );
	}
	else
	{
		strText.append( "/>\n" );
	}
}

/**
 * @ingroup HtmlParser
 * @brief 멤버 변수를 초기화시킨다.
 */
void CHtmlElement::Clear( )
{
	m_strName.clear();
	m_strData.clear();
	m_clsAttributeMap.clear();
	m_clsElementList.clear();
}

/**
 * @ingroup HtmlParser
 * @brief 애트리뷰트에 해당하는 값을 검색한다.
 * @param pszName 애트리뷰트 이름
 * @returns 성공하면 애트리뷰트의 값을 리턴하고 그렇지 않으면 NULL 을 리턴한다.
 */
const char * CHtmlElement::SelectAttribute( const char * pszName )
{
	HTML_ATTRIBUTE_MAP::iterator	itAM;

	itAM = m_clsAttributeMap.find( pszName );
	if( itAM != m_clsAttributeMap.end() )
	{
		return itAM->second.c_str();
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 애트리뷰트에 해당하는 값을 검색한다. 값의 앞, 뒤 공백을 제거한 문자열의 포인터를 리턴한다.
 * @param pszName 애트리뷰트 이름
 * @returns 성공하면 애트리뷰트의 값을 리턴하고 그렇지 않으면 NULL 을 리턴한다.
 */
const char * CHtmlElement::SelectAttributeTrim( const char * pszName )
{
	HTML_ATTRIBUTE_MAP::iterator	itAM;

	itAM = m_clsAttributeMap.find( pszName );
	if( itAM != m_clsAttributeMap.end() )
	{
		TrimString( itAM->second );
		return itAM->second.c_str();
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 애트리뷰트에 해당하는 값을 검색한다.
 * @param pszName		애트리뷰트 이름
 * @param strValue	애트리뷰트 값
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectAttribute( const char * pszName, std::string & strValue )
{
	HTML_ATTRIBUTE_MAP::iterator	itAM;

	strValue.clear();

	itAM = m_clsAttributeMap.find( pszName );
	if( itAM != m_clsAttributeMap.end() )
	{
		strValue = itAM->second;
		return true;
	}

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief 애트리뷰트에 해당하는 값을 검색한다. 검색에 성공하면 값의 왼쪽, 오른쪽 공백을 제거한다.
 * @param pszName		애트리뷰트 이름
 * @param strValue	애트리뷰트 값
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectAttributeTrim( const char * pszName, std::string & strValue )
{
	HTML_ATTRIBUTE_MAP::iterator	itAM;

	strValue.clear();

	itAM = m_clsAttributeMap.find( pszName );
	if( itAM != m_clsAttributeMap.end() )
	{
		strValue = itAM->second;
		TrimString( strValue );
		return true;
	}

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief 애트리뷰트에 해당하는 값을 검색하여 int 변수에 저장한다.
 * @param pszName		애트리뷰트 이름
 * @param iValue		애트리뷰트 값
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectAttribute( const char * pszName, int & iValue )
{
	iValue = 0;

	const char * pszValue = SelectAttributeTrim( pszName );
	if( pszValue )
	{
		iValue = atoi( pszValue );
		return true;
	}

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 를 검색한다.
 * @param pszName		하위 Element 이름
 * @param iIndex		하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 성공하면 하위 Element 객체의 포인터를 리턴하고 그렇지 않으면 NULL 을 리턴한다.
 */
CHtmlElement * CHtmlElement::SelectElement( const char * pszName, const int iIndex )
{
	HTML_ELEMENT_LIST::iterator	itEL;
	int iCount = 0;

	if( iIndex < 0 ) return NULL;

	for( itEL = m_clsElementList.begin(); itEL != m_clsElementList.end(); ++itEL )
	{
		if( !strcmp( pszName, itEL->m_strName.c_str() ) )
		{
			if( iCount == iIndex )
			{
				return &(*itEL);
			}

			++iCount;
		}
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 를 검색한다.
 * @param iIndex 하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 성공하면 하위 Element 객체의 포인터를 리턴하고 그렇지 않으면 NULL 을 리턴한다.
 */
CHtmlElement * CHtmlElement::SelectElement( const int iIndex )
{
	HTML_ELEMENT_LIST::iterator	itEL;
	int iCount = 0;

	if( iIndex < 0 ) return NULL;

	for( itEL = m_clsElementList.begin(); itEL != m_clsElementList.end(); ++itEL )
	{
		if( iCount == iIndex )
		{
			return &(*itEL);
		}

		++iCount;
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 를 검색하여서 Element 리스트에 저장한다.
 * @param pszName		하위 Element 이름
 * @param clsList		하위 Element 를 저장할 변수
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectElementList( const char * pszName, HTML_ELEMENT_LIST & clsList )
{
	HTML_ELEMENT_LIST::iterator	itEL;

	clsList.clear();

	for( itEL = m_clsElementList.begin(); itEL != m_clsElementList.end(); ++itEL )
	{
		if( !strcmp( pszName, itEL->m_strName.c_str() ) )
		{
			clsList.push_back( *itEL );
		}
	}

	if( clsList.empty() == false ) return true;

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 의 값 문자열의 포인터를 리턴한다.
 * @param pszName 하위 Element 이름
 * @param iIndex	하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 하위 Element 이름이 존재하면 해당 Element 의 값 문자열의 포인터를 리턴하고 그렇지 않으면 NULL 을 리턴한다. 
 */
const char * CHtmlElement::GetElementData( const char * pszName, const int iIndex )
{
	CHtmlElement * pclsElement = SelectElement( pszName, iIndex );
	if( pclsElement )
	{
		return pclsElement->m_strData.c_str();
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 의 값 문자열의 포인터를 리턴한다.
 * @param pszName 하위 Element 이름
 * @param iIndex	하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 하위 Element 이름이 존재하면 해당 Element 의 값 문자열의 포인터를 리턴하고 그렇지 않으면 NULL 을 리턴한다. 
 */
const char * CHtmlElement::GetElementDataTrim( const char * pszName, const int iIndex )
{
	CHtmlElement * pclsElement = SelectElement( pszName, iIndex );
	if( pclsElement )
	{
		TrimString( pclsElement->m_strData );
		return pclsElement->m_strData.c_str();
	}

	return NULL;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 를 검색하여서 내용을 저장한다.
 * @param pszName		하위 Element 이름
 * @param strData		하위 Elemnet 의 내용을 저장할 변수
 * @param iIndex		하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectElementData( const char * pszName, std::string & strData, const int iIndex )
{
	strData.clear();

	CHtmlElement * pclsElement = SelectElement( pszName, iIndex );
	if( pclsElement )
	{
		strData = pclsElement->m_strData;
		return true;
	}

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief 하위 Element 를 검색하여서 내용을 저장한다. 검색에 성공하면 내용의 왼쪽, 오른쪽 공백을 제거한다.
 * @param pszName		하위 Element 이름
 * @param strData		하위 Elemnet 의 내용을 저장할 변수
 * @param iIndex		하위 Element 인덱스. 0 을 입력하면 첫번째 검색된 하위 Element 를 리턴하고 1 을 입력하면 두번째 검색된 하위 Element 를 리턴한다.
 * @returns 성공하면 true 를 리턴하고 실패하면 false 를 리턴한다.
 */
bool CHtmlElement::SelectElementTrimData( const char * pszName, std::string & strData, const int iIndex )
{
	strData.clear();

	CHtmlElement * pclsElement = SelectElement( pszName, iIndex );
	if( pclsElement )
	{
		strData = pclsElement->m_strData;
		TrimString( strData );
		return true;
	}

	return false;
}

/**
 * @ingroup HtmlParser
 * @brief Element 이름을 리턴한다.
 * @returns Element 이름을 리턴한다.
 */
const char * CHtmlElement::GetName()
{
	return m_strName.c_str();
}

/**
 * @ingroup HtmlParser
 * @brief Element 내용을 리턴한다.
 * @returns Element 내용을 리턴한다.
 */
const char * CHtmlElement::GetData()
{
	return m_strData.c_str();
}

/**
 * @ingroup HtmlParser
 * @brief Element 내용이 존재하면 true 를 리턴하고 그렇지 않으면 false 를 리턴한다.
 * @returns 
 */
bool CHtmlElement::IsDataEmpty()
{
	return m_strData.empty();
}

void CHtmlElement::SetName( const char * pszText, int iNameLen )
{
	m_strName.append( pszText, iNameLen );

	const char * pszName = m_strName.c_str();

	if( !strcasecmp( pszName, "script" ) || !strcasecmp( pszName, "style" ) )
	{
		m_bNotParseUntilNameEnd = true;
	}
	else
	{
		m_bNotParseUntilNameEnd = false;
	}
}
