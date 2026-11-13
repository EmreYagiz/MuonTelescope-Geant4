#include "ActionInitialization.hh"
#include "DetectorConstruction.hh"

#include "G4OpticalPhysics.hh"
#include "G4RunManagerFactory.hh"
#include "G4UImanager.hh"
#include "QBBC.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "G4SteppingVerbose.hh"

using namespace B1;

int main(int argc, char** argv)
{
  G4UIExecutive* ui = nullptr;
  if (argc == 1) {
    ui = new G4UIExecutive(argc, argv);
  }

  G4int precision = 4;
  G4SteppingVerbose::UseBestUnit(precision);

  auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

  auto detector = new DetectorConstruction();
  runManager->SetUserInitialization(detector);

  auto physicsList = new QBBC;
  physicsList->RegisterPhysics(new G4OpticalPhysics());
  runManager->SetUserInitialization(physicsList);

  runManager->SetUserInitialization(new ActionInitialization(detector));

  auto visManager = new G4VisExecutive(argc, argv);
  visManager->Initialize();

  auto UImanager = G4UImanager::GetUIpointer();

  if (!ui) {
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    UImanager->ApplyCommand(command + fileName);
  } else {
    UImanager->ApplyCommand("/control/execute init_vis.mac");
    ui->SessionStart();
    delete ui;
  }

  delete visManager;
  delete runManager;
}
