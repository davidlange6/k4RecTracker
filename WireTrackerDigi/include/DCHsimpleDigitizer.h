#pragma once

// GAUDI
#include "Gaudi/Algorithm.h"
#include "Gaudi/Property.h"

// K4FWCORE
#include "k4FWCore/DataHandle.h"
#include "k4Interface/IGeoSvc.h"
#include "k4Interface/IUniqueIDGenSvc.h"

// EDM4HEP
#include "edm4hep/EventHeaderCollection.h"
#include "edm4hep/SimTrackerHitCollection.h"
#include "edm4hep/TrackerHit3DCollection.h"

// DD4HEP
#include "DD4hep/Detector.h" // for dd4hep::VolumeManager
#include "DDSegmentation/BitFieldCoder.h"

#include "TRandom3.h"

/** @class DCHsimpleDigitizer
 *
 *  Algorithm for creating digitized drift chamber hits (still based on edm4hep::TrackerHit3D) from
 * edm4hep::SimTrackerHit. You have to specify the expected resolution in z and in xy (distance to the wire). The
 * smearing is applied in the wire reference frame.
 *
 *  @author Brieuc Francois
 *  @date   2023-03
 *
 */

class DCHsimpleDigitizer : public Gaudi::Algorithm {
public:
  explicit DCHsimpleDigitizer(const std::string&, ISvcLocator*);
  virtual ~DCHsimpleDigitizer();
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
  mutable k4FWCore::DataHandle<edm4hep::TrackerHit3DCollection> m_output_digi_hits{"outputDigiHits",
                                                                                   Gaudi::DataHandle::Writer, this};

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

  // Event header, used to seed the random number engine for each event
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_headers{"EventHeader", Gaudi::DataHandle::Reader,
                                                                         this};
  // Unique ID service, used to seed the random number engine for each event
  SmartIF<IUniqueIDGenSvc> m_uidSvc;
  // Random number engine, re-seeded from the UniqueIDGenSvc at the start of every event, so that the result depends
  // only on (seed, run number, event number, algorithm name) and not on the order in which events are processed
  inline static thread_local TRandom3 m_engine;
};
