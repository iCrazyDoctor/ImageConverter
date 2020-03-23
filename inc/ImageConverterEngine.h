/*
 * Copyright (c) 2008-2010 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *    ========= CYBERTEC =============
 */

#ifndef __IMAGECONVERTER_H__
#define __IMAGECONVERTER_H__

#include <e32std.h>
#include <e32base.h>
#include <f32file.h>
#include <ImageConversion.h>
#include <aknglobalnote.h>

class CFbsBitmap;
class CBitmapRotator;
class CBitmapScaler;

class MConverterController
    {
    public:
        virtual void NotifyCompletion( TInt aErr, const TDesC& aMsg ) = 0;
        virtual TBool IsAnimating() = 0;
    };

// states for this engine
enum TState 
    {
    EIdle = 0,
    EDecoding,
    EEncoding,
    ERotating,
    EScaling
    };

class CImageConverterEngine : public CActive
    {

    public:
        // rotation directions
        enum TDirection 
            {
            EClockwise90 = 0,
            EAntiClockwise90 
            };

        public: // contructors/destructors

        /*
        * NewL
        *  
        * Create a CImageConverterEngine object and return a pointer to it.
        *
        * Params: 
        *       aController Pointer to a MConverterController interface.
        *      The engine uses NotifyCompletion callback from this interface
        *      to notify the controller about completions of coding or 
        *      encoding requests.
        *        
        * Returns:
        *       A pointer to the created engine
        *
        */  
        static CImageConverterEngine* NewL( MConverterController* aController );
    
        ~CImageConverterEngine();
        
    public: // interface methods

        /*
        * StartToDecodeL
        *  
        * Starts to decode an image from a file. When completed calls 
        * NotifyCompletion, from iController.
        *
        * Params: 
        *       aFileName Full path and filename of the image to be decoded.
        *        
        * Returns:
        *       Nothing
        */
        void StartToDecodeL( TFileName& aFileName );

        /*
        * StartToEncodeL
        *  
        * Starts to encode an image to a file. When completed calls 
        * NotifyCompletion, from iController.
        *
        * Params: 
        *       aFileName Full path and filename to the image to be encoded.
        *        
        * Returns:
        *       Nothing
        */
        void StartToEncodeL( TFileName& aFileName, 
            TUid aImageType, TUid aImageSubType );

        /*
        * GetImageInfoL
        *  
        * Gets frame 0 info strings. An image has to be decoded before 
        * call to this method, otherwise will return NULL.
        * 
        * Params: 
        *       None
        *        
        * Returns:
        *       CFrameInfoStrings* See Symbian OS documentation for 
        *       reference.
        */
        CFrameInfoStrings* GetImageInfoL();

        /*
        * GetEncoderImageTypesL
        *  
        * Gets descriptions of supported (encoding) image types. 
        *
        * Params: 
        *       aImageTypeArray Reference to an array to be filled.
        *       See Symbian OS documentation for reference.
        *
        * Returns:
        *       Nothing 
        */
        static void GetEncoderImageTypesL( 
            RImageTypeDescriptionArray& aImageTypeArray );
    
        /*
        * Rotate
        *  
        * Rotates the image to the given direction.
        *
        * Params: 
        *       aDirection The direction to rotate to.
        *
        * Returns:
        *       Nothing 
        */
        void Rotate( TDirection aDirection );

        /*
        * Scale
        *  
        * Scales the image to the given percentage of the current size.
        *
        * Params: 
        *       aPercent The new size relative to the current size 
        *       (e.g. 110 will increase the size by 10 %).
        *
        * Returns:
        *       Nothing 
        */
        TInt Scale( TInt aPercent );
    
        /*
        * GetBitmap
        *  
        * Returns a pointer to the decoded image.
        *
        * Params: 
        *       None
        *
        * Returns:
        *       A pointer to the encoded image  
        */
        CFbsBitmap* GetBitmap();

        void ShowProgress();
        void CancelProgress();

        TBool CanDownScaleMore();
        
        inline TState EngineState(){return iState;};
        
    protected: // implementation of CActive
        void DoCancel();
        void RunL();
        TInt RunError(TInt aError);

    private: // internal methods
        CImageConverterEngine( MConverterController* aController ); 
        void ConstructL();
        TBool FitToScreen();
        void DoFitToScreen();


    private: // internal data
        /*
        * Access to the decoded image,
        * images are also encoded from this bitmap.
        */
        CFbsBitmap*             iBitmap; // decoded image
        MConverterController*   iController; // ui controller
        RFs                     iFs; // for opening/saving images from/to files
        CImageDecoder*          iImageDecoder; // decoder from ICL API
        CImageEncoder*          iImageEncoder; // encoder from ICL API
        CBitmapRotator*         iRotator;
        CBitmapScaler*          iScaler;
        TState                  iState;
        CAknGlobalNote*         iGlobalNote;
        TInt                    iNoteId;
        TFileName               iFilename;
        TInt                    iScaleDownCounter;
    };

#endif // #ifndef __IMAGECONVERTER_H__
