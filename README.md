# Cosmic Ray Muon Telescope

## Overview

This project focuses on the development, simulation, and experimental characterization of a cosmic-ray muon telescope based on two plastic scintillator detectors.

The primary objective is to investigate cosmic-ray muon detection through coincidence measurements and to study the response of the detector system using Geant4 simulations and experimental data analysis.

Unlike the subsequent muon tomography preparation studies, this configuration does not include any lead blocks or additional absorbing materials between the detectors.

## Detector Setup

The telescope consists of two plastic scintillators coupled to photomultiplier tubes (PMTs), arranged to detect cosmic-ray muons passing through both detectors.

The system includes:

- Two BC-408 plastic scintillator detectors
- Photomultiplier tubes for scintillation light detection
- Adjustable detector separation and geometry
- Oscilloscope-based signal acquisition
- Coincidence measurements between the two detectors

The coincidence technique is used to identify events producing signals in both scintillators, helping distinguish candidate through-going muons from single-detector signals and background.

## Geant4 Simulation

A Geant4-based simulation was developed to investigate the detector response to atmospheric cosmic-ray muons.

The simulation includes:

- Cosmic-ray muon generation with momentum and angular distributions
- Muon propagation through the detector geometry
- Energy deposition in plastic scintillators
- Scintillation photon production and optical photon transport
- PMT photon detection and photoelectron generation
- Coincidence event identification

The simulation is used to investigate detection efficiency, optical response, and the effects of detector geometry.

## Data Analysis

### 1. Coincidence Analysis

The main analysis focuses on identifying events detected by both scintillators.

The investigated quantities include:

- Individual detector counting rates
- Two-detector coincidence rates
- Coincidence efficiency
- Relative timing between detector signals
- Dependence of coincidence detection on detector configuration

### 2. PMT Signal Analysis

Oscilloscope signals are analyzed to characterize the detector response.

The analysis includes:

- Waveform visualization and pulse characterization
- Baseline estimation and noise analysis
- Signal threshold determination
- Pulse amplitude measurements
- Time differences between signals recorded by the two PMTs

These analyses help evaluate the performance of the detection system and identify coincident events.

### 3. Optical Photon and Photoelectron Analysis

Geant4 simulation outputs are analyzed to study the optical response of the scintillator–PMT system.

The investigated quantities include:

- Number of scintillation photons reaching each PMT
- Number of detected photoelectrons
- Photon arrival-time distributions
- Energy deposition in each scintillator
- Simulated coincidence detection efficiency

### 4. Simulation and Experimental Studies

The Geant4 simulation provides a framework for understanding the experimental response of the telescope.

Detector geometry, optical properties, and signal detection conditions are investigated to support the interpretation of experimental coincidence measurements.

## Software and Tools

- **Geant4:** Particle transport and optical detector simulation
- **C++:** Simulation development
- **ROOT:** Data analysis, histogramming, and visualization
- **Python:** Experimental data processing
- **CMake:** Simulation build configuration

## Research Significance

The muon telescope serves as an initial experimental and computational platform for cosmic-ray muon detection studies.

The coincidence measurements and detector characterization provide a foundation for further studies involving different detector geometries, material interactions, and muon tomography applications.

## Research Context

This work is part of an undergraduate research project at **Middle East Technical University (METU)** focusing on cosmic-ray muon detection and the development of a muon tomography system.
