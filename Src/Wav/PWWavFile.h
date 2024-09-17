/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A WAV file.
 */


#pragma once

#include "../Utilities/PWAlignmentAllocator.h"
#include "../Utilities/PWUtilities.h"

#include <cinttypes>
#include <string>
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

		/** Chunks. */
		enum PW_CHUNKS : uint32_t {
			PW_C_RIFF													= 0x46464952,
			PW_C_WAVE													= 0x45564157,
			PW_C_FMT_													= 0x20746D66,
			PW_C_DATA													= 0x61746164,
			PW_C_SMPL													= 0x6C706D73,
			PW_C_LIST													= 0x5453494C,
			PW_C_ID3_													= 0x20336469,
			PW_C_INST													= 0x74736E69,
			PW_C_INFO													= 0x4F464E49,
			PW_C_LABL													= 0x6C62616C,
			PW_C_ADTL													= 0x6C746461,
			PW_C_DISP													= 0x70736964,	// Or 0x64697370?
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


		// == Types.
		typedef std::vector<double, CAlignmentAllocator<double, 64>>	lwtrack;
		typedef std::vector<lwtrack>									lwaudio;

		/** The save data. */
		struct PW_SAVE_DATA {
			uint32_t													uiHz;					// Only overrides if not 0.
			uint16_t													uiBitsPerSample;		// Only overrides if not 0.

			PW_SAVE_DATA() :
				uiHz( 0 ),
				uiBitsPerSample( 0 ) {}
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


		// == Functions.
		/**
		 * Loads a WAV file.
		 *
		 * \param _pcPath The UTF-8 path to open.
		 * \return Returns true if the file was opened.
		 */
		bool															Open( const char8_t * _pcPath );

		/**
		 * Loads a WAV file.
		 *
		 * \param _pwcPath The UTF-16 path to open.
		 * \return Returns true if the file was opened.
		 */
		bool															Open( const char16_t * _pwcPath );

//#ifdef _WIN32
//		/**
//		 * Loads a WAV file.
//		 *
//		 * \param _pcPath The UTF-8 path to open.
//		 * \return Returns true if the file was opened.
//		 */
//		bool															Open( const char * _pcPath ) { return Open( reinterpret_cast<const char8_t *>(_pcPath) ); }
//
//		/**
//		 * Loads a WAV file.
//		 *
//		 * \param _pwcPath The UTF-16 path to open.
//		 * \return Returns true if the file was opened.
//		 */
//		bool															Open( const wchar_t * _pwcPath ) { return Open( reinterpret_cast<const char16_t *>(_pwcPath) ); }
//#endif	// #ifdef _WIN32

		/**
		 * Loads a WAV file from memory.  This is just an in-memory version of the file.
		 *
		 * \param _vData The in-memory file to load.
		 * \return Returns true if the file is a valid WAV file.
		 */
		bool															LoadFromMemory( const std::vector<uint8_t> &_vData );

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		bool															SaveAsPcm( const char8_t * _pcPath, const lwaudio &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const;

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		bool															SaveAsPcm( const char16_t * _pcPath, const lwaudio &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			return SaveAsPcm( CUtilities::Utf16ToUtf8( _pcPath ).c_str(), _vSamples, _psdSaveSettings );
		}

		/**
		 * Saves as a PCM WAV file.
		 *
		 * \param _pcPath The path to where the file will be saved.
		 * \param _vSamples The samples to convert and write to the file.
		 * \param _psdSaveSettings Settings to override this class's settings.
		 * \return Returns true if the file was created and saved.
		 */
		bool															SaveAsPcm( const char8_t * _pcPath, const lwtrack &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			lwaudio aAudio;
			aAudio.push_back( _vSamples );
			return SaveAsPcm( _pcPath, aAudio,
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
		bool															SaveAsPcm( const char16_t * _pcPath, const lwtrack &_vSamples,
			const PW_SAVE_DATA * _psdSaveSettings = nullptr ) const {
			return SaveAsPcm( CUtilities::Utf16ToUtf8( _pcPath ).c_str(), _vSamples, _psdSaveSettings );
		}

		/**
		 * Resets the object back to scratch.
		 */
		void															Reset();

		/**
		 * Gets the number of samples in the loaded file.
		 *
		 * \return Returns the number of samples in the loaded file.
		 */
		inline uint32_t													TotalSamples() const { return static_cast<uint32_t>(m_vSamples.size() / (m_uiNumChannels * m_uiBytesPerSample)); }

		/**
		 * Fills a vector with the whole range of samples for a given channel.
		 *
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to fit all samples.
		 */
		bool															GetAllSamples( uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Fills a vector with the whole range of samples for a given channel.
		 *
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \param _stFrom The starting sample to get.
		 * \param _stTo The ending sample to get, exclusive.
		 * \return Returns true if the vector was able to fit all samples.
		 */
		bool															GetSamples( uint16_t _uiChan, lwtrack &_vResult, size_t _stFrom, size_t _stTo ) const;

		/**
		 * Fills an array of vectors.  There is an array of vectors for each channel, and each vector contains all
		 *	of the samples for that channel.
		 *
		 * \param _vResult The array of vectors to be filled with all samples in this file.
		 * \return Returns true if the vector(s) was/were able to fit all samples.
		 */
		bool															GetAllSamples( lwaudio &_vResult ) const;

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
		 * Calculates the size of a buffer needed to hold the given number of samples on the given number of channels in te given format.
		 *
		 * \param _fFormat The format of the samples.
		 * \param _uiSamples The number of samples per channel.
		 * \param _uiChannels The number of channels.
		 * \param _uiBitsPerPixel The number of bits per sample.
		 * \return RETURN
		 */
		inline uint32_t													CalcSize( PW_FORMAT _fFormat, uint32_t _uiSamples, uint16_t _uiChannels, uint16_t _uiBitsPerPixel ) const {
			switch ( _fFormat ) {
				case PW_F_PCM : {
					return uint32_t( uint64_t( _uiSamples ) * _uiChannels * _uiBitsPerPixel / 8 );
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
#pragma pack( push, 1 )
		// Chunk.
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

		// Chunk header.
		struct PW_CHUNK_HEADER {
			union {
				char8_t													cName[4];
				uint32_t												uiId;
			}															u;
			uint32_t													uiSize;
		};

		// FMT chunk.
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

		// DATA chunk.
		struct PW_DATA_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			//uint32_t													uiSubchunk2ID;			// "data".
			//uint32_t													uiSubchunk2Size;		// NumSamples * NumChannels * BitsPerSample / 8.
			uint8_t														ui8Data[1];				// Sample data (length = uiSubchunk2Size).
		};

		// SAMPL chunk.
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

		// LIST chunk.
		struct PW_LIST_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			union {
				char8_t													cName[4];
				uint32_t												uiTypeId;
			}															u;
			uint8_t														ui8Data[1];
		};

		// ID3  chunk.
		struct PW_ID3_CHUNK {
			PW_CHUNK_HEADER												chHeader;
			char8_t														sName[3];
			uint16_t													ui16Version;
			uint8_t														ui8Flags;
			uint32_t													ui32Size;
			uint8_t														ui8Data[1];
		};

		// INST chunk.
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

		// A chunk entry.
		struct PW_CHUNK_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiName;
			}															u;
			uint32_t													uiOffset;
			uint32_t													uiSize;
		};

		// A LIST entry.
		struct PW_LIST_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			std::u8string												sText;
		};

		// An ID3 entry.
		struct PW_ID3_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			uint16_t													ui16Flags;
			std::u8string												sValue;
		};

		// A DISP entry.
		struct PW_DISP_ENTRY {
			union {
				char8_t													cName[4];
				uint32_t												uiIfoId;
			}															u;
			uint32_t													ui32Size;
			uint32_t													ui32Type;
			std::vector<uint8_t>										vValue;
		};

		// An INST entry.
		struct PW_INST_ENTRY {
			uint8_t														ui8UnshiftedNote;
			uint8_t														ui8FineTune;
			uint8_t														ui8Gain;
			uint8_t														ui8LowNote;
			uint8_t														ui8HiNote;
			uint8_t														ui8LowVel;
			uint8_t														ui8HiVel;
		};


		// == Members.
		/** The format. */
		PW_FORMAT														m_fFormat;
		/** The number of channels. */
		uint16_t														m_uiNumChannels;		// 1 = mono, 2 = stereo, etc.
		/** The sample rate. */
		uint32_t														m_uiSampleRate;			// 44,100, 48,000, etc.
		/** The bits per sample. */
		uint16_t														m_uiBitsPerSample;		// 8, 16, etc.
		/** The bytes per sample. */
		uint16_t														m_uiBytesPerSample;		// m_uiBitsPerSample / 8.
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
		bool															LoadData( const PW_DATA_CHUNK * _pdcChunk );

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
		 * Converts a bunch of 8-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		bool															Pcm8ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Converts a bunch of 16-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		bool															Pcm16ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Converts a bunch of 24-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		bool															Pcm24ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Converts a bunch of 32-bit PCM samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		bool															Pcm32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Converts a bunch of 32-bit float samples to double.
		 *
		 * \param _ui32From Starting sample index.
		 * \param _ui32To Ending sample index.
		 * \param _uiChan The channel whose sample data is to be obtained.
		 * \param _vResult The vector containing the samples.
		 * \return Returns true if the vector was able to hold all of the values.
		 */
		bool															F32ToF64( uint32_t _ui32From, uint32_t _ui32To, uint16_t _uiChan, lwtrack &_vResult ) const;

		/**
		 * Converts a batch of F64 samples to PCM samples.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns trye if all samples were added to the buffer.
		 */
		static bool														BatchF64ToPcm8( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst );

		/**
		 * Converts a batch of F64 samples to PCM samples.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns trye if all samples were added to the buffer.
		 */
		static bool														BatchF64ToPcm16( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst );

		/**
		 * Converts a batch of F64 samples to PCM samples.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns trye if all samples were added to the buffer.
		 */
		static bool														BatchF64ToPcm24( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst );

		/**
		 * Converts a batch of F64 samples to PCM samples.
		 *
		 * \param _vSrc The samples to convert.
		 * \param _vDst The buffer to which to convert the samples.
		 * \return Returns trye if all samples were added to the buffer.
		 */
		static bool														BatchF64ToPcm32( const lwaudio &_vSrc, std::vector<uint8_t> &_vDst );

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
	};

}	// namespace pw
