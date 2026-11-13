#ifndef B1RunAction_h
#define B1RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;

namespace B1
{

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    ~RunAction() override = default;

    void BeginOfRunAction(const G4Run*) override;
    void EndOfRunAction(const G4Run*) override;

    void AddEdep1(G4double edep);
    void AddEdep2(G4double edep);

    void AddTopHit() { fTopHits += 1; }
    void AddBottomHit() { fBottomHits += 1; }
    void AddCoincidence() { fCoincidences += 1; }

    void AddVoltageTopHit() { fVoltageTopHits += 1; }
    void AddVoltageBottomHit() { fVoltageBottomHits += 1; }
    void AddVoltageCoincidence() { fVoltageCoincidences += 1; }

    void AddPhotonsPMT1(G4int n) { fPhotonsPMT1 += n; }
    void AddPhotonsPMT2(G4int n) { fPhotonsPMT2 += n; }
    void AddPEPMT1(G4int n) { fPEPMT1 += n; }
    void AddPEPMT2(G4int n) { fPEPMT2 += n; }

    void AddPhotonTransitPMT1(G4double firstTime, G4double meanTime);
    void AddPhotonTransitPMT2(G4double firstTime, G4double meanTime);

  private:
    G4Accumulable<G4double> fEdep1 = 0.;
    G4Accumulable<G4double> fEdep1Sq = 0.;
    G4Accumulable<G4double> fEdep2 = 0.;
    G4Accumulable<G4double> fEdep2Sq = 0.;

    G4Accumulable<G4int> fTopHits = 0;
    G4Accumulable<G4int> fBottomHits = 0;
    G4Accumulable<G4int> fCoincidences = 0;

    G4Accumulable<G4int> fVoltageTopHits = 0;
    G4Accumulable<G4int> fVoltageBottomHits = 0;
    G4Accumulable<G4int> fVoltageCoincidences = 0;

    G4Accumulable<G4int> fPhotonsPMT1 = 0;
    G4Accumulable<G4int> fPhotonsPMT2 = 0;
    G4Accumulable<G4int> fPEPMT1 = 0;
    G4Accumulable<G4int> fPEPMT2 = 0;

    G4Accumulable<G4double> fFirstPhotonTransitPMT1Sum = 0.;
    G4Accumulable<G4double> fFirstPhotonTransitPMT2Sum = 0.;
    G4Accumulable<G4double> fMeanPhotonTransitPMT1Sum = 0.;
    G4Accumulable<G4double> fMeanPhotonTransitPMT2Sum = 0.;

    G4Accumulable<G4int> fTransitPMT1Events = 0;
    G4Accumulable<G4int> fTransitPMT2Events = 0;
};

}  // namespace B1

#endif
