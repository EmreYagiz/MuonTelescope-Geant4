#include "EventAction.hh"
#include "RunAction.hh"
#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

#include <algorithm>
#include <cmath>
#include <vector>

namespace B1
{

namespace
{
  // Simplified electronics / PMT waveform model.
  //
  // The model is intentionally phenomenological:
  // each photoelectron contributes a normalized single-PE voltage pulse
  // at its arrival time. The voltage is interpreted as the voltage across
  // a 50 ohm load after calibration.
  constexpr G4double kLoadResistanceOhm = 50.0;
  constexpr G4double kVoltageThreshold_mV = 100.0;

  // Approximate H10721-like pulse parameters.
  // Rise time and fall time control the pulse shape.
  const G4double kRiseTime = 0.57 * ns;
  const G4double kFallTime = 6.0 * ns;

  // Waveform sampling.
  const G4double kWaveformDt = 0.1 * ns;
  const G4double kWaveformDuration = 80.0 * ns;

  // Calibration: peak voltage contribution of a single PE after the 50 ohm
  // load/electronics chain. This is a simplified calibration constant, not a
  // full dynode gain simulation.
  constexpr G4double kSinglePEAmplitude_mV = 2.0;

  // Polarity convention:
  //   processed signal  : positive pulse, hit if Vpeak >= +100 mV
  //   raw oscilloscope  : negative pulse, hit if Vmin  <= -100 mV
  // The two signals have the same pulse time; only the sign convention changes.
}

EventAction::EventAction(RunAction* runAction)
: fRunAction(runAction)
{}

void EventAction::BeginOfEventAction(const G4Event*)
{
  fEdep1 = 0.;
  fEdep2 = 0.;

  fPhotonsPMT1 = 0;
  fPhotonsPMT2 = 0;

  fScintPhotonsPMT1 = 0;
  fScintPhotonsPMT2 = 0;

  fCerenkovPhotonsPMT1 = 0;
  fCerenkovPhotonsPMT2 = 0;

  fOtherOpticalPhotonsPMT1 = 0;
  fOtherOpticalPhotonsPMT2 = 0;

  fPEPMT1 = 0;
  fPEPMT2 = 0;

  fFirstPETimePMT1 = -1.;
  fFirstPETimePMT2 = -1.;

  fFirstPhotonTransitPMT1 = -1.;
  fFirstPhotonTransitPMT2 = -1.;

  fSumPhotonTransitPMT1 = 0.;
  fSumPhotonTransitPMT2 = 0.;

  fPETimesPMT1.clear();
  fPETimesPMT2.clear();

  fVpeakPMT1 = 0.;
  fVpeakPMT2 = 0.;

  fPulseTimePMT1 = -1.;
  fPulseTimePMT2 = -1.;

  fVoltageHitPMT1 = false;
  fVoltageHitPMT2 = false;
  fVoltageCoincidence = false;

  fRawVminPMT1 = 0.;
  fRawVminPMT2 = 0.;

  fRawPulseTimePMT1 = -1.;
  fRawPulseTimePMT2 = -1.;

  fRawVoltageHitPMT1 = false;
  fRawVoltageHitPMT2 = false;
  fRawVoltageCoincidence = false;
}

void EventAction::EndOfEventAction(const G4Event* event)
{
  if (!fRunAction) return;

  fRunAction->AddEdep1(fEdep1);
  fRunAction->AddEdep2(fEdep2);

  // Construct simplified PMT voltage waveforms from the PE arrival times.
  // This does not replace the PE-based hit definition below; it is stored as
  // an additional electronics-level observable.
  ComputePMTWaveforms();

  constexpr G4int peThreshold = 1;
  constexpr G4double timeWindow = 15.0 * ns;

  const G4bool hitTop = (fPEPMT1 >= peThreshold);
  const G4bool hitBottom = (fPEPMT2 >= peThreshold);
  const G4bool bothHit = hitTop && hitBottom;

  G4double deltaT = -1.;
  G4bool timeCoincidence = false;

  if (fFirstPETimePMT1 >= 0. && fFirstPETimePMT2 >= 0.) {
    deltaT = fFirstPETimePMT2 - fFirstPETimePMT1;
    timeCoincidence = (std::abs(deltaT) <= timeWindow);
  }

  const G4bool coincidence = bothHit && timeCoincidence;

  // Voltage-based coincidence. This uses the peak times of the simplified
  // voltage pulses and the same coincidence time window.
  G4double deltaPulseT = -1.;
  if (fPulseTimePMT1 >= 0. && fPulseTimePMT2 >= 0.) {
    deltaPulseT = fPulseTimePMT2 - fPulseTimePMT1;
    fVoltageCoincidence =
      fVoltageHitPMT1 && fVoltageHitPMT2 && (std::abs(deltaPulseT) <= timeWindow);
  }

  // Equivalent raw PMT polarity as observed before an inverting/front-end stage.
  // This is useful for comparing with a real oscilloscope trace where the PMT
  // pulse was seen as a negative-going peak.
  fRawVminPMT1 = -fVpeakPMT1;
  fRawVminPMT2 = -fVpeakPMT2;
  fRawPulseTimePMT1 = fPulseTimePMT1;
  fRawPulseTimePMT2 = fPulseTimePMT2;
  fRawVoltageHitPMT1 = (fRawVminPMT1 <= -kVoltageThreshold_mV);
  fRawVoltageHitPMT2 = (fRawVminPMT2 <= -kVoltageThreshold_mV);

  G4double deltaRawPulseT = -1.;
  if (fRawPulseTimePMT1 >= 0. && fRawPulseTimePMT2 >= 0.) {
    deltaRawPulseT = fRawPulseTimePMT2 - fRawPulseTimePMT1;
    fRawVoltageCoincidence =
      fRawVoltageHitPMT1 && fRawVoltageHitPMT2 &&
      (std::abs(deltaRawPulseT) <= timeWindow);
  }

  G4double meanTransitPMT1 = -1.;
  G4double meanTransitPMT2 = -1.;

  if (fPhotonsPMT1 > 0) {
    meanTransitPMT1 = fSumPhotonTransitPMT1 / fPhotonsPMT1;
  }

  if (fPhotonsPMT2 > 0) {
    meanTransitPMT2 = fSumPhotonTransitPMT2 / fPhotonsPMT2;
  }

  if (hitTop) {
    fRunAction->AddTopHit();
  }

  if (hitBottom) {
    fRunAction->AddBottomHit();
  }

  if (coincidence) {
    fRunAction->AddCoincidence();
  }

  if (fVoltageHitPMT1) {
    fRunAction->AddVoltageTopHit();
  }

  if (fVoltageHitPMT2) {
    fRunAction->AddVoltageBottomHit();
  }

  if (fVoltageCoincidence) {
    fRunAction->AddVoltageCoincidence();
  }

  fRunAction->AddPhotonsPMT1(fPhotonsPMT1);
  fRunAction->AddPhotonsPMT2(fPhotonsPMT2);
  fRunAction->AddPEPMT1(fPEPMT1);
  fRunAction->AddPEPMT2(fPEPMT2);

  fRunAction->AddPhotonTransitPMT1(fFirstPhotonTransitPMT1, meanTransitPMT1);
  fRunAction->AddPhotonTransitPMT2(fFirstPhotonTransitPMT2, meanTransitPMT2);

  G4double muonEnergy = 0.;
  G4double muonMomentum = 0.;
  G4double muonTheta = 0.;
  G4int muonCharge = 0;

  auto generatorAction = static_cast<const PrimaryGeneratorAction*>(
      G4RunManager::GetRunManager()->GetUserPrimaryGeneratorAction());

  if (generatorAction) {
    muonEnergy = generatorAction->GetCurrentMuonEnergy();
    muonMomentum = generatorAction->GetCurrentMuonMomentum();
    muonTheta = generatorAction->GetCurrentMuonTheta();
    muonCharge = generatorAction->GetCurrentMuonCharge();
  }

  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->FillNtupleIColumn(0, event->GetEventID());
  analysisManager->FillNtupleDColumn(1, muonEnergy / GeV);
  analysisManager->FillNtupleIColumn(2, muonCharge);
  analysisManager->FillNtupleDColumn(3, fEdep1 / MeV);
  analysisManager->FillNtupleDColumn(4, fEdep2 / MeV);
  analysisManager->FillNtupleIColumn(5, coincidence ? 1 : 0);
  analysisManager->FillNtupleIColumn(6, fPhotonsPMT1);
  analysisManager->FillNtupleIColumn(7, fPhotonsPMT2);
  analysisManager->FillNtupleIColumn(8, fPEPMT1);
  analysisManager->FillNtupleIColumn(9, fPEPMT2);
  analysisManager->FillNtupleIColumn(10, hitTop ? 1 : 0);
  analysisManager->FillNtupleIColumn(11, hitBottom ? 1 : 0);
  analysisManager->FillNtupleIColumn(12, peThreshold);

  analysisManager->FillNtupleDColumn(13, fFirstPETimePMT1 / ns);
  analysisManager->FillNtupleDColumn(14, fFirstPETimePMT2 / ns);
  analysisManager->FillNtupleDColumn(15, deltaT / ns);
  analysisManager->FillNtupleIColumn(16, timeCoincidence ? 1 : 0);
  analysisManager->FillNtupleDColumn(17, timeWindow / ns);

  analysisManager->FillNtupleDColumn(18, fFirstPhotonTransitPMT1 / ns);
  analysisManager->FillNtupleDColumn(19, fFirstPhotonTransitPMT2 / ns);
  analysisManager->FillNtupleDColumn(20, meanTransitPMT1 / ns);
  analysisManager->FillNtupleDColumn(21, meanTransitPMT2 / ns);

  analysisManager->FillNtupleIColumn(22, fScintPhotonsPMT1);
  analysisManager->FillNtupleIColumn(23, fScintPhotonsPMT2);
  analysisManager->FillNtupleIColumn(24, fCerenkovPhotonsPMT1);
  analysisManager->FillNtupleIColumn(25, fCerenkovPhotonsPMT2);
  analysisManager->FillNtupleIColumn(26, fOtherOpticalPhotonsPMT1);
  analysisManager->FillNtupleIColumn(27, fOtherOpticalPhotonsPMT2);

  analysisManager->FillNtupleDColumn(28, fVpeakPMT1);
  analysisManager->FillNtupleDColumn(29, fVpeakPMT2);
  analysisManager->FillNtupleDColumn(30, fPulseTimePMT1 / ns);
  analysisManager->FillNtupleDColumn(31, fPulseTimePMT2 / ns);
  analysisManager->FillNtupleDColumn(32, deltaPulseT / ns);
  analysisManager->FillNtupleIColumn(33, fVoltageHitPMT1 ? 1 : 0);
  analysisManager->FillNtupleIColumn(34, fVoltageHitPMT2 ? 1 : 0);
  analysisManager->FillNtupleIColumn(35, fVoltageCoincidence ? 1 : 0);
  analysisManager->FillNtupleDColumn(36, kVoltageThreshold_mV);
  analysisManager->FillNtupleDColumn(37, kLoadResistanceOhm);
  analysisManager->FillNtupleDColumn(38, kSinglePEAmplitude_mV);

  // Raw negative-polarity PMT signal. This represents the unprocessed
  // oscilloscope-like pulse before the planned inverting/front-end electronics.
  analysisManager->FillNtupleDColumn(39, fRawVminPMT1);
  analysisManager->FillNtupleDColumn(40, fRawVminPMT2);
  analysisManager->FillNtupleDColumn(41, fRawPulseTimePMT1 / ns);
  analysisManager->FillNtupleDColumn(42, fRawPulseTimePMT2 / ns);
  analysisManager->FillNtupleDColumn(43, deltaRawPulseT / ns);
  analysisManager->FillNtupleIColumn(44, fRawVoltageHitPMT1 ? 1 : 0);
  analysisManager->FillNtupleIColumn(45, fRawVoltageHitPMT2 ? 1 : 0);
  analysisManager->FillNtupleIColumn(46, fRawVoltageCoincidence ? 1 : 0);
  analysisManager->FillNtupleDColumn(47, -kVoltageThreshold_mV);

  // Primary-source validation columns appended in RunAction.
  analysisManager->FillNtupleDColumn(48, muonMomentum / GeV);
  analysisManager->FillNtupleDColumn(49, muonTheta / deg);

  analysisManager->AddNtupleRow(0);

  WriteOscilloscopeSamples(event->GetEventID());
}

void EventAction::AddEdep1(G4double edep)
{
  fEdep1 += edep;
}

void EventAction::AddEdep2(G4double edep)
{
  fEdep2 += edep;
}

void EventAction::AddPhotonPMT1(G4double transitTime)
{
  fPhotonsPMT1++;
  fSumPhotonTransitPMT1 += transitTime;

  if (fFirstPhotonTransitPMT1 < 0. || transitTime < fFirstPhotonTransitPMT1) {
    fFirstPhotonTransitPMT1 = transitTime;
  }
}

void EventAction::AddPhotonPMT2(G4double transitTime)
{
  fPhotonsPMT2++;
  fSumPhotonTransitPMT2 += transitTime;

  if (fFirstPhotonTransitPMT2 < 0. || transitTime < fFirstPhotonTransitPMT2) {
    fFirstPhotonTransitPMT2 = transitTime;
  }
}

void EventAction::AddPEPMT1(G4double globalTime)
{
  fPEPMT1++;
  fPETimesPMT1.push_back(globalTime);

  if (fFirstPETimePMT1 < 0. || globalTime < fFirstPETimePMT1) {
    fFirstPETimePMT1 = globalTime;
  }
}

void EventAction::AddPEPMT2(G4double globalTime)
{
  fPEPMT2++;
  fPETimesPMT2.push_back(globalTime);

  if (fFirstPETimePMT2 < 0. || globalTime < fFirstPETimePMT2) {
    fFirstPETimePMT2 = globalTime;
  }
}

void EventAction::AddScintPhotonPMT1()
{
  fScintPhotonsPMT1++;
}

void EventAction::AddScintPhotonPMT2()
{
  fScintPhotonsPMT2++;
}

void EventAction::AddCerenkovPhotonPMT1()
{
  fCerenkovPhotonsPMT1++;
}

void EventAction::AddCerenkovPhotonPMT2()
{
  fCerenkovPhotonsPMT2++;
}

void EventAction::AddOtherOpticalPhotonPMT1()
{
  fOtherOpticalPhotonsPMT1++;
}

void EventAction::AddOtherOpticalPhotonPMT2()
{
  fOtherOpticalPhotonsPMT2++;
}

G4double EventAction::PulseShape(G4double t, G4double t0) const
{
  if (t < t0) return 0.;

  const G4double x = t - t0;

  // Difference of exponentials. The pulse is zero at t0, rises rapidly,
  // and then decays with the fall-time constant.
  const G4double raw =
      std::exp(-x / kFallTime) - std::exp(-x / kRiseTime);

  if (raw <= 0.) return 0.;

  // Normalize the single-PE shape to have peak value equal to 1.
  const G4double tPeak =
      (kRiseTime * kFallTime / (kFallTime - kRiseTime))
      * std::log(kFallTime / kRiseTime);

  const G4double rawPeak =
      std::exp(-tPeak / kFallTime) - std::exp(-tPeak / kRiseTime);

  return (rawPeak > 0.) ? raw / rawPeak : 0.;
}

G4double EventAction::VoltageAtTime(const std::vector<G4double>& peTimes,
                                    G4double t) const
{
  G4double voltage = 0.;

  for (const auto tPE : peTimes) {
    voltage += kSinglePEAmplitude_mV * PulseShape(t, tPE);
  }

  return voltage;
}

void EventAction::ComputePMTWaveforms()
{
  auto computeOnePMT =
      [this](const std::vector<G4double>& peTimes,
             G4double& vPeak,
             G4double& pulseTime,
             G4bool& voltageHit)
  {
    vPeak = 0.;
    pulseTime = -1.;
    voltageHit = false;

    if (peTimes.empty()) {
      return;
    }

    const auto minmax = std::minmax_element(peTimes.begin(), peTimes.end());
    const G4double tStart = *minmax.first;
    const G4double tStop =
        std::min(*minmax.second + 5.0 * kFallTime,
                 tStart + kWaveformDuration);

    for (G4double t = tStart; t <= tStop; t += kWaveformDt) {
      const G4double voltage = VoltageAtTime(peTimes, t);

      if (voltage > vPeak) {
        vPeak = voltage;
        pulseTime = t;
      }
    }

    voltageHit = (vPeak >= kVoltageThreshold_mV);
  };

  computeOnePMT(fPETimesPMT1, fVpeakPMT1, fPulseTimePMT1, fVoltageHitPMT1);
  computeOnePMT(fPETimesPMT2, fVpeakPMT2, fPulseTimePMT2, fVoltageHitPMT2);
}

void EventAction::WriteOscilloscopeSamples(G4int eventID) const
{
  // Save event-by-event oscilloscope traces only for events where at least one
  // PMT crosses the voltage threshold. This keeps the ROOT output small while
  // still preserving real pulse shapes for plotting.
  if (!fVoltageHitPMT1 && !fVoltageHitPMT2) {
    return;
  }

  if (fPETimesPMT1.empty() && fPETimesPMT2.empty()) {
    return;
  }

  G4double tReference = 1.e99;

  for (const auto t : fPETimesPMT1) {
    tReference = std::min(tReference, t);
  }

  for (const auto t : fPETimesPMT2) {
    tReference = std::min(tReference, t);
  }

  if (tReference > 1.e90) {
    return;
  }

  // Scope display: 2 ns pre-trigger baseline and 30 ns total window.
  const G4double tLocalStart = -2.0 * ns;
  const G4double tLocalStop = 30.0 * ns;

  auto analysisManager = G4AnalysisManager::Instance();

  G4int sampleIndex = 0;

  for (G4double tLocal = tLocalStart;
       tLocal <= tLocalStop + 0.5 * kWaveformDt;
       tLocal += kWaveformDt)
  {
    const G4double tGlobal = tReference + tLocal;

    const G4double vProcessed1 = VoltageAtTime(fPETimesPMT1, tGlobal);
    const G4double vProcessed2 = VoltageAtTime(fPETimesPMT2, tGlobal);

    const G4double vRaw1 = -vProcessed1;
    const G4double vRaw2 = -vProcessed2;

    // Ntuple id 1 is the "Oscilloscope" ntuple created in RunAction.
    analysisManager->FillNtupleIColumn(1, 0, eventID);
    analysisManager->FillNtupleIColumn(1, 1, sampleIndex);
    analysisManager->FillNtupleDColumn(1, 2, tLocal / ns);
    analysisManager->FillNtupleDColumn(1, 3, vProcessed1);
    analysisManager->FillNtupleDColumn(1, 4, vProcessed2);
    analysisManager->FillNtupleDColumn(1, 5, vRaw1);
    analysisManager->FillNtupleDColumn(1, 6, vRaw2);
    analysisManager->FillNtupleDColumn(1, 7, kVoltageThreshold_mV);
    analysisManager->FillNtupleDColumn(1, 8, -kVoltageThreshold_mV);
    analysisManager->FillNtupleIColumn(1, 9, fVoltageHitPMT1 ? 1 : 0);
    analysisManager->FillNtupleIColumn(1, 10, fVoltageHitPMT2 ? 1 : 0);
    analysisManager->FillNtupleIColumn(1, 11, fVoltageCoincidence ? 1 : 0);
    analysisManager->AddNtupleRow(1);

    ++sampleIndex;
  }
}


}  // namespace B1
