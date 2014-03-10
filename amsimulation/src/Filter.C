#include <iostream>
#include <math.h>

#include "AMSimulationFunctions.h"

#include "TChain.h"

int main(int av, char** ac)
{
  TChain *inputChain = new TChain("L1TrackTrigger");
  std::cout << "ac[1] = " << ac[1] << std::endl;
  inputChain->Add(ac[1]);

  cout << " Creating new root-file ..."<< endl;
  TFile *newFile = new TFile(std::string("small_"+std::string(ac[2])+".root").data(),"recreate");
  cout << " Creating new tree ..."<< endl;
  TChain *newchain = (TChain*)inputChain->CloneTree(0);
  TTree *tree = newchain->GetTree();

  // Containers to load the TTree branches
  int m_stub;
  // vector<int>                   m_stub_tp;  // tp of the particule
  vector<int>                   m_stub_layer;  // Layer du stub (5 a 10 pour les 6 layers qui nous interessent)
  vector<int>                   m_stub_module; // Position en Z du module contenant le stub
  vector<int>                   m_stub_ladder; // Position en PHI du module contenant le stub
  vector<int>                   m_stub_seg;    // Segment du module contenant le stub
  vector<int>                   m_stub_strip;  // Strip du cluster interne du stub
  vector<float>                 m_stub_pxGEN;  // Px de la particule initiale (en GeV/c)
  vector<float>                 m_stub_pyGEN;  // Py de la particule initiale (en GeV/c)
  vector<float>                 m_stub_etaGEN;  // Eta de la particule initiale

  // vector<int>                   *p_m_stub_tp;
  vector<int>                   *p_m_stub_layer;
  vector<int>                   *p_m_stub_module;
  vector<int>                   *p_m_stub_ladder;
  vector<int>                   *p_m_stub_seg;
  vector<int>                   *p_m_stub_strip;
  vector<float>                 *p_m_stub_pxGEN;
  vector<float>                 *p_m_stub_pyGEN;
  vector<float>                 *p_m_stub_etaGEN;


  //  p_m_stub_tp = &m_stub_tp;
  p_m_stub_layer = &m_stub_layer;
  p_m_stub_module = &m_stub_module;
  p_m_stub_ladder = &m_stub_ladder;
  p_m_stub_seg = &m_stub_seg;
  p_m_stub_strip = &m_stub_strip;
  p_m_stub_pxGEN = &m_stub_pxGEN;
  p_m_stub_pyGEN = &m_stub_pyGEN;
  p_m_stub_etaGEN = &m_stub_etaGEN;

  inputChain->SetBranchAddress("STUB_n",         &m_stub);
  //  inputChain->SetBranchAddress("STUB_tp",        &p_m_stub_tp);
  inputChain->SetBranchAddress("STUB_layer",     &p_m_stub_layer);
  inputChain->SetBranchAddress("STUB_module",    &p_m_stub_module);
  inputChain->SetBranchAddress("STUB_ladder",    &p_m_stub_ladder);
  inputChain->SetBranchAddress("STUB_seg",       &p_m_stub_seg);
  inputChain->SetBranchAddress("STUB_strip",     &p_m_stub_strip);
  inputChain->SetBranchAddress("STUB_pxGEN",     &p_m_stub_pxGEN);
  inputChain->SetBranchAddress("STUB_pyGEN",     &p_m_stub_pyGEN);
  inputChain->SetBranchAddress("STUB_etaGEN",    &p_m_stub_etaGEN);


  namespace po = boost::program_options;
  po::variables_map vm;
  po::options_description desc("Allowed options");
  fillConfigurationFromFile(av, ac, desc, vm, "amsimulation.cfg");

  // vector<int> layers;
  SectorTree st;
  int stripSize=0;
  int dcBits=0;
  string partDirName="";
  string bankFileName="";
  string rootFileName="";
  string activeLayers = "";
  vector<int> active_layers;
  vector<int> forced_layers;
  vector<int> desactivated_layers;
  float threshold=0;
  float min=0;
  float max=0;
  float minEta=0;
  float maxEta=0;
  int maxNbFake=0;
  int sector_tklayout_id=0;
  map<int,pair<float,float> > eta = CMSPatternLayer::getLayerDefInEta();

  try{
    stripSize=vm["ss_size"].as<int>();
    cout<<"Superstrip size : "<<stripSize<<endl;
    dcBits=vm["dc_bits"].as<int>();
    cout<<"DC bits number : "<<dcBits<<endl;
    min=vm["pt_min"].as<int>();
    cout<<"PT min : "<<min<<endl;
    max=vm["pt_max"].as<int>();
    cout<<"PT max : "<<max<<endl;
    minEta=vm["eta_min"].as<float>();
    cout<<"ETA min : "<<minEta<<endl;
    maxEta=vm["eta_max"].as<float>();
    cout<<"ETA max : "<<maxEta<<endl;
    maxNbFake=vm["maxFakeSStrips"].as<int>();
    cout<<"Max number of fake superstrips : "<<maxNbFake<<endl;
    cout<<"Coverage : "<<vm["coverage"].as<float>()<<endl;
    partDirName=vm["input_directory"].as<string>();
    cout<<"Using particules from "<<partDirName<<endl;
    bankFileName=vm["bank_name"].as<string>();
    cout<<"Output file name is "<<bankFileName<<endl;
    activeLayers=vm["active_layers"].as<string>();
    cout<<"Using layers "<<activeLayers<<endl;
    sector_tklayout_id=vm["sector_id"].as<int>();
    cout<<"Using sector "<<sector_tklayout_id<<" from "<<vm["sector_file"].as<string>()<<endl;

    //Get the active/forced/inactive layers
    //layersWithoutSigns : list of all layers

    //forceLayers : layers prefixed with a '+' : no fake stub will be allowed
    //if there is no stub on this layer -> there will be no pattern

    //inactiveLayers : layers prefixed with a '-' : only fake stubs on this layer
    //even if there is a stub it will be replaced with a fake one

    string layersWithoutSigns = activeLayers;
    string forceLayers="";
    string inactiveLayers="";
    size_t found = layersWithoutSigns.find("+");
    while (found!=string::npos){
      layersWithoutSigns.erase(found,1);//remove the '+'
      size_t endIndex = layersWithoutSigns.find(" ", found);
      if(endIndex!=string::npos)
        endIndex=endIndex-found+1;
      forceLayers.append(layersWithoutSigns.substr(found,endIndex));//add the layer number and the following space
      found = layersWithoutSigns.find("+"); // search for the next '+'
    }
    found = layersWithoutSigns.find("-");
    while (found!=string::npos){
      layersWithoutSigns.erase(found,1);
      size_t endIndex = layersWithoutSigns.find(" ", found);
      if(endIndex!=string::npos)
        endIndex=endIndex-found+1;
      inactiveLayers.append(layersWithoutSigns.substr(found,endIndex));
      found = layersWithoutSigns.find("-");
    }

    std::istringstream is( layersWithoutSigns );
    int n;
    while( is >> n ) {
      active_layers.push_back(n);
    }
    std::istringstream fl( forceLayers );
    while( fl >> n ) {
      forced_layers.push_back(n);
    }
    std::istringstream il( inactiveLayers );
    while( il >> n ) {
      desactivated_layers.push_back(n);
    }

    //remove the force_layers from the eta list -> no fake stub will be added
    for(unsigned int i=0;i<forced_layers.size();i++){
      map<int,pair<float,float> >::iterator it = eta.find(forced_layers[i]);
      eta.erase(it);
    }

    //change the eta definition of the desactivated layers
    //they will not be reachable and a fake stub will be created
    for(unsigned int i=0;i<desactivated_layers.size();i++){
      eta[desactivated_layers[i]].first=10;
      eta[desactivated_layers[i]].second=10;
    }

    size_t end_index = bankFileName.find(".pbk");
    if(end_index==string::npos)
      end_index=bankFileName.length()-4;
    rootFileName = bankFileName.substr(0,end_index)+"_report.root";
    threshold=vm["coverage"].as<float>();
    createSectorFromRootFile(&st,vm["sector_file"].as<string>(), active_layers, sector_tklayout_id);
  }
  catch(boost::bad_any_cast e){
    cout<<"At least one option is missing! Please check : "<<endl;
    cout<<desc<<endl;
    return -1;
  }

  vector<Sector*> list = st.getAllSectors();
  cout<<"Sector :"<<endl;
  for(unsigned int i=0;i<list.size();i++){
    cout<<*list[i];
    cout<<endl;
  }

  if(active_layers.size()>8){
    cout<<"ERROR : your sector contains "<<active_layers.size()<<" layers : maximum number of layers is 8!"<<endl;
    return -1;
  }


  std::cout << "active_layers = " << active_layers.size() << std::endl;


  // Loop on the events and save what is needed

  if (inputChain == 0) return 1;
  Int_t nb_entries = Int_t(inputChain->GetEntries());
  std::cout<<nb_entries<<" events found."<<std::endl;

  int layers[active_layers.size()];
  std::vector<int> ladder_per_layer(active_layers.size());
  std::vector<int> module_per_layer(active_layers.size());

  Int_t nbytes = 0, nb = 0;
  for( Int_t jentry=0; jentry < nb_entries; ++jentry ) {
    // Int_t ientry =
    inputChain->LoadTree(jentry); //in case of a TChain,
    nb = inputChain->GetEntry(jentry);
    nbytes += nb;

    if( (jentry+1)%(nb_entries/100) == 0 ) {
      std::cout << "Analyzed " << int((jentry+1)/(nb_entries/100)) << "% of the events" << std::endl;
    }

    // // Check that the track is in the desired pt and eta range
    // if( m_stub > 0 ) {
    //   float stubPt = sqrt(pow((*p_m_stub_pxGEN)[0],2) + pow((*p_m_stub_pyGEN)[0],2));
    //   if( stubPt < min || stubPt > max || (*p_m_stub_etaGEN)[0] < minEta || (*p_m_stub_etaGEN)[0] > maxEta ) continue;
    // }

    //initialize arrays
    for(unsigned int j=0;j<active_layers.size();j++){
      layers[j]=-1;
    }
    for(unsigned int j=0;j<active_layers.size();j++){
      ladder_per_layer[j]=-1;
    }
    for(unsigned int j=0;j<active_layers.size();j++){
      module_per_layer[j]=-1;
    }

    float current_eta = -10;















    // ----------------------------------------------------------------------------------
    // GENERATED PT AND ETA ARE THE SAME FOR ALL STUBS. MOVE THE CHECKS OUTSIDE THE LOOP.
    // ----------------------------------------------------------------------------------
    // Might be an issue if there is more than one generated track in the event. Leave it like this for now.
    // -----------------------------------------------------------------------------------------------------

    //check the layers of the stubs
    for(int j=0;j<m_stub;j++){
      //if(m_stub_tp[j]!=0){//The stub is not coming from the primary particule : we do not use it for pattern generation
      //	continue;
      //}
      if(m_stub_etaGEN[j]<minEta){// eta of the generating particule is bellow the threshold -> we do not use it for pattern generation
        continue;
      }
      if(m_stub_etaGEN[j]>maxEta){// eta of the generating particule is above the threshold -> we do not use it for pattern generation
        continue;
      }
      float pt_GEN = sqrt(m_stub_pxGEN[j]*m_stub_pxGEN[j]+m_stub_pyGEN[j]*m_stub_pyGEN[j]);
      if(pt_GEN<min){// The PT of the generating particule is below the minimum required -> we do not use it for pattern generation
        continue;
      }
      if(pt_GEN>max){// The PT of the generating particule is above the maximum accepted -> we do not use it for pattern generation
        continue;
      }
      int layer = m_stub_layer[j];

//      vector<int>::iterator iter;
//      iter=find(inactive_layers.begin(),inactive_layers.end(),layer);
//      if(iter!=inactive_layers.end()){
//        continue;
//      }

      int layer_position=-1;
      for(unsigned int cpt=0;cpt<active_layers.size();cpt++){
        if(layer==active_layers[cpt]){
          layer_position=(int)cpt;
          break;
        }
      }

      if(layer_position!=-1){ // is this layer in the layer list?
        layers[layer_position]=j;
        ladder_per_layer[layer_position]=CMSPatternLayer::getLadderCode(layer, m_stub_ladder[j]);
        short module = -1;
        module = CMSPatternLayer::getModuleCode(layer, m_stub_module[j]);
        module_per_layer[layer_position]=module;
      }

      current_eta = m_stub_etaGEN[j];

    }





    /**************************************
    Selection on the stubs/layer
    We need at least one stub per layer
    **************************************/
    bool missing_stub = false;
    int nbFakeSuperstrip = 0;
    for(unsigned int j=0;j<active_layers.size();j++) {
      if(layers[j]==-1) {
        missing_stub=true;
        // std::cout << "layers["<<j<<"] = " << layers[j] << std::endl;
        if(st.getNbSectors()==1 && current_eta!=-10) { // we can use fake superstrips if we know the sector in which to add the tracks and
          // if we have at least one stub (current_eta!=10)
          if(eta.find(active_layers[j])!=eta.end()) {//we have eta boundaries for this layer
            // std::cout << "j = " << j << std::endl;
            pair<float,float> limits = eta[active_layers[j]];
            if(current_eta<limits.first || current_eta>limits.second){ // we are outside the eta limits for this layer -> we will add a fake
              // superstrip for this layer
              if(nbFakeSuperstrip<maxNbFake){//we don't want to have more than nbMaxFakeSuperstrips fake superstrips in the pattern
                // std::cout<<"missing hit on layer "<<layers[j]<<" for track with eta="<<current_eta<<std::endl;
                layers[j]=-2;
                //we put a ladder and a module just to be inside the sector
                // std::cout << "allSectors size = " << st.getAllSectors()[0]->getLadders(j).size() << std::endl;
                ladder_per_layer[j]=st.getAllSectors()[0]->getLadders(j)[0];
                // std::cout << "here" << std::endl;
                module_per_layer[j]=st.getAllSectors()[0]->getModules(j,ladder_per_layer[j])[0];
                // std::cout<<"Add stub for sector : "<<ladder_per_layer[j]<<" / "<<module_per_layer[j]<<std::endl;
                //debug=true;
                missing_stub=false;//we will create a fake superstrip, so the stub is not missing
                nbFakeSuperstrip++;
              }
            }
          }
        }
      }
      if(missing_stub)
        break;

      // std::cout << "missing_stub = " << missing_stub << std::endl;
      if(missing_stub) {
        /*
      cout<<"stubs manquants ";
      for(unsigned int j=0;j<tracker_layers.size();j++){
    cout<<layers[j]<<",";
      }
      cout<<endl;
      */
          continue;//no stub on each layer -> drop the event
      }
    }

    // nbInLayer++;

    //    cout<<"trace ok"<<endl;

    /****************************************
    Check that the track is part of a sector
    ****************************************/



    // Check that the track is in one of the desired sectors
    Sector* sector = st.getSector(ladder_per_layer, module_per_layer);
    if(sector==NULL){
      //cout<<"No sector found"<<endl;
      continue;
    }


//    for( int j = 0; j < m_stub; ++j ) {
//      float stubPt = sqrt(pow((*p_m_stub_pxGEN)[j],2) + pow((*p_m_stub_pyGEN)[j],2));
//      std::cout << "stubPt["<<jentry<<"] = " << stubPt << std::endl;
//    }
    // if( ientry > 10 ) std::exit(1);
    tree->Fill();

  } // end loop over events

  newFile->cd();
  tree->Write();
}
