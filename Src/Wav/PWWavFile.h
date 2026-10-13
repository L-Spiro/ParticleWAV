/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A WAV file.
 */


#pragma once

#include "../Files/PWStdFile.h"
#include "../Utilities/PWAlignmentAllocator.h"
#include "../Utilities/PWStreamBase.h"
#include "../Utilities/PWUtilities.h"

#include <cinttypes>
#include <condition_variable>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>



namespace pw {

	/**
	 * Class CWavFile
	 * \brief A WAV file.
	 *
	 * Description: A WAV file.
	 */
	class CWavFile {
	public :
		struct PW_STREAMING;
		CWavFile();
		~CWavFile();


		// == Enumerations.
		/** Formats. */
		enum PW_FORMAT : uint16_t {
			PW_F_PCM													= 0x1,
			PW_F_ADPCM													= 0x2,
			PW_F_IEEE_FLOAT												= 0x3,
			PW_F_ALAW													= 0x6,
			PW_F_MULAW													= 0x7,
			PW_F_DVI_ADPCM												= 0x11,
			PW_F_YAMAHA_ADPCM											= 0x16,
			PW_F_GSM_6_10												= 0x31,
			PW_F_ITU_G_721_ADPCM										= 0x40,
			PW_F_MPEG													= 0x50,
			PW_F_EXTENSIBLE												= 0xFFFE,
		};

		/** Loading options. */
		enum PW_LOAD_FLAGS : uint32_t {
			PW_LF_DATA													= (1 << 0),
			PW_LF_SMPL													= (1 << 1),
			PW_LF_LIST													= (1 << 2),
			PW_LF_ID3													= (1 << 3),
			PW_LF_INST													= (1 << 4),
		};

		/** Chunks. */
		enum PW_CHUNKS : uint32_t {
			PW_C_RIFF													= 0x46464952,
			PW_C_WAVE													= 0x45564157,
			PW_C_FMT_													= 0x20746D66,
			PW_C_DATA													= 0x61746164,
			PW_C_FACT													= 0x74636166,
			PW_C_SMPL													= 0x6C706D73,
			PW_C_LIST													= 0x5453494C,
			PW_C_ID3_													= 0x20336469,
			PW_C_INST													= 0x74736E69,
			PW_C_INFO													= 0x4F464E49,
			PW_C_LABL													= 0x6C62616C,
			PW_C_ADTL													= 0x6C746461,
			PW_C_DISP													= 0x50534944,			// "DISP".
		};

		/** Metadata. */
		enum PW_META {
			PW_M_INAM													= 0x4D414E49,			// Track name.
			PW_M_IPRD													= 0x44525049,			// Album title.
			PW_M_IART													= 0x54524149,			// Artist name.
			PW_M_ICMT													= 0x544D4349,			// Comments.
			PW_M_ICRD													= 0x44524349,			// Year.
			PW_M_IGNR													= 0x524E4749,			// Genre.
			PW_M_ITRK													= 0x4B525449,			// Track number.
			PW_M_IENG													= 0x474E4549,			// The engineer.
		};

		/** Start conditions. */
		enum PW_START_CONDITIONS {
			PW_SC_NONE,
			PW_SC_START_AT_SAMPLE,
			PW_SC_FIRST_NON_ZERO,
			PW_SC_ZERO_FOR_DURATION,
		};

		/** End conditions. */
		enum PW_END_CONDITIONS {
			PW_EC_NONE,
			PW_EC_END_AT_SAMPLE,
			PW_EC_ZERO_FOR_DURATION,
			PW_EC_DURATION,
		};

		/** Metadata formats. */
		enum PW_META_FORMAT {
			PW_MF_AUDACITY,
		};


		// == Types.
		typedef std::vector<double, CAlignmentAllocator<double, 64>>	lwtrack;
		typedef std::vector<lwtrack>									lwaudio;
		typedef bool (PW_STDCALL *										PfAddSampleFunc)( float, struct CWavFile::PW_STREAMING & );
		typedef void (PW_STDCALL *										PfBatchConvNWrite)( const std::vector<float> &, std::vector<uint8_t> &, struct CWavFile::PW_STREAMING & );
		typedef bool (PW_STDCALL *										PfAddMetaDataFunc)( void *, struct CWavFile::PW_STREAMING & );
		typedef void (PW_STDCALL *										PfMetaDataThreadFunc)( CWavFile *, void *, struct CWavFile::PW_STREAMING &, std::vector<uint8_t, CAlignmentAllocator<uint8_t, 64>> &, std::vector<uint8_t> & );
		

		/** The save data. */
		struct PW_SAVE_DATA {
			PW_FORMAT													fFormat = PW_F_PCM;
			uint32_t													uiHz = 0;					// Only overrides if not 0.
			uint16_t													uiBitsPerSample = 0;		// Only overrides if not 0.
			bool														bDither = false;

			PW_SAVE_DATA() :
				fFormat( PW_F_PCM ),
				uiHz( 0 ),
				uiBitsPerSample( 0 ) {}
			PW_SAVE_DATA( uint32_t _ui32Hz, uint16_t _ui16Bits ) :
				fFormat( PW_F_PCM ),
				uiHz( _ui32Hz ),
				uiBitsPerSample( _ui16Bits ) {}
		};

		/** Loop points. */
		struct PW_LOOP_POINT {
			uint32_t													uiCuePointID;
			uint32_t													uiType;
			uint32_t													uiStart;
			uint32_t													uiEnd;
			uint32_t													uiFraction;
			uint32_t													uiPlayCount;
		};

		/** Stream-to-file options. */
		struct PW_STREAM_TO_FILE_OPTIONS {
			std::wstring												wsPath;
			std::wstring												wsMetaPath;
			PW_FORMAT													fFormat = PW_F_IEEE_FLOAT;
			uint32_t													ui32Bits = 24;
			uint32_t													ui32Hz = 44100;			// Not set by the user.  Only used to pass information to the set-options functions.
			bool														bEnabled = false;
			bool														bDither = false;

			PW_START_CONDITIONS											scStartCondition = PW_SC_NONE;
			uint64_t													ui64StartParm = 0;
			double														dStartParm = 0.0;
			PW_END_CONDITIONS											seEndCondition = PW_EC_NONE;
			uint64_t													ui64EndParm = 0;
			double														dEndParm = 0.0;

			uint64_t													ui64MetaParm = 0;
			void *														pvMetaParm = nullptr;
			PfAddMetaDataFunc											pfMetaFunc = nullptr;
			PfMetaDataThreadFunc										pfMetaThreadFunc = nullptr;
			int32_t														i32MetaFormat = 0;
			bool														bMetaEnabled = false;
		};

		/** Stream-to-file condition parameters. */
		union PW_STREAM_COND_PARM {
			uint64_t													ui64Parm;
			double														dParm;
		};

		/** Condition data. */
		struct PW_CONDITIONS_DATA {
			uint64_t													ui64Counter = 0;
			uint64_t													ui64Parm0 = 0;
			float														fParm1 = 0.0f;
		};
		
		typedef bool (PW_STDCALL *										PfStartConditionFunc)( uint64_t, float, PW_CONDITIONS_DATA & );
		typedef bool (PW_STDCALL *										PfEndConditionFunc)( uint64_t, uint64_t, float, PW_CONDITIONS_DATA & );
		
		/** Chunk.  Not packed: it is never overlaid on file data, and a std::vector at a packed (misaligned) offset is undefined behavior. */
		struct PW_CHUNK {
			union {
				char8_t													cName[4];
				uint32_t												uiName;
			}															u;
			uint32_t													uiSize;
			union {
				char8_t													cName[4];
				uint32_t												uiFormat;
			}															u2;
			std::vector<uint8_t>										vData;
		};

#pragma pack( push, 1 )

		/** Chunk header. */
		struct PW_CHUNK_HEADER {
			union {
				char8_t													cName[4];
				uint32_t												uiId;
			}															u;
			uint32_t													uiSize;
		};

