#include <ostream>

#include "IOMC/ParticleGuns/interface/FlatRandomOneOverPtGunProducer.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "CLHEP/Units/GlobalSystemOfUnits.h"
using namespace edm;

FlatRandomOneOverPtGunProducer::FlatRandomOneOverPtGunProducer(const edm::ParameterSet& pset) : 
  BaseFlatGunProducer(pset) {


  edm::ParameterSet defpset ;
  edm::ParameterSet pgun_params = 
    pset.getParameter<ParameterSet>("PGunParameters") ;
  
  fMinOneOverPt = pgun_params.getParameter<double>("MinOneOverPt");
  fMaxOneOverPt = pgun_params.getParameter<double>("MaxOneOverPt");
  
  fXFlatSpread = pgun_params.getParameter<double>("XFlatSpread");
  fYFlatSpread = pgun_params.getParameter<double>("YFlatSpread");
  fZFlatSpread = pgun_params.getParameter<double>("ZFlatSpread");
  spreadVertex = pgun_params.getParameter<bool>("SpreadVertex");

  produces<HepMCProduct>();
  produces<GenEventInfoProduct>();

  edm::LogInfo("ParticleGun") << "FlatRandomOneOverPtGunProducer: initialized with minimum and maximum 1/pt " << fMinOneOverPt << ":" << fMaxOneOverPt;
}

FlatRandomOneOverPtGunProducer::~FlatRandomOneOverPtGunProducer() {
  // no need to cleanup GenEvent memory - done in HepMCProduct
}

void FlatRandomOneOverPtGunProducer::produce(Event &e, const EventSetup& es) {

  LogDebug("ParticleGun") << " FlatRandomOneOverPtGunProducer : Begin New Event Generation"; 

  // event loop (well, another step in it...)
          
  // no need to clean up GenEvent memory - done in HepMCProduct
  // 
   
  // here re-create fEvt (memory)
  //
  fEvt = new HepMC::GenEvent() ;
   
  // now actualy, cook up the event from PDGTable and gun parameters
  //
  // 1st, primary vertex
  //
  double xpos = 0.;
  double ypos = 0.;
  double zpos = 0.;
  if( spreadVertex ) {
    xpos = fRandomGenerator->fire(-fXFlatSpread,fYFlatSpread);
    ypos = fRandomGenerator->fire(-fYFlatSpread,fXFlatSpread);
    zpos = fRandomGenerator->fire(-fZFlatSpread,fZFlatSpread);
  }
  HepMC::GenVertex* Vtx = new HepMC::GenVertex(HepMC::FourVector(xpos,ypos,zpos));

  // loop over particles
  //
  int barcode = 1 ;
  for (unsigned int ip=0; ip<fPartIDs.size(); ++ip) {

    double xx     = fRandomGenerator->fire(0.0,1.0);
    double pt     = std::exp((1.-xx)*std::log(fMinOneOverPt)+
			     xx*std::log(fMaxOneOverPt)) ;
    double eta    = fRandomGenerator->fire(fMinEta, fMaxEta) ;
    double phi    = fRandomGenerator->fire(fMinPhi, fMaxPhi) ;
    if (pt != 0) pt = 1./pt;

    int PartID = fPartIDs[ip] ;
    double charge   = fRandomGenerator->fire(0,1);
    if (charge<0.5) PartID = -PartID;

    const HepPDT::ParticleData* 
      PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID))) ;
    double mass   = PData->mass().value() ;
    double theta  = 2.*atan(exp(-eta)) ;
    double mom    = pt/sin(theta) ;
    double px     = pt*cos(phi) ;
    double py     = pt*sin(phi) ;
    double pz     = mom*cos(theta) ;
    double energy2= mom*mom + mass*mass ;
    double energy = sqrt(energy2) ; 
    HepMC::FourVector p(px,py,pz,energy) ;
    HepMC::GenParticle* Part = new HepMC::GenParticle(p,PartID,1);
    Part->suggest_barcode( barcode ) ;
    barcode++ ;
    Vtx->add_particle_out(Part);
    LogDebug("ParticleGun") << "FlatRandomOneOverPtGunProducer: Event generated with pt:eta:phi " << pt << " " << eta << " " << phi << " (" << theta/CLHEP::deg << ":" << phi/CLHEP::deg << ")";

    if ( fAddAntiParticle ) {
      HepMC::FourVector ap(-px,-py,-pz,energy) ;
      int APartID = -PartID ;
      if ( PartID == 22 || PartID == 23 ) {
	APartID = PartID ;
      }	  
      HepMC::GenParticle* APart = new HepMC::GenParticle(ap,APartID,1);
      APart->suggest_barcode( barcode ) ;
      barcode++ ;
      Vtx->add_particle_out(APart) ;
    }

  }

  fEvt->add_vertex(Vtx) ;
  fEvt->set_event_number(e.id().event()) ;
  fEvt->set_signal_process_id(20) ; 
        
  if ( fVerbosity > 0 ) {
    fEvt->print() ;  
  }

  std::auto_ptr<HepMCProduct> BProduct(new HepMCProduct()) ;
  BProduct->addHepMCData( fEvt );
  e.put(BProduct);

  std::auto_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
  e.put(genEventInfo);
    
  LogDebug("ParticleGun") << " FlatRandomOneOverPtGunProducer : Event Generation Done ";
}
