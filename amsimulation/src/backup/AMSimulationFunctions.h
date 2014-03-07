#ifndef AMSIMULATIONFUNCTIONS_H
#define AMSIMULATIONFUNCTIONS_H

#include <fstream>
#include <sstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>
#include <boost/progress.hpp>

#include <iostream>

#include "PatternTree.h"
#include "PatternGenerator.h"
#include "PatternFinder.h"
#include "SectorTree.h"
#include "Detector.h"
#include "PrincipalTrackFitter.h"
#include "PrincipalFitGenerator.h"


#include <TH1I.h>
#include <TFile.h>

#ifndef __APPLE__
BOOST_CLASS_EXPORT_IMPLEMENT(CMSPatternLayer)
BOOST_CLASS_EXPORT_IMPLEMENT(PrincipalTrackFitter)
#endif



using namespace std;

bool sorting (GradedPattern* p1, GradedPattern* p2) { return (*p2<*p1); }

void getLayers(vector<int> &l){
  cout<<"Enter the layer numbers (separated by spaces) :"<<endl;
  string result;
  getline(cin, result);
  std::istringstream is( result );
  int n;
  while( is >> n ) {
    l.push_back(n);
  }
}

float getPhiMin(){
  cout<<"Enter the minimum PHI0 :"<<endl;
  float result;
  cin>>result;
  if(result<0)
    result=0;
  return result;
}

float getPhiMax(){
  cout<<"Enter the maximum PHI0 :"<<endl;
  float result;
  cin>>result;
  if(result<0)
    result=0;
  return result;
}

float getEtaMin(){
  cout<<"Enter the minimum ETA :"<<endl;
  float result;
  cin>>result;
  if(result<0)
    result=0;
  return result;
}

float getEtaMax(){
  cout<<"Enter the maximum ETA :"<<endl;
  float result;
  cin>>result;
  if(result<0)
    result=0;
  return result;
}

string getSectorDefFileName(){
  cout<<"Enter the sector definition file name :"<<endl;
  string result;
  cin>>result;
  return result;
}

void getSectors(const vector<int> &layers, SectorTree &st){
  bool go=true;
  int sectorNb=1;
  while(go){
    cout<<"Enter the sector num "<<sectorNb<<" : "<<endl;
    Sector s(layers);
    for(unsigned int i=0;i<layers.size();i++){
      cout<<"\tFirst Ladder for layer "<<layers[i]<<" :"<<endl;
      int first;
      int nb;
      cin>>first;
      cout<<"\tNumber of ladders for layer "<<layers[i]<<" :"<<endl;
      cin>>nb;
      s.addLadders(layers[i],first,nb);
      if(!go)
    break;
    }
    if(go){
      st.addSector(s);
      sectorNb++;
    }
  }
}

vector< vector<int> > getRestrictions(const vector<int> &layers){
  vector<vector<int> > res;
  for(unsigned int i=0;i<layers.size();i++){
    vector<int> l;
    cout<<"\tLadder numbers for layer "<<layers[i]<<" (separated by spaces) :"<<endl;
    string result;
    getline(cin, result);
    getline(cin, result);
    std::istringstream is( result );
    int n;
    while( is >> n ) {
      if(n<0){
    break;
      }
      l.push_back(n);
    }
    if(l.size()>0)
      res.push_back(l);
    else
      return res;
  }
  return res;
}

