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
#ifndef PDFTRANSLATOR_H
#define PDFTRANSLATOR_H

#include "podofo.h"

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <istream>



using namespace PoDoFo;

/**
  @author Pierre Marchand <pierre@moulindetouvois.com>
  */
class PageRecord
{
	public:
		PageRecord ( int s,int d,double r, double tx, double ty  );
		PageRecord( );
		~PageRecord() {};
		int sourcePage;
		int xobjIndex;
		int destPage;
		double rotate;
		double transX;
		double transY;
		bool isValid() const;
		double calc ( const std::string& s );
		double calc ( const std::vector<std::string>& t );
		void load ( const std::string& s );
};


/**
PdfTranslator create a new PDF file which is the imposed version, following the imposition
plan provided by the user, of the source PDF file.
Pdftranslate does not really create a new PDF doc, it rather works on source doc, getting all page contents
as XObjects and put these XObjects on new pages. At the end, it removes original pages from the doc, but since
PoDoFo keeps them --- just removing from the pages tree ---, if it happens that you have a lot of content
in content stream rather than in resources, you'll get a huge file.
Usage is something like :
p = new PdfTranslator;
p->setSource("mydoc.pdf");
p->setTarget("myimposeddoc.pdf");
p->loadPlan("in4-32p.plan");
p->impose();
p->mailItToMyPrinterShop("job@proprint.com");//Would be great, doesn't it ?
*/
class PdfTranslator
{
	public:
		/**
		Constructor takes just  1 arg, the sheet margin expressed in point.
		*/
		PdfTranslator();

		~PdfTranslator() { }

		PdfMemDocument *sourceDoc;
		PdfMemDocument *targetDoc;

		/**
		Set the source document(s) to be imposed.
		Argument source is the path of the PDF file, or the path of a file containing a list of paths of PDF files...
		*/
		void setSource ( const std::string & source );

		/**
		Another way to set many files as source document.
		Note that a source must be set before you call addToSource().
		*/
		void addToSource ( const std::string & source );

		/**
		Set the path of the file where the imposed PDF doc will be save.
		*/
		void setTarget ( const std::string & target );

		/**
		Load an imposition plan file of form:
		widthOfSheet heightOfSheet
		sourcePage destPage rotation translationX translationY
		...        ...      ...      ...          ...
		*/
		void loadPlan ( const std::string & plan );

		/**
		An experimental method (don't use it) intended to perform basic imposition such In-n.
		Well known plans are : in-folio = 2; in-quatro = 4; etc. But for now, there is just in-4 (feel free to write support for more plans!).
		Later there will be a way to know what plans are available.
		sheets per booklet is sheets per booklet
		*/
		void computePlan ( int wellKnownPlan, int sheetsPerBooklet );

		/**
		When all is prepared, call it to do the job.
		*/
		void impose();

	private:
		std::string inFilePath;
		std::string outFilePath;
		int pcount;

		PdfReference globalResRef;

		std::vector<PageRecord> planImposition;
		std::map<int, int> pagesIndex;
		std::map<int, PdfXObject*> xobjects;
		std::map<int,PdfObject*> resources;
		std::map<int, PdfRect> trimRect;
		std::map<int,PdfRect> bleedRect;
		std::map<int, PdfDictionary*> pDict;
		std::map<int, int> virtualMap;
		double destWidth;
		double destHeight;
		double scaleFactor;
		int maxPageDest;

		bool checkIsPDF ( std::string path );
		PdfObject* getInheritedResources ( PdfPage* page );
		void mergeResKey ( PdfObject *base, PdfName key,  PdfObject *tomerge );
		PdfObject* migrateResource(PdfObject * obj);
		void drawLine ( double x, double y, double xx, double yy, std::ostringstream & a );
		void signature ( double x , double y, int sheet, const std::vector<int> & pages, std::ostringstream & a );
		int pageRange ( int plan, int sheet , int pagesInBooklet, int numBooklet ); // much more a macro !

		std::string useFont;
		PdfReference useFontRef;
		double extraSpace;

		std::vector<std::string> multiSource;
		
		std::map<std::string, PdfObject*> migrateMap;


};

#endif
