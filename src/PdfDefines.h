/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef _PDF_DEFINES_H_
#define _PDF_DEFINES_H_

/** This file should be included as first file in every header
 *  of PoDoFo lib. It includes all standard files, defines
 *  all error codes, some useful macros, some datatypes and 
 *  all important enumeration types.
 */ 

// Include common system files
#include <stdio.h>

// Include common STL files
#include <map>
#include <string>
#include <vector>

// Debug define: enable if needed
// #define _DEBUG

// Error Handling Defines
#include "PdfError.h"

/*
//#define ERRCODE        int
#define ErrOk          0

#define ERROR_PDF_FILE_NOT_FOUND        -41
#define ERROR_PDF_NO_PDF_FILE           -42
#define ERROR_PDF_NO_XREF               -43
#define ERROR_PDF_UNEXPECTED_EOF        -44
#define ERROR_PDF_INVALID_HANDLE        -45
#define ERROR_PDF_NO_OBJ                -46
#define ERROR_PDF_INVALID_STREAM_LENGTH -47
#define ERROR_PDF_OUT_OF_MEMORY         -48
#define ERROR_PDF_NO_TRAILER            -49
#define ERROR_PDF_INVALID_TRAILER_SIZE  -50
#define ERROR_PDF_NO_NUMBER             -51
#define ERROR_PDF_INVALID_LINEARIZATION -52
#define ERROR_PDF_MISSING_ENDSTREAM     -53
#define ERROR_PDF_INVALID_DATA_TYPE     -54
#define ERROR_PDF_INVALID_XREF          -55
#define ERROR_PDF_UNSUPPORTED_FILTER    -56
#define ERROR_PDF_DATE_ERROR            -57
#define ERROR_PDF_VALUE_OUT_OF_RANGE    -58
#define ERROR_PDF_FILE_UNEXPECTED_EOF   -59
#define ERROR_PDF_INVALID_HEX_STRING    -60
#define ERROR_PDF_FLATE_ERROR           -61
#define ERROR_PDF_INVALID_XREF_STREAM_W -62
#define ERROR_PDF_INVALID_XREF_TYPE     -63
#define ERROR_PDF_INVALID_PREDICTOR     -64
#define ERROR_PDF_INVALID_STROKE_STYLE  -65
*/

// Conversation constant to convert 1/1000th mm to 1/72 inch
#define CONVERSION_CONSTANT 0.002834645669291339

namespace PoDoFo {

// Datatypes which are required to have a certain size when porting
typedef unsigned int pdf_uint32;


// Enums

/** Enum to identify diferent versions of the PDF file format
 */
typedef enum EPdfVersion {
    ePdfVersion_1_0 = 0,
    ePdfVersion_1_1,
    ePdfVersion_1_2,
    ePdfVersion_1_3,
    ePdfVersion_1_4,
    ePdfVersion_1_5,
    ePdfVersion_1_6,

    ePdfVersion_Unknown = 0xff
};

typedef enum EPdfDataType {
    ePdfDataType_Bool,
    ePdfDataType_Number,
    ePdfDataType_Real,
    ePdfDataType_String,
    ePdfDataType_HexString,
    ePdfDataType_Name,
    ePdfDataType_Array,
    ePdfDataType_Dictionary,
    ePdfDataType_Stream,
    ePdfDataType_Null,
    ePdfDataType_Reference,

    ePdfDataType_Unknown = 0xff
};

typedef enum EPdfFilter {
    ePdfFilter_ASCIIHexDecode,
    ePdfFilter_ASCII85Decode,
    ePdfFilter_LZWDecode,
    ePdfFilter_FlateDecode,
    ePdfFilter_RunLengthDecode,
    ePdfFilter_CCITTFaxDecode,
    ePdfFilter_JBIG2Decode,
    ePdfFilter_DCTDecode,
    ePdfFilter_JPXDecode,
    ePdfFilter_Crypt,

    ePdfFilter_Unknown = 0xff    
};

typedef enum EPdfColorSpace {
    ePdfColorSpace_DeviceGray,
    ePdfColorSpace_DeviceRGB,
    ePdfColorSpace_DeviceCMYK,

    ePdfColorSpace_Unknown = 0xff
};

typedef enum EPdfStrokeStyle {
    ePdfStrokeStyle_Solid,
    ePdfStrokeStyle_Dash,
    ePdfStrokeStyle_Dot,
    ePdfStrokeStyle_DashDot,
    ePdfStrokeStyle_DashDotDot,
    ePdfStrokeStyle_Custom,

    ePdfStrokeStyle_Unknown = 0xff
};

typedef enum EPdfLineCapStyle {
    ePdfLineCapStyle_Butt    = 0,
    ePdfLineCapStyle_Round   = 1,
    ePdfLineCapStyle_Square  = 2,

    ePdfLineCapStyle_Unknown = 0xff
};

typedef enum EPdfLineJoinStyle {
    ePdfLineJoinStyle_Miter   = 0,
    ePdfLineJoinStyle_Round   = 1,
    ePdfLineJoinStyle_Bevel   = 2,

    ePdfLineJoinStyle_Unknown = 0xff
};

// TODO: Define all major page sizes
//       and add them also to PdfPage::CreateStadardPageSize
typedef enum EPdfPageSize {
    ePdfPageSize_A4,
    ePdfPageSize_Letter,

    ePdfPageSize_Unknown = 0xff
};

// data structures
struct TXRefEntry {
    long lOffset;
    long lGeneration;
    char cUsed;
};

struct TSize {
    long lWidth;
    long lHeight;