		/** FMT chunk. */
		struct PW_FMT_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			//uint32_t													uiSubchunk1ID;			// "fmt ".
			//uint32_t													uiSubchunk1Size;		// Normally 16 (for PCM).
			uint16_t													uiAudioFormat;			// PCM = 1 (linear quantization).
			uint16_t													uiNumChannels;			// 1 = mono, 2 = stereo.
			uint32_t													uiSampleRate;			// 44,100, 48,000, etc.
			uint32_t													uiByteRate;				// SampleRate * NumChannels * BitsPerSample / 8.
			uint16_t													uiBlockAlign;			// NumChannels * BitsPerSample / 8.
			uint16_t													uiBitsPerSample;		// 8, 16, etc.
			uint16_t													uiExtraParamSize;		// Invalid on PCM.
			uint8_t														ui8ExtraParams[1];		// uiExtraParamSize bytes of extra data.
		};

		/** DATA chunk. */
		struct PW_DATA_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			//uint32_t													uiSubchunk2ID;			// "data".
			//uint32_t													uiSubchunk2Size;		// NumSamples * NumChannels * BitsPerSample / 8.
			uint8_t														ui8Data[1];				// Sample data (length = uiSubchunk2Size).
		};

		/** SAMPL chunk. */
		struct PW_SMPL_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			uint32_t													uiManufacturer;
			uint32_t													uiProduct;
			uint32_t													uiSamplePeriod;
			uint32_t													uiMIDIUnityNote;
			uint32_t													uiMIDIPitchFraction;
			uint32_t													uiSMPTEFormat;
			uint32_t													uiSMPTEOffset;
			uint32_t													uiNumSampleLoops;
			uint32_t													uiSamplerData;
			PW_LOOP_POINT												lpLoops[1];
		};

		/** LIST chunk. */
		struct PW_LIST_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			union {
				char8_t													cName[4];
				uint32_t												uiTypeId;
			}															u;
			uint8_t														ui8Data[1];
		};

		/** ID3  chunk. */
		struct PW_ID3_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			char8_t														sName[3];
			uint16_t													ui16Version;
			uint8_t														ui8Flags;
			uint32_t													ui32Size;
			uint8_t														ui8Data[1];
		};

		/** INST chunk. */
		struct PW_INST_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			uint8_t														ui8UnshiftedNote;
			uint8_t														ui8FineTune;
			uint8_t														ui8Gain;
			uint8_t														ui8LowNote;
			uint8_t														ui8HiNote;
			uint8_t														ui8LowVel;
			uint8_t														ui8HiVel;
		};