void createAnalysis(SectorTree &st){
  vector<Sector*> list = st.getAllSectors();
  int nbLayers = 0;
  if(list.size()>0)
    nbLayers = list[0]->getNbLayers();
  vector<TH1I*> modulesPlot;

  for(int i=0;i<nbLayers;i++){
    ostringstream oss;
    oss<<"Layer "<<i;
    modulesPlot.push_back(new TH1I(oss.str().c_str(),"Module Z position", 14, 0, 14));
  }
  // We put all patterns in the same vector
  vector<GradedPattern*> allPatterns;
  int nbTracks=0;
  for(unsigned int i=0;i<list.size();i++){
    vector<GradedPattern*> patterns = list[i]->getPatternTree()->getLDPatterns();
    for(unsigned int j=0;j<patterns.size();j++){
      nbTracks+=patterns[j]->getGrade();
      allPatterns.push_back(patterns[j]);
    }
  }
  //sorting the patterns
  sort(allPatterns.begin(), allPatterns.end(), sorting);

  //float patterns[allPatterns.size()];
  //float tracks[allPatterns.size()];
  //float tracks_nb[allPatterns.size()];
  //float average_pt[allPatterns.size()];
  //int coveredTracks = 0;
  //TH1I* histo = new TH1I("Pattern Eff","Nb Tracks per pattern", allPatterns[0]->getGrade()+10, 0, allPatterns[0]->getGrade()+10);

  //Creates the layer plots
  /*
  for(unsigned int k=0;k<allPatterns.size();k++){
    patterns[k]=k;
    coveredTracks+=allPatterns[k]->getGrade();
    tracks[k]=coveredTracks*100/(float)nbTracks;
    histo->Fill(allPatterns[k]->getGrade());
    average_pt[k]=allPatterns[k]->getAveragePt();
    tracks_nb[k]=allPatterns[k]->getGrade();
    for(int j=0;j<nbLayers;j++){
      CMSPatternLayer* pl = (CMSPatternLayer*)allPatterns[k]->getLayerStrip(j);
      modulesPlot[j]->Fill(pl->getModule());
    }
  }
  histo->SetFillColor(41);
  histo->Write();
  delete histo;
  */
  for(unsigned int k=0;k<list.size();k++){
    vector<int> PT = list[k]->getPatternTree()->getPTHisto();
    TH1I* pt_histo = new TH1I("PT sector "+k,"PT of pattern generating tracks", 110, 0, 110);
    for(int i=0;i<101;i++){
      //cout<<PT[i]<<"-";
      for(int j=0;j<PT[i];j++){
    pt_histo->Fill(i);
      }
    }
    //cout<<endl;
    pt_histo->SetFillColor(41);
    pt_histo->Write();
    delete pt_histo;
  }

  /*
  TGraph* nbPatt = new TGraph(allPatterns.size(),patterns,tracks);
  nbPatt->GetXaxis()->SetTitle("Patterns bank size");
  nbPatt->GetYaxis()->SetTitle("Tracks covered (%)");
  nbPatt->SetTitle("Patterns coverage");
  nbPatt->Write();
  delete nbPatt;
  */
  /*
  TGraph* ptFreq = new TGraph(allPatterns.size(),tracks_nb,average_pt);
  ptFreq->GetXaxis()->SetTitle("nb tracks per pattern");
  ptFreq->GetYaxis()->SetTitle("average Pt per pattern");
  ptFreq->SetTitle("Average Pt / pattern size");
  ptFreq->Write();
  delete ptFreq;
  */
  for(int i=0;i<nbLayers;i++){
    modulesPlot[i]->Write();
    delete modulesPlot[i];
  }

  for(unsigned int k=0;k<allPatterns.size();k++){
    delete allPatterns[k];
  }

}

 /**
     \brief Display the layers, ladders and modules hit by the tracks contained in the given file.
     \param fileName The name of the root file
     \param tracker_layers Gives the number of the layers in the tracker
     \param restriction If the vector contains data, only tracks going throught these ladders will be tacken into account
     \param phi_min Minimum value of PHI0 for the selected tracks
     \param phi_max Maximum value of PHI0 for the selected tracks
     \param eta_min Minimum value of etaGEN for the selected tracks
     \param eta_max Maximum value of etaGEN for the selected tracks
  **/
