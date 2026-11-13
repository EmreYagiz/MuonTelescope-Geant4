#ifndef B1PrimaryGeneratorAction_h
#define B1PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"

class G4ParticleGun;
class G4Event;

namespace B1
{

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    ~PrimaryGeneratorAction() override;

    void GeneratePrimaries(G4Event*) override;

    const G4ParticleGun* GetParticleGun() const
    {
      return fParticleGun;
    }

    G4double GetCurrentMuonEnergy() const
    {
      return fCurrentMuonEnergy;
    }

    G4double GetCurrentMuonMomentum() const
    {
      return fCurrentMuonMomentum;
    }

    G4double GetCurrentMuonTheta() const
    {
      return fCurrentMuonTheta;
    }

    G4int GetCurrentMuonCharge() const
    {
      return fCurrentMuonCharge;
    }

  private:
    G4ParticleGun* fParticleGun = nullptr;

    G4double fCurrentMuonEnergy = 0.;
    G4double fCurrentMuonMomentum = 0.;
    G4double fCurrentMuonTheta = 0.;
    G4int fCurrentMuonCharge = 0;
};

}  // namespace B1

#endif
