#include "SteppingAction.hh"
#include "EventAction.hh"
#include "DetectorConstruction.hh"

#include "G4Step.hh"
#include "G4RunManager.hh"
#include "G4LogicalVolume.hh"
#include "G4OpticalPhoton.hh"
#include "Randomize.hh"
#include "G4VProcess.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4StepStatus.hh"

#include <array>


namespace
{

// Approximate wavelength-dependent quantum-efficiency curve for the
// Hamamatsu H10721-110 PMT module. The values represent a digitized/
// approximated form of the typical spectral-response curve.
// Wavelengths must be in increasing order for linear interpolation.
G4double H10721_110_QuantumEfficiency(G4double photonEnergy)
{
  if (photonEnergy <= 0.0) {
    return 0.0;
  }

  const G4double wavelength = h_Planck * c_light / photonEnergy;

  constexpr std::array<G4double, 15> wavelengthNm =
  {
    230.0, 260.0, 300.0, 330.0, 350.0,
    375.0, 400.0, 425.0, 450.0, 500.0,
    550.0, 600.0, 650.0, 680.0, 700.0
  };

  constexpr std::array<G4double, 15> quantumEfficiency =
  {
    0.005, 0.060, 0.150, 0.240, 0.290,
    0.330, 0.340, 0.325, 0.295, 0.210,
    0.120, 0.055, 0.015, 0.005, 0.002
  };

  const G4double wavelengthInNm = wavelength / nm;

  if (wavelengthInNm < wavelengthNm.front() ||
      wavelengthInNm > wavelengthNm.back()) {
    return 0.0;
  }

  for (std::size_t i = 0; i + 1 < wavelengthNm.size(); ++i) {
    if (wavelengthInNm <= wavelengthNm[i + 1]) {
      const G4double fraction =
          (wavelengthInNm - wavelengthNm[i]) /
          (wavelengthNm[i + 1] - wavelengthNm[i]);

      return quantumEfficiency[i] +
             fraction * (quantumEfficiency[i + 1] -
                         quantumEfficiency[i]);
    }
  }

  return 0.0;
}

}  // namespace

namespace B1
{

SteppingAction::SteppingAction(EventAction* eventAction)
: fEventAction(eventAction)
{}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
  const auto detectorConstruction =
      static_cast<const DetectorConstruction*>(
          G4RunManager::GetRunManager()->GetUserDetectorConstruction());

  auto scint1Volume = detectorConstruction->GetScintillator1Volume();
  auto scint2Volume = detectorConstruction->GetScintillator2Volume();
  auto pmt1Volume = detectorConstruction->GetPMT1Volume();
  auto pmt2Volume = detectorConstruction->GetPMT2Volume();

  auto track = step->GetTrack();
  auto particle = track->GetDefinition();

  if (particle == G4OpticalPhoton::OpticalPhotonDefinition()) {
    constexpr G4double maxOpticalPhotonTime = 100.0 * ns;
    constexpr G4int maxOpticalPhotonSteps = 1000;

    if (track->GetLocalTime() > maxOpticalPhotonTime ||
        track->GetCurrentStepNumber() > maxOpticalPhotonSteps) {
      track->SetTrackStatus(fStopAndKill);
      return;
    }

    auto prePoint = step->GetPreStepPoint();
    auto postPoint = step->GetPostStepPoint();

    // Count only real geometry-boundary crossings from the corresponding
    // scintillator directly into its PMT entrance window.
    if (postPoint->GetStepStatus() != fGeomBoundary) {
      return;
    }

    auto preVolume = prePoint->GetTouchableHandle()->GetVolume();
    auto postVolume = postPoint->GetTouchableHandle()->GetVolume();

    if (!preVolume || !postVolume) {
      return;
    }

    auto preLogical = preVolume->GetLogicalVolume();
    auto postLogical = postVolume->GetLogicalVolume();

    const G4bool entersPMT1 =
        (preLogical == scint1Volume && postLogical == pmt1Volume);
    const G4bool entersPMT2 =
        (preLogical == scint2Volume && postLogical == pmt2Volume);

    if (!entersPMT1 && !entersPMT2) {
      return;
    }

    const G4double quantumEfficiency =
        H10721_110_QuantumEfficiency(track->GetTotalEnergy());
    const G4double photonTransitTime = track->GetLocalTime();

    G4String creatorName = "Unknown";
    const auto creatorProcess = track->GetCreatorProcess();
    if (creatorProcess) {
      creatorName = creatorProcess->GetProcessName();
    }

    if (entersPMT1) {
      fEventAction->AddPhotonPMT1(photonTransitTime);

      if (creatorName == "Scintillation") {
        fEventAction->AddScintPhotonPMT1();
      } else if (creatorName == "Cerenkov") {
        fEventAction->AddCerenkovPhotonPMT1();
      } else {
        fEventAction->AddOtherOpticalPhotonPMT1();
      }

      if (G4UniformRand() < quantumEfficiency) {
        fEventAction->AddPEPMT1(track->GetGlobalTime());
      }
    } else {
      fEventAction->AddPhotonPMT2(photonTransitTime);

      if (creatorName == "Scintillation") {
        fEventAction->AddScintPhotonPMT2();
      } else if (creatorName == "Cerenkov") {
        fEventAction->AddCerenkovPhotonPMT2();
      } else {
        fEventAction->AddOtherOpticalPhotonPMT2();
      }

      if (G4UniformRand() < quantumEfficiency) {
        fEventAction->AddPEPMT2(track->GetGlobalTime());
      }
    }

    // The PMT window is treated as an absorbing detector entrance. QE is
    // applied once above, so the photon is terminated after crossing it.
    track->SetTrackStatus(fStopAndKill);
    return;
  }

  auto preStepPoint = step->GetPreStepPoint();
  auto physicalVolume = preStepPoint->GetTouchableHandle()->GetVolume();

  if (!physicalVolume) return;

  auto logicalVolume = physicalVolume->GetLogicalVolume();

  G4double edep = step->GetTotalEnergyDeposit();

  if (edep <= 0.) return;

  if (logicalVolume == scint1Volume) {
    fEventAction->AddEdep1(edep);
  }

  if (logicalVolume == scint2Volume) {
    fEventAction->AddEdep2(edep);
  }
}

}  // namespace B1
