#include "RunAction.hh"

#include "G4Run.hh"
#include "G4RunManager.hh"
#include "G4AccumulableManager.hh"
#include "G4UnitsTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4AnalysisManager.hh"

#include <cmath>

namespace B1
{

RunAction::RunAction()
{
  auto accumulableManager = G4AccumulableManager::Instance();

  accumulableManager->Register(fEdep1);
  accumulableManager->Register(fEdep1Sq);
  accumulableManager->Register(fEdep2);
  accumulableManager->Register(fEdep2Sq);

  accumulableManager->Register(fTopHits);
  accumulableManager->Register(fBottomHits);
  accumulableManager->Register(fCoincidences);

  accumulableManager->Register(fVoltageTopHits);
  accumulableManager->Register(fVoltageBottomHits);
  accumulableManager->Register(fVoltageCoincidences);

  accumulableManager->Register(fPhotonsPMT1);
  accumulableManager->Register(fPhotonsPMT2);
  accumulableManager->Register(fPEPMT1);
  accumulableManager->Register(fPEPMT2);

  accumulableManager->Register(fFirstPhotonTransitPMT1Sum);
  accumulableManager->Register(fFirstPhotonTransitPMT2Sum);
  accumulableManager->Register(fMeanPhotonTransitPMT1Sum);
  accumulableManager->Register(fMeanPhotonTransitPMT2Sum);
  accumulableManager->Register(fTransitPMT1Events);
  accumulableManager->Register(fTransitPMT2Events);

  auto analysisManager = G4AnalysisManager::Instance();

  analysisManager->SetDefaultFileType("root");

  analysisManager->CreateNtuple("MuonTelescope", "Muon telescope event data");

  analysisManager->CreateNtupleIColumn("EventID");
  analysisManager->CreateNtupleDColumn("MuonEnergy_GeV");
  analysisManager->CreateNtupleIColumn("MuonCharge");
  analysisManager->CreateNtupleDColumn("EdepTop_MeV");
  analysisManager->CreateNtupleDColumn("EdepBottom_MeV");
  analysisManager->CreateNtupleIColumn("Coincidence");
  analysisManager->CreateNtupleIColumn("PhotonsPMT1");
  analysisManager->CreateNtupleIColumn("PhotonsPMT2");
  analysisManager->CreateNtupleIColumn("PEPMT1");
  analysisManager->CreateNtupleIColumn("PEPMT2");
  analysisManager->CreateNtupleIColumn("PMT1Hit");
  analysisManager->CreateNtupleIColumn("PMT2Hit");
  analysisManager->CreateNtupleIColumn("PEThreshold");
  analysisManager->CreateNtupleDColumn("FirstPETimePMT1_ns");
  analysisManager->CreateNtupleDColumn("FirstPETimePMT2_ns");
  analysisManager->CreateNtupleDColumn("DeltaT_ns");
  analysisManager->CreateNtupleIColumn("TimeCoincidence");
  analysisManager->CreateNtupleDColumn("TimeWindow_ns");

  analysisManager->CreateNtupleDColumn("FirstPhotonTransitPMT1_ns");
  analysisManager->CreateNtupleDColumn("FirstPhotonTransitPMT2_ns");
  analysisManager->CreateNtupleDColumn("MeanPhotonTransitPMT1_ns");
  analysisManager->CreateNtupleDColumn("MeanPhotonTransitPMT2_ns");

  analysisManager->CreateNtupleIColumn("ScintPhotonsPMT1");
  analysisManager->CreateNtupleIColumn("ScintPhotonsPMT2");
  analysisManager->CreateNtupleIColumn("CerenkovPhotonsPMT1");
  analysisManager->CreateNtupleIColumn("CerenkovPhotonsPMT2");
  analysisManager->CreateNtupleIColumn("OtherOpticalPhotonsPMT1");
  analysisManager->CreateNtupleIColumn("OtherOpticalPhotonsPMT2");

  // Simplified PMT waveform / electronics observables.
  analysisManager->CreateNtupleDColumn("VpeakPMT1_mV");
  analysisManager->CreateNtupleDColumn("VpeakPMT2_mV");
  analysisManager->CreateNtupleDColumn("PulseTimePMT1_ns");
  analysisManager->CreateNtupleDColumn("PulseTimePMT2_ns");
  analysisManager->CreateNtupleDColumn("DeltaPulseTime_ns");
  analysisManager->CreateNtupleIColumn("VoltageHitPMT1");
  analysisManager->CreateNtupleIColumn("VoltageHitPMT2");
  analysisManager->CreateNtupleIColumn("VoltageCoincidence");
  analysisManager->CreateNtupleDColumn("VoltageThreshold_mV");
  analysisManager->CreateNtupleDColumn("LoadResistance_ohm");
  analysisManager->CreateNtupleDColumn("SinglePEAmplitude_mV");

  // Raw negative-polarity PMT waveform observables. These are the same
  // simplified waveform with the sign inverted, corresponding to the raw
  // oscilloscope signal before an inverting/front-end electronics stage.
  analysisManager->CreateNtupleDColumn("RawVminPMT1_mV");
  analysisManager->CreateNtupleDColumn("RawVminPMT2_mV");
  analysisManager->CreateNtupleDColumn("RawPulseTimePMT1_ns");
  analysisManager->CreateNtupleDColumn("RawPulseTimePMT2_ns");
  analysisManager->CreateNtupleDColumn("DeltaRawPulseTime_ns");
  analysisManager->CreateNtupleIColumn("RawVoltageHitPMT1");
  analysisManager->CreateNtupleIColumn("RawVoltageHitPMT2");
  analysisManager->CreateNtupleIColumn("RawVoltageCoincidence");
  analysisManager->CreateNtupleDColumn("RawVoltageThreshold_mV");

  // Primary-source validation variables. Appended at the end so that all
  // existing event-column indices remain unchanged.
  analysisManager->CreateNtupleDColumn("MuonMomentum_GeV");
  analysisManager->CreateNtupleDColumn("MuonTheta_deg");

  analysisManager->FinishNtuple();

  // Oscilloscope-like waveform samples.
  // Each row is one time sample for a selected event. This ntuple is used
  // only for drawing event-by-event oscilloscope traces, not for event totals.
  analysisManager->CreateNtuple("Oscilloscope", "Selected PMT waveform samples");
  analysisManager->CreateNtupleIColumn("EventID");
  analysisManager->CreateNtupleIColumn("SampleIndex");
  analysisManager->CreateNtupleDColumn("Time_ns");
  analysisManager->CreateNtupleDColumn("ProcessedVPMT1_mV");
  analysisManager->CreateNtupleDColumn("ProcessedVPMT2_mV");
  analysisManager->CreateNtupleDColumn("RawVPMT1_mV");
  analysisManager->CreateNtupleDColumn("RawVPMT2_mV");
  analysisManager->CreateNtupleDColumn("Threshold_mV");
  analysisManager->CreateNtupleDColumn("RawThreshold_mV");
  analysisManager->CreateNtupleIColumn("VoltageHitPMT1");
  analysisManager->CreateNtupleIColumn("VoltageHitPMT2");
  analysisManager->CreateNtupleIColumn("VoltageCoincidence");
  analysisManager->FinishNtuple();
}

void RunAction::BeginOfRunAction(const G4Run*)
{
  G4RunManager::GetRunManager()->SetRandomNumberStore(false);
  G4AccumulableManager::Instance()->Reset();

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->OpenFile("muon_telescope_data");
}

void RunAction::EndOfRunAction(const G4Run* run)
{
  const G4int nofEvents = run->GetNumberOfEvent();

  if (nofEvents == 0) {
    return;
  }

  auto accumulableManager = G4AccumulableManager::Instance();
  accumulableManager->Merge();

  const G4double edep1 = fEdep1.GetValue();
  const G4double edep1Sq = fEdep1Sq.GetValue();
  const G4double edep2 = fEdep2.GetValue();
  const G4double edep2Sq = fEdep2Sq.GetValue();

  const G4int topHits = fTopHits.GetValue();
  const G4int bottomHits = fBottomHits.GetValue();
  const G4int coincidences = fCoincidences.GetValue();

  const G4int voltageTopHits = fVoltageTopHits.GetValue();
  const G4int voltageBottomHits = fVoltageBottomHits.GetValue();
  const G4int voltageCoincidences = fVoltageCoincidences.GetValue();

  const G4int photonsPMT1 = fPhotonsPMT1.GetValue();
  const G4int photonsPMT2 = fPhotonsPMT2.GetValue();
  const G4int pePMT1 = fPEPMT1.GetValue();
  const G4int pePMT2 = fPEPMT2.GetValue();

  const G4int transitEventsPMT1 = fTransitPMT1Events.GetValue();
  const G4int transitEventsPMT2 = fTransitPMT2Events.GetValue();

  G4double avgFirstTransitPMT1 = -1.;
  G4double avgFirstTransitPMT2 = -1.;
  G4double avgMeanTransitPMT1 = -1.;
  G4double avgMeanTransitPMT2 = -1.;

  if (transitEventsPMT1 > 0) {
    avgFirstTransitPMT1 =
        fFirstPhotonTransitPMT1Sum.GetValue() / transitEventsPMT1;
    avgMeanTransitPMT1 =
        fMeanPhotonTransitPMT1Sum.GetValue() / transitEventsPMT1;
  }

  if (transitEventsPMT2 > 0) {
    avgFirstTransitPMT2 =
        fFirstPhotonTransitPMT2Sum.GetValue() / transitEventsPMT2;
    avgMeanTransitPMT2 =
        fMeanPhotonTransitPMT2Sum.GetValue() / transitEventsPMT2;
  }

  G4double mean1 = edep1 / nofEvents;
  G4double mean2 = edep2 / nofEvents;

  G4double rms1 = edep1Sq / nofEvents - mean1 * mean1;
  G4double rms2 = edep2Sq / nofEvents - mean2 * mean2;

  rms1 = (rms1 > 0.) ? std::sqrt(rms1) : 0.;
  rms2 = (rms2 > 0.) ? std::sqrt(rms2) : 0.;

  G4cout << G4endl;

  if (IsMaster()) {
    G4cout << "--------------------End of Global Run-----------------------" << G4endl;
  } else {
    G4cout << "--------------------End of Local Run------------------------" << G4endl;
  }

  G4cout
    << " Primary source : Modified Gaisser sea-level muons" << G4endl
    << " Momentum range : 1 - 100 GeV/c" << G4endl
    << " Zenith range   : 0 - 70 deg" << G4endl
    << " Number of events : " << nofEvents << G4endl

    << " Top scintillator mean deposited energy per event : "
    << G4BestUnit(mean1, "Energy")
    << "   rms = " << G4BestUnit(rms1, "Energy") << G4endl

    << " Bottom scintillator mean deposited energy per event : "
    << G4BestUnit(mean2, "Energy")
    << "   rms = " << G4BestUnit(rms2, "Energy") << G4endl

    << " PE-based top hit count        : " << topHits << G4endl
    << " PE-based bottom hit count     : " << bottomHits << G4endl
    << " PE-based coincidence count    : " << coincidences << G4endl

    << " PE-based top hit fraction     : "
    << 100.0 * topHits / nofEvents << " %" << G4endl

    << " PE-based bottom hit fraction  : "
    << 100.0 * bottomHits / nofEvents << " %" << G4endl

    << " PE-based coincidence fraction : "
    << 100.0 * coincidences / nofEvents << " %" << G4endl

    << " Voltage-based top hit count        : " << voltageTopHits << G4endl
    << " Voltage-based bottom hit count     : " << voltageBottomHits << G4endl
    << " Voltage-based coincidence count    : " << voltageCoincidences << G4endl

    << " Voltage-based top hit fraction     : "
    << 100.0 * voltageTopHits / nofEvents << " %" << G4endl

    << " Voltage-based bottom hit fraction  : "
    << 100.0 * voltageBottomHits / nofEvents << " %" << G4endl

    << " Voltage-based coincidence fraction : "
    << 100.0 * voltageCoincidences / nofEvents << " %" << G4endl

    << " Photons reaching PMT1 : " << photonsPMT1 << G4endl
    << " Photons reaching PMT2 : " << photonsPMT2 << G4endl
    << " Photoelectrons PMT1   : " << pePMT1 << G4endl
    << " Photoelectrons PMT2   : " << pePMT2 << G4endl

    << " PMT1 avg first photon transit time : "
    << avgFirstTransitPMT1 / ns << " ns" << G4endl

    << " PMT2 avg first photon transit time : "
    << avgFirstTransitPMT2 / ns << " ns" << G4endl

    << " PMT1 avg mean photon transit time  : "
    << avgMeanTransitPMT1 / ns << " ns" << G4endl

    << " PMT2 avg mean photon transit time  : "
    << avgMeanTransitPMT2 / ns << " ns" << G4endl

    << "------------------------------------------------------------" << G4endl
    << G4endl;

  auto analysisManager = G4AnalysisManager::Instance();
  analysisManager->Write();
  analysisManager->CloseFile();
}

void RunAction::AddEdep1(G4double edep)
{
  fEdep1 += edep;
  fEdep1Sq += edep * edep;
}

void RunAction::AddEdep2(G4double edep)
{
  fEdep2 += edep;
  fEdep2Sq += edep * edep;
}

void RunAction::AddPhotonTransitPMT1(G4double firstTime, G4double meanTime)
{
  if (firstTime >= 0. && meanTime >= 0.) {
    fFirstPhotonTransitPMT1Sum += firstTime;
    fMeanPhotonTransitPMT1Sum += meanTime;
    fTransitPMT1Events += 1;
  }
}

void RunAction::AddPhotonTransitPMT2(G4double firstTime, G4double meanTime)
{
  if (firstTime >= 0. && meanTime >= 0.) {
    fFirstPhotonTransitPMT2Sum += firstTime;
    fMeanPhotonTransitPMT2Sum += meanTime;
    fTransitPMT2Events += 1;
  }
}

}  // namespace B1