#pragma pack( pop )

		/** A stream-to-file structure. */
		struct PW_STREAMING {
			uint64_t													ui64SamplesReceived = 0;	/**< The number of samples sent to the stream. */
			uint64_t													ui64SamplesWritten = 0;		/**< The number of samples written to the file. */
			uint64_t													ui64MetaWritten = 0;		/**< The number of metadata items written. */
			uint64_t													ui64WavFileOffset_Size = 0;	/**< The offset of the size value in the WAV file. */
			uint64_t													ui64WavFileOffset_DSize = 0;/**< The offset of the data-size value in the WAV file. */
			uint64_t													ui64WavFileOffset_Fact = 0;	/**< The offset of the sample-count value in the "fact" chunk (non-PCM formats only, else 0). */
			uint64_t													ui64MaxSamples = 0;			/**< The most samples that fit in the file before the RIFF size would overflow 32 bits. */
			uint64_t													ui64MetaParm = 0;			/**< The metadata uint64_t parameter. */
			uint64_t													ui64MetaThreadParm = 0;		/**< The metadata thread data used by the thread function to keep track of how many items it has processed, or for any other reason. */
			double														dDitherError = 0.0;			/**< The dither error. */
			size_t														stBufferSize = 1024 * 10;	/**< The size of the buffer to fill before flushing. */
			std::vector<float>											vCurBuffer;					/**< The current buffer awaiting samples. */
			std::vector<uint8_t, CAlignmentAllocator<uint8_t, 64>>		vMetaBuffer;				/**< The buffer for gathering metadata. */
			std::wstring												wsPath;						/**< The path to the file to which we are streaming. */
			CStdFile													sfFile;						/**< The file to which to write the WAV data. */

			std::wstring												wsMetaPath;					/**< The path to the metadata file to which we are streaming. */
			CStdFile													sfMetaFile;					/**< The metadata file to which to write the Wmetadata. */
			void *														pvMetaParm = nullptr;		/**< The metadata pointer parameter. */
			PfAddMetaDataFunc											pfMetaFunc = nullptr;		/**< The function for adding metadata. */
			PfMetaDataThreadFunc										pfMetaThreadFunc = nullptr;	/**< The function for parsing metadata into the final file output. */

			std::queue<std::vector<float>>								qBufferQueue;				/**< The queue of buffers handled by the thread. */
			std::mutex													mMutex;						/**< The thread mutex for accessing qBufferQueue and bStreaming. */
			std::condition_variable										cvCondition;				/**< The conditional variable for the lock. */
			std::thread													tThread;					/**< The thread for writing to the file. */

			std::queue<std::vector<uint8_t, CAlignmentAllocator<uint8_t, 64>>>
																		qMetaBufferQueue;			/**< The queue of buffers handled by the metadata thread. */
			std::mutex													mMetaMutex;					/**< The thread mutex for metadata streaming. */
			std::condition_variable										cvMetaCondition;			/**< The conditional variable for the metadata lock. */
			std::thread													tMetaThread;				/**< The thread for writing to the metadata file. */
			std::vector<uint8_t, CAlignmentAllocator<uint8_t, 64>>		vMetaThreadScratch;			/**< A scratch buffer for any data the thread needs to manage. */
			
			PW_CONDITIONS_DATA											cdStartData;				/**< The start-condition data. */
			PW_CONDITIONS_DATA											cdStopData;					/**< The stop-condition data. */
			PfStartConditionFunc										pfStartCondFunc = nullptr;	/**< The start-condition function. */
			PfEndConditionFunc											pfEndCondFunc = nullptr;	/**< The end-condition function. */
			PfBatchConvNWrite											pfCvtAndWriteFunc = nullptr;/**< The function for batch conversion and writing to the WAV file. */
			PfAddSampleFunc												pfAddSampleFunc = nullptr;	/**< The function for adding a sample.  There is one that checks the starting condition and then one that keeps adding samples until the stopping condition is reached. */
			uint32_t													ui32WavFile_Size = 0;		/**< The size of everything in the RIFF chunk except the sample data and its pad byte ("WAVE", "fmt ", "fact", "data" header). */
			int32_t														i32MetaFormat = 0;			/**< The metadata format. */
			uint32_t													ui32Hz = 44100;				/**< The file Hz. */
			PW_FORMAT													fFormat = PW_F_PCM;		/**< The WAV-file format. */
			uint16_t													ui16Bits = 16;				/**< The number of bits-per-sample. */			
			uint16_t													ui16Channels = 2;			/**< Total channels to output. */
			bool														bEnd = true;				/**< Tells the thread to stop. */
			bool														bStreaming = false;			/**< If true, the file must be closed either manually or in the destructor. */
			bool														bAdding = false;			/**< Set to true after the starting condition is met.  Indicates that samples are being added. */
			bool														bDither = false;			/**< To dither 16-bit PCM or not. */
			bool														bMeta = false;				/**< Whether metadata is being streamed or not. */
		};


		// == Functions.
		/**
		 * Loads a WAV file.
		 *
		 * \param _pcPath The UTF-8 path to open.
		 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
		 * \param _ui32StartSample The first sample to load.
		 * \param _ui32EndSample The last sample (exclusive) to load.
		 * \return Returns true if the file was opened.
		 */
		bool															Open( const char8_t * _pcPath, uint32_t _ui32LoadFlags = 0xFFFFFFFF, uint32_t _ui32StartSample = 0, uint32_t _ui32EndSample = 0xFFFFFFFF );

		/**
		 * Loads a WAV file.
		 *
		 * \param _pwcPath The UTF-16 path to open.
		 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
		 * \param _ui32StartSample The first sample to load.
		 * \param _ui32EndSample The last sample (exclusive) to load.
		 * \return Returns true if the file was opened.
		 */
		bool															Open( const char16_t * _pwcPath, uint32_t _ui32LoadFlags = 0xFFFFFFFF, uint32_t _ui32StartSample = 0, uint32_t _ui32EndSample = 0xFFFFFFFF );

		/**
		 * Loads a WAV file from a stream.  Will usually be a file stream but can be any stream.
		 * 
		 * \param _sbStream The stream from which to load the file.
		 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
		 * \param _ui32StartSample The first sample to load.
		 * \param _ui32EndSample The last sample (exclusive) to load.
		 * \return Returns true if the file is a valid WAV file and there are no memory problems loading it.
		 **/
		bool															LoadFromStream( CStreamBase &_sbStream, uint32_t _ui32LoadFlags = 0xFFFFFFFF, uint32_t _ui32StartSample = 0, uint32_t _ui32EndSample = 0xFFFFFFFF );

		/**
		 * Loads a WAV file from memory.  This is just an in-memory version of the file.
		 *
		 * \param _vData The in-memory file to load.
		 * \param _ui32LoadFlags The loading flags, which allow to not load large portions of the file.
		 * \param _ui32StartSample The first sample to load.
		 * \param _ui32EndSample The last sample (exclusive) to load.
		 * \return Returns true if the file is a valid WAV file.
		 */
		bool															LoadFromMemory( const std::vector<uint8_t> &_vData, uint32_t _ui32LoadFlags = 0xFFFFFFFF, uint32_t _ui32StartSample = 0, uint32_t _ui32EndSample = 0xFFFFFFFF );

		/**
		 * Saves as a WAV file.  Despite the name, the format is not limited to PCM: with no settings the file is saved in the same format (PCM or
		 *	IEEE float) and bit depth as the loaded file, and _psdSaveSettings can select PCM 8/16/24/32 or IEEE float 32/64.  The file is only
		 *	created once the whole image has been built successfully, so a failure never leaves a truncated file behind.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.  Every channel must hold the same number of samples.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		template <typename _tType = lwaudio>
		bool															SaveAsPcm( const char8_t * _pcPath, const _tType &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			if ( !_vSamples.size() || _vSamples.size() > 0xFFFF ) { return false; }
			const size_t stFrames = _vSamples[0].size();
			for ( size_t I = 1; I < _vSamples.size(); ++I ) {
				if ( _vSamples[I].size() != stFrames ) { return false; }
			}

			PW_FMT_CHUNK fcChunk = CreateFmt( DefaultSaveFormat(), static_cast<uint16_t>(_vSamples.size()),
				_psdSaveSettings );
			if ( !fcChunk.uiSampleRate || !fcChunk.uiBlockAlign ) { return false; }
			const uint32_t uiFmtSize = fcChunk.chHeader.uiSize + 8;
			const uint64_t ui64DataSize = uint64_t( stFrames ) * fcChunk.uiBlockAlign;
			if ( ui64DataSize > 0xFFFFFF00ULL ) { return false; }		// Leaves room for the headers within the 4-gigabyte RIFF limit.

			try {
				std::vector<uint8_t> vRet;
				vRet.reserve( static_cast<size_t>(ui64DataSize) + 1024 );
#define PW_PUSH32( VAL )		vRet.push_back( static_cast<uint8_t>((VAL) >> 0) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 8) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 16) ); vRet.push_back( static_cast<uint8_t>((VAL) >> 24) )
				PW_PUSH32( PW_C_RIFF );						// "RIFF"
				PW_PUSH32( 0 );								// The RIFF size is filled in once everything has been appended.
				PW_PUSH32( PW_C_WAVE );						// "WAVE"

				// Append the "fmt " chunk.
				vRet.insert( vRet.end(), reinterpret_cast<const uint8_t *>(&fcChunk), reinterpret_cast<const uint8_t *>(&fcChunk) + uiFmtSize );

				// Append the "fact" chunk (required for every non-PCM format).
				if ( fcChunk.uiAudioFormat != PW_F_PCM ) {
					PW_PUSH32( PW_C_FACT );					// "fact"
					PW_PUSH32( 4 );
					PW_PUSH32( static_cast<uint32_t>(stFrames) );
				}

				// Append the "data" chunk.  Its size field is the true data size; the pad byte is not part of it.
				PW_PUSH32( PW_C_DATA );
				PW_PUSH32( static_cast<uint32_t>(ui64DataSize) );
				const size_t stDataStart = vRet.size();
				switch ( fcChunk.uiAudioFormat ) {
					case PW_F_PCM : {
						switch ( fcChunk.uiBitsPerSample ) {
							case 8 : {
								if ( !BatchF64ToPcm8( _vSamples, vRet ) ) { return false; }
								break;
							}
							case 16 : {
								if ( _psdSaveSettings && _psdSaveSettings->bDither ) {
									if ( !BatchF64ToPcm16_Dither( _vSamples, vRet ) ) { return false; }
								}
								else {
									if ( !BatchF64ToPcm16( _vSamples, vRet ) ) { return false; }
								}
								break;
							}
							case 24 : {
								if ( !BatchF64ToPcm24( _vSamples, vRet ) ) { return false; }
								break;
							}
							case 32 : {
								if ( !BatchF64ToPcm32( _vSamples, vRet ) ) { return false; }
								break;
							}
							default : { return false; }
						}
						break;
					}
					case PW_F_IEEE_FLOAT : {
						switch ( fcChunk.uiBitsPerSample ) {
							case 32 : {
								if ( !BatchF64ToF32( _vSamples, vRet ) ) { return false; }
								break;
							}
							case 64 : {
								if ( !BatchF64ToF64( _vSamples, vRet ) ) { return false; }
								break;
							}
							default : { return false; }
						}
						break;
					}
					default : { return false; }
				}
				if ( vRet.size() - stDataStart != ui64DataSize ) { return false; }
				if ( vRet.size() & 1 ) {
					vRet.push_back( 0 );
				}

				// Append the metadata chunks.  Each Create*() returns a complete, padded chunk (or nothing).
				if ( m_vLoops.size() ) {
					AppendBytes( vRet, CreateSmpl() );
				}
				AppendBytes( vRet, CreateList() );
				AppendBytes( vRet, CreateInst() );
				AppendBytes( vRet, CreateDisp() );
				AppendBytes( vRet, CreateId3() );

				if ( vRet.size() - 8 > 0xFFFFFFFFULL ) { return false; }
				uint32_t ui32RiffSize = static_cast<uint32_t>(vRet.size() - 8);
				vRet[4] = static_cast<uint8_t>(ui32RiffSize >> 0);
				vRet[5] = static_cast<uint8_t>(ui32RiffSize >> 8);
				vRet[6] = static_cast<uint8_t>(ui32RiffSize >> 16);
				vRet[7] = static_cast<uint8_t>(ui32RiffSize >> 24);
#undef PW_PUSH32

				auto sPath = FixFileName( _pcPath );
				CStdFile sfFile;
				if ( !sfFile.Create( sPath.c_str() ) ) {
					std::wprintf( L"Failed to create PCM file: %s.\r\n", reinterpret_cast<const wchar_t *>(CUtilities::Utf8ToUtf16( sPath.c_str() ).c_str()) );
					return false;
				}
				if ( !sfFile.WriteToFile( vRet ) ) { return false; }
			}
			catch ( ... ) { return false; }
			return true;
		}

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		template <typename _tType = lwaudio>
		bool															SaveAsPcm( const char16_t * _pcPath, const lwaudio &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			return SaveAsPcm<_tType>( CUtilities::Utf16ToUtf8( _pcPath ).c_str(), _vSamples, _psdSaveSettings );
		}

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		template <typename _tType = lwaudio>
		bool															SaveAsPcm( const char8_t * _pcPath, const lwtrack &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			lwaudio aAudio;
			aAudio.push_back( _vSamples );
			return SaveAsPcm<_tType>( _pcPath, aAudio,
				_psdSaveSettings );
		}

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		template <typename _tType = lwaudio>
		bool															SaveAsPcm( const char16_t * _pcPath, const lwtrack &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			return SaveAsPcm<_tType>( CUtilities::Utf16ToUtf8( _pcPath ).c_str(), _vSamples, _psdSaveSettings );
		}

		/**
		 * Opens a stream to a WAV file.  Samples can be written over time.
		 * 
		 * \param _stfoFileOptions Contains all of the settings for streaming to a file.
		 * \param _ui32Hz The WAV Hz.
		 * \param _stBufferSize The buffer size (in samples).
		 * \return Returns true if the file could initially be created and all parameters are correct.
		 **/
		bool															StreamToFile( const PW_STREAM_TO_FILE_OPTIONS &_stfoFileOptions, uint32_t _ui32Hz,
			size_t _stBufferSize = 10 * 1024 );

		/**
		 * Stops the streaming file.
		 **/
		void															StopStream();

		/**
		 * Adds a sample to the stream.
		 * 
		 * \param _fSample The sample to add.
		 **/
		void															AddStreamSample( float _fSample );

		/**
		 * Called to update the metadata stream output.
		 **/
		void															AddMetaData();

		/**
		 * Gets a constant reference to the current streaming data.
		 * 
		 * \return Returns a constant reference to the current streaming data.
		 **/
		const PW_STREAMING &											GetStreamData() const { return m_sStream; }

		/**
		 * Resets the object back to scratch.
		 */
		void															Reset();

		/**
		 * Gets the number of samples in the loaded file.
		 *
		 * \return Returns the number of samples in the loaded file.
		 */
		inline uint32_t													TotalSamples() const { const uint32_t ui32Frame = uint32_t( m_uiNumChannels ) * m_uiBytesPerSample; return ui32Frame ? static_cast<uint32_t>(m_vSamples.size() / ui32Frame) : 0; }

		/**
		 * Gets the number of samples originally in the WAV file.
		 * 
		 * \return Returns the number of samples that were originally in the WAV file.
		 **/
		inline uint32_t													FileSampleCnt() const { return m_ui32OriginalSampleCount; }

