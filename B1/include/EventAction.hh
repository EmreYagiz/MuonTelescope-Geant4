#ifndef B1EventAction_h
#define B1EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"

#include <vector>

class G4Event;

namespace B1
{

class RunAction;

class EventAction : public G4UserEventAction
{
  public:
    explicit EventAction(RunAction* runAction);
    ~EventAction() override = default;

    void BeginOfEventAction(const G4Event*) override;
    void EndOfEventAction(const G4Event*) override;

    void AddEdep1(G4double edep);
    void AddEdep2(G4double edep);

    void AddPhotonPMT1(G4double transitTime);
    void AddPhotonPMT2(G4double transitTime);

    void AddScintPhotonPMT1();
    void AddScintPhotonPMT2();

    void AddCerenkovPhotonPMT1();
    void AddCerenkovPhotonPMT2();

    void AddOtherOpticalPhotonPMT1();
    void AddOtherOpticalPhotonPMT2();

    void AddPEPMT1(G4double globalTime);
    void AddPEPMT2(G4double globalTime);

  private:
    void ComputePMTWaveforms();
    void WriteOscilloscopeSamples(G4int eventID) const;
    G4double PulseShape(G4double t, G4double t0) const;
    G4double VoltageAtTime(const std::vector<G4double>& peTimes, G4double t) const;

    RunAction* fRunAction = nullptr;

    G4double fEdep1 = 0.;
    G4double fEdep2 = 0.;

    G4int fPhotonsPMT1 = 0;
    G4int fPhotonsPMT2 = 0;

    G4int fScintPhotonsPMT1 = 0;
    G4int fScintPhotonsPMT2 = 0;

    G4int fCerenkovPhotonsPMT1 = 0;
    G4int fCerenkovPhotonsPMT2 = 0;

    G4int fOtherOpticalPhotonsPMT1 = 0;
    G4int fOtherOpticalPhotonsPMT2 = 0;

    G4int fPEPMT1 = 0;
    G4int fPEPMT2 = 0;

    G4double fFirstPETimePMT1 = -1.;
    G4double fFirstPETimePMT2 = -1.;

    G4double fFirstPhotonTransitPMT1 = -1.;
    G4double fFirstPhotonTransitPMT2 = -1.;

    G4double fSumPhotonTransitPMT1 = 0.;
    G4double fSumPhotonTransitPMT2 = 0.;

    // Photoelectron arrival times are stored only during the current event.
    // They are used to construct a simplified PMT voltage waveform at the end
    // of the event. Only summary quantities are written to the ROOT ntuple.
    std::vector<G4double> fPETimesPMT1;
    std::vector<G4double> fPETimesPMT2;

    // Simplified PMT waveform quantities.
    // Vpeak values are stored in mV and pulse times in ns in the ROOT output.
    G4double fVpeakPMT1 = 0.;
    G4double fVpeakPMT2 = 0.;

    G4double fPulseTimePMT1 = -1.;
    G4double fPulseTimePMT2 = -1.;

    G4bool fVoltageHitPMT1 = false;
    G4bool fVoltageHitPMT2 = false;
    G4bool fVoltageCoincidence = false;

    // Raw PMT polarity convention. In the real setup the unprocessed PMT
    // signal can appear as a negative pulse on the oscilloscope. The positive
    // waveform above represents the processed/inverted electronics response,
    // while these quantities store the equivalent raw negative-polarity signal.
    G4double fRawVminPMT1 = 0.;
    G4double fRawVminPMT2 = 0.;

    G4double fRawPulseTimePMT1 = -1.;
    G4double fRawPulseTimePMT2 = -1.;

    G4bool fRawVoltageHitPMT1 = false;
    G4bool fRawVoltageHitPMT2 = false;
    G4bool fRawVoltageCoincidence = false;
};

}  // namespace B1

#endif
