/***************************************************************************
 *   Copyright (C) 2007 by Pierre Marchand   *
 *   pierre@moulindetouvois.com   *
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
#include "pdftranslator.h"
// #include "charpainter.h"

#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <istream>
#include <ostream>
using std::ostringstream;
using std::map;
using std::vector;
using std::string;
using std::ifstream;
using std::istream;
using std::ostream;
using std::endl;
using std::runtime_error;

#include <iostream> //XXX

namespace PoDoFoImpose
{
	static std::map<std::string, std::string> vars;
}

PageRecord::PageRecord ( int s,int d,double r, double tx, double ty )
		: sourcePage ( s ),
		destPage ( d ),
		rotate ( r ),
		transX ( tx ),
		transY ( ty )
{
};

PageRecord::PageRecord ( )
		: sourcePage ( 0 ),
		destPage ( 0 ),
		rotate ( 0 ),
		transX ( 0 ),
		transY ( 0 )
{};

void PageRecord::load ( const std::string& buffer )
{
	int blen ( buffer.length() );
	std::vector<std::string> tokens;
	std::string ts;
	for ( int i ( 0 ); i < blen; ++i )
	{
		char ci ( buffer.at ( i ) );
		if ( ci == ' ' )
			continue;
		else if ( ci == ';' )
		{
			tokens.push_back ( ts );
			ts.clear();
			continue;
		}
		ts += ci;
	}

	if ( tokens.size() != 5 )
	{
		sourcePage = destPage = 0; // will return false for isValid()
		std::cerr<<"INVALID_RECORD("<< tokens.size() <<") "<<buffer<<std::endl;
		for(uint i(0);i<tokens.size();++i)
			std::cerr<<"\t+ "<<tokens.at(i)<<std::endl;
	}

	sourcePage	= calc ( tokens.at ( 0 ) );
	destPage	= calc ( tokens.at ( 1 ) );
	rotate	= calc ( tokens.at ( 2 ) );
	transX	= calc ( tokens.at ( 3 ) );
	transY	= calc ( tokens.at ( 4 ) );
		
	std::cerr<<" "<<sourcePage<<" "<<destPage<<" "<<rotate<<" "<<transX<<" "<<transY <<std::endl;

}

double PageRecord::calc ( const std::string& s )
{
// 	std::cerr<<"C1 "<< s<<std::endl;
	std::vector<std::string> tokens;
	int tlen ( s.length() );
	std::string ts;
	std::map<std::string, std::string>::iterator vit;
	for ( int i ( 0 ); i < tlen; ++i )
	{
		char ci ( s.at ( i ) );
		if ( ci == 0x20 || ci == 0x9 )// skip spaces and horizontal tabs
			continue;
		else if ( ( ci == '+' ) || ( ci == '-' ) || ( ci == '*' ) || ( ci == '/' ) || ( ci == '(' ) || ( ci == ')' ) )
		{
			// commit current string
			if(ts.length() > 0)
			{
				vit = PoDoFoImpose::vars.find ( ts );
				if ( vit != PoDoFoImpose::vars.end() )
				{
// 					std::cerr<<"Found "<<ts<<std::endl; 
					tokens.push_back ( vit->second );
				}
				else
				{
// 					std::cerr<<"Not Found "<<ts<<std::endl;
					tokens.push_back ( ts );
				}
			}
			ts.clear();
			// append operator
			ts += ci;
			tokens.push_back ( ts );
			ts.clear();
		}
		else
			ts += ci;
	}
	if(ts.length() > 0)
	{
		vit = PoDoFoImpose::vars.find ( ts );
		if ( vit != PoDoFoImpose::vars.end() )
		{
// 			std::cerr<<"Found "<<ts<<std::endl; 
			tokens.push_back ( vit->second );
		}
		else
		{
// 			std::cerr<<"Not Found "<<ts<<std::endl;
			tokens.push_back ( ts );
		}
	}

	return calc ( tokens );

}

double PageRecord::calc ( const std::vector<std::string>& t )
{
// 	std::cerr<<"C2"<<std::endl;
// 	for(uint i(0);i<t.size();++i)
// 		std::cerr<<"+ \""<< t.at(i) <<"\""<<std::endl;
		
		
	if ( t.size() == 0 )
		return 0.0;

	double ret ( 0.0 );
	
	std::vector<double> values;
	std::vector<std::string> ops;
	
		
	for ( uint vi ( 0 ); vi < t.size(); ++vi )
	{
		if ( t.at ( vi ) == "(" )
		{
			std::vector<std::string> tokens;
			int cdeep ( 0 );
// 			std::cerr<<"(";
			for ( ++vi ; vi < t.size(); ++vi )
			{
// 				std::cerr<<t.at ( ti );
				if ( t.at ( vi ) == ")" )
				{
					if ( cdeep == 0 )
						break;
					else
					{
						--cdeep;
					}
				}
				else if ( t.at ( vi ) == "(" )
				{
					++cdeep;
				}
	
				tokens.push_back ( t.at ( vi ) );
			}
// 			std::cerr<<std::endl;
			values.push_back( calc ( tokens ) );
		}		
		else if ( t.at ( vi ) == "+" )
			ops.push_back( "+" );
		else if ( t.at ( vi ) == "-" )
			ops.push_back( "-" );
		else if ( t.at ( vi ) == "*" )
			ops.push_back( "*" );
		else if ( t.at ( vi ) == "/" )
			ops.push_back( "/" );
		else
			values.push_back( std::atof ( t.at ( vi ).c_str() ));
	}
	
	if(values.size() == 1)
		ret = 	values.at(0);
	else
	{
		for(uint vi(0); (vi + 1) < values.size(); ++vi)
		{
			if ( ops.at ( vi ) == "+" )
				ret += values.at(vi) + values.at(vi + 1);
			else if ( ops.at ( vi ) == "-" )
				ret += values.at(vi) - values.at(vi + 1);
			else if ( ops.at ( vi ) == "*" )
				ret += values.at(vi) * values.at(vi + 1);
			else if ( ops.at ( vi ) == "/" )
				ret += values.at(vi) / values.at(vi + 1);
		}
	}
// 	std::cerr<<"R "<<ret<<std::endl;
	return ret;
}

bool PageRecord::isValid() const
{
	//TODO
	if ( !sourcePage || !destPage )
		return false;
	return true;
}

bool PdfTranslator::checkIsPDF ( std::string path )
{
	ifstream in ( path.c_str(), ifstream::in );
	if ( !in.good() )
		throw runtime_error ( "setSource() failed to open input file" );

	const int magicBufferLen = 5;
	char magicBuffer[magicBufferLen ];
	in.read ( magicBuffer, magicBufferLen );
	std::string magic ( magicBuffer , magicBufferLen );

	in.close();
	if ( magic.find ( "%PDF" ) < 5 )
		return true;

	return false;
}

PdfTranslator::PdfTranslator ( )
{
	sourceDoc = 0;
	targetDoc = 0;
	extraSpace = 0;
}

void PdfTranslator::setSource ( const std::string & source )
{

	if ( checkIsPDF ( source ) )
	{
// 		std::cerr << "Appending "<<source<<" to source" << endl;
		multiSource.push_back ( source );
	}
	else
	{

		ifstream in ( source.c_str(), ifstream::in );
		if ( !in.good() )
			throw runtime_error ( "setSource() failed to open input file" );


		char *filenameBuffer = new char[1000];
		do
		{
			in.getline ( filenameBuffer, 1000 );
			std::string ts ( filenameBuffer, in.gcount() );
			if ( ts.size() > 4 ) // at least ".pdf" because just test if ts is empty doesn't work.
			{
				multiSource.push_back ( ts );
				std::cerr << "Appending "<< ts <<" to source" << endl;
			}
		}
		while ( !in.eof() );
		in.close();
		delete filenameBuffer;
	}

	for ( std::vector<std::string>::const_iterator ms = multiSource.begin(); ms != multiSource.end(); ++ms )
	{
		if ( ms == multiSource.begin() )
		{
			std::cerr << "First doc is "<< *ms  << endl;
			sourceDoc = new PdfMemDocument ( ( *ms ).c_str() );
		}
		else
		{
			PdfMemDocument mdoc ( ( *ms ).c_str() );
			std::cerr << "Appending "<< mdoc.GetPageCount() << " page(s) of " << *ms  << endl;

			sourceDoc->InsertPages ( mdoc, 0, mdoc.GetPageCount() );
		}
	}
}

void PdfTranslator::addToSource ( const std::string & source )
{
	if ( !sourceDoc )
		return;

	PdfMemDocument extraDoc ( source.c_str() );
	sourceDoc->InsertPages ( extraDoc, 0,  extraDoc.GetPageCount() );
	multiSource.push_back ( source );

}

// When getting resources along pages tree, it would be bad to overwrite an XObject dictionnary.
void PdfTranslator::mergeResKey ( PdfObject *base,PdfName key, PdfObject *tomerge )
{
	if ( key == PdfName ( "ProcSet" ) )
		return;

	PdfObject * kbase = base->GetDictionary().GetKey ( key );
	if ( kbase->IsReference() && tomerge->IsReference() )
	{
		if ( kbase->GetReference() != tomerge->GetReference() )
		{
			PdfObject *kbaseO = targetDoc->GetObjects().GetObject ( kbase->GetReference() );
			PdfObject *tomergeO = targetDoc->GetObjects().GetObject ( tomerge->GetReference() ) ;
			TKeyMap tomergemap = tomergeO->GetDictionary().GetKeys();
			TCIKeyMap itres;
			for ( itres = tomergemap.begin(); itres != tomergemap.end(); ++itres )
			{
				if ( !kbaseO->GetDictionary().HasKey ( ( *itres ).first ) )
				{
					kbaseO->GetDictionary().AddKey ( ( *itres ).first, ( *itres ).second );
				}
			}
		}

	}
	else if ( kbase->IsDictionary() && tomerge->IsDictionary() )
	{
		TKeyMap tomergemap = tomerge->GetDictionary().GetKeys();
		TCIKeyMap itres;
		for ( itres = tomergemap.begin(); itres != tomergemap.end(); ++itres )
		{
			if ( !kbase->GetDictionary().HasKey ( ( *itres ).first ) )
			{
				kbase->GetDictionary().AddKey ( ( *itres ).first, ( *itres ).second );
			}
		}
	}
}

PdfObject* PdfTranslator::getInheritedResources ( PdfPage* page )
{
	PdfObject *res = new PdfObject;
	PdfObject *rparent = page->GetObject();
	while ( rparent && rparent->IsDictionary() )
	{
		PdfObject *curRes = rparent->GetDictionary().GetKey ( PdfName ( "Resources" ) );
		if ( curRes )
		{
			if ( curRes->IsDictionary() )
			{
				TKeyMap resmap = curRes->GetDictionary().GetKeys();
				TCIKeyMap itres;
				for ( itres = resmap.begin(); itres != resmap.end(); ++itres )
				{
					if ( res->GetDictionary().HasKey ( ( *itres ).first ) )
					{
						mergeResKey ( res, ( *itres ).first , ( *itres ).second );
					}
					else
					{
						res->GetDictionary().AddKey ( ( *itres ).first, ( *itres ).second );
					}
				}
			}
			else if ( curRes->IsReference() )
			{
				curRes = targetDoc->GetObjects().GetObject ( curRes->GetReference() );
				TKeyMap resmap = curRes->GetDictionary().GetKeys();
				TCIKeyMap itres;
				for ( itres = resmap.begin(); itres != resmap.end(); ++itres )
				{
					if ( res->GetDictionary().HasKey ( ( *itres ).first ) )
					{
						mergeResKey ( res, ( *itres ).first , ( *itres ).second );
					}
					else
					{
						res->GetDictionary().AddKey ( ( *itres ).first, ( *itres ).second );
					}
				}
			}
		}
		rparent = rparent->GetIndirectKey ( "Parent" );
	}
	return res;

}

void PdfTranslator::setTarget ( const std::string & target )
{
	if ( !sourceDoc )
		throw std::logic_error ( "setTarget() called before setSource()" );

	// DOCUMENT: Setting `targetDoc' to the input path will be confusing when reading the code.
	// I guess, but appending new content to a duplicated source doc rather than rebuild a brand new PDF file is far more easy.
	// But it seems we don't need to duplicate & can do all job on source doc ! I try it now. (pm)
	targetDoc = sourceDoc;
	outFilePath  = target;
	pcount = targetDoc->GetPageCount();
	std::cerr << "Document has "<< pcount << " page(s) " << endl;
	for ( int i = 0; i < pcount ; ++i )
	{
		PdfPage * page = targetDoc->GetPage ( i );
		PdfMemoryOutputStream outMemStream ( 1 );

		PdfXObject *xobj = new PdfXObject ( page->GetMediaBox(), targetDoc );
		if ( page->GetContents()->HasStream() )
		{
			page->GetContents()->GetStream()->GetFilteredCopy ( &outMemStream );
		}
		else if ( page->GetContents()->IsArray() )
		{
			PdfArray carray ( page->GetContents()->GetArray() );
			for ( unsigned int ci = 0; ci < carray.GetSize(); ++ci )
			{
// 			std::cerr << "carray at "<< ci <<" is "<< carray[ci].GetDataTypeString () <<endl;
				if ( carray[ci].HasStream() )
				{
					carray[ci].GetStream()->GetFilteredCopy ( &outMemStream );
				}
				else if ( carray[ci].IsReference() )
				{
// 				std::cerr << "carray at "<< ci <<" is "<< carray[ci].GetReference().ToString() << endl;

					PdfObject *co = targetDoc->GetObjects().GetObject ( carray[ci].GetReference() );

					while ( co != NULL )
					{

						if ( co->IsReference() )
						{
							co = targetDoc->GetObjects().GetObject ( co->GetReference() );
						}
						else if ( co->HasStream() )
						{
							co->GetStream()->GetFilteredCopy ( &outMemStream );
							break;
						}
					}

				}

			}
		}

		outMemStream.Close();

		PdfMemoryInputStream inStream ( outMemStream.TakeBuffer(),outMemStream.GetLength() );
		xobj->GetContents()->GetStream()->Set ( &inStream );

		resources[i+1] = getInheritedResources ( page );
		xobjects[i+1] = xobj;
		trimRect[i+1] = page->GetTrimBox();
		bleedRect[i+1] = page->GetBleedBox();

		std::cerr << "XObject added("<< i + 1 << ")" << endl;
	}
}

void PdfTranslator::loadPlan ( const std::string & plan )
{
	ifstream in ( plan.c_str(), ifstream::in );
	if ( !in.good() )
		throw runtime_error ( "Failed to open plan file" );

	int dup = 40000; // So, we can't process a file that have more than 40000 pages, feel free to increase it if you need.
	std::string line;
	char cbuffer[1024];
	do
	{
		in.getline ( cbuffer,1024 );
		int blen ( in.gcount() );
		std::string buffer ( cbuffer, blen );
		std::cerr<< blen <<" \""<< buffer <<"\""<<std::endl;
		if ( blen < 2 ) // Nothing
			continue;
		if ( buffer.at ( 0 ) == '#' ) // Comment
			continue;
		if ( buffer.at ( 0 ) == '$' ) // Variable
		{
			int sepPos ( buffer.find_first_of ( '=' ) );
			std::string key(buffer.substr ( 0,sepPos ));
			std::string value(buffer.substr ( sepPos + 1 ));
			
			if ( PoDoFoImpose::vars.find ( key ) != PoDoFoImpose::vars.end() )
			{
				PoDoFoImpose::vars[key] = value;
			}
			else
			{
				PoDoFoImpose::vars.insert ( std::pair<std::string, std::string> ( key,value ) );
			}
			std::cerr<< sepPos << " "<<key << " = " << PoDoFoImpose::vars[key] <<std::endl;
		}
		else // Record? We hope!
		{
// 			std::cerr<<"R "<<buffer<<std::endl;
			PageRecord p;
			p.load ( buffer ) ;
			if(!p.isValid())
				continue;
			maxPageDest = std::max ( maxPageDest, p.destPage );
			if ( pagesIndex.find ( p.sourcePage ) != pagesIndex.end() )
			{
				//qDebug() << "duplicate "<< p.sourcePage << " in " << dup;

				PdfXObject *xobj = new PdfXObject ( targetDoc->GetPage ( p.sourcePage - 1 )->GetMediaBox(), targetDoc );
				PdfMemoryOutputStream outMemStream ( 1 );
				xobjects[p.sourcePage]->GetContents()->GetStream()->GetFilteredCopy ( &outMemStream );
				outMemStream.Close();
				PdfMemoryInputStream inStream ( outMemStream.TakeBuffer(),outMemStream.GetLength() );
				xobj->GetContents()->GetStream()->Set ( &inStream );

				xobjects[dup] = xobj;
				resources[dup] = getInheritedResources ( targetDoc->GetPage ( p.sourcePage - 1 ) );
				trimRect[dup] = targetDoc->GetPage ( p.sourcePage - 1 )->GetTrimBox();
				bleedRect[dup] = targetDoc->GetPage ( p.sourcePage - 1 )->GetBleedBox();

				std::cerr << "Page "<< p.sourcePage << ": MediaBox" << targetDoc->GetPage ( p.sourcePage - 1 )->GetMediaBox().ToString() <<endl;
				std::cerr << "Page "<< p.sourcePage << ": TrimBox" << trimRect[dup].ToString() <<endl;


				p.sourcePage = dup;
				++dup;
			}
			planImposition.push_back ( p );
			pagesIndex[p.sourcePage] = planImposition.size() - 1;
// 			std::cerr <<"record "<< planImposition.size() <<endl;
		}
		
	}
	while ( !in.eof() );
	
	if ( PoDoFoImpose::vars.find("$PageWidth") == PoDoFoImpose::vars.end() )
		throw runtime_error ( "PageWidth not specified" );
	if ( PoDoFoImpose::vars.find("$PageHeight") == PoDoFoImpose::vars.end() )
		throw runtime_error ( "Pageheight not specified" );
	
	destWidth = atof( PoDoFoImpose::vars["$PageWidth"].c_str() );
	destHeight = atof( PoDoFoImpose::vars["$PageHeight"].c_str() );
	
	std::cerr <<"Plan completed "<< planImposition.size() <<endl;
	
}

//returns the number of processed pages.
int PdfTranslator::pageRange ( int plan, int sheet, int pagesInBooklet, int numBooklet )
{
	double pw = sourceDoc->GetPage ( 0 )->GetMediaBox().GetWidth();
	double ph = sourceDoc->GetPage ( 0 )->GetMediaBox().GetHeight();
	if ( plan == 4 ) // For now, it is the only "well known" plan ;-)
	{
		destWidth = pw * 2;
		destHeight = ph * 2;
		int firstpage = ( plan * ( sheet - 1 ) ) + 1 + ( ( numBooklet - 1 ) * pagesInBooklet );
		int lastpage = ( numBooklet * pagesInBooklet ) - ( ( sheet-1 ) * plan );
		{
			PageRecord p;
			//recto
			p.sourcePage = firstpage;
			p.destPage = sheet * 2 - 1;
			p.rotate = 0;
			p.transX = 1.0 * pw;
			p.transY = 0.0;
			planImposition.push_back ( p );

			p.sourcePage = firstpage + 3;
			p.destPage = sheet * 2- 1;
			p.rotate = 180.0;
			p.transX = 2.0 * pw;
			p.transY = 2.0 * ph;
			planImposition.push_back ( p );

			p.sourcePage = lastpage - 3;
			p.destPage = sheet * 2- 1;
			p.rotate = 180.0;
			p.transX = 1.0 * pw;
			p.transY = 2.0 * ph;
			planImposition.push_back ( p );

			p.sourcePage = lastpage;
			p.destPage = sheet * 2- 1;
			p.rotate = 0.0;
			p.transX = 0.0;
			p.transY = 0.0;
			planImposition.push_back ( p );

			//verso
			p.sourcePage = firstpage + 1;
			p.destPage = sheet * 2 ;
			p.rotate = 0;
			p.transX = 0.0 ;
			p.transY = 0.0;
			planImposition.push_back ( p );

			p.sourcePage = firstpage + 2;
			p.destPage = sheet * 2 ;
			p.rotate = 180.0;
			p.transX = 1.0 * pw;
			p.transY = 2.0 * ph;
			planImposition.push_back ( p );

			p.sourcePage = lastpage - 2;
			p.destPage = sheet * 2;
			p.rotate = 180.0;
			p.transX = 2.0 * pw;
			p.transY = 2.0 * ph;
			planImposition.push_back ( p );

			p.sourcePage = lastpage - 1;
			p.destPage = sheet * 2 ;
			p.rotate = 0.0;
			p.transX = 1.0 * pw;
			p.transY = 0.0;
			planImposition.push_back ( p );

			return 8;
		}
	}

	// Fix a gcc warning,
	// this return should never be reached though
	return 0;
}

void PdfTranslator::computePlan ( int wellKnownPlan, int sheetsPerBooklet )
	{
	std::cerr << " computePlan(" <<  wellKnownPlan << ", "<<sheetsPerBooklet <<")";

// 	if(wellKnownPlan < 2 || sheetsPerBooklet < 1 || !sourceDoc);
// 	{
// 		return;
// 	}

	int groupSize = wellKnownPlan * 2;
	int pagesPerBooklet = groupSize * sheetsPerBooklet;
//gcc says I don't use it, it must be right	int bookletCount = sourceDoc->GetPageCount() / pagesPerBooklet;
//### 	have to deal with padding
	int processedPages = 0;
	int numBooklet = 1;
	int nextBooklet = pagesPerBooklet;
	int s = 1;
	int p = 0;
	while ( processedPages < sourceDoc->GetPageCount() )
	{
		p = pageRange ( wellKnownPlan, s, pagesPerBooklet, numBooklet );
		std::cerr << p << " pages processed";
		++s;
		processedPages += p;
		if ( processedPages > nextBooklet )
		{
			nextBooklet += pagesPerBooklet;
			++numBooklet;
		}
	}

	//planImposition is filed, no duplicated pages here.
	for ( unsigned int i = 0; i < planImposition.size(); ++i )
	{
		pagesIndex[planImposition[i].sourcePage] = i ;
	}
}

void PdfTranslator::impose()
{
	if ( ! ( sourceDoc && targetDoc ) )
		throw std::invalid_argument ( "impose() called with empty source or destination path" );



	PdfObject trimbox;
	PdfRect trim ( 0, 0, destWidth, destHeight );
	trim.ToVariant ( trimbox );

	typedef map<int, vector<int> > groups_t;
	groups_t groups;
	for ( unsigned int i = 0; i < planImposition.size(); ++i )
	{
		groups[planImposition[i].destPage].push_back ( planImposition[i].sourcePage );
	}

	groups_t::const_iterator  git = groups.begin();
	const groups_t::const_iterator gitEnd = groups.end();
	while ( git != gitEnd )
	{
		PdfPage * newpage = targetDoc->CreatePage ( PdfRect ( 0.0, 0.0, destWidth, destHeight ) );
		newpage->GetObject()->GetDictionary().AddKey ( PdfName ( "TrimBox" ), trimbox );
		PdfDictionary xdict;

		ostringstream buffer;
		for ( unsigned int i = 0; i < ( *git ).second.size(); ++i )
		{

			int curPage = ( *git ).second[i];

			int index = pagesIndex[curPage];
			PdfRect rect = trimRect[curPage];

			std::cerr<< " "<<planImposition[index].sourcePage
					<<" "<<planImposition[index].destPage
					<<" "<<planImposition[index].rotate
					<<" "<<planImposition[index].transX
					<<" "<<planImposition[index].transY 
					<<std::endl;
			
			double cosR = cos ( planImposition[index].rotate  *  3.14159 / 180.0 );
			double sinR = sin ( planImposition[index].rotate  *  3.14159 / 180.0 );
			double tx = planImposition[index].transX ;
			double ty = planImposition[index].transY ;

			PdfXObject *xo = xobjects[curPage];
			ostringstream op;
			op << "OriginalPage" << curPage;
			xdict.AddKey ( PdfName ( op.str() ) , xo->GetObjectReference() );

			if ( resources[curPage] )
			{
				TKeyMap resmap = resources[curPage]->GetDictionary().GetKeys();
				TCIKeyMap itres;
				for ( itres = resmap.begin(); itres != resmap.end(); ++itres )
				{
					xo->GetResources()->GetDictionary().AddKey ( ( *itres ).first, ( *itres ).second );
				}
			}
			
			
			buffer << "q\n";
			buffer << std::fixed << cosR <<" "<< sinR<<" "<<-sinR<<" "<< cosR<<" "<< tx <<" "<<  ty << " cm\n";
			buffer << "/OriginalPage" << curPage << " Do\n";
			buffer << "Q\n";
		}
		string bufStr = buffer.str();
		newpage->GetContentsForAppending()->GetStream()->Set ( bufStr.data(), bufStr.size() );
		newpage->GetResources()->GetDictionary().AddKey ( PdfName ( "XObject" ), xdict );
		++git;
	}
	try
	{
		targetDoc->DeletePages ( 0,pcount );
	}
	catch ( PoDoFo::PdfError & e )
	{
		e.PrintErrorMsg();
		std::cerr <<"For the above reason, PoDoFo didn't delete original pages,\nas we can live with that, process continues"<<endl;
	}

	targetDoc->Write ( outFilePath.c_str() );

}

// void PdfTranslator::drawLine ( double x, double y, double xx, double yy, ostringstream & s )
// {
// 	//TODO: ensure safe double formatting for PDF
// 	s << "q\n"
// 	"0.5 w\n"
// 	"1 0 0 1 0 0 cm\n"
// 	<< x << ' ' << y << " m\n"
// 	<< xx << ' ' << yy << " l\n"
// 	"S\n"
// 	"Q\n";
// }
// 
// void PdfTranslator::signature ( double x, double y, int sheet, const vector< int > & pages, ostringstream & s )
// {
// 	CharPainter painter;
// 	s << "q\n"
// 	"1 0 0 1 0 0 cm\n";
// 	painter.multipaint ( s, sheet, extraSpace / 2.0 , x  , y );
// 	// 	for(int i = 0;i < pages.count(); ++i)
// 	// 	{
// 	// 		s << painter.multipaint(pages[i], extraSpace /3.0 , x + extraSpace + (i * extraSpace / 2.0),y );
// 	// 	}
// 	//
// 	s << "Q\n";
// 
// }
// 


