/**
 * Copyright L. Spiro 2024
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: Useful utility functions.
 */


#pragma once

#include "../OS/PWOs.h"
#include "../OS/PWFeatureSet.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
//#include <intrin.h>
#include <numbers>
#include <stdexcept>
#include <string>
#include <vector>


#ifndef PW_ELEMENTS
#define PW_ELEMENTS( x )									((sizeof( x ) / sizeof( 0[x] )) / (static_cast<size_t>(!(sizeof( x ) % sizeof(0[x])))))
#endif	// #ifndef PW_ELEMENTS

#ifndef PW_PI
#define PW_PI												3.14159265358979323846264338327950288419716939937510	// You can tell how cool a programmer is by how many digits she puts in the PI macro.  True story.
#endif	// #ifndef PW_PI

#ifndef M_PI
#define M_PI												PW_PI
#endif	// #ifndef M_PI


namespace pw {

	/**
	 * Class CUtilities
	 * \brief Useful utility functions.
	 *
	 * Description: Useful utility functions.
	 */
	class CUtilities {
	public :
		// == Functions.
		/**
		 * Creates a string with _cReplaceMe replaced with _cWithMe inside _s16String.
		 *
		 * \param _s16String The string in which replacements are to be made.
		 * \param _cReplaceMe The character to replace.
		 * \param _cWithMe The character with which to replace _cReplaceMe.
		 * \return Returns the new string with the given replacements made.
		 */
		template <typename _tType = std::u16string>
		static _tType										Replace( const _tType &_s16String, _tType::value_type _cReplaceMe, _tType::value_type _cWithMe ) {
			_tType s16Copy = _s16String;
			auto aFound = s16Copy.find( _cReplaceMe );
			while ( aFound != _tType::npos ) {
				s16Copy[aFound] = _cWithMe;
				aFound = s16Copy.find( _cReplaceMe, aFound + 1 );
			}
			return s16Copy;
		}

		/**
		 * Creates a string with _cReplaceMe replaced with _cWithMe inside _s16String.
		 *
		 * \param _s16String The string in which replacements are to be made.
		 * \param _cReplaceMe The character to replace.
		 * \param _cWithMe The character with which to replace _cReplaceMe.
		 * \return Returns the new string with the given replacements made.
		 */
		template <typename _tType = std::u16string>
		static _tType										Replace( const _tType &_s16String, const _tType &_cReplaceMe, const _tType &_cWithMe ) {
			_tType sCopy = _s16String;
			const size_t sLen = _cReplaceMe.size();
			size_t sIdx = sCopy.find( _cReplaceMe );
			while ( _tType::npos != sIdx ) {
				sCopy = sCopy.replace( sIdx, sLen, _cWithMe );
				sIdx = sCopy.find( _cReplaceMe );
			}
			return sCopy;
		}

		/**
		 * Replaces a string inside a data vector.
		 * 
		 * \param _vData The buffer of data in which to replace a string.
		 * \param _sReplaceMe The string to replace.
		 * \param _sWithMe The string with which to replace _sReplaceMe inside _vData.
		 * \return Returns a reference to _vData.
		 **/
		static std::vector<uint8_t> &						Replace( std::vector<uint8_t> &_vData, const std::string &_sReplaceMe, const std::string &_sWithMe );

		/**
		 * Converts a UTF-8 string to a UTF-16 string.  The resulting string may have allocated more characters than necessary but will be terminated with a NULL.
		 *
		 * \param _pcString String to convert.
		 * \param _pbErrored If not nullptr, holds a returned boolean indicating success or failure of the conversion.
		 * \return Returns the converted UTF-16 string.
		 */
		static std::u16string								Utf8ToUtf16( const char8_t * _pcString, bool * _pbErrored = nullptr );

		/**
		 * Converts a UTF-16 string to a UTF-8 string.  The resulting string may have allocated more characters than necessary but will be terminated with a NULL.
		 *
		 * \param _pcString String to convert.
		 * \param _pbErrored If not nullptr, holds a returned boolean indicating success or failure of the conversion.
		 * \return Returns the converted UTF-8 string.
		 */
		static std::u8string								Utf16ToUtf8( const char16_t * _pcString, bool * _pbErrored = nullptr );

