#include "AkaoInstr.h"

QString AkaoInstr::toString() const
{
	if (_op < 0xA0) {
		return QString("Note %1").arg(_op);
	}

	switch (Instruction(_op)) {
	case Finished:
		return QString("Finished");
	case LoadInstrument:
		return QString("LoadInstrument(%1)").arg(_params.param8.p1);
	case OverwriteNextNoteLength:
		return QString("OverwriteNextNoteLength(%1)").arg(_params.param8.p1);
	case ChannelMasterVolume:
		return QString("ChannelMasterVolume(%1)").arg(_params.param8.p1);
	case PitchBendSlide:
		return QString("PitchBendSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case SetOctave:
		return QString("SetOctave(%1)")
		        .arg(_params.param8.p1);
	case IncreaseOctave:
		return QString("IncreaseOctave");
	case DecreaseOctave:
		return QString("DecreaseOctave");
	case ChannelVolume:
		return QString("ChannelVolume(%1)").arg(_params.param8.p1);
	case ChannelVolumeSlide:
		return QString("ChannelVolumeSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case SetChannelPan:
		return QString("SetChannelPan(%1)").arg(_params.param8.p1);
	case SetChannelPanSlide:
		return QString("SetChannelPanSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case NoiseClockFrequency:
		return QString("NoiseClockFrequency(%1)").arg(_params.param8.p1);
	case AdsrAttackRate:
		return QString("AdsrAttackRate(%1)").arg(_params.param8.p1);
	case AdsrDecayRate:
		return QString("AdsrDecayRate(%1)").arg(_params.param8.p1);
	case AdsrSustainLevel:
		return QString("AdsrSustainLevel(%1)").arg(_params.param8.p1);
	case AdsrDecayRateAndSustainLevel:
		return QString("AdsrDecayRateAndSustainLevel(%1, %2)")
		        .arg(_params.param8.p1)
		        .arg(_params.param8.p2);
	case AdsrSustainRate:
		return QString("AdsrSustainRate(%1)").arg(_params.param8.p1);
	case AdsrReleaseRate:
		return QString("AdsrReleaseRate(%1)").arg(_params.param8.p1);
	case AdsrReset:
		return QString("AdsrReset");
	case VibratoChannelPitchLfo:
		return QString("VibratoChannelPitchLfo(%1, %2, %3)")
		        .arg(_params.param8.p1)
		        .arg(_params.param8.p2)
		        .arg(_params.param8.p3);
	case VibratoDepth:
		return QString("VibratoDepth(%1)")
		        .arg(_params.param8.p1);
	case TurnOffVibrato:
		return QString("TurnOffVibrato");
	case AdsrAttackMode:
		return QString("AdsrAttackMode(%1)")
		        .arg(_params.param8.p1);
	case TremoloChannelVolumeLfo:
		return QString("TremoloChannelVolumeLfo(%1, %2, %3)")
		        .arg(_params.param8.p1)
		        .arg(_params.param8.p2)
		        .arg(_params.param8.p3);
	case TremoloDepth:
		return QString("TremoloDepth(%1)")
		        .arg(_params.param8.p1);
	case TurnOffTremolo:
		return QString("TurnOffTremolo");
	case AdsrSustainMode:
		return QString("AdsrSustainMode(%1)")
		        .arg(_params.param8.p1);
	case ChannelPanLfo:
		return QString("ChannelPanLfo(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case ChannelPanLfoDepth:
		return QString("ChannelPanLfoDepth(%1)")
		        .arg(_params.param8.p1);
	case TurnOffChannelPanLfo:
		return QString("TurnOffChannelPanLfo");
	case AdsrReleaseMode:
		return QString("AdsrReleaseMode(%1)")
		        .arg(_params.param8.p1);
	case ChannelTransposeAbsolute:
		return QString("ChannelTransposeAbsolute(%1)")
		        .arg(_params.param8.p1);
	case ChannelTransposeRelative:
		return QString("ChannelTransposeRelative(%1)")
		        .arg(_params.param8.p1);
	case TurnOnReverb:
		return QString("TurnOnReverb");
	case TurnOffReverb:
		return QString("TurnOffReverb");
	case TurnOnNoise:
		return QString("TurnOnNoise");
	case TurnOffNoise:
		return QString("TurnOffNoise");
	case TurnOnFrequencyModulation:
		return QString("TurnOnFrequencyModulation");
	case TurnOffFrenquencyModulation:
		return QString("TurnOffFrenquencyModulation");
	case LoopPoint:
		return QString("LoopPoint");
	case ReturnToLoopPointUpToNTimes:
		return QString("ReturnToLoopPointUpToNTimes(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case ReturnToLoopPoint:
		return QString("ReturnToLoopPoint");
	case ResetSoundEffects:
		return QString("ResetSoundEffects");
	case TurnOnLegato:
		return QString("TurnOnLegato");
	case DoNothingCD:
		return QString("DoNothingCD");
	case TurnOnNoiseAndToggleNoiseOnOffAfterAPeriodOfTime:
		return QString("TurnOnNoiseAndToggleNoiseOnOffAfterAPeriodOfTime(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case ToggleNoiseOnOffAfterAPeriodOfTime:
		return QString("ToggleNoiseOnOffAfterAPeriodOfTime(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case TurnOnFullLengthNoteMode:
		return QString("TurnOnFullLengthNoteMode");
	case DoNothingD1:
		return QString("DoNothingD1");
	case ToggleFreqModulationLaterAndTurnOnFrequencyModulation:
		return QString("ToggleFreqModulationLaterAndTurnOnFrequencyModulation(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case ToggleFreqModulationLater:
		return QString("ToggleFreqModulationLater(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case TurnOnPlaybackRateSideChain:
		return QString("TurnOnPlaybackRateSideChain");
	case TurnOffPlaybackRateSideChain:
		return QString("TurnOffPlaybackRateSideChain");
	case TurnOnPitchVolumeSideChain:
		return QString("TurnOnPitchVolumeSideChain");
	case TurnOffPitchVOlumeSideChain:
		return QString("TurnOffPitchVOlumeSideChain");
	case ChannelFineTuningAbsolute:
		return QString("ChannelFineTuningAbsolute(%1)").arg(_params.param8.p1);
	case ChannelFineTuningRelative:
		return QString("ChannelFineTuningRelative(%1)").arg(_params.param8.p1);
	case TurnOnPortamento:
		return QString("TurnOnPortamento(%1)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1);
	case TurnOffPortamento:
		return QString("TurnOffPortamento");
	case FixNoteLength:
		return QString("FixNoteLength(%1)").arg(_params.param8.p1);
	case VibratoDepthSlide:
		return QString("VibratoDepthSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case TremoloDepthSlide:
		return QString("TremoloDepthSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case ChannelPanLfoDepthSlide:
		return QString("ChannelPanLfoDepthSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case Tempo:
		return QString("Tempo(%1)").arg(_params.param16.p1);
	case TempoSlide:
		return QString("TempoSlide(%1, %2)")
		        .arg(_params.param816.p1 == 0 ? 256 : _params.param816.p1)
		        .arg(_params.param816.p2);
	case ReverbDepth:
		return QString("ReverbDepth(%1)").arg(_params.param16.p1);
	case ReverbDepthSlide:
		return QString("ReverbDepthSlide(%1, %2)")
		        .arg(_params.param816.p1 == 0 ? 256 : _params.param816.p1)
		        .arg(_params.param816.p2);
	case TurnOnDrumMode:
		return QString("TurnOnDrumMode(%1)").arg(_params.param16.p1);
	case TurnOffDrumMode:
		return QString("TurnOffDrumMode");
	case UnconditionalJump:
		return QString("UnconditionalJump(%1)").arg(qint16(_params.param16.p1));
	case CpuConditionalJump:
		return QString("CpuConditionalJump(%1, %2)")
		        .arg(_params.param816.p1)
		        .arg(qint16(_params.param816.p2));
	case JumpOnTheNthRepeat:
		return QString("JumpOnTheNthRepeat(%1, %2)")
		        .arg(_params.param816.p1 == 0 ? 256 : _params.param816.p1)
		        .arg(qint16(_params.param816.p2));
	case BreakTheLoopOnTheNthRepeat:
		return QString("BreakTheLoopOnTheNthRepeat(%1, %2)")
		        .arg(_params.param816.p1 == 0 ? 256 : _params.param816.p1)
		        .arg(_params.param816.p2);
	case LoadInstrumentNoAttackSample:
		return QString("LoadInstrumentNoAttackSample(%1)").arg(_params.param8.p1);
	case SetPause:
		return QString("SetPause");
	case TurnOnOverlayVoice:
		return QString("TurnOnOverlayVoice(%1, %2")
		        .arg(_params.param8.p1)
		        .arg(_params.param8.p2);
	case TurnOffOverlayVoice:
		return QString("TurnOffOverlayVoice");
	case OverlayVolumeBalance:
		return QString("OverlayVolumeBalance(%1)").arg(_params.param8.p1);
	case OverlayVolumeBalanceSlide:
		return QString("OverlayVolumeBalanceSlide(%1, %2)")
		        .arg(_params.param8.p1 == 0 ? 256 : _params.param8.p1)
		        .arg(_params.param8.p2);
	case TurnOnAlternateVoice:
		return QString("TurnOnAlternateVoice(%1)").arg(_params.param8.p1);
	case TurnOffAlternateVoice:
		return QString("TurnOffAlternateVoice");
	case TimeSignature:
		return QString("TimeSignature(%1, %2)")
		        .arg(_params.param8.p1)
		        .arg(_params.param8.p2);
	case MeasureNumber:
		return QString("MeasureNumber(%1)").arg(_params.param16.p1);
	case Label:
		return QString("Label(%1)").arg(_params.param32);
	}

	return QString("Do Nothing %1").arg(_op);
}
