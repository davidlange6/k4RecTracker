#pragma once

// GAUDI
#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"

// K4FWCORE
#include "k4FWCore/DataHandle.h"
#include "k4Interface/IGeoSvc.h"
#include "k4Interface/IUniqueIDGenSvc.h"

// EDM4HEP & PODIO
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "podio/UserDataCollection.h"

// EDM4HEP extension
#include "extension/DriftChamberDigiCollection.h"
#include "extension/DriftChamberDigiLocalCollection.h"
#include "extension/MCRecoDriftChamberDigiAssociationCollection.h"

// DD4HEP
#include "DD4hep/Detector.h" // for dd4hep::VolumeManager
#include "DDSegmentation/BitFieldCoder.h"

#include "TRandom3.h"

/** @class DCHsimpleDigitizerExtendedEdm
 *
 *  Algorithm for creating digitized drift chamber hits (extension::DriftChamberDigi) from edm4hep::SimTrackerHit.
 *  You have to specify the expected resolution in z and in xy (distance to the wire). The smearing is applied in the
 * wire reference frame.
 *
 *  @author Brieuc Francois
 *  @date   2023-03
 *
 */

class DCHsimpleDigitizerExtendedEdm : public Gaudi::Algorithm {
public:
  explicit DCHsimpleDigitizerExtendedEdm(const std::string&, ISvcLocator*);
  virtual ~DCHsimpleDigitizerExtendedEdm();
  /**  Initialize.
   *   @return status code
   */
  virtual StatusCode initialize() final;
  /**  Execute.
   *   @return status code
   */
  virtual StatusCode execute(const EventContext&) const final;
  /**  Finalize.
   *   @return status code
   */
  virtual StatusCode finalize() final;

private:
  // Input sim tracker hit collection name
  mutable k4FWCore::DataHandle<edm4hep::SimTrackerHitCollection> m_input_sim_hits{"inputSimHits",
                                                                                  Gaudi::DataHandle::Reader, this};
  // Output digitized tracker hit collection name
  mutable k4FWCore::DataHandle<extension::DriftChamberDigiCollection> m_output_digi_hits{
      "outputDigiHits", Gaudi::DataHandle::Writer, this};
  // Output association between digitized and simulated hit collections
  mutable k4FWCore::DataHandle<extension::MCRecoDriftChamberDigiAssociationCollection> m_output_sim_digi_association{
      "outputSimDigiAssociation", Gaudi::DataHandle::Writer, this};
  // Output digitized tracker hit in local coordinates collection name. Only filled in debug mode
  mutable k4FWCore::DataHandle<extension::DriftChamberDigiLocalCollection> m_output_digi_local_hits{
      "outputDigiLocalHits", Gaudi::DataHandle::Writer, this};

  // Detector readout name
  Gaudi::Property<std::string> m_readoutName{this, "readoutName", "CDCHHits", "Name of the detector readout"};
  // Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;
  // Decoder for the cellID
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  // Volume manager to get the physical cell sensitive volume
  dd4hep::VolumeManager m_volman;

  // z position resolution in mm
  FloatProperty m_z_resolution{this, "zResolution", 1.0,
                               "Spatial resolution in the z direction (from reading out the wires at both sides) [mm]"};
  // xy resolution in mm
  FloatProperty m_xy_resolution{this, "xyResolution", 0.1, "Spatial resolution in the xy direction [mm]"};
  // Flag to produce debugging distributions
  Gaudi::Property<bool> m_debugMode{this, "debugMode", false, "Flag to produce debugging distributions"};
  // Declaration of debugging distributions
  mutable k4FWCore::DataHandle<podio::UserDataCollection<double>> m_leftHitSimHitDeltaDistToWire{
      "leftHitSimHitDeltaDistToWire", Gaudi::DataHandle::Writer, this}; // mm
  mutable k4FWCore::DataHandle<podio::UserDataCollection<double>> m_leftHitSimHitDeltaLocalZ{
      "leftHitSimHitDeltaLocalZ", Gaudi::DataHandle::Writer, this}; // mm
  mutable k4FWCore::DataHandle<podio::UserDataCollection<double>> m_rightHitSimHitDeltaDistToWire{
      "rightHitSimHitDeltaDistToWire", Gaudi::DataHandle::Writer, this}; // mm
  mutable k4FWCore::DataHandle<podio::UserDataCollection<double>> m_rightHitSimHitDeltaLocalZ{
      "rightHitSimHitDeltaLocalZ", Gaudi::DataHandle::Writer, this}; // mm

  // Event header, used to seed the random number engine for each event
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_headers{"EventHeader", Gaudi::DataHandle::Reader,
                                                                         this};
  // Unique ID service, used to seed the random number engine for each event
  SmartIF<IUniqueIDGenSvc> m_uidSvc;
  // Random number engine, re-seeded from the UniqueIDGenSvc at the start of every event, so that the result depends
  // only on (seed, run number, event number, algorithm name) and not on the order in which events are processed
  inline static thread_local TRandom3 m_engine;
};