		/**
		 * Converts a value to a string.
		 * 
		 * \param _tVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		template <typename _tType>
		static std::u16string								ToString( _tType _tVal, size_t _stDigits = 0 ) {
			std::wstring wsNumber = std::to_wstring( _tVal );
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			while ( usNumber.size() < _stDigits ) {
				usNumber.insert( usNumber.begin(), u'0' );
			}
			return usNumber;
		}

		/**
		 * Converts a value to a string.
		 * 
		 * \param _pwcVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		static std::u16string								ToString( const wchar_t * _pwcVal ) {
			std::wstring wsNumber = _pwcVal;
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			return usNumber;
		}

		/**
		 * Converts a value to a string.
		 * 
		 * \param _pcVal The value to convert.
		 * \param _stDigits The number of digits to which to pad.
		 * \return Returns the string form of the given value.
		 **/
		static std::u16string								ToString( const char * _pcVal ) {
			std::string wsNumber = _pcVal;
			std::u16string usNumber( wsNumber.begin(), wsNumber.end() );
			return usNumber;
		}

		/**
		 * Converts an * string to a std::wstring.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::wstring							XStringToWString( const _tCharType * _pwcStr, size_t _sLen ) {
			std::wstring u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<wchar_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Converts an * string to a std::u16string.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::u16string						XStringToU16String( const _tCharType * _pwcStr, size_t _sLen ) {
			std::u16string u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<char16_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Converts an * string to a std::u8string.  Call inside try{}catch(...){}.
		 * 
		 * \param _pwcStr The string to convert.
		 * \param _sLen The length of the string or 0.
		 * \return Returns the converted string.
		 **/
		template <typename _tCharType>
		static inline std::u8string							XStringToU8String( const _tCharType * _pwcStr, size_t _sLen ) {
			std::u8string u16Tmp;
			if ( _sLen ) {
				u16Tmp.reserve( _sLen );
			}
			for ( size_t I = 0; (I < _sLen) || (_sLen == 0 && !_pwcStr[I]); ++I ) {
				u16Tmp.push_back( static_cast<char8_t>(_pwcStr[I]) );
			}
			return u16Tmp;
		}

		/**
		 * Reads a line from a buffer.
		 * 
		 * \param _vBuffer The buffer from which to read a line.
		 * \param _stPos The current position inside the buffer, updated on return.
		 * \return Returns the line read from the file.
		 **/
		static std::string									ReadLine( const std::vector<uint8_t> &_vBuffer, size_t &_stPos );