void createFromSimu(string fileName, vector<int> tracker_layers, vector< vector<int> > restriction, float phi_min, float phi_max, float eta_min, float eta_max){

  map<int, set<int> > usedLadders;
  map<int, map<int, set<int> > > usedModules;

  TChain* TT = new TChain("L1TrackTrigger");
  TT->Add(fileName.c_str());

  //--> Signification (et dimension) des variables

  // Stub info (fait a partir de paires de clusters matches)

  //static const int      m_stub_MAX    = 10000;     // Nombre maximal de stubs

  int m_stub;
  vector<int>           m_stub_layer;  // Layer du stub (5 a 10 pour les 6 layers qui nous interessent)
  vector<int>           m_stub_module; // Position en Z du module contenant le stub
  vector<int>           m_stub_ladder; // Position en PHI du module contenant le stub
  vector<int>           m_stub_seg;    // Segment du module contenant le stub
  vector<int>           m_stub_strip;  // Strip du cluster interne du stub
  vector<float>         m_stub_pxGEN;  // pxGEN de la particule originelle
  vector<float>         m_stub_pyGEN;  // pyGEN de la particule originelle
  vector<float>         m_stub_etaGEN;  // etaGEN de la particule originelle

  vector<int>           *p_m_stub_layer = &m_stub_layer;
  vector<int>           *p_m_stub_module = &m_stub_module;
  vector<int>           *p_m_stub_ladder = &m_stub_ladder;
  vector<int>           *p_m_stub_seg = &m_stub_seg;
  vector<int>           *p_m_stub_strip = &m_stub_strip;
  vector<float>         *p_m_stub_pxGEN = &m_stub_pxGEN;
  vector<float>         *p_m_stub_pyGEN = &m_stub_pyGEN;
  vector<float>         *p_m_stub_etaGEN = &m_stub_etaGEN;

  TT->SetBranchAddress("STUB_n",         &m_stub);
  TT->SetBranchAddress("STUB_layer",     &p_m_stub_layer);
  TT->SetBranchAddress("STUB_module",    &p_m_stub_module);
  TT->SetBranchAddress("STUB_ladder",    &p_m_stub_ladder);
  TT->SetBranchAddress("STUB_seg",       &p_m_stub_seg);
  TT->SetBranchAddress("STUB_strip",     &p_m_stub_strip);
  TT->SetBranchAddress("STUB_pxGEN",     &p_m_stub_pxGEN);
  TT->SetBranchAddress("STUB_pyGEN",     &p_m_stub_pyGEN);
  TT->SetBranchAddress("STUB_etaGEN",    &p_m_stub_etaGEN);

  int n_entries_TT = TT->GetEntries();

  int nbInLayer=0;

  int minLayer = *(min_element(tracker_layers.begin(),tracker_layers.end()));

  int layers[tracker_layers.size()];
  int ladder_per_layer[tracker_layers.size()];
  int module_per_layer[tracker_layers.size()];

  int nbUsedTracks = 0;

  float found_phi_min = 1000;
  float found_phi_max = -1000;

  float phi0=0;

  for(int i=0;i<n_entries_TT;i++){
    TT->GetEntry(i);
    for(unsigned int j=0;j<tracker_layers.size();j++){
      layers[j]=-1;
    }
    for(unsigned int j=0;j<tracker_layers.size();j++){
      ladder_per_layer[j]=-1;
    }
    for(unsigned int j=0;j<tracker_layers.size();j++){
      module_per_layer[j]=-1;
    }
    //check the layers of the stubs
    for(int j=0;j<m_stub;j++){

      float pt_GEN = sqrt(m_stub_pxGEN[j]*m_stub_pxGEN[j]+m_stub_pyGEN[j]*m_stub_pyGEN[j]);

      if(pt_GEN<2){//we only need particules with PT>2
        continue;
      }

      phi0 = atan2(m_stub_pyGEN[j], m_stub_pxGEN[j]);

      if(phi0<phi_min){//The stub is coming from a particule outside the considered sector
        continue;
      }

      if(phi0>phi_max){//The stub is coming from a particule outside the considered sector
        continue;
      }

      if(m_stub_etaGEN[j]<eta_min){//The stub is coming from a particule outside the considered sector
        continue;
      }

      if(m_stub_etaGEN[j]>eta_max){//The stub is coming from a particule outside the considered sector
        continue;
      }

      int layer = m_stub_layer[j];
      if(((unsigned int)(layer-minLayer)<tracker_layers.size()) // layer is not above the last considered layer
         && layers[layer-minLayer]!=-1){//we have 2 stubs on the same layer-> problem
        layers[layer-minLayer]=-1;
        continue;
      }
      if(find(tracker_layers.begin(),tracker_layers.end(), layer)!=tracker_layers.end()){ // is this layer in the layer list?
    layers[layer-minLayer]=j;
    ladder_per_layer[layer-minLayer]=m_stub_ladder[j];
    module_per_layer[layer-minLayer] = m_stub_module[j];
      }
    }
    /**************************************
    Selection on the stubs/layer
    We need at least one stub per layer
    **************************************/
    bool missing_stub = false;
    for(unsigned int j=0;j<tracker_layers.size();j++){
      if(layers[j]==-1){
    missing_stub=true;

      }
    }
    if(missing_stub)
      continue;//no stub on each layer -> drop the event
    nbInLayer++;
    //restriction to some ladders
    bool useTrack = true;
    for(unsigned int j=0;j<restriction.size();j++){
      vector<int> rLayer = restriction[j];
      if(find(rLayer.begin(), rLayer.end(),ladder_per_layer[j])==rLayer.end()){//the track is no going throught the right ladder
    useTrack = false;
    break;
      }
    }
    if(useTrack){
      nbUsedTracks++;
      if(phi0<found_phi_min)
    found_phi_min=phi0;
      if(phi0>found_phi_max)
    found_phi_max=phi0;
      for(unsigned int j=0;j<tracker_layers.size();j++){
    int layer_id=tracker_layers[j];

    usedLadders[layer_id].insert(CMSPatternLayer::getLadderCode(layer_id, ladder_per_layer[j]));
    usedModules[layer_id][ladder_per_layer[j]].insert(CMSPatternLayer::getModuleCode(layer_id, module_per_layer[j]));

      }
    }
    if(nbUsedTracks>100000)
      break;
  }

  cout<<"Nb Events : "<<n_entries_TT<<endl;
  cout<<"Nb Events with stubs on all layers : "<<nbInLayer<<endl;
  cout<<"PHI min : "<<found_phi_min<<endl;
  cout<<"PHI max : "<<found_phi_max<<endl;

  for(map<int, set<int> >::const_iterator it_layer=usedLadders.begin();it_layer!=usedLadders.end();it_layer++){
    cout<<"Layer "<<it_layer->first<<" : "<<endl;
    for(set<int>::const_iterator it_lad=it_layer->second.begin();it_lad!=it_layer->second.end();it_lad++){
      cout<<"    "<<*it_lad<<" : ";
      set<int> modules = usedModules[it_layer->first][*it_lad];
      for(set<int>::const_iterator it_mod=modules.begin();it_mod!=modules.end();it_mod++){
    cout<<*it_mod<<" ";
      }
      cout<<endl;
    }
    cout<<endl;
  }

  delete TT;
}

