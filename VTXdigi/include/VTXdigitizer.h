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
#include "edm4hep/TrackerHitSimTrackerHitLinkCollection.h"

// DD4HEP
#include "DD4hep/Detector.h" // for dd4hep::VolumeManager
#include "DDRec/SurfaceManager.h"
#include "DDRec/Vector3D.h"

#include "DDSegmentation/BitFieldCoder.h"

#include "TRandom3.h"

#include <vector>

/** @class VTXdigitizer
 *
 *  Algorithm for creating digitized (meaning 'reconstructed' for now) vertex detector hits (edm4hep::TrackerHit3D) from
 * Geant4 hits (edm4hep::SimTrackerHit).
 *
 *  @author Brieuc Francois, Armin Ilg
 *  @date   2023-03
 *
 */

class VTXdigitizer : public Gaudi::Algorithm {
public:
  explicit VTXdigitizer(const std::string&, ISvcLocator*);
  virtual ~VTXdigitizer();
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
  // Input sim vertex hit collection name
  mutable k4FWCore::DataHandle<edm4hep::SimTrackerHitCollection> m_input_sim_hits{"inputSimHits",
                                                                                  Gaudi::DataHandle::Reader, this};
  // Output digitized vertex hit collection name
  mutable k4FWCore::DataHandle<edm4hep::TrackerHit3DCollection> m_output_digi_hits{"outputDigiHits",
                                                                                   Gaudi::DataHandle::Writer, this};
  // Output link between sim hits and digitized hits
  mutable k4FWCore::DataHandle<edm4hep::TrackerHitSimTrackerHitLinkCollection> m_output_sim_digi_link{
      "outputSimDigiAssociation", Gaudi::DataHandle::Writer, this};

  // Detector name
  Gaudi::Property<std::string> m_detectorName{this, "detectorName", "Vertex", "Name of the detector (default: Vertex)"};
  // Detector readout names
  Gaudi::Property<std::string> m_readoutName{this, "readoutName", "VertexBarrelCollection",
                                             "Name of the detector readout"};
  // Pointer to the geometry service
  ServiceHandle<IGeoSvc> m_geoSvc;
  // Decoder for the cellID
  dd4hep::DDSegmentation::BitFieldCoder* m_decoder;
  // Volume manager to get the physical cell sensitive volume
  dd4hep::VolumeManager m_volman;

  // x resolution in mm
  Gaudi::Property<std::vector<float>> m_x_resolution{
      this,
      "xResolution",
      {0.1},
      "Spatial resolutions in the x direction per layer [mm] (r-phi direction in barrel, z direction in disks)"};

  // y resolution in mm
  Gaudi::Property<std::vector<float>> m_y_resolution{
      this,
      "yResolution",
      {0.1},
      "Spatial resolutions in the y direction per layer [mm] (r direction in barrel, r-phi direction in disks)"};

  // t resolution in ns
  Gaudi::Property<std::vector<float>> m_t_resolution{this, "tResolution", {0.1}, "Time resolutions per layer [ns]"};

  // Surface manager used to project hits onto sensitive surface with forceHitsOntoSurface argument
  mutable const dd4hep::rec::SurfaceMap* _map;

  // Option to force hits onto sensitive surface
  BooleanProperty m_forceHitsOntoSurface{
      this, "forceHitsOntoSurface", false,
      "Project hits onto the surface in case they are not yet on the surface (default: false"};

  // Event header, used to seed the random number engine for each event
  mutable k4FWCore::DataHandle<edm4hep::EventHeaderCollection> m_headers{"EventHeader", Gaudi::DataHandle::Reader,
                                                                         this};
  // Unique ID service, used to seed the random number engine for each event
  SmartIF<IUniqueIDGenSvc> m_uidSvc;
  // Random number engine, re-seeded from the UniqueIDGenSvc at the start of every event, so that the result depends
  // only on (seed, run number, event number, algorithm name) and not on the order in which events are processed
  inline static thread_local TRandom3 m_engine;
};