		/**
		 * Tokenizes a string by a given character.
		 * 
		 * \param _sString The string to tokenize.
		 * \param _vtDelimiter The character by which to deliminate the string into sections.
		 * \param _bAllowEmptyStrings If true, the return value could contain empty strings when the delimiter is found more than once in a row.
		 * \return DESC
		 **/
		static std::vector<std::string>						Tokenize( const std::string &_sString, std::string::value_type _vtDelimiter, bool _bAllowEmptyStrings );

#ifdef __AVX512F__
		/**
		 * Horizontally adds all the floats in a given AVX-512 register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m512 _mReg ) {
			return _mm512_reduce_add_ps( _mReg );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX512_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 16 floats at a time.  But like, 2 16-at-a-times at a time.
			for ( ; I + 31 < _sTotal; I += 32 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 32 );
				PW_PREFETCH_LINE( _pfOp1 + 32 );

				// Load and add the first 16 floats.
				__m512 mA1 = _mm512_loadu_ps( &_pfOp0[I] );
				__m512 mB1 = _mm512_loadu_ps( &_pfOp1[I] );
				__m512 mC1 = _mm512_add_ps( mA1, mB1 );
				_mm512_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 16 floats.
				__m512 mA2 = _mm512_loadu_ps( &_pfOp0[I+16] );
				__m512 mB2 = _mm512_loadu_ps( &_pfOp1[I+16] );
				__m512 mC2 = _mm512_add_ps( mA2, mB2 );
				_mm512_storeu_ps( &_pfOut[I+16], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX512( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 16 floats at a time.  But like, 2 16-at-a-times at a time.
			for ( ; I + 31 < _sTotal; I += 32 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 32 );
				PW_PREFETCH_LINE( _pfOp1 + 32 );

				// Load and add the first 16 floats.
				__m512 mA1 = _mm512_load_ps( &_pfOp0[I] );
				__m512 mB1 = _mm512_load_ps( &_pfOp1[I] );
				__m512 mC1 = _mm512_add_ps( mA1, mB1 );
				_mm512_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 16 floats.
				__m512 mA2 = _mm512_load_ps( &_pfOp0[I+16] );
				__m512 mB2 = _mm512_load_ps( &_pfOp1[I+16] );
				__m512 mC2 = _mm512_add_ps( mA2, mB2 );
				_mm512_store_ps( &_pfOut[I+16], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __AVX512F__

#ifdef __AVX__
		/**
		 * Horizontally adds all the floats in a given AVX register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m256 &_mReg ) {
			__m256 mTmp = _mm256_add_ps( _mReg, _mm256_permute2f128_ps( _mReg, _mReg, 1 ) );
			mTmp = _mm256_hadd_ps( mTmp, mTmp );
			mTmp = _mm256_hadd_ps( mTmp, mTmp );
			return _mm256_cvtss_f32( mTmp );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 8 floats at a time.  But like, 2 8-at-a-times at a time
			for ( ; I + 15 < _sTotal; I += 16 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 16 );
				PW_PREFETCH_LINE( _pfOp1 + 16 );

				// Load and add the first 8 floats
				__m256 mA1 = _mm256_loadu_ps( &_pfOp0[I] );
				__m256 mB1 = _mm256_loadu_ps( &_pfOp1[I] );
				__m256 mC1 = _mm256_add_ps( mA1, mB1 );
				_mm256_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 8 floats
				__m256 mA2 = _mm256_loadu_ps( &_pfOp0[I+8] );
				__m256 mB2 = _mm256_loadu_ps( &_pfOp1[I+8] );
				__m256 mC2 = _mm256_add_ps( mA2, mB2 );
				_mm256_storeu_ps( &_pfOut[I+8], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_AVX( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 8 floats at a time.  But like, 2 8-at-a-times at a time.
			for ( ; I + 15 < _sTotal; I += 16 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 16 );
				PW_PREFETCH_LINE( _pfOp1 + 16 );

				// Load and add the first 8 floats.
				__m256 mA1 = _mm256_load_ps( &_pfOp0[I] );
				__m256 mB1 = _mm256_load_ps( &_pfOp1[I] );
				__m256 mC1 = _mm256_add_ps( mA1, mB1 );
				_mm256_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 8 floats.
				__m256 mA2 = _mm256_load_ps( &_pfOp0[I+8] );
				__m256 mB2 = _mm256_load_ps( &_pfOp1[I+8] );
				__m256 mC2 = _mm256_add_ps( mA2, mB2 );
				_mm256_store_ps( &_pfOut[I+8], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __AVX__

#ifdef __SSE4_1__
		/**
		 * Horizontally adds all the floats in a given SSE register.
		 * 
		 * \param _mReg The register containing all of the values to sum.
		 * \return Returns the sum of all the floats in the given register.
		 **/
		static inline float									HorizontalSum( __m128 &_mReg ) {
			__m128 mAddH1 = _mm_hadd_ps( _mReg, _mReg );
			__m128 mAddH2 = _mm_hadd_ps( mAddH1, mAddH1 );
			return _mm_cvtss_f32( mAddH2 );
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (unaligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_SSE4_U( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 4 floats at a time.  But like, 2 4-at-a-times at a time.
			for ( ; I + 7 < _sTotal; I += 8 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				// Load and add the first 4 floats.
				__m128 mA1 = _mm_loadu_ps( &_pfOp0[I] );
				__m128 mB1 = _mm_loadu_ps( &_pfOp1[I] );
				__m128 mC1 = _mm_add_ps( mA1, mB1 );
				_mm_storeu_ps( &_pfOut[I], mC1 );

				// Load and add the next 4 floats.
				__m128 mA2 = _mm_loadu_ps( &_pfOp0[I+4] );
				__m128 mB2 = _mm_loadu_ps( &_pfOp1[I+4] );
				__m128 mC2 = _mm_add_ps( mA2, mB2 );
				_mm_storeu_ps( &_pfOut[I+4], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Sums a pair of arrays of floats to a given destination (aligned).
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray_SSE4( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			size_t I = 0;

			// Prefetch the first set of data.
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			// Process 4 floats at a time.  But like, 2 4-at-a-times at a time.
			for ( ; I + 7 < _sTotal; I += 8 ) {
				// Prefetch the next set of data.
				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				// Load and add the first 4 floats.
				__m128 mA1 = _mm_load_ps( &_pfOp0[I] );
				__m128 mB1 = _mm_load_ps( &_pfOp1[I] );
				__m128 mC1 = _mm_add_ps( mA1, mB1 );
				_mm_store_ps( &_pfOut[I], mC1 );

				// Load and add the next 4 floats.
				__m128 mA2 = _mm_load_ps( &_pfOp0[I+4] );
				__m128 mB2 = _mm_load_ps( &_pfOp1[I+4] );
				__m128 mC2 = _mm_add_ps( mA2, mB2 );
				_mm_store_ps( &_pfOut[I+4], mC2 );
			}

			// Handle remaining elements.
			for ( ; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}
#endif	// #ifdef __SSE4_1__

		/**
		 * Sums a pair of arrays of floats to a given destination.
		 * 
		 * \param _pfOp0 THe left array of operands.
		 * \param _pfOp1 The right array of operands.
		 * \param _pfOut The output array.
		 * \param _sTotal The number of floats in each array.
		 **/
		static inline void									SumArray( const float * _pfOp0, const float * _pfOp1, float * _pfOut, size_t _sTotal ) {
			PW_PREFETCH_LINE( _pfOp0 );
			PW_PREFETCH_LINE( _pfOp1 );

			while ( _sTotal >= 8 ) {
				// Let's do a loop of 8 at a time.
				_pfOut[0] = _pfOp0[0] + _pfOp1[0];
				_pfOut[1] = _pfOp0[1] + _pfOp1[1];
				_pfOut[2] = _pfOp0[2] + _pfOp1[2];
				_pfOut[3] = _pfOp0[3] + _pfOp1[3];
				_pfOut[4] = _pfOp0[4] + _pfOp1[4];

				PW_PREFETCH_LINE( _pfOp0 + 8 );
				PW_PREFETCH_LINE( _pfOp1 + 8 );

				_pfOut[5] = _pfOp0[5] + _pfOp1[5];
				_pfOut[6] = _pfOp0[6] + _pfOp1[6];
				_pfOut[7] = _pfOp0[7] + _pfOp1[7];
				_pfOp0 += 8;
				_pfOp0 += 8;
				_pfOp1 += 8;
				_sTotal -= 8;
			}
			// Finish the rest.
			for ( size_t I = 0; I < _sTotal; ++I ) {
				_pfOut[I] = _pfOp0[I] + _pfOp1[I];
			}
		}

		/**
		 * Standard sinc() function.
		 * 
		 * \param _dX The operand.
		 * \return Returns sin(x) / x.
		 **/
		static inline double								Sinc( double _dX ) {
			_dX *= std::numbers::pi;
			if ( _dX < 0.01 && _dX > -0.01 ) {
				return 1.0 + _dX * _dX * (-1.0 / 6.0 + _dX * _dX * 1.0 / 120.0);
			}

			return std::sin( _dX ) / _dX;
		}

		/**
		 * Is AVX supported?
		 *
		 * \return Returns true if AVX is supported.
		 **/
		static inline bool									IsAvxSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX 2 supported?
		 *
		 * \return Returns true if AVX is supported.
		 **/
		static inline bool									IsAvx2Supported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX2();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX-512F supported?
		 *
		 * \return Returns true if AVX-512F is supported.
		 **/
		static inline bool									IsAvx512FSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX512F();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is AVX-512BW supported?
		 *
		 * \return Returns true if AVX-512BW is supported.
		 **/
		static inline bool									IsAvx512BWSupported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::AVX512BW();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}

		/**
		 * Is SSE 4 supported?
		 *
		 * \return Returns true if SSE 4 is supported.
		 **/
		static inline bool									IsSse4Supported() {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER )
			return CFeatureSet::SSE41();
#else
			return false;
#endif	// #if defined( __i386__ ) || defined( __x86_64__ )
		}
		
		/**
		 * Converts a sample from a floating-point format to a uint8_t.  8-bit PCM data is expressed as an unsigned value over the range 0 to 255, 128 being an
		 *	audio output level of zero.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline uint8_t								SampleToUi8( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = (dClampedSample + 1.0) * 0.5 * 255.0;
			return static_cast<uint8_t>(std::round( dScaledSample ));
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  Note that both -32768 and -32767 are -1.0; a proper
		 *	conversion never generates -32768.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline int16_t								SampleToI16( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = dClampedSample * 32767.0;
			return static_cast<int16_t>(std::round( dScaledSample ));
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t while applying error-diffusion dithering.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  Note that both -32768 and -32767 are -1.0; a proper
		 *	conversion never generates -32768.
		 * 
		 * \param _dSample The sample to convert.
		 * \param _dError The running error state.
		 * \return Returns the converted sample.
		 **/
		static inline int16_t								SampleToI16_Dither( double _dSample, double &_dError ) {
			double dThis = _dSample + _dError;
			if PW_UNLIKELY( dThis < -1.0 ) { dThis = -1.0; }
			else if PW_UNLIKELY( dThis > 1.0 ) { dThis = 1.0; }
		
			int16_t i16Final = static_cast<int16_t>(std::round( static_cast<double>(dThis * 32767.0) ));

			double dScaled = double( i16Final );
			double dQuantized = dScaled * (1.0 / 32767.0);
			_dError = double( dThis - dQuantized );

			return i16Final;
		}
		
		/**
		 * Converts a sample from a floating-point format to an int32_t.  24-bit PCM data is expressed as a signed value over the
		 *	range -8388607 to 8388607, 0 being an audio output level of zero.  Note that both -8388608 and -8388607 are -1.0; a proper
		 *	conversion never generates -8388608.
		 *
		 * \param _dSample The sample to convert.
		 * \return Returns the converted sample.
		 **/
		static inline int32_t								SampleToI24( double _dSample ) {
			double dClampedSample = std::clamp( _dSample, -1.0, 1.0 );
			double dScaledSample = dClampedSample * 8388607.0;
			return static_cast<int32_t>(std::round( dScaledSample ));
		}

#ifdef __AVX2__
		/**
		 * Converts a sample from a floating-point format to a uint8_t.  8-bit PCM data is expressed as an unsigned value over the range 0 to 255, 128 being an
		 *	audio output level of zero.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pui8Dst Pointer to the output.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToUi8_AVX2( const float * _pfSample, uint8_t * _pui8Dst ) {
			auto vSamples	= _mm256_loadu_ps( _pfSample );
			auto vClamped	= _mm256_max_ps( _mm256_set1_ps( -1.0f ), _mm256_min_ps( vSamples, _mm256_set1_ps( 1.0f ) ) );
			auto vScaled	= _mm256_mul_ps( _mm256_mul_ps( _mm256_add_ps( vClamped, _mm256_set1_ps( 1.0f ) ), _mm256_set1_ps( 0.5f ) ), _mm256_set1_ps( 255.0f ) );
			auto vRounded	= _mm256_round_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
			// Paack into 8-bit PCM values.
			auto vInt32Vals = _mm256_cvtps_epi32( vRounded );

			auto vLo		= _mm256_castsi256_si128( vInt32Vals );				// Lower 128 bits.
			auto vHi		= _mm256_extracti128_si256( vInt32Vals, 1 );			// Upper 128 bits.
			auto vPacked16	= _mm_packus_epi32( vLo, vHi );

			// Finally, pack the 16-bit integers into 8-bit integers.
			auto vPacked8	= _mm_packus_epi16( vPacked16, vPacked16 );
			_mm_storeu_epi8( _pui8Dst, vPacked8 );
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  Note that both -32768 and -32767 are -1.0; a proper
		 *	conversion never generates -32768.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi16Dst Pointer to the output.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToI16_AVX2( const float * _pfSample, int16_t * _pi16Dst ) {
			auto vSamples	= _mm256_loadu_ps( _pfSample );
			auto vClamped	= _mm256_max_ps( _mm256_set1_ps( -1.0f ), _mm256_min_ps( vSamples, _mm256_set1_ps( 1.0f ) ) );
			auto vScaled	= _mm256_mul_ps( vClamped, _mm256_set1_ps( 32767.0f ) );
			auto vRounded	= _mm256_round_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );

			auto vInt32Vals	= _mm256_cvtps_epi32( vRounded );
    
			// Extract the lower and upper 128-bit lanes.
			auto vLo		= _mm256_castsi256_si128( vInt32Vals );				// Lower 4 integers.
			auto vHi		= _mm256_extracti128_si256( vInt32Vals, 1 );			// Upper 4 integers.
			auto vPacked16	= _mm_packs_epi32( vLo, vHi );
    
			_mm_storeu_epi16( _pi16Dst, vPacked16 );
		}

		/**
		 * Converts a sample from a floating-point format to an int32_t.  24-bit PCM data is expressed as a signed value over the
		 *	range -8388607 to 8388607, 0 being an audio output level of zero.  Note that both -8388608 and -8388607 are -1.0; a proper
		 *	conversion never generates -8388608.
		 *
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi32Dst Pointer to the output.  Must be aligned to a 32-byte boundary.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToI24_AVX2( const float * _pfSample, int32_t * _pi32Dst ) {
			auto vSamples	= _mm256_loadu_ps( _pfSample );
			auto vClamped	= _mm256_max_ps( _mm256_set1_ps( -1.0f ), _mm256_min_ps( vSamples, _mm256_set1_ps( 1.0f ) ) );
			auto vScaled	= _mm256_mul_ps( vClamped, _mm256_set1_ps( 8388607.0f ) );
			auto vRounded	= _mm256_round_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );

			auto vInt32Vals	= _mm256_cvtps_epi32( vRounded );
			_mm256_store_si256( reinterpret_cast<__m256i *>(_pi32Dst), vInt32Vals );
		}
#endif	// #ifdef __AVX2__

#ifdef __AVX512F__
		/**
		 * Converts a sample from a floating-point format to a uint8_t.  8-bit PCM data is expressed as an unsigned value over the range 0 to 255, 128 being an
		 *	audio output level of zero.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pui8Dst Pointer to the output.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToUi8_AVX512( const float * _pfSample, uint8_t * _pui8Dst ) {
			auto vSamples   = _mm512_loadu_ps( _pfSample );
			auto vClamped   = _mm512_max_ps( _mm512_set1_ps( -1.0f ), _mm512_min_ps( vSamples, _mm512_set1_ps( 1.0f ) ) );
			auto vScaled    = _mm512_mul_ps( _mm512_mul_ps( _mm512_add_ps( vClamped, _mm512_set1_ps( 1.0f ) ), _mm512_set1_ps( 0.5f ) ), _mm512_set1_ps( 255.0f ) );
			auto vRounded	= _mm512_roundscale_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
			// Pack into 8-bit PCM values.
			auto vInt32Vals = _mm512_cvtps_epi32( vRounded );
			
			// Extract lower and upper 256-bit lanes.
			auto vLo256     = _mm512_castsi512_si256( vInt32Vals );					// Lower 256 bits.
			auto vHi256     = _mm512_extracti64x4_epi64( vInt32Vals, 1 );			// Upper 256 bits.
			auto vPacked16  = _mm256_packus_epi32( vLo256, vHi256 );				// 16-bit integers.
			
			// Pack the 16-bit integers into 8-bit integers.
			auto vPacked16Lo = _mm256_castsi256_si128( vPacked16 );
			auto vPacked16Hi = _mm256_extracti128_si256( vPacked16, 1 );
			auto vPacked8    = _mm_packus_epi16( vPacked16Lo, vPacked16Hi );
			_mm_storeu_si128( reinterpret_cast<__m128i *>( _pui8Dst ), vPacked8 );
		}

		/**
		 * Converts a sample from a floating-point format to an int16_t.  16-bit PCM data is expressed as a signed value over the
		 *	range -32768 to 32767, 0 being an audio output level of zero.  Note that both -32768 and -32767 are -1.0; a proper
		 *	conversion never generates -32768.
		 * 
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi16Dst Pointer to the output.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToI16_AVX512( const float * _pfSample, int16_t * _pi16Dst ) {
			auto vSamples   = _mm512_loadu_ps( _pfSample );
			auto vClamped   = _mm512_max_ps( _mm512_set1_ps( -1.0f ), _mm512_min_ps( vSamples, _mm512_set1_ps( 1.0f ) ) );
			auto vScaled    = _mm512_mul_ps( vClamped, _mm512_set1_ps( 32767.0f ) );
			auto vRounded	= _mm512_roundscale_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
			
			auto vInt32Vals = _mm512_cvtps_epi32( vRounded );
			// Extract lower and upper 256-bit lanes.
			auto vLo256     = _mm512_castsi512_si256( vInt32Vals );					// Lower 256 bits.
			auto vHi256     = _mm512_extracti64x4_epi64( vInt32Vals, 1 );			// Upper 256 bits.
			auto vPacked16  = _mm256_packs_epi32( vLo256, vHi256 );					// 16-bit integers.
			
			_mm256_storeu_si256( reinterpret_cast<__m256i *>( _pi16Dst ), vPacked16 );
		}

		/**
		 * Converts a sample from a floating-point format to an int32_t.  24-bit PCM data is expressed as a signed value over the
		 *	range -8388607 to 8388607, 0 being an audio output level of zero.  Note that both -8388608 and -8388607 are -1.0; a proper
		 *	conversion never generates -8388608.
		 *
		 * \param _pfSample Pointer to the samples to convert.
		 * \param _pi32Dst Pointer to the output.  Must be aligned to a 64-byte boundary.
		 * \return Returns the converted sample.
		 **/
		static inline void									SampleToI24_AVX512( const float * _pfSample, int32_t * _pi32Dst ) {
			auto vSamples   = _mm512_loadu_ps( _pfSample );
			auto vClamped   = _mm512_max_ps( _mm512_set1_ps( -1.0f ), _mm512_min_ps( vSamples, _mm512_set1_ps( 1.0f ) ) );
			auto vScaled    = _mm512_mul_ps( vClamped, _mm512_set1_ps( 8388607.0f ) );
			auto vRounded	= _mm512_roundscale_ps( vScaled, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC );
			
			auto vInt32Vals = _mm512_cvtps_epi32( vRounded );
			_mm512_store_si512( reinterpret_cast<__m512i *>( _pi32Dst ), vInt32Vals );
		}
#endif	// #ifdef __AVX512F__

		/**
		 * Computes the mean squared error between two waveforms.
		 * 
		 * \param _vA First waveform samples.
		 * \param _vB Second waveform samples.
		 * \returns Mean squared error.
		 */
		template <typename _tType = std::vector<double>>
		static double										GradeSimilarity( const _tType &_vA, const _tType &_vB ) {
			size_t sN = std::min( _vA.size(), _vB.size() );
			double dMSE = 0.0;
			for ( size_t I = 0; I < sN; ++I ) {
				double dErr		= _vA[I] - _vB[I];
				dMSE		   += dErr * dErr;
			}
			return dMSE / double( sN );
		}

		/**
		 * Finds the next fuzzy DC crossing cluster starting at a given sample index.
		 * 
		 * \param _vSamples Waveform samples.
		 * \param _sStartIndex Index to start search.
		 * \param _dThreshold Absolute amplitude threshold for considering zero.
		 * \returns Pair of (firstIndex, lastIndex) of the cluster, or (n,n) if none.
		 */
		template <typename _tType = std::vector<double>>
		static std::pair<size_t, size_t>					FindFuzzyDCCrossing( const _tType &_vSamples, size_t _sStartIndex, double _dThreshold ) {
			size_t sN = _vSamples.size();
			for ( size_t I = _sStartIndex + 1; I < sN; ++I ) {
				double dY0 = _vSamples[I-1];
				double dY1 = _vSamples[I];
				if ( (dY0 > 0 && dY1 < 0) || (dY0 < 0 && dY1 > 0) || (std::abs( dY1 ) <= _dThreshold) ) {
					size_t sFirst	= I - 1;
					size_t sLast	= I;
					size_t J		= I + 1;
					for ( ; J < sN; ++J ) {
						if ( std::abs( _vSamples[J] ) <= _dThreshold ) {
							sLast = J;
						}
						else { break; }
					}
					return std::make_pair( sFirst, sLast );
				}
			}
			return std::make_pair( sN, sN );
		}

		/**
		 * Loads a given region of a given WAV file and analyzes for the given number of HPF’s.  Call within try/catch.
		 * 
		 * \param _pwcPath The WAV file to load.
		 * \param _sNumPoles The number of HPF influences to find.
		 * \param _ui32Start The starting sample from the file to analyze.
		 * \param _ui64Total The number of samples to analyze.
		 * \return Returns true if the file was loaded and analyzed.  False indicates a missing or unloadable file.
		 * \throws std::runtime_error in cases of solver failures, std::bad_alloc on memory failure.
		 **/
		static bool											SolveForHpfs( const char16_t * _pwcPath, size_t _sNumPoles, uint32_t _ui32Start, uint32_t _ui32Total );
	};

}	// namespace pw
