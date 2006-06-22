/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ImageExtractor.h"

#include <sys/stat.h>

ImageExtractor::ImageExtractor()
    : m_pszOutputDirectory( NULL ), m_nCount( 0 )
{

}

ImageExtractor::~ImageExtractor()
{
}

PdfError ImageExtractor::Init( const char* pszInput, const char* pszOutput, int* pnNum )
{
    PdfError    eCode;
    PdfVariant  var;

    if( !pszInput || !pszOutput )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    m_pszOutputDirectory = (char*)pszOutput;

    SAFE_OP( m_parser.Init( pszInput ) );
    
    const TVecObjects&  vecObjects = m_parser.GetObjects();
    TCIVecObjects it = vecObjects.begin();

    if( pnNum )
        *pnNum = 0;

    while( it != vecObjects.end() )
    {
        if( (*it)->HasKey( PdfName::KeyType ) )
        {
            SAFE_OP( (*it)->GetKeyValueVariant( PdfName::KeyType, var ) );
            if( var.GetName().Name() && strcmp( var.GetName().Name(), "XObject" ) == 0 )
            {
                if( (*it)->HasKey( PdfName::KeySubtype ) )
                {
                    SAFE_OP( (*it)->GetKeyValueVariant( PdfName::KeySubtype, var ) );
                    if( var.GetName().Name() && strcmp( var.GetName().Name(), "Image" ) == 0 )
                    {
						SAFE_OP( (*it)->GetKeyValueVariant( PdfName::KeyFilter, var ) );
						if( var.GetName().Name() && strcmp( var.GetName().Name(), "DCTDecode" ) == 0 )
						{	// ONLY images with filter of DCTDecode can be extracted out as JPEG this way!

							SAFE_OP( ExtractImage( *it ) );

							if( pnNum )
								++(*pnNum);
						}
                    }
                }
            }
                    
        }

        ++it;
    }

    return eCode;
}

PdfError ImageExtractor::ExtractImage( PdfObject* pObject )
{
    PdfError eCode;
    FILE*    hFile = NULL;
//    long     lLen;

    // Do not overwrite existing files:
    do {
        snprintf( m_szBuffer, MAX_PATH, "%s/pdfimage_%i.jpg", m_pszOutputDirectory, m_nCount++ );
    } while( FileExists( m_szBuffer ) );

    hFile = fopen( m_szBuffer, "wb" );
    if( !hFile )
    {
        RAISE_ERROR( ePdfError_InvalidHandle );
    }

    printf("-> Writing image object %s to the file: %s\n", pObject->Reference().ToString().c_str(), m_szBuffer);

    fwrite( pObject->Stream()->Get(), pObject->Stream()->Length(), sizeof(char), hFile );
    fclose( hFile );

    return eCode;
}

bool ImageExtractor::FileExists( const char* pszFilename )
{
    bool result = true;
    
	// if there is an error, it's probably because the file doesn't yet exist
	struct	stat	stBuf;
	if ( stat( pszFilename, &stBuf ) == -1 )	result = false;

    return result;
}
