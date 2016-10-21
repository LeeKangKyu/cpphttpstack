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

#ifndef _SOAP_CLASS_H_
#define _SOAP_CLASS_H_

#include "SoapMethod.h"

/**
 * @ingroup WsdlParser
 * @brief SOAP 서비스를 저장하는 클래스
 */
class CSoapClass
{
public:
	CSoapClass();
	~CSoapClass();

	/** 서비스 이름 */
	std::string m_strName;

	/** 서비스 URL - HTTP POST URL */
	std::string m_strUrl;

	/** 서비스 namespave */
	std::string m_strNameSpace;

	/** 서비스에 포함된 메소드 맵 */
	SOAP_METHOD_MAP m_clsMethodMap;
};

#endif
