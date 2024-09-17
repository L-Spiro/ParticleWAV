#pragma once

#include <cstdint>
#include <format>
#include <iostream>
#include <string>
#include <vector>


namespace pw {

	// == Enumerations.
	/** Errors. */
	enum PW_ERRORS : int16_t {
		PW_E_SUCCESS													= 0,		                                                    /**< No problem. */
		PW_E_OUTOFMEMORY												= -1,							                                /**< Out of memory. */
		PW_E_FILENOTFOUND												= -2,						                                	/**< File was not found at the given path. */
		PW_E_INVALIDWRITEPERMISSIONS									= -3,						                                	/**< Unable to write to a file. */
		PW_E_NODISKSPACE												= -4,						                                	/**< Unable to write to a file (disk space). */
		PW_E_INVALIDFILETYPE											= -5,						                                	/**< File exists but is not in the expected format. */
		PW_E_INVALIDCALL												= -6,							                                /**< Invalid call. */
		PW_E_INVALIDDATA												= -7,							                                /**< Invalid input data. */
		PW_E_INTERNALERROR												= -8,							                                /**< Internal error. */
		PW_E_FEATURENOTSUPPORTED										= -9,							                                /**< Feature not yet supported. */
		PW_E_PARTIALFAILURE												= -10,							                                /**< Within multiple tasks, one or more failed. */
		PW_E_BADVERSION													= -11,							                                /**< Unsupported version (of a file etc.) */
		PW_E_FILEOVERFLOW												= -12,							                                /**< The file exceeded the maximum size supported by the system. */
		PW_E_FILEWRITEERROR												= -13,							                                /**< An error occurred while writing the file. */
		PW_E_BADFORMAT													= -14,							                                /**< Bad data format. */
		PW_E_UNSUPPORTEDSIZE											= -15,							                                /**< A value is too large for the type required by a given file format. */
	};


    // == Types.
    /** A WAV modifier. */
    typedef bool (*                                                     PfWavMod)( class CWavFile &_wfFile, struct PW_MODIFIER &_mModifiers, struct PW_OPTIONS &_oOptions );

    /** A modifier. */
    struct PW_MODIFIER {
        PfWavMod                                                        pfModifier = nullptr;                                           /**< The modification function. */
        uint32_t                                                        ui32Parm0 = 0;                                                  /**< Parm 0. */
        uintptr_t                                                       uiptrParm1 = 0;                                                 /**< Parm 1. */
        void *                                                          pvParm2 = nullptr;                                              /**< Parm 2. */
        double                                                          dParm3 = 0.0;                                                   /**< Parm 3. */
        std::vector<uint8_t>                                            vParm4;                                                         /**< Parm 4. */
        std::u16string                                                  sParm5;                                                         /**< Parm 5. */
    };

	/** Options. */
	struct PW_OPTIONS {
        std::vector<std::u16string>                                     vInputs;                                                        /**< Input files. */
        std::vector<std::u16string>                                     vOutputs;                                                       /**< Output files. */
        std::vector<PW_MODIFIER>                                        vFuncs;                                                         /**< The operations to perform on each file. */
        bool															bPause = false;													/**< If true, the program pauses before closing the command window. */
		bool															bShowTime = true;												/**< If true, the time taken to perform the conversion is printed. */
    };


	// == Functions.
	/**
     * Returns a string representing the given error code.
     * 
     * \param _eError The error code to print to a string.
     * \return Returns a string representing the given error code.
     **/
    std::u16string ErrorToString( PW_ERRORS _eError ) {
        switch ( _eError ) {
            case PW_E_OUTOFMEMORY : {
                return std::u16string( u"Out of memory." );
            }
		    case PW_E_FILENOTFOUND : {
                return std::u16string( u"File not found." );
            }
		    case PW_E_INVALIDWRITEPERMISSIONS : {
                return std::u16string( u"Invalid write permissions." );
            }
		    case PW_E_NODISKSPACE : {
                return std::u16string( u"Not enough disk space for file write operation." );
            }
		    case PW_E_INVALIDFILETYPE : {
                return std::u16string( u"File exists but is in an unexpected format." );
            }
		    case PW_E_INVALIDCALL : {
                return std::u16string( u"Invalid call." );
            }
		    case PW_E_INVALIDDATA : {
                return std::u16string( u"Invalid data." );
            }
		    case PW_E_INTERNALERROR : {
                return std::u16string( u"Internal error." );
            }
		    case PW_E_FEATURENOTSUPPORTED : {
                return std::u16string( u"Feature not yet supported." );
            }
		    case PW_E_PARTIALFAILURE : {
                return std::u16string( u"One or more tasks failed." );
            }
		    case PW_E_BADVERSION : {
                return std::u16string( u"Invalid version." );
            }
		    case PW_E_FILEOVERFLOW : {
                return std::u16string( u"File overflow." );
            }
		    case PW_E_FILEWRITEERROR : {
                return std::u16string( u"File write error." );
            }
            case PW_E_BADFORMAT : {
                return std::u16string( u"Bad data format." );
            }
            case PW_E_UNSUPPORTEDSIZE : {
                return std::u16string( u"A value is too large for the type required by a given file format." );
            }
        }
        return std::u16string();
    }

	/**
	 * Prints a given error code to the console.
	 * 
	 * \param _pcText Text to print.
	 * \param _eError The error code to print.
	 **/
	void PrintError( const char16_t * _pcText, PW_ERRORS _eError ) {
        if ( _eError != PW_E_SUCCESS ) {
            std::u16string sError;
            if ( _pcText ) {
                sError = _pcText;
                sError += u"\r\n";
            }
            sError += ErrorToString( _eError );
            std::wcout << reinterpret_cast<const wchar_t *>(sError.c_str()) << std::endl;
#ifdef PW_WINDOWS
            ::OutputDebugStringW( reinterpret_cast<const wchar_t *>((sError + u"\r\n").c_str()) );
#endif  // #ifdef PW_WINDOWS
        }
    }
}	// namespace pw