// Get the first ladder and number of ladders from a ladders list
void getOrderData(vector<int> list, int* first, int* nb){
  sort(list.begin(),list.end());
  *nb = (int)list.size();
  if((*nb)>0){
    int index = list.size()-1;
    *first = list[index];//greatest value

    //we decrease the greatest value and stop as soon as there is a gap
    while(index>0 && list[index-1]==(*first)-1){
      index--;
      (*first)--;
    }
  }
}

void createSectorFromRootFile(SectorTree* st, string fileName, vector<int> layers, int sector_id)
{

  Sector s(layers);
  TChain* tree = NULL;
  vector<int> modules;
  vector<float> coords;

  if(fileName.substr(fileName.length()-4,fileName.length()).compare(".csv")==0){
    ifstream is(fileName.c_str());
    string line;
    int line_index=0;
    if (is.is_open()){
      while ( is.good() ){
        getline (is,line);
        if(line_index==sector_id+1){ // line describing the sector we want
          std::stringstream ss(line);
          std::string item;
          int column_index=0;
          while (std::getline(ss, item, ',')) {
            if(column_index>1){
              int number=0;
              std::istringstream ss( item );
              ss >> number;
              if(number!=0){
                //cout<<number<<endl;
                modules.push_back(number);
              }
            }
            column_index++;
          }
        }
        line_index++;
      }
      is.close();
    }
  }
  else{
    tree = new TChain("Sectors");
    tree->Add(fileName.c_str());
    vector< vector<int> > m_mod_tot;  // Secteurs dans le endcap
    vector< vector<float> > m_coords;
    vector< vector<int> > * p_m_mod_tot =  &m_mod_tot;
    vector< vector<float> > * p_m_coords = &m_coords;
    tree->SetBranchAddress("sectors",   &p_m_mod_tot);
    tree->SetBranchAddress("sectors_coord",    &p_m_coords);

    tree->GetEntry(0);

    modules = m_mod_tot[sector_id];
    cout<<"\tmodule number : "<<modules.size()<<endl;

    coords = m_coords[sector_id];
  }

  map<int, vector<int> > ladders_from_layer;
  map<int, map<int, vector<int> > > modules_from_ladder;

  for(unsigned int i=0;i<modules.size();i++){

    ostringstream oss;
    oss<<std::setfill('0');
    oss<<setw(6)<<modules[i];

    int layer,ladder,module;
    istringstream ss_layer(oss.str().substr(0,2));
    ss_layer>>layer;
    istringstream ss_ladder(oss.str().substr(2,2));
    ss_ladder>>ladder;
    std::cout << "oss = " << oss.str() << std::endl;
    // This line is useless at the moment because the getLadderCode function returns "ladder".
    ladder = CMSPatternLayer::getLadderCode(layer,ladder);

    ////// TMP FIX FOR TKLAYOUT NUMBERING (10 JUL 2013)

    if(layer<11){
      int tmp_nb_ladders = CMSPatternLayer::getNbLadders(layer);
      ladder = (ladder+tmp_nb_ladders*3/4) % tmp_nb_ladders;
    }

    //////////////////////////////////////////////////

    istringstream ss_module(oss.str().substr(4,2));
    ss_module>>module;
    module = CMSPatternLayer::getModuleCode(layer,module);

    vector<int> tmp_ladders = ladders_from_layer[layer];
    if(find(tmp_ladders.begin(),tmp_ladders.end(),ladder)==tmp_ladders.end()){
      ladders_from_layer[layer].push_back(ladder);
    }

    vector<int> tmp_modules = modules_from_ladder[layer][ladder];
    if(find(tmp_modules.begin(),tmp_modules.end(),module)==tmp_modules.end()){
      modules_from_ladder[layer][ladder].push_back(module);
    }
  }

  for(unsigned int i=0;i<layers.size();i++){
    vector<int> tmp_ladders = ladders_from_layer[layers[i]];
    int first=0, nb=0;
    getOrderData(tmp_ladders, &first, &nb);
    s.addLadders(layers[i], first, nb);
    for(unsigned int k=0;k<tmp_ladders.size();k++){
      vector<int> tmp_modules = modules_from_ladder[layers[i]][tmp_ladders[k]];
      getOrderData(tmp_modules, &first, &nb);
      s.addModules(layers[i], tmp_ladders[k], first, nb);
    }
  }

  if(tree!=NULL){
    delete tree;
    cout<<"\tPHI min : "<<coords[0]<<endl;
    cout<<"\tPHI max : "<<coords[1]<<endl;
    cout<<"\tETA min : "<<coords[2]<<endl;
    cout<<"\tETA max : "<<coords[3]<<endl;
    cout<<endl;
  }

  s.setOfficialID(sector_id);

  st->addSector(s);
}

