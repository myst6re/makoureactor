#ifndef AKAOINSTR_H
#define AKAOINSTR_H

#include <QtCore>

struct AkaoParam8 {
	quint8 p1, p2, p3, p4;
};

struct AkaoParam16 {
	quint16 p1, p3;
};

struct AkaoParam816 {
	quint8 p1;
	quint16 p2;
	quint8 p4;
};

union AkaoParams {
	AkaoParam8 param8;
	AkaoParam16 param16;
	AkaoParam816 param816;
	quint32 param32;
};

class AkaoInstr
{
public:
	enum Instruction {
		Finished = 0xA0,
		LoadInstrument = 0xA1,
		OverwriteNextNoteLength = 0xA2,
		ChannelMasterVolume = 0xA3,
		PitchBendSlide = 0xA4,
		SetOctave = 0xA5,
		IncreaseOctave = 0xA6,
		DecreaseOctave = 0xA7,
		ChannelVolume = 0xA8,
		ChannelVolumeSlide = 0xA9,
		SetChannelPan = 0xAA,
		SetChannelPanSlide = 0xAB,
		NoiseClockFrequency = 0xAC,
		AdsrAttackRate = 0xAD,
		AdsrDecayRate = 0xAE,
		AdsrSustainLevel = 0xAF,
		AdsrDecayRateAndSustainLevel = 0xB0,
		AdsrSustainRate = 0xB1,
		AdsrReleaseRate = 0xB2,
		AdsrReset = 0xB3,
		VibratoChannelPitchLfo = 0xB4,
		VibratoDepth = 0xB5,
		TurnOffVibrato = 0xB6,
		AdsrAttackMode = 0xB7,
		TremoloChannelVolumeLfo = 0xB8,
		TremoloDepth = 0xB9,
		TurnOffTremolo = 0xBA,
		AdsrSustainMode = 0xBB,
		ChannelPanLfo = 0xBC,
		ChannelPanLfoDepth = 0xBD,
		TurnOffChannelPanLfo = 0xBE,
		AdsrReleaseMode = 0xBF,
		ChannelTransposeAbsolute = 0xC0,
		ChannelTransposeRelative = 0xC1,
		TurnOnReverb = 0xC2,
		TurnOffReverb = 0xC3,
		TurnOnNoise = 0xC4,
		TurnOffNoise = 0xC5,
		TurnOnFrequencyModulation = 0xC6,
		TurnOffFrenquencyModulation = 0xC7,
		LoopPoint = 0xC8,
		ReturnToLoopPointUpToNTimes = 0xC9,
		ReturnToLoopPoint = 0xCA,
		ResetSoundEffects = 0xCB,
		TurnOnLegato = 0xCC,
		DoNothingCD = 0xCD,
		TurnOnNoiseAndToggleNoiseOnOffAfterAPeriodOfTime = 0xCE,
		ToggleNoiseOnOffAfterAPeriodOfTime = 0xCF,
		TurnOnFullLengthNoteMode = 0xD0,
		DoNothingD1 = 0xD1,
		ToggleFreqModulationLaterAndTurnOnFrequencyModulation = 0xD2,
		ToggleFreqModulationLater = 0xD3,
		TurnOnPlaybackRateSideChain = 0xD4,
		TurnOffPlaybackRateSideChain = 0xD5,
		TurnOnPitchVolumeSideChain = 0xD6,
		TurnOffPitchVOlumeSideChain = 0xD7,
		ChannelFineTuningAbsolute = 0xD8,
		ChannelFineTuningRelative = 0xD9,
		TurnOnPortamento = 0xDA,
		TurnOffPortamento = 0xDB,
		FixNoteLength = 0xDC,
		VibratoDepthSlide = 0xDD,
		TremoloDepthSlide = 0xDE,
		ChannelPanLfoDepthSlide = 0xDF,
		Tempo = 0xE8,
		TempoSlide = 0xE9,
		ReverbDepth = 0xEA,
		ReverbDepthSlide = 0xEB,
		TurnOnDrumMode = 0xEC,
		TurnOffDrumMode = 0xED,
		UnconditionalJump = 0xEE,
		CpuConditionalJump = 0xEF,
		JumpOnTheNthRepeat = 0xF0,
		BreakTheLoopOnTheNthRepeat = 0xF1,
		LoadInstrumentNoAttackSample = 0xF2,
		SetPause = 0xF3,
		TurnOnOverlayVoice = 0xF4,
		TurnOffOverlayVoice = 0xF5,
		OverlayVolumeBalance = 0xF6,
		OverlayVolumeBalanceSlide = 0xF7,
		TurnOnAlternateVoice = 0xF8,
		TurnOffAlternateVoice = 0xF9,
		TimeSignature = 0xFD,
		MeasureNumber = 0xFE,
		Label = 0x100
	};

	explicit  AkaoInstr(quint8 op, AkaoParams params = AkaoParams()) :
	    _op(op), _params(params) {}
	explicit  AkaoInstr(Instruction op, AkaoParams params = AkaoParams()) :
	    _op(op), _params(params) {}
	inline quint16 op() const {
		return _op;
	}
	inline void setOp(quint16 op) {
		_op = op;
	}
	inline void setOp(Instruction op) {
		_op = quint16(op);
	}
	inline AkaoParams params() const {
		return _params;
	}
	inline void setParams(const AkaoParams &params) {
		_params = params;
	}
	QString toString() const;
private:
	quint16 _op;
	AkaoParams _params;
};

#endif // AKAOINSTR_H
