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

#include "JsonNull.h"

CJsonNull::CJsonNull()
{
	m_cType = JSON_TYPE_NULL;
}

CJsonNull::~CJsonNull()
{
}

/**
 * @ingroup JsonParser
 * @brief JSON null ���ڿ� �Ľ��Ͽ��� �ڷᱸ���� �����Ѵ�.
 * @param pszText		JSON null ���ڿ�
 * @param iTextLen	JSON null ���ڿ� ����
 * @returns JSON null ���ڿ� �Ľ̿� �����ϸ� �Ľ��� ���̸� �����ϰ� �׷��� ������ -1 �� �����Ѵ�.
 */
int CJsonNull::Parse( const char * pszText, int iTextLen )
{
	if( !strncmp( pszText, "null", 4 ) )
	{
		return 4;
	}

	return -1;
}

/**
 * @ingroup JsonParser
 * @brief �ڷᱸ���� JSON null ���ڿ��� ��ȯ�Ѵ�.
 * @param strText JSON ���ڿ� ���� ����
 * @returns JSON boolean ���ڿ� ���̸� �����Ѵ�.
 */
int CJsonNull::ToString( std::string & strText )
{
	strText.append( "null" );

	return 4;
}