/// The desc should be created with name "Allowed options"
void fillConfigurationFromFile(const int av, char** ac,
                               boost::program_options::options_description & desc,
                               boost::program_options::variables_map & vm,
                               const std::string & configurationFileName)
{
  namespace po = boost::program_options;
  // Declare the supported options.
  desc.add_options()
    ("help", "produce help message")
    ("generateBank", "Generates a pattern bank from root simulation file (needs --ss_size --dc_bits --pt_min --pt_max --eta_min --eta_max --coverage --input_directory --bank_name --sector_file --sector_id --active_layers)")
    ("testSectors", "Get the tracks sectors")
    ("MergeSectors", "Merge 2 root files having same events but different sectors (needs --inputFile --secondFile and --outputFile)")
    ("MergeBanks", "Merge 2 bank files having only 1 sector (needs --inputFile --secondFile and --outputFile)")
    ("buildFitParams", "Computes the Fit parameters for the given bank using tracks from the given directory (needs --bankFile, --inputFile and --outputFile)")
    ("findPatterns", "Search for patterns in an event file (needs --ss_threshold --inputFile, --bankFile, --outputFile, --startEvent and --stopEvent)")
    ("printBank", "Display all patterns from a bank (needs --bankFile)")
    ("printBankBinary", "Display all patterns from a bank using a decimal representation of the binary values (needs --bankFile)")
    ("testCode", "Dev tests")
    ("analyseBank", "Creates histograms from a pattern bank file")
    ("inputFile", po::value<string>(), "The file to analyse")
    ("secondFile", po::value<string>(), "Second file to merge")
    ("bankFile", po::value<string>(), "The patterns bank file to use")
    ("outputFile", po::value<string>(), "The root output file")
    ("ss_threshold", po::value<int>(), "The minimum number of hit superstrips to activate a pattern")
    ("startEvent", po::value<int>(), "The first event index")
    ("stopEvent", po::value<int>(), "The last event index")
    ("decode", po::value<int>(), "Decode the given super strip")
    ("ss_size", po::value<int>(), "Number of strips in a super strip {16,32,64,128,256,512,1024}")
    ("dc_bits", po::value<int>(), "Number of used DC bits [0-3]")
    ("pt_min", po::value<int>(), "Only tracks having a greater PT will be used to generate a pattern")
    ("pt_max", po::value<int>(), "Only tracks having a smaller PT will be used to generate a pattern")
    ("eta_min", po::value<float>(), "Only tracks having a greater ETA will be used to generate a pattern")
    ("eta_max", po::value<float>(), "Only tracks having a smaller ETA will be used to generate a pattern")
    ("maxFakeSStrips", po::value<int>(), "The maximum number of fake superstrips used in a pattern (0=desactivated)")
    ("coverage", po::value<float>(), "Value to reach to stop the process [0-1]")
    ("input_directory", po::value<string>(), "The directory containing the single particule root files (local or RFIO)")
    ("sector_file", po::value<string>(), "The file (.root or .csv) containing the sectors definition")
    ("sector_id", po::value<int>(), "The index of the sector to use in the sector file. In a CSV file the first sector has index 0.")
    ("active_layers", po::value<string>(), "The layers to use in the sector (8 at most)")
    ("bank_name", po::value<string>(), "The bank file name")
    ;

  std::ifstream in_file( configurationFileName.data() );
  po::store(po::parse_command_line(av, ac, desc), vm);
  po::store(po::parse_config_file(in_file, desc), vm);
  po::notify(vm);
}

#endif // AMSIMULATIONFUNCTIONS_H