    TSize& operator=( const TSize & rhs )
        {
            lWidth  = rhs.lWidth;
            lHeight = rhs.lHeight;

            return *this;
        }
};

class PdfObject;

typedef std::vector<TXRefEntry>      TVecOffsets;
typedef TVecOffsets::iterator        TIVecOffsets;
typedef TVecOffsets::const_iterator  TCIVecOffsets;

// character constants
#define MAX_PDF_VERSION_STRING_INDEX  6

static const char* s_szPdfVersions[] = {
    "%PDF-1.0",
    "%PDF-1.1",
    "%PDF-1.2",
    "%PDF-1.3",
    "%PDF-1.4",
    "%PDF-1.5",
    "%PDF-1.6"
};

static const int s_nNumWhiteSpaces = 6;
static const char s_cWhiteSpaces[] = {
    0x00, // NULL
    0x09, // TAB
    0x0A, // Line Feed
    0x0C, // Form Feed
    0x0D, // Carriage Return
    0x20   // White Space
};

static const int s_nNumDelimiters = 10;
static const char s_cDelimiters[] = {
    '(',
    ')',
    '<',
    '>',
    '[',
    ']',
    '{',
    '}',
    '/',
    '%'
};

};

// macros
#define PDF_MAX(x,y) (x>y?x:y)
#define PDF_MIN(x,y) (x<y?x:y)

#define SAFE_OP_ADV( x, msg ) eCode = x;\
                              if( eCode.IsError() ) {\
                                fprintf( stderr, "Error: %s\n", msg );\
                                return eCode;\
                               }

#define SAFE_OP( x ) eCode = x;\
                     if( eCode.IsError() ) {\
                       return eCode;\
                     }

/**
 * \mainpage
 *
 * <b>PoDoFo</b> is a library to work with the PDF file format and includes also a few
 * tools. The name comes from the first letter of PDF (Portable Document
 * Format). 
 * 
 * The <b>PoDoFo</b> library is a free portable C++ library which includes
 * classes to parse a PDF file and modify its contents into memory. The changes
 * can be written back to disk easily. The parser could also be used to write a
 * PDF viewer. Besides parsing PoDoFo includes also very simple classes to create
 * your own PDF files. All classes are documented so it is easy to start writing
 * your own application using PoDoFo.
 * 
 * The <b>PoDoFo</b> tools are simple tools build around the <b>PoDoFo</b> library. These tools
 * are first of all examples on how to use the <b>PoDoFo</b> library in your own
 * projects. But secondly they offer also features for working with PDF
 * files. More tools will come with future release and the existing tools will
 * gain more features. Currently there are two tools: podofoimgextract (which
 * extracts all jpeg images from a given PDF file) and podofouncompress (which
 * removes all compression filters from a PDF file - this is useful for debugging
 * existing PDF files).
 * 
 * Additionally there is the external tool <b>PoDoFoBrowser</b> which is not included in
 * this package, but can be downloaded from the <b>PoDoFo</b> webpage. <b>PoDoFoBrowser</b> is
 * a Qt application for browsing the objects in a PDF file and modifying their
 * keys easily. It is very useful if you want to look on the internal structure
 * of PDF files.
 * 
 * As of now <b>PoDoFo</b> is only available for Unix platforms. But the code is
 * portable C++ and a Windows version will be available soon.
 *
 * More information can be found at: http://podofo.sourceforge.net
 *
 * <b>PoDoFo</b> is created by Dominik Seichter <domseichter@web.de>
 */

#endif // _PDF_DEFINES_H_
