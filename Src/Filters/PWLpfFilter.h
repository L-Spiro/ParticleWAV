/**
 * Copyright L. Spiro 2026
 *
 * Written by: Shawn (L. Spiro) Wilcoxen
 *
 * Description: A 1-pole LPF filter.
 */


#pragma once

#include "../OS/PWOs.h"
#include "PWAudioFilterBase.h"

namespace pw {

	/**
	 * Class CLpfFilter
	 * \brief A 1-pole LPF filter.
	 *
	 * Description: A 1-pole LPF filter.  Same form as BeesNES's CPoleFilter:
	 *	y[n] = y[n-1] * (1 - w) + (x[n] + x[n-1]) * (w / 2), where w = 2 * pi * Fc / SampleRate.
	 *	Stable while w < 2 (Fc < SampleRate / pi).
	 */
	class CLpfFilter : public CAudioFilterBase {
	public :
		inline CLpfFilter();
		virtual ~CLpfFilter() {}


		// == Functions.
		/**
		 * Sets the cut-off given a non-normalized cut-off frequency for an LPF and sample rate.
		 *
		 * \param _fFc The non-normalized cut-off frequency.
		 * \param _fSampleRate The input/output sample rate.
		 * \return Returns true.
		 **/
		virtual bool				CreateLpf( float _fFc, float _fSampleRate ) {
			if PW_UNLIKELY( Dirty( _fFc, _fSampleRate ) ) {
				SetCoefficients( double( _fFc ), double( _fSampleRate ) );
			}
			return true;
		}

		/**
		 * Sets the cut-off given a non-normalized cut-off frequency for an LPF and sample rate, keeping full double precision.  Used by the
		 *	LPF solver, whose cut-offs must not be rounded to float.
		 *
		 * \param _dFc The non-normalized cut-off frequency.
		 * \param _dSampleRate The input/output sample rate.
		 * \return Returns true.
		 **/
		bool						CreateLpfD( double _dFc, double _dSampleRate ) {
			if PW_UNLIKELY( _dFc != m_dFcD || _dSampleRate != m_dSampleRateD ) {
				m_dFcD = _dFc;
				m_dSampleRateD = _dSampleRate;
				SetCoefficients( _dFc, _dSampleRate );
			}
			return true;
		}

		/**
		 * Processes a single sample.
		 *
		 * \param _dSample The sample to process.
		 * \return Returns the filtered sample.
		 **/
		virtual double				Process( double _dSample ) {
			if PW_LIKELY( m_bEnabled ) {
				m_dLastSample = m_dLastSample * m_dGain + (_dSample + m_dLastInput) * m_dCornerFreq;
				m_dLastInput = _dSample;
				return m_dLastSample;
			}
			return _dSample;
		}

		/**
		 * Processes a buffer of samples.
		 *
		 * \param _vIn The input buffer.
		 * \param _vOut The output buffer.
		 **/
		template <typename _tType>
		void						Process( const _tType &_vIn, _tType &_vOut ) {
			_vOut.resize( _vIn.size() );
			for ( size_t I = 0; I < _vIn.size(); ++I ) {
				_vOut[I] = Process( _vIn[I] );
			}
		}

		/**
		 * Resets the state of the LPF without resetting the parameters.
		 **/
		virtual void				ResetState() {
			m_dLastSample = m_dLastInput = 0.0;
		}


	protected :
		// == Members.
		double						m_dLastSample;			/**< The last output sample. */
		double						m_dLastInput;			/**< The last input. */
		double						m_dGain;				/**< The gain control (a0). */
		double						m_dCornerFreq;			/**< The corner frequency (b1). */
		double						m_dFcD;					/**< The double-precision cut-off last passed to CreateLpfD(). */
		double						m_dSampleRateD;			/**< The double-precision sample rate last passed to CreateLpfD(). */


	private :
		// == Functions.
		/**
		 * Computes the coefficients from a cut-off and sample rate.
		 *
		 * \param _dFc The non-normalized cut-off frequency.
		 * \param _dSampleRate The input/output sample rate.
		 **/
		void						SetCoefficients( double _dFc, double _dSampleRate ) {
			m_dCornerFreq = (2.0 * std::numbers::pi * (_dFc / _dSampleRate));
			m_dGain = 1.0 - m_dCornerFreq;
			m_dCornerFreq *= 0.5;
			if ( _dFc >= _dSampleRate / 2.0 ) { m_bEnabled = false; }
		}
	};



	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	// DEFINITIONS
	// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
	inline CLpfFilter::CLpfFilter() :
		CAudioFilterBase(),
		m_dLastSample( 0.0 ),
		m_dLastInput( 0.0 ),
		m_dGain( 0.0 ),
		m_dCornerFreq( 0.0 ),
		m_dFcD( 0.0 ),
		m_dSampleRateD( 0.0 ) {
	}

	// == Functions.

}	// namespace pw