#pragma optimize( "gt", on )
		/**
		 * Fills a vector with the whole range of samples for a given channel.  Samples are appended to _vResult.
		 *
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to fit all samples.  Returns false for formats that cannot be decoded.
		 */
		template <typename _tType = lwtrack>
		bool															GetAllSamples( uint16_t _uiChan, _tType &_vResult ) const {
			return GetSamples( _uiChan, _vResult, 0, TotalSamples() );
		}

		/**
		 * Fills a vector with a range of samples for a given channel.  Samples are appended to _vResult.  The range is clamped to the loaded samples.
		 *
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \param _stFrom The starting sample to get.
		 * \param _stTo The ending sample to get, exclusive.
		 * \return Returns true if the vector was able to fit all samples.  Returns false for formats that cannot be decoded.
		 */
		template <typename _tType = lwtrack>
		bool															GetSamples( uint16_t _uiChan, _tType &_vResult, size_t _stFrom, size_t _stTo ) const {
			if ( _uiChan >= m_uiNumChannels ) { return false; }
			const uint32_t ui32To = static_cast<uint32_t>(std::min<size_t>( _stTo, TotalSamples() ));
			const uint32_t ui32From = static_cast<uint32_t>(std::min<size_t>( _stFrom, ui32To ));
			try {
				switch ( m_fFormat ) {
					case PW_F_PCM : {
						switch ( m_uiBitsPerSample ) {
							case 8 : { return Pcm8ToF64( ui32From, ui32To, _uiChan, _vResult ); }
							case 16 : { return Pcm16ToF64( ui32From, ui32To, _uiChan, _vResult ); }
							case 24 : { return Pcm24ToF64( ui32From, ui32To, _uiChan, _vResult ); }
							case 32 : { return Pcm32ToF64( ui32From, ui32To, _uiChan, _vResult ); }
						}
						break;
					}
					case PW_F_IEEE_FLOAT : {
						switch ( m_uiBitsPerSample ) {
							case 32 : { return F32ToF64( ui32From, ui32To, _uiChan, _vResult ); }
							case 64 : { return F64ToF64( ui32From, ui32To, _uiChan, _vResult ); }
						}
						break;
					}
					default : {}
				}
			}
			catch ( ... ) {}
			return false;
		}

		/**
		 * Fills an array of vectors.  There is an array of vectors for each channel, and each vector contains all
		 *	of the samples for that channel.  _vResult is resized to the number of channels and each channel's vector is replaced.
		 *
		 * \param _vResult The array of vectors to be filled with all samples in this file.
		 * \return Returns true if the vector(s) was/were able to fit all samples.
		 */
		template <typename _tType = lwaudio>
		bool															GetAllSamples( _tType &_vResult ) const {
			return GetSamples( _vResult, 0, TotalSamples() );
		}

		/**
		 * Fills an array of vectors.  There is an array of vectors for each channel, and each vector contains all
		 *	of the selected samples for that channel.  _vResult is resized to the number of channels and each channel's vector is replaced.
		 *
		 * \param _vResult The array of vectors to be filled with all samples in this file.
		 * \param _stFrom The starting sample to get.
		 * \param _stTo The ending sample to get, exclusive.
		 * \return Returns true if the vector(s) was/were able to fit all samples.
		 */
		template <typename _tType = lwaudio>
		bool															GetSamples( _tType &_vResult, size_t _stFrom, size_t _stTo ) const {
			try {
				if ( !m_uiNumChannels ) { return false; }
				_vResult.resize( m_uiNumChannels );
				for ( auto I = m_uiNumChannels; I--; ) {
					_vResult[I].clear();
					if ( !GetSamples( I, _vResult[I], _stFrom, _stTo ) ) { return false; }
				}
				return true;
			}
			catch ( ... ) { return false; }
		}

		/**
		 * Releases all samples from memory.  Used to conserve RAM.
		 **/
		void															FreeSamples() {
			m_vSamples.clear();
		}

		/**
		 * Gets the Hz.
		 *
		 * \return Returns the Hz.
		 */
		inline uint32_t													Hz() const { return m_uiSampleRate; }

		/**
		 * Gets the number of channels.
		 *
		 * \return Returns the number of channels.
		 */
		inline uint16_t													Channels() const { return m_uiNumChannels; }

		/**
		 * Gets the number of bits per sample.
		 *
		 * \return Returns the number of bits per sample.
		 */
		inline uint16_t													BitsPerSample() const { return m_uiBitsPerSample; }

		/**
		 * Gets the format of the WAV file.
		 * 
		 * \return Returns the WAV-file format.
		 **/
		inline PW_FORMAT												Format() const { return m_fFormat; }

		/**
		 * Calculates the size of a buffer needed to hold the given number of samples on the given number of channels in te given format.
		 *
		 * \param _fFormat The format of the samples.
		 * \param _uiSamples The number of samples per channel.
		 * \param _uiChannels The number of channels.
		 * \param _uiBitsPerPixel The number of bits per sample.
		 * \return Returns the size of a buffer needed to hold the given number of samples on the given number of channels in te given format.
		 */
		static inline uint32_t											CalcSize( PW_FORMAT _fFormat, uint32_t _uiSamples, uint16_t _uiChannels, uint16_t _uiBitsPerPixel ) {
			switch ( _fFormat ) {
				case PW_F_PCM : {
					return uint32_t( uint64_t( _uiSamples ) * _uiChannels * _uiBitsPerPixel / 8 );
				}
				case PW_F_IEEE_FLOAT : {
					return uint32_t( uint64_t( _uiSamples ) * _uiChannels * ((_uiBitsPerPixel == 64) ? sizeof( double ) : sizeof( float )) );
				}
			}
			return 0;
		}

		/**
		 * Adds a LIST entry.
		 *
		 * \param _uiId The ID of the entry.
		 * \param _sVal The value of the entry.
		 * \return Returns true if the entry was added.
		 */
		bool															AddListEntry( uint32_t _uiId, const std::u8string &_sVal );

		/**
		 * Adds a DISP image.
		 * 
		 * \param _ui32Type The image type.  One of the CF_* values.
		 * \param _vImage The image data.
		 * \return Returns true if the entry was added.
		 **/
		bool															AddImage( uint32_t _ui32Type, const std::vector<uint8_t> &_vImage );

		/**
		 * Gets the loop array.
		 *
		 * \return Returns the loop array.
		 */
		const std::vector<PW_LOOP_POINT> &								Loops() const { return m_vLoops; }


	protected :
		// == Types.
		/** A chunk entry. */
		struct PW_CHUNK_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiName;
			}															u;
			uint32_t													uiOffset;
			uint32_t													uiSize;
		};

		/** A LIST entry. */
		struct PW_LIST_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			std::u8string												sText;
		};

		/** An ID3 entry. */
		struct PW_ID3_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			uint16_t													ui16Flags;
			std::u8string												sValue;
		};

		/** A DISP entry. */
		struct PW_DISP_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			uint32_t													ui32Size;
			uint32_t													ui32Type;
			std::vector<uint8_t>										vValue;
		};

		/** An INST entry. */
		struct PW_INST_ENTRY {
			uint8_t														ui8UnshiftedNote = 0;
			uint8_t														ui8FineTune = 0;
			uint8_t														ui8Gain = 0;
			uint8_t														ui8LowNote = 0;
			uint8_t														ui8HiNote = 0;
			uint8_t														ui8LowVel = 0;
			uint8_t														ui8HiVel = 0;
		};

		
		// == Members.
		/** The number of samples in the original file.  How many samples are actually loaded can vary. */
		uint32_t														m_ui32OriginalSampleCount;
		/** The format. */
		PW_FORMAT														m_fFormat;
		/** The number of channels. */
		uint16_t														m_uiNumChannels;			// 1 = mono, 2 = stereo, etc.
		/** The sample rate. */
		uint32_t														m_uiSampleRate;				// 44,100, 48,000, etc.
		/** The bits per sample. */
		uint16_t														m_uiBitsPerSample;			// 8, 16, etc.
		/** The bytes per sample. */
		uint16_t														m_uiBytesPerSample;			// m_uiBitsPerSample / 8.
		/** Base note. */
		uint32_t														m_uiBaseNote;
		/** The raw sample data. */
		std::vector<uint8_t>											m_vSamples;
		/** Loop points. */
		std::vector<PW_LOOP_POINT>										m_vLoops;
		/** "LIST" metadata. */
		std::vector<PW_LIST_ENTRY>										m_vListEntries;
		/** "id3 " metadata. */
		std::vector<PW_ID3_ENTRY>										m_vId3Entries;
		/** "DISP" metadata. */
		std::vector<PW_DISP_ENTRY>										m_vDisp;
		/** Instrument metadata. */
		PW_INST_ENTRY													m_ieInstEntry;
		/** Whether an "inst" chunk was loaded (and should be written back). */
		bool															m_bHasInst;
		/** The ID3v2 major version of the loaded "id3 " chunk (3 or 4). */
		uint8_t															m_ui8Id3Version;
		/** Streaming data. */
		PW_STREAMING													m_sStream;


		// == Functions.
		/**
		 * Loads data from the "fmt ".
		 *
		 * \param _pfcChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadFmt( const PW_FMT_CHUNK * _pfcChunk );

		/**
		 * Loads the "data" chunk.
		 *
		 * \param _pdcChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadData( const PW_DATA_CHUNK * _pdcChunk, uint32_t _ui32StartSample = 0, uint32_t _ui32EndSample = 0xFFFFFFFF );

		/**
		 * Loads the "smpl" chunk.
		 *
		 * \param _pscChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadSmpl( const PW_SMPL_CHUNK * _pscChunk );

		/**
		 * Loads a "LIST" chunk.
		 *
		 * \param _plcChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadList( const PW_LIST_CHUNK * _plcChunk );

		/**
		 * Loads an "id3 " chunk.
		 *
		 * \param _plcChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadId3( const PW_ID3_CHUNK * _picChunk );

		/**
		 * Loads an "inst" chunk.
		 *
		 * \param _plcChunk The chunk of data to load
		 * \return Returns true if everything loaded fine.
		 */
		bool															LoadInst( const PW_INST_CHUNK * _picChunk );

		/**
		 * Converts a bunch of 8-bit PCM samples to double.  8-bit WAV data is unsigned (128 is silence).
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															Pcm8ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				_vResult.push_back( (static_cast<int32_t>(m_vSamples[sIdx]) - 128) / 127.0 );
			}
			return true;
		}

		/**
		 * Converts a bunch of 16-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															Pcm16ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			const double dFactor = std::pow( 2.0, 16.0 - 1.0 ) - 1.0;
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				int16_t i16Sample;
				std::memcpy( &i16Sample, &m_vSamples[sIdx], sizeof( i16Sample ) );
				_vResult.push_back( i16Sample / dFactor );
			}
			return true;
		}

		/**
		 * Converts a bunch of 24-bit PCM samples to double.  Exactly 3 bytes are read per sample.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															Pcm24ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			const double dFactor = std::pow( 2.0, 24.0 - 1.0 ) - 1.0;
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				const uint8_t * pui8Sample = &m_vSamples[sIdx];
				int32_t i32Sample = static_cast<int32_t>(uint32_t( pui8Sample[0] ) << 8 | uint32_t( pui8Sample[1] ) << 16 | uint32_t( pui8Sample[2] ) << 24) >> 8;
				_vResult.push_back( i32Sample / dFactor );
			}
			return true;
		}

		/**
		 * Converts a bunch of 32-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															Pcm32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			const double dFactor = std::pow( 2.0, 32.0 - 1.0 ) - 1.0;
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				int32_t i32Sample;
				std::memcpy( &i32Sample, &m_vSamples[sIdx], sizeof( i32Sample ) );
				_vResult.push_back( i32Sample / dFactor );
			}
			return true;
		}

		/**
		 * Converts a bunch of 32-bit float samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															F32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				float fSample;
				std::memcpy( &fSample, &m_vSamples[sIdx], sizeof( fSample ) );
				_vResult.push_back( fSample );
			}
			return true;
		}

		/**
		 * Converts a bunch of 64-bit float samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \tparam _tType The vector tpe to which to write the result.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		template <typename _tType = lwtrack>
		bool															F64ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, _tType &_vResult ) const {
			_vResult.reserve( _vResult.size() + (_ui32To - _ui32From) );
			uint32_t uiStride;
			size_t sIdx = CalcOffsetsForSample( _uiChan, _ui32From, uiStride );
			for ( ; _ui32From < _ui32To; ++_ui32From, sIdx += uiStride ) {
				double dSample;
				std::memcpy( &dSample, &m_vSamples[sIdx], sizeof( dSample ) );
				_vResult.push_back( dSample );
			}
			return true;
		}

		/**
		 * Converts a batch of F64 samples to interleaved little-endian integer PCM samples and appends them to a buffer.  The conversion is
		 *	CUtilities::SampleToInt() (scale, round half away from zero, clamp in the integer domain), so samples that were read from a file
		 *	are written back unchanged.
		 *
		 * \param _vSrc The samples to convert (one vector per channel, all the same length).
		 * \param _vDst The buffer to which to append the converted samples.
		 * \param _dScale The value by which to scale the samples.
		 * \param _i32Min The smallest allowed integer value.
		 * \param _i32Max The largest allowed integer value.
		 * \param _sBytes The number of bytes per sample (1, 2, 3, or 4).
		 * \param _i32Bias The value to add after conversion (128 for unsigned 8-bit PCM).
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcmInt( const _tType &_vSrc, std::vector<uint8_t> &_vDst, double _dScale, int32_t _i32Min, int32_t _i32Max, size_t _sBytes, int32_t _i32Bias ) {
			const size_t stStart = _vDst.size();
			try {
				if ( !_vSrc.size() ) { return false; }
				const size_t stFrames = _vSrc[0].size();
				const size_t stChannels = _vSrc.size();
				const size_t stStride = stChannels * _sBytes;
				_vDst.resize( stStart + stFrames * stStride );
				std::vector<int32_t> vTmp( stFrames );
				for ( size_t C = 0; C < stChannels; ++C ) {
					if ( _vSrc[C].size() != stFrames ) { _vDst.resize( stStart ); return false; }
					CUtilities::SamplesToInt( _vSrc[C].data(), stFrames, vTmp.data(), _dScale, _i32Min, _i32Max );
					uint8_t * pui8Dst = _vDst.data() + stStart + C * _sBytes;
					for ( size_t I = 0; I < stFrames; ++I, pui8Dst += stStride ) {
						const uint32_t ui32Val = static_cast<uint32_t>(vTmp[I] + _i32Bias);
						for ( size_t B = 0; B < _sBytes; ++B ) {
							pui8Dst[B] = static_cast<uint8_t>(ui32Val >> (B * 8));
						}
					}
				}
				return true;
			}
			catch ( ... ) {
				try { _vDst.resize( stStart ); } catch ( ... ) {}
				return false;
			}
		}

		/**
		 * Converts a batch of F64 samples to unsigned 8-bit PCM samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcm8( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToPcmInt( _vSrc, _vDst, 127.0, -128, 127, 1, 128 );
		}

		/**
		 * Converts a batch of F64 samples to 16-bit PCM samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcm16( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToPcmInt( _vSrc, _vDst, 32767.0, -32768, 32767, 2, 0 );
		}

		/**
		 * Converts a batch of F64 samples to dithered 16-bit PCM samples and appends them to a buffer.  Each channel keeps its own error state.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcm16_Dither( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			const size_t stStart = _vDst.size();
			try {
				if ( !_vSrc.size() ) { return false; }
				const size_t stFrames = _vSrc[0].size();
				const size_t stChannels = _vSrc.size();
				for ( size_t C = 1; C < stChannels; ++C ) {
					if ( _vSrc[C].size() != stFrames ) { return false; }
				}
				_vDst.resize( stStart + stFrames * stChannels * sizeof( int16_t ) );
				std::vector<double> vError( stChannels );
				uint8_t * pui8Dst = _vDst.data() + stStart;
				for ( size_t I = 0; I < stFrames; ++I ) {
					for ( size_t C = 0; C < stChannels; ++C ) {
						const int16_t i16Sample = CUtilities::SampleToI16_Dither( _vSrc[C][I], vError[C] );
						(*pui8Dst++) = static_cast<uint8_t>(i16Sample);
						(*pui8Dst++) = static_cast<uint8_t>(static_cast<uint16_t>(i16Sample) >> 8);
					}
				}
				return true;
			}
			catch ( ... ) {
				try { _vDst.resize( stStart ); } catch ( ... ) {}
				return false;
			}
		}

		/**
		 * Converts a batch of F64 samples to 24-bit PCM samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcm24( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToPcmInt( _vSrc, _vDst, 8388607.0, -8388608, 8388607, 3, 0 );
		}

		/**
		 * Converts a batch of F64 samples to 32-bit PCM samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToPcm32( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToPcmInt( _vSrc, _vDst, 2147483647.0, INT32_MIN, INT32_MAX, 4, 0 );
		}

		/**
		 * Converts a batch of F64 samples to interleaved IEEE floats of the given type and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \tparam _tFloat float or double.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tFloat, typename _tType = lwaudio>
		static bool														BatchF64ToFloat( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			const size_t stStart = _vDst.size();
			try {
				if ( !_vSrc.size() ) { return false; }
				const size_t stFrames = _vSrc[0].size();
				const size_t stChannels = _vSrc.size();
				for ( size_t C = 1; C < stChannels; ++C ) {
					if ( _vSrc[C].size() != stFrames ) { return false; }
				}
				_vDst.resize( stStart + stFrames * stChannels * sizeof( _tFloat ) );
				uint8_t * pui8Dst = _vDst.data() + stStart;
				for ( size_t I = 0; I < stFrames; ++I ) {
					for ( size_t C = 0; C < stChannels; ++C ) {
						const _tFloat tSample = static_cast<_tFloat>(_vSrc[C][I]);
						std::memcpy( pui8Dst, &tSample, sizeof( tSample ) );
						pui8Dst += sizeof( tSample );
					}
				}
				return true;
			}
			catch ( ... ) {
				try { _vDst.resize( stStart ); } catch ( ... ) {}
				return false;
			}
		}

		/**
		 * Converts a batch of F64 samples to F32 samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToF32( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToFloat<float>( _vSrc, _vDst );
		}

		/**
		 * Converts a batch of F64 samples to F64 samples and appends them to a buffer.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns true if all samples were added to the buffer.
		 */
		template <typename _tType = lwaudio>
		static bool														BatchF64ToF64( const _tType &_vSrc, std::vector<uint8_t> &_vDst ) {
			return BatchF64ToFloat<double>( _vSrc, _vDst );
		}

		/**
		 * Appends a block of bytes to a buffer.
		 *
		 * \param _vDst The buffer to which to append.
		 * \param _vSrc The bytes to append.
		 */
		static void														AppendBytes( std::vector<uint8_t> &_vDst, const std::vector<uint8_t> &_vSrc ) {
			_vDst.insert( _vDst.end(), _vSrc.begin(), _vSrc.end() );
		}

		/**
		 * Gets the format in which SaveAsPcm() saves by default: IEEE float if the loaded file was floating-point, otherwise PCM.
		 *
		 * \return Returns the default save format.
		 */
		PW_FORMAT														DefaultSaveFormat() const { return m_fFormat == PW_F_IEEE_FLOAT ? PW_F_IEEE_FLOAT : PW_F_PCM; }

		/**
		 * Gets the byte indices of PCM data given an offset and channel.
		 *
		 * \param _uiChan The channel.
		 * \param _uiIdx The sample index.
		 * \param _uiStride The returned stride.
		 * \return Returns the byte index calculated using the given information and the sample depth.
		 */
		size_t															CalcOffsetsForSample( uint16_t _uiChan, uint32_t _uiIdx, uint32_t &_uiStride ) const;

		/**
		 * Converts a 28-bit size value from ID3 into regular 32-bit.
		 *
		 * \param _uiSize The size value to decode.
		 * \return Returns the decoded size value.
		 */
		static uint32_t													DecodeSize( uint32_t _uiSize );

		/**
		 * Creates an "fmt " chunk based off either this object's parameters or optional given overrides.
		 *
		 * \param _fFormat The format to set.
		 * \param _uiChannels The total channels.
		 * \param _psdSaveSettings Optional overrides.
		 * \return Returns a created object.
		 */
		PW_FMT_CHUNK													CreateFmt( PW_FORMAT _fFormat, uint16_t _uiChannels, const PW_SAVE_DATA * _psdSaveSettings ) const;

		/**
		 * Writes file-image loop-point bytes to a vector.
		 *
		 * \return Returns the bytes that represent the "smpl" chunk in a file.
		 */
		std::vector<uint8_t>											CreateSmpl() const;

		/**
		 * Writes file-image "LIST" chunk to a vector.
		 *
		 * \return Returns the bytes that represent the "LIST" chunk in a file.
		 */
		std::vector<uint8_t>											CreateList() const;

		/**
		 * Writes a file-image "inst" chunk to a vector, if an "inst" chunk was loaded.
		 *
		 * 
eturn Returns the bytes that represent the "inst" chunk in a file, or an empty vector.
		 */
		std::vector<uint8_t>											CreateInst() const;

		/**
		 * Writes file-image "DISP" chunks to a vector, one per image added with AddImage().
		 *
		 * 
eturn Returns the bytes that represent the "DISP" chunks in a file, or an empty vector.
		 */
		std::vector<uint8_t>											CreateDisp() const;

		/**
		 * Writes a file-image "id3 " chunk to a vector containing the loaded ID3v2 frames, using the loaded tag's version.
		 *
		 * 
eturn Returns the bytes that represent the "id3 " chunk in a file, or an empty vector.
		 */
		std::vector<uint8_t>											CreateId3() const;

		/**
		 * Fixes the output file name of a given full path.
		 * 
		 * \param _pcFullpath The full path whose file name is to fix.
		 * \return Returns the fixed file path.
		 **/
		static std::u8string											FixFileName( const char8_t * _pcFullpath );

		/**
		 * Creates the file for streaming and writes the header data to it, preparing it for writing samples.
		 * 
		 * \param _pcPath Uses data loaded into m_sStream to create a new file.
		 * \return Returns true if the file was created and the header was written to it.
		 **/
		bool															CreateStreamFile( const char8_t * _pcPath );

		/**
		 * Closes the current streaming file.
		 **/
		void															CloseStreamFile();

		/**
		 * Creates the file for streaming metadata.  All writes to the file are handled by a callback.
		 * 
		 * \param _pcPath Uses data loaded into m_sStream to create a new file.
		 * \return Returns true if the file was created.
		 **/
		bool															CreateStreamMetaFile( const char8_t * _pcPath );

		/**
		 * Closes the current streaming metadata file.
		 **/
		void															CloseStreamMetaFile();

		/**
		 * The stream-to-file writer thread.
		 **/
		void															StreamWriterThread();

		/**
		 * The metadata-to-file writer thread.
		 **/
		void 															MetadataWriterThread();

		/**
		 * The None start condition.  Returns true.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns true if the start condition has been met.
		 **/
		static bool PW_STDCALL											StartCondFunc_None( uint64_t /*_ui64AbsoluteIdx*/, float /*_fSample*/, PW_CONDITIONS_DATA &/*_cdData*/ );

		/**
		 * The Start-at-Sample start condition.  Returns true if _ui64AbsoluteIdx >= _cdData.ui64Parm0.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns true if the start condition has been met.
		 **/
		static bool PW_STDCALL											StartCondFunc_StartAtSample( uint64_t _ui64AbsoluteIdx, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData );

		/**
		 * The First-Non-Zero start condition.  Returns true if _fSample != 0.0f.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns true if the start condition has been met.
		 **/
		static bool PW_STDCALL											StartCondFunc_FirstNonZero( uint64_t /*_ui64AbsoluteIdx*/, float _fSample, PW_CONDITIONS_DATA &/*_cdData*/ );

		/**
		 * The Zero-For-Duration start condition.  Returns true if _cdData.ui64Counter >= _cdData.ui64Parm0, with _cdData.ui64Counter being counted on each 0 sample.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns true if the start condition has been met.
		 **/
		static bool PW_STDCALL											StartCondFunc_ZeroForDuration( uint64_t /*_ui64AbsoluteIdx*/, float _fSample, PW_CONDITIONS_DATA &_cdData );

		/**
		 * The None end condition.  Returns true.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns false if the end condition has been met, at which point recording should stop.
		 **/
		static bool PW_STDCALL											EndCondFunc_None( uint64_t /*_ui64AbsoluteIdx*/, uint64_t /*_ui64IdxSinceStart*/, float /*_fSample*/, PW_CONDITIONS_DATA &/*_cdData*/ );

		/**
		 * The End-At-Sample end condition.  Returns true if _ui64AbsoluteIdx < _cdData.ui64Parm0.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns false if the end condition has been met, at which point recording should stop.
		 **/
		static bool PW_STDCALL											EndCondFunc_EndAtSample( uint64_t _ui64AbsoluteIdx, uint64_t /*_ui64IdxSinceStart*/, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData );

		/**
		 * The Zero-For-Duration end condition.  Returns true if _cdData.ui64Counter < _cdData.ui64Parm0, with _cdData.ui64Counter counting consecutive 0's.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns false if the end condition has been met, at which point recording should stop.
		 **/
		static bool PW_STDCALL											EndCondFunc_ZeroForDuration( uint64_t /*_ui64AbsoluteIdx*/, uint64_t /*_ui64IdxSinceStart*/, float _fSample, PW_CONDITIONS_DATA &_cdData );

		/**
		 * The Duration end condition.  Returns true if _ui64IdxSinceStart < _cdData.ui64Parm0.
		 * 
		 * \param _ui64AbsoluteIdx The absolute index of the sample being tested.
		 * \param _ui64IdxSinceStart The index of the sample being tested since recording began.
		 * \param _fSample The value of the sample being tested.
		 * \param _cdData Function-specific data.
		 * \return Returns false if the end condition has been met, at which point recording should stop.
		 **/
		static bool PW_STDCALL											EndCondFunc_Duration( uint64_t /*_ui64AbsoluteIdx*/, uint64_t _ui64IdxSinceStart, float /*_fSample*/, PW_CONDITIONS_DATA &_cdData );

		/**
		 * Checks the starting condition before adding a sample.  Once the starting condition has been reached, _sStream.pfAddSampleFunc is changed to AddSample_CheckStopCond().
		 * 
		 * \param _fSample The sampple to add.
		 * \param _sStream The stream data.
		 **/
		static bool PW_STDCALL											AddSample_CheckStartCond( float _fSample, PW_STREAMING &_sStream );

		/**
		 * Checks the stopping condition before adding a sample.  Once the stopping condition has been reached, the stream ends. 
		 * 
		 * \param _fSample The sampple to add.
		 * \param _sStream The stream data.
		 * \return Returns true if the starting condition has been reached.
		 **/
		static bool PW_STDCALL											AddSample_CheckStopCond( float _fSample, PW_STREAMING &_sStream );

		/**
		 * Performs no conversion.  Input is directly sent to the file stream.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 * \return Returns true for as long as samples should be output.  Once false is returned, the ending condition has been reached and the stream should close without adding the sample.
		 **/
		static inline void PW_STDCALL									BatchF32ToF32( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 8-bit PCM using raw C++.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm8( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 16-bit PCM using raw C++.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm16( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 16-bit PCM using raw C++.  Applies dither (state stored in _sStream).
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm16_Dither( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 24-bit PCM using raw C++.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm24( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 32-bit PCM using raw C++.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm32( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

#ifdef __AVX2__
		/**
		 * Converts a batch of floats to 8-bit PCM using AVX 2.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm8_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 16-bit PCM using AVX 2.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm16_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 24-bit PCM using AVX 2.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm24_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );
#endif	// #ifdef __AVX2__

#ifdef __AVX512F__
		/* Converts a batch of floats to 8-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm8_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 16-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm16_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );

		/**
		 * Converts a batch of floats to 24-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		static inline void PW_STDCALL									BatchF32ToPcm24_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream );
#endif	// #ifdef __AVX512F__
		
#pragma optimize( "", on )
	};
	


	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	// DEFINITIONS
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	// == Functions.
	/**
	 * Performs no conversion.  Input is directly sent to the file stream.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToF32( const std::vector<float> &_vSrc, std::vector<uint8_t> &/*_vOut*/, PW_STREAMING &_sStream ) {
		if PW_LIKELY( _vSrc.size() ) {
			_sStream.sfFile.WriteToFile( reinterpret_cast<const uint8_t *>(_vSrc.data()),
				_vSrc.size() * sizeof( float ) );
		}
	}

	/**
	 * Converts a batch of floats to 8-bit PCM using raw C++.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm8( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize ) {
				_vOut.resize( sSize );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			for ( size_t I = 0; I < sSize; ++I ) {
				_vOut[I] = CUtilities::SampleToUi8( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 16-bit PCM using raw C++.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm16( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize << 1 ) {
				_vOut.resize( _vSrc.size() << 1 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			for ( size_t I = 0; I < sSize; ++I ) {
				reinterpret_cast<int16_t *>(_vOut.data())[I] = CUtilities::SampleToI16( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * sizeof( uint16_t ) );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 16-bit PCM using raw C++.  Applies dither (state stored in _sStream).
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm16_Dither( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize << 1 ) {
				_vOut.resize( _vSrc.size() << 1 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			for ( size_t I = 0; I < sSize; ++I ) {
				// SampleToI16_Dither() clamps the sample plus error so the error stays bounded while the signal clips.
				reinterpret_cast<int16_t *>(_vOut.data())[I] = CUtilities::SampleToI16_Dither( _vSrc[I], _sStream.dDitherError );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * sizeof( uint16_t ) );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 24-bit PCM using raw C++.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm24( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize * 3 ) {
				_vOut.resize( sSize * 3 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			
			// Assuming penalty for misaligned writes is minor.  Some platforms may benefit from a slower algorithm
			//	that does only aligned writes.
			sSize--;
			size_t I = 0;
			for ( ; I < sSize; ++I ) {
				int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
				auto sIdx = I * 3;
				(*reinterpret_cast<int32_t *>(&_vOut[sIdx])) = i32Tmp;
			}

			// Last one has to be carefully constructed.
			int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
			auto sIdx = I * 3;
			(*reinterpret_cast<int16_t *>(&_vOut[sIdx])) = int16_t( i32Tmp );
			_vOut[sIdx+2] = uint8_t( i32Tmp >> 16 );

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * 3 );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 32-bit PCM using raw C++.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm32( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize * sizeof( int32_t ) ) {
				_vOut.resize( sSize * sizeof( int32_t ) );
			}
			for ( size_t I = 0; I < sSize; ++I ) {
				const int32_t i32Sample = CUtilities::SampleToI32( _vSrc[I] );
				std::memcpy( &_vOut[I*sizeof( int32_t )], &i32Sample, sizeof( i32Sample ) );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				sSize * sizeof( int32_t ) );
		}
		catch ( ... ) {}
	}

#ifdef __AVX2__
	/**
	 * Converts a batch of floats to 8-bit PCM using AVX 2.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm8_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize ) {
				_vOut.resize( sSize );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m256 ) / sizeof( float );
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToUi8_AVX2( &_vSrc[I], &_vOut[I] );
					I += sRegSize;
				}
			}

			for ( ; I < sSize; ++I ) {
				_vOut[I] = CUtilities::SampleToUi8( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 16-bit PCM using AVX 2.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm16_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize << 1 ) {
				_vOut.resize( _vSrc.size() << 1 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m256 ) / sizeof( float );
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToI16_AVX2( &_vSrc[I], reinterpret_cast<int16_t *>(&_vOut[I<<1]) );
					I += sRegSize;
				}
			}

			for ( ; I < sSize; ++I ) {
				(*reinterpret_cast<int16_t *>(&_vOut[I<<1])) = CUtilities::SampleToI16( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * sizeof( uint16_t ) );
		}
		catch ( ... ) {}
	}

	/**
	 * Converts a batch of floats to 24-bit PCM using AVX 2.
	 * 
	 * \param _vSrc The input samples.
	 * \param _vOut The output samples.
	 * \param _sStream The stream data.
	 **/
	inline void PW_STDCALL CWavFile::BatchF32ToPcm24_AVX2( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
		try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize * 3 ) {
				_vOut.resize( sSize * 3 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			

			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m256 ) / sizeof( float );
			PW_ALIGN( 32 )
			int32_t i32uffer[sRegSize];
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToI24_AVX2( &_vSrc[I], i32uffer );
					for ( size_t J = 0; J < (sRegSize - 1); ++J ) {
						auto sIdx = (I + J) * 3;
						(*reinterpret_cast<int32_t *>(&_vOut[sIdx])) = i32uffer[J];
					}
					(*reinterpret_cast<int16_t *>(&_vOut[(I+(sRegSize - 1))*3])) = int16_t( i32uffer[(sRegSize-1)] );
					_vOut[(I+(sRegSize - 1))*3+2] = int8_t( i32uffer[(sRegSize-1)] >> 16 );
					I += sRegSize;
				}
				
			}
			if PW_UNLIKELY( sSize > I ) {	// Unlikely because almost always we will be passing in buffers that are a size divisible by 8 (sRegSize).
				--sSize;
				for ( ; I < sSize; ++I ) {
					int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
					auto sIdx = I * 3;
					(*reinterpret_cast<int32_t *>(&_vOut[sIdx])) = i32Tmp;
				}

				// Last one has to be carefully constructed.
				int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
				auto sIdx = I * 3;
				(*reinterpret_cast<int16_t *>(&_vOut[sIdx])) = int16_t( i32Tmp );
				_vOut[sIdx+2] = uint8_t( i32Tmp >> 16 );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * 3 );
		}
		catch ( ... ) {}
	}
#endif	// #ifdef __AVX2__

#ifdef __AVX512F__
		/* Converts a batch of floats to 8-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		inline void PW_STDCALL CWavFile::BatchF32ToPcm8_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
			try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize ) {
				_vOut.resize( sSize );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m512 ) / sizeof( float );
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToUi8_AVX512( &_vSrc[I], &_vOut[I] );
					I += sRegSize;
				}
			}

			for ( ; I < sSize; ++I ) {
				_vOut[I] = CUtilities::SampleToUi8( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() );
		}
		catch ( ... ) {}
		}

		/**
		 * Converts a batch of floats to 16-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		inline void PW_STDCALL CWavFile::BatchF32ToPcm16_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
			try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize << 1 ) {
				_vOut.resize( _vSrc.size() << 1 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m512 ) / sizeof( float );
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToI16_AVX512( &_vSrc[I], reinterpret_cast<int16_t *>(&_vOut[I<<1]) );
					I += sRegSize;
				}
			}

			for ( ; I < sSize; ++I ) {
				(*reinterpret_cast<int16_t *>(&_vOut[I<<1])) = CUtilities::SampleToI16( _vSrc[I] );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * sizeof( uint16_t ) );
		}
		catch ( ... ) {}
		}

		/**
		 * Converts a batch of floats to 24-bit PCM using AVX-512.
		 * 
		 * \param _vSrc The input samples.
		 * \param _vOut The output samples.
		 * \param _sStream The stream data.
		 **/
		inline void PW_STDCALL CWavFile::BatchF32ToPcm24_AVX512( const std::vector<float> &_vSrc, std::vector<uint8_t> &_vOut, PW_STREAMING &_sStream ) {
			try {
			if PW_UNLIKELY( !_vSrc.size() ) { _vOut.clear(); return; }
			size_t sSize = _vSrc.size();
			if PW_UNLIKELY( _vOut.size() < sSize * 3 ) {
				_vOut.resize( sSize * 3 );
			}
			PW_PREFETCH_LINE( _vSrc.data() + (sSize >> 1) );
			

			size_t I = 0;
			constexpr size_t sRegSize = sizeof( __m512 ) / sizeof( float );
			PW_ALIGN( 64 )
			int32_t i32uffer[sRegSize];
			if PW_LIKELY( sSize >= sRegSize ) {
				size_t sTotal = sSize - sRegSize;
				while ( I <= sTotal ) {
					CUtilities::SampleToI24_AVX512( &_vSrc[I], i32uffer );
					for ( size_t J = 0; J < (sRegSize - 1); ++J ) {
						auto sIdx = (I + J) * 3;
						(*reinterpret_cast<int32_t *>(&_vOut[sIdx])) = i32uffer[J];
					}
					(*reinterpret_cast<int16_t *>(&_vOut[(I+(sRegSize - 1))*3])) = int16_t( i32uffer[(sRegSize-1)] );
					_vOut[(I+(sRegSize - 1))*3+2] = int8_t( i32uffer[(sRegSize-1)] >> 16 );
					I += sRegSize;
				}
				
			}
			if PW_UNLIKELY( sSize > I ) {	// Unlikely because almost always we will be passing in buffers that are a size divisible by 16 (sRegSize).
				--sSize;
				for ( ; I < sSize; ++I ) {
					int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
					auto sIdx = I * 3;
					(*reinterpret_cast<int32_t *>(&_vOut[sIdx])) = i32Tmp;
				}

				// Last one has to be carefully constructed.
				int32_t i32Tmp = CUtilities::SampleToI24( _vSrc[I] );
				auto sIdx = I * 3;
				(*reinterpret_cast<int16_t *>(&_vOut[sIdx])) = int16_t( i32Tmp );
				_vOut[sIdx+2] = uint8_t( i32Tmp >> 16 );
			}

			_sStream.sfFile.WriteToFile( _vOut.data(),
				_vSrc.size() * 3 );
		}
		catch ( ... ) {}
		}
#endif	// #ifdef __AVX512F__

}	// namespace pw
