/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ============== CYBERTEC ==============
 */

#ifndef IMAGECONVERTERDOCUMENT_H
#define IMAGECONVERTERDOCUMENT_H

// INCLUDES
#include <akndoc.h>
   
// CONSTANTS

// FORWARD DECLARATIONS
class  CEikAppUi;

// CLASS DECLARATION

/**
*  CImageConverterDocument application class.
*/
class CImageConverterDocument : public CAknDocument
    {
    public: // Constructors and destructor
        /**
        * Two-phased constructor.
        */
        static CImageConverterDocument* NewL(CEikApplication& aApp);

        /**
        * Destructor.
        */
        virtual ~CImageConverterDocument();

    public: // New functions

    public: // Functions from base classes
    protected:  // New functions

    protected:  // Functions from base classes

    private:

        /**
        * EPOC default constructor.
        */
        CImageConverterDocument(CEikApplication& aApp);
        void ConstructL();

    private:

        /**
        * From CEikDocument, create CImageConverterAppUi "App UI" object.
        */
        CEikAppUi* CreateAppUiL();
    };

#endif

// End of File

