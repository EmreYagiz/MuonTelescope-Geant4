#ifndef B1DetectorConstruction_h
#define B1DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"

class G4VPhysicalVolume;
class G4LogicalVolume;

namespace B1
{

class DetectorConstruction : public G4VUserDetectorConstruction
{
  public:
    DetectorConstruction() = default;
    ~DetectorConstruction() override = default;

    G4VPhysicalVolume* Construct() override;

    G4LogicalVolume* GetScintillator1Volume() const { return fScintillator1Volume; }
    G4LogicalVolume* GetScintillator2Volume() const { return fScintillator2Volume; }

    G4LogicalVolume* GetPMT1Volume() const { return fPMT1Volume; }
    G4LogicalVolume* GetPMT2Volume() const { return fPMT2Volume; }

  private:
    G4LogicalVolume* fScintillator1Volume = nullptr;
    G4LogicalVolume* fScintillator2Volume = nullptr;

    G4LogicalVolume* fPMT1Volume = nullptr;
    G4LogicalVolume* fPMT2Volume = nullptr;
};

}  // namespace B1

#endif
